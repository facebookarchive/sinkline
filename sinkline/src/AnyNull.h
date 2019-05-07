/**
 * Copyright (c) 2016-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT-style license found in the
 * LICENSE file in the root directory of this source tree. 

 */

#ifndef FB_SINKLINE_ANY_NULL_H
#define FB_SINKLINE_ANY_NULL_H

#include <type_traits>

namespace fb { namespace sinkline {

/// Used to indicate that a value passed to IsNull is not comparable to null.
struct IsNullNothingComparable final
{};

static inline bool operator| (IsNullNothingComparable, bool result)
{
  return result;
}

static inline bool operator| (bool result, IsNullNothingComparable)
{
  return result;
}

static inline auto operator| (IsNullNothingComparable, IsNullNothingComparable)
{
  return IsNullNothingComparable();
}

static inline bool operator== (IsNullNothingComparable, IsNullNothingComparable)
{
  return true;
}

/// Type trait which determines whether a given type is comparable to nullptr.
template<typename Value, bool Comparable = std::is_array<Value>::value || std::is_pointer<Value>::value || std::is_null_pointer<Value>::value || std::is_convertible<std::nullptr_t, Value>::value>
struct IsNullComparable final : std::integral_constant<bool, Comparable>
{};

/// Used in the implementation of anyNull() to determine whether a given value
/// (which may or may not be a pointer) is null.
template<typename Value, bool Comparable = IsNullComparable<Value>::value>
struct IsNull final
{
  public:
    IsNull () = delete;

    static bool isNull (Value value)
    {
      return value == nullptr;
    }
};

template<typename Value>
struct IsNull<Value, false> final
{
  public:
    IsNull () = delete;

    static auto isNull (Value)
    {
      return IsNullNothingComparable();
    }
};

static inline auto anyNull ()
{
  return IsNullNothingComparable();
}

/// Checks whether any of the values passed in represent a null pointer (as
/// determined by comparing against nullptr).
///
/// If none of the values are comparable to nullptr, the return type will be
/// IsNullNothingComparable.
template<typename Value, typename... Remaining>
auto anyNull (Value value, Remaining ...remaining)
{
  return IsNull<Value>::isNull(value) | anyNull(remaining...);
}

} } // namespace fb::sinkline

#endif
