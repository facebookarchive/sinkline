/**
 * Copyright (c) 2016-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT-style license found in the
 * LICENSE file in the root directory of this source tree. 

 */

#ifndef FB_SINKLINE_OPERATOR_DEFINITIONS_H
#define FB_SINKLINE_OPERATOR_DEFINITIONS_H

#include <forward_list>
#include <functional>
#include <memory>
#include <mutex>
#include <type_traits>
#include <utility>

#include "BlockConvertible.h"
#include "CallableType.h"
#include "Optional.h"
#include "PlatformSupport.h"
#include "TupleExt.h"

namespace fb { namespace sinkline {

/// Implements map().
template<typename Transform>
struct MapOperator final
{
  public:
    MapOperator () = delete;

    explicit MapOperator (const Transform &transform) noexcept(std::is_nothrow_copy_constructible<Transform>::value)
      : _transform(transform)
    {}

    explicit MapOperator (Transform &&transform) noexcept(std::is_nothrow_move_constructible<Transform>::value)
      : _transform(std::move(transform))
    {}

    template<typename NewNext>
    auto compose (NewNext newNext) const
    {
      return makeBlockConvertible([newNext = std::move(newNext), transform = _transform](auto &&...inputs) {
        return newNext(transform(std::forward<decltype(inputs)>(inputs)...));
      });
    }

  private:
    Transform _transform;
};

/// Implements filter().
template<typename Predicate>
struct FilterOperator final
{
  public:
    FilterOperator () = delete;

    explicit FilterOperator (const Predicate &predicate) noexcept(std::is_nothrow_copy_constructible<Predicate>::value)
      : _predicate(predicate)
    {}

    explicit FilterOperator (Predicate &&predicate) noexcept(std::is_nothrow_move_constructible<Predicate>::value)
      : _predicate(std::move(predicate))
    {}

    template<typename NewNext>
    auto compose (NewNext newNext) const
    {
      return makeBlockConvertible([newNext = std::move(newNext), predicate = _predicate](auto &&...inputs) {
        auto shouldCall = predicate(inputs...);
        return callIf(shouldCall, newNext, std::forward<decltype(inputs)>(inputs)...);
      });
    }

  private:
    Predicate _predicate;
};

/// Implements reduce().
struct ReduceOperator final
{
  public:
    template<typename NewNext>
    auto compose (NewNext newNext) const
    {
      return makeBlockConvertible([newNext = std::move(newNext)](auto &&tuple) {
        return callWithTuple(newNext, std::forward<decltype(tuple)>(tuple));
      });
    }
};

/// Implements scan().
template<typename Mutex, typename Accumulator, typename Transform>
struct ScanOperator final
{
  public:
    ScanOperator () = delete;

    explicit ScanOperator (Accumulator initialValue, const Transform &transform) noexcept(std::is_nothrow_move_constructible<Accumulator>::value && std::is_nothrow_copy_constructible<Transform>::value)
      : _initial(std::move(initialValue))
      , _transform(transform)
    {}

    explicit ScanOperator (Accumulator initialValue, Transform &&transform) noexcept(std::is_nothrow_move_constructible<Accumulator>::value && std::is_nothrow_move_constructible<Transform>::value)
      : _initial(std::move(initialValue))
      , _transform(std::move(transform))
    {}

    template<typename NewNext>
    auto compose (NewNext newNext) const
    {
      return makeBlockConvertible([newNext = std::move(newNext), transform = _transform, accum = std::make_shared<Accumulator>(_initial), mutex = std::make_shared<Mutex>()](auto &&...inputs) {
        std::unique_lock<Mutex> lock(*mutex);

        auto newAccum = transform(const_cast<const Accumulator &>(*accum), std::forward<decltype(inputs)>(inputs)...);
        *accum = newAccum;

        lock.unlock();

        return newNext(std::move(newAccum));
      });
    }

  private:
    Accumulator _initial;
    Transform _transform;
};

template<typename Accumulator, typename Transform>
struct ScanOperator<void, Accumulator, Transform> final
{
  public:
    ScanOperator () = delete;

    explicit ScanOperator (Accumulator initialValue, const Transform &transform) noexcept(std::is_nothrow_move_constructible<Accumulator>::value && std::is_nothrow_copy_constructible<Transform>::value)
      : _initial(std::move(initialValue))
      , _transform(transform)
    {}

    explicit ScanOperator (Accumulator initialValue, Transform &&transform) noexcept(std::is_nothrow_move_constructible<Accumulator>::value && std::is_nothrow_move_constructible<Transform>::value)
      : _initial(std::move(initialValue))
      , _transform(std::move(transform))
    {}

    template<typename NewNext>
    auto compose (NewNext newNext) const
    {
      return makeBlockConvertible([newNext = std::move(newNext), transform = _transform, accum = std::make_shared<Accumulator>(_initial)](auto &&...inputs) {
        auto newAccum = transform(const_cast<const Accumulator &>(*accum), std::forward<decltype(inputs)>(inputs)...);
        *accum = newAccum;

        return newNext(std::move(newAccum));
      });
    }

