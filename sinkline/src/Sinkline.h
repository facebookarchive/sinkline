/**
 * Copyright (c) 2016-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT-style license found in the
 * LICENSE file in the root directory of this source tree. 

 */

#ifndef FB_SINKLINE_SINKLINE_H
#define FB_SINKLINE_SINKLINE_H

#include <utility>

// convenience include
#include "Operators.h"

#include "OperatorDefinitions.h"

namespace fb { namespace sinkline {

template<typename Sink>
auto sinkline (Sink &&sink) noexcept
{
  return std::forward<Sink>(sink);
}

/// Composes together a bunch of operators, so that inputs will now be processed by
/// them in order.
///
/// Sinklines should end with something callable, like a function, a C++ lambda,
/// or an Objective-C block.
template<typename Operator, typename... Remaining>
auto sinkline (Operator &&op, Remaining &&...remaining)
{
  return std::forward<Operator>(op).compose(sinkline(std::forward<Remaining>(remaining)...));
}

/// Creates a sinkline only if the given condition is true.
///
/// This can be embedded at the end of another sinkline, but not in the middle.
template<typename... Sinks>
auto sinklineIf (bool condition, Sinks &&...sinks)
{
  auto &&sink = sinkline(std::forward<Sinks>(sinks)...);
  return OptionalSink<std::remove_reference_t<decltype(sink)>>(std::forward<decltype(sink)>(sink), condition);
}

/// Creates a sinkline which will invoke the given sink only if it is non-null.
///
/// This can be embedded at the end of another sinkline, but not in the middle.
template<typename Sink>
auto sinklineIf (Sink &&sink)
{
  bool enabled = sink != nullptr;
  return OptionalSink<std::remove_reference_t<Sink>>(std::forward<Sink>(sink), enabled);
}

} } // namespace fb::sinkline

#endif
