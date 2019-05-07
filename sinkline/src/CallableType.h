/**
 * Copyright (c) 2016-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT-style license found in the
 * LICENSE file in the root directory of this source tree. 

 */

#ifndef FB_SINKLINE_CALLABLE_TYPE_H
#define FB_SINKLINE_CALLABLE_TYPE_H

#include <functional>
#include <tuple>

#include "PlatformSupport.h"

namespace fb { namespace sinkline {

/// Deduces the argument types and result of any callable object.
///
/// This does not work with overloaded functions or overloaded implementations
/// of operator().
template<typename Callable>
struct CallableType final : public CallableType<decltype(&Callable::operator())>
{
  public:
    CallableType () = delete;
};

template<typename Callable, typename CallableResult, typename... CallableArgs>
struct CallableType<CallableResult (Callable::*)(CallableArgs...) const> : CallableType<CallableResult(CallableArgs...)>
{
  public:
    CallableType () = delete;
};

template<typename CallableResult, typename... CallableArgs>
struct CallableType<CallableResult (*)(CallableArgs...)> final : CallableType<CallableResult(CallableArgs...)>
{
  public:
    CallableType () = delete;
};

#if __has_feature(blocks)

template<typename CallableResult, typename... CallableArgs>
struct CallableType<CallableResult (^)(CallableArgs...)> final : CallableType<CallableResult(CallableArgs...)>
{
  public:
    CallableType () = delete;
};

#endif // __has_feature(blocks)

/// This is the "main" definition of CallableType that the others ultimately
/// inherit from.
template<typename CallableResult, typename... CallableArgs>
struct CallableType<CallableResult(CallableArgs...)>
{
  public:
    using result_type = CallableResult;
    using argument_types = std::tuple<CallableArgs...>;
    using function_type = std::function<result_type(CallableArgs...)>;

    #if __has_feature(blocks)
    using block_type = result_type (^)(CallableArgs...);
    #endif

    CallableType () = delete;
};

} } // namespace fb::sinkline

#endif