  private:
    Accumulator _initial;
    Transform _transform;
};

/// Implements onError().
template<typename Handler>
struct ErrorOperator final
{
  public:
    using error_type = typename std::tuple_element<0, typename CallableType<Handler>::argument_types>::type;

    ErrorOperator () = delete;

    explicit ErrorOperator (const Handler &handler) noexcept(std::is_nothrow_copy_constructible<Handler>::value)
      : _handler(handler)
    {}

    explicit ErrorOperator (Handler &&handler) noexcept(std::is_nothrow_move_constructible<Handler>::value)
      : _handler(std::move(handler))
    {}

    template<typename NewNext>
    auto compose (NewNext newNext) const
    {
      return makeBlockConvertible([newNext = std::move(newNext), handler = _handler](auto &&...inputs) {
        error_type error{};
        auto inputsWithoutError = extract(std::make_tuple(std::forward<decltype(inputs)>(inputs)...), &error);

        if (error) {
          return handler(error);
        } else {
          return callWithTuple(newNext, inputsWithoutError);
        }
      });
    }

  private:
    Handler _handler;
};

/// Implements recover().
template<typename Handler>
struct RecoverOperator final
{
  public:
    using error_type = typename std::tuple_element<0, typename CallableType<Handler>::argument_types>::type;

    RecoverOperator () = delete;

    explicit RecoverOperator (const Handler &handler) noexcept(std::is_nothrow_copy_constructible<Handler>::value)
      : _handler(handler)
    {}

    explicit RecoverOperator (Handler &&handler) noexcept(std::is_nothrow_move_constructible<Handler>::value)
      : _handler(std::move(handler))
    {}

    template<typename NewNext>
    auto compose (NewNext newNext) const
    {
      return makeBlockConvertible([newNext = std::move(newNext), handler = _handler](auto &&...inputs) {
        error_type error{};
        auto inputsWithoutError = extract(std::make_tuple(std::forward<decltype(inputs)>(inputs)...), &error);

        if (error) {
          return newNext(handler(error));
        } else {
          return newNext(std::get<0>(inputsWithoutError));
        }
      });
    }

  private:
    Handler _handler;
};

/// One of the input sinks to a CombineOperator.
///
/// This type of sink cannot be constructed directly. It is only obtained by
/// calling CombineOperator::sinks().
template<typename NextResult, size_t Index, typename... Values>
struct CombineInputOperator final
{
  public:
    using tuple_type = std::tuple<Values...>;
    using input_type = std::tuple<Optional<Values>...>;
    using storage_type = std::shared_ptr<input_type>;

    using result_type = Optional<NextResult>;
    using next_type = std::function<NextResult(Values...)>;

    CombineInputOperator () = delete;

    result_type operator() (std::tuple_element_t<Index, tuple_type> value) const
    {
      // TODO: Thread safety
      std::get<Index>(*_values) = std::move(value);

      if (auto repacked = flattenOptionals<0, input_type, Values...>(*_values)) {
        return result_type(callWithTuple(_next, std::move(*repacked)));
      } else {
        return result_type();
      }
    }

  private:
    explicit CombineInputOperator (next_type next, storage_type values) noexcept(std::is_nothrow_move_constructible<next_type>::value && std::is_nothrow_move_constructible<storage_type>::value)
      : _next(std::move(next))
      , _values(std::move(values))
    {}

    // TODO: This should probably be behind a shared_ptr too.
    next_type _next;

    storage_type _values;

    template<typename X, typename... XS>
    friend class CombineOperator;
};

template<size_t Index, typename... Values>
struct CombineInputOperator<void, Index, Values...> final
{
  public:
    using tuple_type = std::tuple<Values...>;
    using input_type = std::tuple<Optional<Values>...>;
    using storage_type = std::shared_ptr<input_type>;

    using result_type = bool;
    using next_type = std::function<void(Values...)>;

    CombineInputOperator () = delete;

    result_type operator() (std::tuple_element_t<Index, tuple_type> value) const
    {
      // TODO: Thread safety
      std::get<Index>(*_values) = std::move(value);

      if (auto repacked = flattenOptionals<0, input_type, Values...>(*_values)) {
        callWithTuple(_next, std::move(*repacked));
        return true;
      } else {
        return false;
      }
    }

  private:
    explicit CombineInputOperator (next_type next, storage_type values) noexcept(std::is_nothrow_move_constructible<next_type>::value && std::is_nothrow_move_constructible<storage_type>::value)
      : _next(std::move(next))
      , _values(std::move(values))
    {}

    next_type _next;
    storage_type _values;

    template<typename X, typename... XS>
    friend class CombineOperator;
};

/// Combines N different inputs and forwards them to another sink as one
/// argument list.
template<typename NextResult, typename... Values>
class CombineOperator final
{
  public:
    using input_type = std::tuple<Optional<Values>...>;
    using tuple_type = std::tuple<Values...>;

