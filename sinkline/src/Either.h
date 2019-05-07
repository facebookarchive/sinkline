/**
 * Copyright (c) 2016-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT-style license found in the
 * LICENSE file in the root directory of this source tree. 

 */

#ifndef FB_SINKLINE_EITHER_H
#define FB_SINKLINE_EITHER_H

#include <type_traits>

namespace fb { namespace sinkline {

template<typename Left, typename Right>
struct Either final
{
  public:
    using left_type = Left;
    using right_type = Right;

    Either () = delete;

    Either (left_type left, std::true_type isLeft = std::true_type()) noexcept(std::is_nothrow_move_constructible<Left>::value)
      : _storage(std::move(left), isLeft)
      , _isLeft(isLeft)
    {}

    Either (right_type right, std::false_type isLeft = std::false_type()) noexcept(std::is_nothrow_move_constructible<Right>::value)
      : _storage(std::move(right), isLeft)
      , _isLeft(isLeft)
    {}

    Either (const Either &other) noexcept(std::is_nothrow_copy_constructible<Left>::value && std::is_nothrow_copy_constructible<Right>::value)
      : _isLeft(other._isLeft)
    {
      assignFrom(other._storage);
    }

    Either &operator= (const Either &other) noexcept(std::is_nothrow_copy_constructible<Left>::value && std::is_nothrow_copy_constructible<Right>::value && std::is_nothrow_destructible<Left>::value && std::is_nothrow_destructible<Right>::value)
    {
      destroy();

      _isLeft = other._isLeft;
      assignFrom(other._storage);

      return *this;
    }

    Either (Either &&other) noexcept(std::is_nothrow_move_constructible<Left>::value && std::is_nothrow_move_constructible<Right>::value)
      : _isLeft(other._isLeft)
    {
      assignFrom(std::move(other._storage));
    }

    Either &operator= (Either &&other) noexcept(std::is_nothrow_move_constructible<Left>::value && std::is_nothrow_move_constructible<Right>::value && std::is_nothrow_destructible<Left>::value && std::is_nothrow_destructible<Right>::value)
    {
      destroy();

      _isLeft = other._isLeft;
      assignFrom(std::move(other._storage));

      return *this;
    }

    ~Either () noexcept(std::is_nothrow_destructible<Left>::value && std::is_nothrow_destructible<Right>::value)
    {
      destroy();
    }

    bool hasLeft () const noexcept
    {
      return _isLeft;
    }

    Left &left () noexcept
    {
      return _storage._left;
    }

    const Left &left () const noexcept
    {
      return _storage._left;
    }

    bool hasRight () const noexcept
    {
      return !_isLeft;
    }

    Right &right () noexcept
    {
      return _storage._right;
    }

    const Right &right () const noexcept
    {
      return _storage._right;
    }

    template<typename IfLeft, typename IfRight>
    auto match (IfLeft &&ifLeft, IfRight &&ifRight) const
    {
      if (_isLeft) {
        return std::forward<IfLeft>(ifLeft)(left());
      } else {
        return std::forward<IfRight>(ifRight)(right());
      }
    }

  private:
    union Storage {
      left_type _left;
      right_type _right;

      Storage () noexcept
      {}

      Storage (left_type left, std::true_type isLeft) noexcept(std::is_nothrow_move_constructible<Left>::value)
        : _left(std::move(left))
      {}

      Storage (right_type right, std::false_type isLeft) noexcept(std::is_nothrow_move_constructible<Right>::value)
        : _right(std::move(right))
      {}

      ~Storage ()
      {}
    } _storage;

    bool _isLeft;

    void destroy () noexcept(std::is_nothrow_destructible<Left>::value && std::is_nothrow_destructible<Right>::value)
    {
      if (_isLeft) {
        _storage._left.~Left();
      } else {
        _storage._right.~Right();
      }
    }

    void assignFrom (const Storage &otherStorage) noexcept(std::is_nothrow_copy_constructible<Left>::value && std::is_nothrow_copy_constructible<Right>::value)
    {
      if (_isLeft) {
        _storage._left = otherStorage._left;
      } else {
        _storage._right = otherStorage._right;
      }
    }

    void assignFrom (Storage &&otherStorage) noexcept(std::is_nothrow_move_constructible<Left>::value && std::is_nothrow_move_constructible<Right>::value)
    {
      if (_isLeft) {
        _storage._left = std::move(otherStorage._left);
      } else {
        _storage._right = std::move(otherStorage._right);
      }
    }
};

template<typename Left, typename Right>
auto makeLeft (Left value)
{
  return Either<Left, Right>(std::forward<Left>(value), std::true_type());
}

template<typename Left, typename Right>
auto makeRight (Right value)
{
  return Either<Left, Right>(std::forward<Right>(value), std::false_type());
}

template<typename Left, typename Right>
bool operator== (const Either<Left, Right> &lhs, const Either<Left, Right> &rhs)
{
  if (lhs.hasLeft()) {
    if (rhs.hasLeft()) {
      return lhs.left() == rhs.left();
    } else {
      return false;
    }
  } else {
    if (rhs.hasRight()) {
      return lhs.right() == rhs.right();
    } else {
      return false;
    }
  }
}

} } // namespace fb::sinkline

#endif
