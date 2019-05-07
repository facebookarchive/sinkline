/**
 * Copyright (c) 2016-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT-style license found in the
 * LICENSE file in the root directory of this source tree. 

 */

#ifndef FB_SINKLINE_OPERATORS_H
#define FB_SINKLINE_OPERATORS_H

#include <memory>
#include <mutex>
#include <tuple>
#include <utility>

#include "AnyNull.h"
#include "OperatorDefinitions.h"
#include "TupleExt.h"

namespace fb { namespace sinkline { namespace operators {

/// Maps input values using the given transformation.
template<typename Callable>
auto map (Callable &&transform)
{
  return MapOperator<std::remove_reference_t<Callable>>(std::forward<Callable>(transform));
}

/// Forwards only those inputs which pass the given predicate. Any inputs which
/// fail the predicate are discarded.
template<typename Callable>
auto filter (Callable &&predicate)
{
  return FilterOperator<std::remove_reference_t<Callable>>(std::forward<Callable>(predicate));
}

/// Splits a tuple input into multiple arguments.
///
/// For example, an invocation with the following type:
///
///   std::tuple<int, double, bool>
///
/// will be forwarded as an argument list of:
///
///   (int, double, bool)
static inline auto reduce ()
{
  return ReduceOperator();
}

/// Forwards input arguments only if they are all non-null.
///
/// This is similar to:
///
///   filter([](auto input) {
///     return input != nullptr;
///   })
///
/// except that it considers every argument which is comparable against nullptr.
///
/// It is an error to use this operator when no arguments are comparable against
/// nullptr.
static inline auto ignoreNull ()
{
  // This explicit return type is here to disallow returning
  // IsNullNothingComparable (even by accident) at compile-time.
  return filter([](auto &...inputs) -> bool {
    return !anyNull(inputs...);
  });
}

/// Combines each new input with an accumulator, starting with the given initial
/// value, using the new combined result as the accumulator, and forwarding it
/// to the next operator or callback.
///
/// For example:
///
///   scan(0, [](int sum, int x) {
///     return sum + x;
///   })
///
/// will add each input together (starting at 0), then forward each new sum
/// added in this way.
///
/// By default, scan() will use a non-recursive mutex to prevent the accumulator
/// from being invoked multiple times concurrently. To construct a different mutex
/// type, specify it (or `void`, to use none at all) for the `Mutex` template
/// parameter.
template<typename Mutex = std::mutex, typename Accumulator, typename Callable>
auto scan (Accumulator &&initialValue, Callable &&transform)
{
  return ScanOperator<Mutex, std::remove_reference_t<Accumulator>, std::remove_reference_t<Callable>>(std::forward<Accumulator>(initialValue), std::forward<Callable>(transform));
}

/// Forwards each input while running on the given scheduler. This can be used
/// to specify which thread or queue further processing should happen upon.
template<typename Scheduler>
auto scheduleOn (std::shared_ptr<Scheduler> scheduler)
{
  return SchedulingOperator<Scheduler>(std::move(scheduler));
}

template<typename Scheduler>
auto scheduleOn (Scheduler &&scheduler)
{
  return SchedulingOperator<Scheduler>(std::move(scheduler));
}

/// Invokes the given side effect before forwarding each input.
template<typename Callable>
auto sideEffect (Callable &&action)
{
  return SideEffectOperator<std::remove_reference_t<Callable>>(std::forward<Callable>(action));
}

/// Matches upon an error type, invoking the given handler if an error is found, or
/// forwarding the remaining arguments if not.
///
/// For example, if the given error handler accepts `const char *`:
///
///  - An invocation with `("foobar", 1234)` will invoke the error handler
///    with `"foobar"` as an argument, then return its result and skip processing
///    any following operators and callbacks.
///  - Invoking the sink with `((const char *)nullptr, 1234)` will pass `1234`
///    (and only that value) to the next operator or callback.
///
/// Argument order does not matter. The operator will find any error (whose
/// existence is determined by implicitly converting to `bool`), extract it, and
/// then call the error handler or forward the input as appropriate.
template<typename Callable>
auto onError (Callable &&handler)
{
  return ErrorOperator<std::remove_reference_t<Callable>>(std::forward<Callable>(handler));
}

/// Matches upon an error type, similar to onError(), but invokes a handler
/// which is responsible for recovering from that error, and creating
/// a "successful" value that will be forwarded.
///
/// For example, if the given error handler accepts `const char *`:
///
///  - Invoking the sink with `("foobar", 1234)` will invoke the error handler
///    with `"foobar"` as an argument, and the error handler will be responsible
///    for returning an `int` to pass to the next operator or callback.
///  - Invoking the sink with `((const char *)nullptr, 1234)` will forward `1234`
///    (and only that value) to the next operator or callback.
///
/// Argument order does not matter. The operator will find any error (whose
/// existence is determined by implicitly converting to `bool`), extract it, and
/// then call the error handler as appropriate.
template<typename Callable>
auto recover (Callable &&handler)
{
  return RecoverOperator<std::remove_reference_t<Callable>>(std::forward<Callable>(handler));
}

/// Invokes the given action for each input value, with a callback for
/// forwarding results that the action can execute when ready.
///
/// The callback may be invoked any number of times (including zero), with any
/// input permitted by its type. Any values provided in this way will be passed
/// to the next operator or callback that many times.
///
/// This can be used to chain together callbacks from different APIs.
template<typename Callable>
auto then (Callable &&action)
{
  return ThenOperator<std::remove_reference_t<Callable>>(std::forward<Callable>(action));
}

} } } // namespace fb::sinkline::operators

#endif