    using result_type = Optional<NextResult>;
    using next_type = std::function<NextResult(Values...)>;

    CombineOperator () = delete;

    explicit CombineOperator (next_type next)
      : _next(std::move(next))
      , _values(std::make_shared<input_type>(Optional<Values>()...))
    {}

    // Returns a tuple of CombineInputOperators, corresponding to each input.
    auto sinks ()
    {
      return generateOperators<0, Values...>();
    }

  private:
    next_type _next;
    std::shared_ptr<input_type> _values;

    /// Generates the sinks which accept each of the separate inputs to the
    /// CombineOperator.
    ///
    /// @param Index The index of the next sink to generate.
    /// @param Value The type of value which the sink at `Index` should accept.
    /// @param Rest The remaining input types for which sinks should be
    /// generated.
    template<size_t Index, typename Value, typename... Rest>
    auto generateOperators ()
    {
      CombineInputOperator<NextResult, Index, Values...> sink(_next, _values);
      return std::tuple_cat(std::make_tuple(std::move(sink)), generateOperators<Index + 1, Rest...>());
    }

    template<size_t Index>
    auto generateOperators () const
    {
      return std::tuple<>();
    }
};

/// Implements scheduleOn().
template<typename Scheduler>
struct SchedulingOperator final
{
  public:
    using scheduler_type = std::shared_ptr<Scheduler>;

    SchedulingOperator () = delete;

    explicit SchedulingOperator (scheduler_type scheduler) noexcept(std::is_nothrow_move_constructible<scheduler_type>::value)
      : _scheduler(std::move(scheduler))
    {}

    explicit SchedulingOperator (Scheduler &&scheduler)
      : _scheduler(std::make_shared<Scheduler>(std::move(scheduler)))
    {}

    template<typename NewNext>
    auto compose (NewNext newNext) const
    {
      return makeBlockConvertible([newNext = std::move(newNext), scheduler = _scheduler](auto &&...inputs) {
        auto mutableScheduler = const_cast<std::remove_const_t<Scheduler> *>(scheduler.get());

        return mutableScheduler->schedule(newNext, std::forward<decltype(inputs)>(inputs)...);
      });
    }

  private:
    scheduler_type _scheduler;
};

/// Implements sideEffect().
template<typename Action>
struct SideEffectOperator final
{
  public:
    SideEffectOperator () = delete;

    explicit SideEffectOperator (const Action &action) noexcept(std::is_nothrow_copy_constructible<Action>::value)
      : _action(action)
    {}

    explicit SideEffectOperator (Action &&action) noexcept(std::is_nothrow_move_constructible<Action>::value)
      : _action(std::move(action))
    {}

    template<typename NewNext>
    auto compose (NewNext newNext) const
    {
      return makeBlockConvertible([newNext = std::move(newNext), action = _action](auto &&...inputs) {
        action(inputs...);
        return newNext(std::forward<decltype(inputs)>(inputs)...);
      });
    }

  private:
    Action _action;
};

/// Implements then().
template<typename Action>
struct ThenOperator final
{
  public:
    ThenOperator () = delete;

    explicit ThenOperator (const Action &action) noexcept(std::is_nothrow_copy_constructible<Action>::value)
      : _action(action)
    {}

    explicit ThenOperator (Action &&action) noexcept(std::is_nothrow_move_constructible<Action>::value)
      : _action(std::move(action))
    {}

    template<typename NewNext>
    auto compose (NewNext newNext) const
    {
      return makeBlockConvertible([newNext = std::move(newNext), action = _action](auto &&...inputs) {
        return action(std::forward<decltype(inputs)>(inputs)..., newNext);
      });
    }

  private:
    Action _action;
};

/// Used by sinklineIf() to create a type-correct sink which can be enabled or
/// disabled at construction time.
///
/// Calling this sink will return Optional values containing the type that the
/// underlying sink returns. If disabled, these Optionals will all be empty.
///
/// If the underlying sink returns void, this sink will instead return booleans
/// indicating whether it was invoked.
template<typename Sink>
struct OptionalSink final
{
  public:
    OptionalSink () = delete;

    explicit OptionalSink (const Sink &sink, bool enabled) noexcept(std::is_nothrow_copy_constructible<Sink>::value)
      : _next(sink)
      , _enabled(enabled)
    {}

    explicit OptionalSink (Sink &&sink, bool enabled) noexcept(std::is_nothrow_move_constructible<Sink>::value)
      : _next(std::move(sink))
      , _enabled(enabled)
    {}

    template<typename... Inputs>
    auto operator() (Inputs &&...inputs) const
    {
      return callIf(_enabled, _next, std::forward<Inputs>(inputs)...);
    }

    template<typename Block, typename Result = typename IsBlock<Block>::result_type>
    operator Block () const noexcept
    {
      return IsBlock<Block>::convert(*this);
    }

  private:
    Sink _next;
    bool _enabled;
};

} } // namespace fb::sinkline

#endif
