/**
 * Copyright (c) 2016-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT-style license found in the
 * LICENSE file in the root directory of this source tree. 

 */

#ifndef FB_SINKLINE_OPTIONAL_H
#define FB_SINKLINE_OPTIONAL_H

#include <type_traits>
#include <utility>

namespace fb { namespace sinkline {

template<typename T>
struct Optional final
{
  public:
    using value_type = T;

    constexpr Optional () noexcept
      : _hasValue(false)
    {}

    Optional (const T &value) noexcept(std::is_nothrow_copy_constructible<T>::value)
      : _hasValue(true)
    {
      construct(value);
    }

    Optional (T &&value) noexcept(std::is_nothrow_move_constructible<T>::value)
      : _hasValue(true)
    {
      construct(std::move(value));
    }

    Optional (const Optional &other) noexcept(std::is_nothrow_copy_constructible<T>::value)
      : _hasValue(other._hasValue)
    {
      if (_hasValue) {
        construct(other.value());
      }
    }

    Optional &operator= (const Optional &other) noexcept(std::is_nothrow_destructible<T>::value && std::is_nothrow_copy_constructible<T>::value)
    {
      if (&other == this) {
        return this;
      }

      destroyIfNeeded();

      if ((_hasValue = other._hasValue)) {
        construct(other.value());
      }

      return *this;
    }

    Optional (Optional &&other) noexcept(std::is_nothrow_move_constructible<T>::value)
      : _hasValue(other._hasValue)
    {
      if (_hasValue) {
        construct(std::move(other.value()));
      }
    }

    Optional &operator= (Optional &&other) noexcept(std::is_nothrow_destructible<T>::value && std::is_nothrow_move_constructible<T>::value)
    {
      destroyIfNeeded();

      if ((_hasValue = other._hasValue)) {
        construct(std::move(other.value()));
      }

      return *this;
    }

    ~Optional () noexcept(std::is_nothrow_destructible<T>::value)
    {
      destroyIfNeeded();
    }

    explicit operator bool () const noexcept
    {
      return _hasValue;
    }

    T &value () noexcept
    {
      return *reinterpret_cast<T *>(&_storage);
    }

    const T &value () const noexcept
    {
      return *reinterpret_cast<const T *>(&_storage);
    }

    T *pointer () noexcept
    {
      return _hasValue ? reinterpret_cast<T *>(&_storage) : nullptr;
    }

    const T *pointer () const noexcept
    {
      return _hasValue ? reinterpret_cast<const T *>(&_storage) : nullptr;
    }

    T &operator* () noexcept
    {
      return value();
    }

    const T &operator* () const noexcept
    {
      return value();
    }

    T *operator-> () noexcept
    {
      return pointer();
    }

    const T *operator-> () const noexcept
    {
      return pointer();
    }

  private:
    std::aligned_storage_t<sizeof(T), alignof(T)> _storage;
    bool _hasValue;

    void destroyIfNeeded () noexcept(std::is_nothrow_destructible<T>::value)
    {
      if (_hasValue) {
        value().~T();
      }
    }

    void construct (const T &value) noexcept(std::is_nothrow_copy_constructible<T>::value)
    {
      new(&_storage) T(value);
    }

    void construct (T &&value) noexcept(std::is_nothrow_move_constructible<T>::value)
    {
      new(&_storage) T(std::move(value));
    }
};

template<typename T>
bool operator== (const Optional<T> &lhs, const Optional<T> &rhs)
{
  if (lhs) {
    return rhs && lhs.value() == rhs.value();
  } else {
    return !rhs;
  }
}

/// Implements callIf() using partial specialization.
template<typename T>
struct OptionalCallHelper final
{
  public:
    using result_type = Optional<T>;

    OptionalCallHelper () = delete;

    template<typename Callable, typename... Inputs>
    static result_type callIf (bool shouldCall, Callable &&fn, Inputs &&...inputs)
    {
      if (shouldCall) {
        return result_type(std::forward<Callable>(fn)(std::forward<Inputs>(inputs)...));
      } else {
        return result_type();
      }
    }
};

template<>
struct OptionalCallHelper<void> final
{
  public:
    using result_type = bool;

    OptionalCallHelper () = delete;

    template<typename Callable, typename... Inputs>
    static result_type callIf (bool shouldCall, Callable &&fn, Inputs &&...inputs)
    {
      if (shouldCall) {
        std::forward<Callable>(fn)(std::forward<Inputs>(inputs)...);
        return true;
      } else {
        return false;
      }
    }
};

/// Conditionally forwards a call, based on a boolean.
///
/// If the callable object would return a value, it will be wrapped in an
/// Optional. When not actually called, the returned Optional will be empty.
///
/// If the callable object would return `void`, this function will return `true`
/// if the object was called or `false` if it was not.
template<typename Callable, typename... Inputs>
auto callIf (bool shouldCall, Callable &&fn, Inputs &&...inputs)
{
  return OptionalCallHelper<decltype(std::forward<Callable>(fn)(std::forward<Inputs>(inputs)...))>::callIf(shouldCall, std::forward<Callable>(fn), std::forward<Inputs>(inputs)...);
}

} } // namespace fb::sinkline

#endif
