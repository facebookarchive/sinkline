/**
 * Copyright (c) 2016-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT-style license found in the
 * LICENSE file in the root directory of this source tree. 

 */

#ifndef FB_SINKLINE_TUPLE_EXT_H
#define FB_SINKLINE_TUPLE_EXT_H

#include <tuple>
#include <type_traits>
#include <utility>

#include "Optional.h"

namespace fb { namespace sinkline {

/// Unpacks a tuple into an argument list, which is then used to invoke the
/// given callable object.
///
/// @param Indices An std::index_sequence containing all the tuple indices to
/// extract for the call.
template<typename Indices>
struct TupleCallWrapper;

template<std::size_t... N>
struct TupleCallWrapper<std::index_sequence<N...>> final
{
  public:
    TupleCallWrapper () = delete;

    template<typename Callable, typename Tuple>
    static auto call (Callable &&fn, Tuple &&tuple)
    {
      return std::forward<Callable>(fn)(std::get<N>(std::forward<Tuple>(tuple))...);
    }
};

/// Unpacks a tuple into an argument list, which is then used to invoke the
/// given callable object.
template<typename Callable, typename Tuple>
auto callWithTuple (Callable &&fn, Tuple &&tuple)
{
  static constexpr auto tupleSize = std::tuple_size<std::remove_reference_t<Tuple>>::value;

  return TupleCallWrapper<std::make_index_sequence<tupleSize>>::call(std::forward<Callable>(fn), std::forward<Tuple>(tuple));
}

/// Implements the behavior of extract() by recursively invoking itself.
template<typename Extract, typename... Remaining>
struct ExtractWrapper final
{
  public:
    ExtractWrapper () = delete;

    static auto extract(Extract* /*extracted*/) {
      return std::make_tuple();
    }
};

template<typename Extract, typename Next, typename... Remaining>
struct ExtractWrapper<Extract, Next, Remaining...> final
{
  public:
    ExtractWrapper () = delete;

    static auto extract (Extract *extracted, const Next &next, const Remaining &...remaining)
    {
      auto nextTuple = std::make_tuple(next);
      auto remainingTuple = ExtractWrapper<Extract, Remaining...>::extract(extracted, remaining...);

      return std::tuple_cat(nextTuple, remainingTuple);
    }
};

template<typename Extract, typename... Remaining>
struct ExtractWrapper<Extract, Extract, Remaining...> final
{
  public:
    ExtractWrapper () = delete;

    static auto extract (Extract *extracted, const Extract &next, const Remaining &...remaining)
    {
      if (extracted) {
        *extracted = next;
      }

      return ExtractWrapper<Extract, Remaining...>::extract(extracted, remaining...);
    }
};

/// Removes a value from a tuple based on its type, returning a new tuple with
/// one fewer element.
///
/// @param tuple The tuple to remove the element from.
/// @param extracted If not NULL, set to the value of the removed element.
template<typename Extract, typename... Inputs>
auto extract (const std::tuple<Inputs...> &tuple, Extract *extracted)
{
  auto callable = [extracted](auto &...inputs) {
    return ExtractWrapper<Extract, Inputs...>::extract(extracted, inputs...);
  };

  return callWithTuple(callable, tuple);
}

/// Removes a value from an argument list based on its type, returning a tuple with
/// one fewer element than the argument list.
///
/// @param extracted If not NULL, set to the value of the removed argument.
/// @param inputs The argument list from which to remove the element.
template<typename Extract, typename... Inputs>
auto extract (Extract *extracted, Inputs &&...inputs)
{
  return ExtractWrapper<Extract, Inputs...>::extract(extracted, std::forward<Inputs>(inputs)...);
}

/// Attempts to flatten the Optional values in the stored tuple into a single
/// Optional tuple of unpacked values. If any slot of the input tuple is empty,
/// the result will be empty as well.
///
/// @param Index The index of the next tuple slot to retrieve.
/// @param Tuple A tuple of Optionals to try flattening.
/// @param Value The type of value to be extracted from the slot at `Index`.
/// @param Rest The remaining value types to be extracted from the tuple.
template<size_t Index, typename Tuple, typename Value, typename ...Rest>
auto flattenOptionals (const Tuple &values)
{
  using FlattenedType = Optional<std::tuple<Value, Rest...>>;

  auto maybeValue = std::get<Index>(values);
  if (!maybeValue) {
    return FlattenedType();
  }

  auto maybeRest = flattenOptionals<Index + 1, Tuple, Rest...>(values);
  if (!maybeRest) {
    return FlattenedType();
  }

  auto combined = std::tuple_cat(std::make_tuple(std::move(*maybeValue)), std::move(*maybeRest));
  return FlattenedType(std::move(combined));
}

template <size_t Index, typename Tuple>
auto flattenOptionals(const Tuple& /*values*/) {
  return Optional<std::tuple<>>(std::tuple<>());
}

} } // namespace fb::sinkline

#endif
