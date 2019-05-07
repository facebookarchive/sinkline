/**
 * Copyright (c) 2016-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT-style license found in the
 * LICENSE file in the root directory of this source tree. 

 */

#ifndef FB_SINKLINE_BLOCK_CONVERTIBLE_H
#define FB_SINKLINE_BLOCK_CONVERTIBLE_H

#include <type_traits>
#include <utility>

#include "PlatformSupport.h"
#include "TupleExt.h"

namespace fb { namespace sinkline {

/// Determines whether the given type parameter represents an Objective-C block,
/// similarly to std::is_function.
template<typename Block>
struct IsBlock final : public std::false_type
{
  public:
    IsBlock () = delete;
};

#if __has_feature(blocks)

template<typename Result, typename... Arguments>
struct IsBlock<Result (^)(Arguments...)> final : public std::true_type
{
  public:
    using result_type = Result;

    IsBlock () = delete;

    /// Converts an arbitrary callable object into a block of this type.
    template<typename Callable>
    static auto convert (Callable fn)
    {
      return ^(Arguments ...args) {
        return fn(args...);
      };
    }
};

template<typename... Arguments>
struct IsBlock<void (^)(Arguments...)> final : public std::true_type
{
  public:
    using result_type = void;

    IsBlock () = delete;

    /// Converts an arbitrary callable object into a block of this type.
    template<typename Callable>
    static auto convert (Callable fn)
    {
      return ^(Arguments ...args) {
        fn(args...);
      };
    }
};

/// Implements implicit block conversions for a callable object that permits
/// generic arguments.
template<typename Callable>
struct BlockConvertible final
{
  public:
    BlockConvertible () = delete;

    explicit BlockConvertible (const Callable &fn)
      : _fn(fn)
    {}

    explicit BlockConvertible (Callable &&fn)
      : _fn(std::move(fn))
    {}

    template<typename... Inputs>
    auto operator() (Inputs &&...inputs) const
    {
      return _fn(std::forward<Inputs>(inputs)...);
    }

    template<typename Block, typename Result = typename IsBlock<Block>::result_type>
    operator Block () const noexcept
    {
      return IsBlock<Block>::convert(*this);
    }

  private:
    Callable _fn;
};

template<typename Callable>
auto makeBlockConvertible (Callable &&fn)
{
  return BlockConvertible<Callable>(std::forward<Callable>(fn));
}

#else

template<typename Callable>
auto makeBlockConvertible (Callable &&fn)
{
  return std::forward<Callable>(fn);
}

#endif // __has_feature(blocks)

} } // namespace fb::sinkline

#endif
