/*
 *
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 * Date: Sunday, April 14, 2024.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
 * Middletown, DE 19709, New Castle County, USA.
 *
 */

#ifndef _CUDA___ITERATOR_SHUFFLE_ITERATOR_H
#define _CUDA___ITERATOR_SHUFFLE_ITERATOR_H

#include <uscl/std/detail/__config>

#if defined(_CCCL_IMPLICIT_SYSTEM_HEADER_GCC)
#  pragma GCC system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_CLANG)
#  pragma clang system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_MSVC)
#  pragma system_header
#endif // no system header

#if _LIBCUDACXX_HAS_SPACESHIP_OPERATOR()
#  include <cuda/std/__compare/three_way_comparable.h>
#endif // _LIBCUDACXX_HAS_SPACESHIP_OPERATOR()
#include <uscl/__random/random_bijection.h>
#include <uscl/std/__concepts/constructible.h>
#include <uscl/std/__random/is_valid.h>
#include <uscl/std/__type_traits/is_constructible.h>
#include <uscl/std/__type_traits/is_integral.h>
#include <uscl/std/__type_traits/is_nothrow_constructible.h>
#include <uscl/std/__type_traits/is_nothrow_copy_constructible.h>
#include <uscl/std/__type_traits/is_nothrow_default_constructible.h>
#include <uscl/std/__type_traits/is_nothrow_move_constructible.h>
#include <uscl/std/__type_traits/is_same.h>
#include <uscl/std/__type_traits/make_signed.h>
#include <uscl/std/__utility/forward.h>
#include <uscl/std/__utility/move.h>
#include <uscl/std/cstdint>

#include <uscl/std/__cccl/prologue.h>

_CCCL_BEGIN_NAMESPACE_CUDA

//! @brief Verifies that a given type @tparam _Bijection is a valid bijection function.
//! It verifies
//! * The bijection has a type alias ``index_type`` that satisfies ``integral``
//! * The bijection has a non-mutable member function size() that returns the number of elements as ``index_type``
//! * The bijection has a non-mutable call operator that takes a value of type ``index_type`` in the range
//! ``[0, size())`` and projects it into the range ``[0, size())``
template <class _Bijection>
_CCCL_CONCEPT __is_bijection = _CCCL_REQUIRES_EXPR((_Bijection), const _Bijection& __fun)(
  typename(typename _Bijection::index_type),
  requires(::cuda::std::is_integral_v<typename _Bijection::index_type>),
  requires(::cuda::std::is_same_v<decltype(__fun.size()), typename _Bijection::index_type>),
  requires(
    ::cuda::std::is_same_v<decltype(__fun(typename _Bijection::index_type(0))), typename _Bijection::index_type>));

//! @brief shuffle_iterator is an iterator which generates a sequence of integral values representing a random
//! permutation.
//! @tparam _IndexType The type of the index to shuffle. Defaults to uint64_t
//! @tparam _BijectionFunc The bijection to use. This should be a bijective function that maps [0..n) -> [0..n). It must
//! be deterministic and stateless. Defaults to cuda::random_biijection<_IndexType>
//!
//! @class shuffle_iterator is an iterator which generates a sequence of values representing a random permutation. This
//! iterator is useful for working with random permutations of a range without explicitly storing them in memory. The
//! shuffle iterator is also useful for sampling from a range by selecting only a subset of the elements in the
//! permutation.
//!
//! The following code snippet demonstrates how to create a @c shuffle_iterator which generates a random permutation
//! of the range[0, 4)
//!
//! @code
//! #include <uscl/iterator>
//! ...
//! // create a shuffle iterator
//! cuda::shuffle_iterator iterator{cuda::random_bijection{4, cuda::std::minstd_rand(0xDEADBEEF)}};
//! // iterator[0] returns 1
//! // iterator[1] returns 3
//! // iterator[2] returns 2
//! // iterator[3] returns 0
//! @endcode
template <class _IndexType = ::cuda::std::size_t, class _Bijection = random_bijection<_IndexType>>
class shuffle_iterator
{
private:
  _Bijection __bijection_{};
  _IndexType __current_{0};

  static_assert(::cuda::std::is_integral_v<_IndexType>, "_IndexType must be an integral type");
  static_assert(__is_bijection<_Bijection>, "_Bijection must be a valid bijection function");

public:
  using iterator_category = ::cuda::std::random_access_iterator_tag;
  using iterator_concept  = ::cuda::std::random_access_iterator_tag;
  using value_type        = _IndexType;
  using difference_type   = ::cuda::std::make_signed_t<value_type>;

  _CCCL_HIDE_FROM_ABI constexpr shuffle_iterator() noexcept = default;

  //! @brief Constructs a shuffle iterator from a given bijection and an optional start position
  //! @param __bijection The bijection representing the shuffled integer sequence
  //! @param __start The position of the iterator in the shuffled integer sequence
  _CCCL_API constexpr shuffle_iterator(_Bijection __bijection, value_type __start = 0) noexcept(
    ::cuda::std::is_nothrow_move_constructible_v<_Bijection>)
      : __bijection_(::cuda::std::move(__bijection))
      , __current_(__start)
  {}

  //! @brief Constructs a shuffle iterator representing a sequence of size @param __num_elements from a random number
  //! generator @param __gen and an optional start position @param __start
  //! It constructs the bijection function @tparam _Bijection from the desired size of the sequence and a random number
  //! generator
  template <class _RNG>
  // constraining here breaks CTAD
  _CCCL_API explicit constexpr shuffle_iterator(value_type __num_elements, _RNG&& __gen, value_type __start = 0) //
    noexcept(::cuda::std::is_nothrow_constructible_v<_Bijection, value_type, _RNG>)
      : __bijection_(__num_elements, ::cuda::std::forward<_RNG>(__gen))
      , __current_(__start)
  {}

  [[nodiscard]] _CCCL_API constexpr value_type operator*() const noexcept(noexcept(__bijection_(0)))
  {
    _CCCL_ASSERT(__current_ < static_cast<value_type>(__bijection_.size()),
                 "shuffle_iterator::operator*: Trying to dereference a shuffle_iterator past the end!");
    return static_cast<value_type>(__bijection_(static_cast<typename _Bijection::index_type>(__current_)));
  }

  [[nodiscard]] _CCCL_API constexpr value_type operator[](difference_type __n) const noexcept(noexcept(__bijection_(0)))
  {
    _CCCL_ASSERT(static_cast<value_type>(static_cast<difference_type>(__current_) + __n)
                   < static_cast<value_type>(__bijection_.size()),
                 "shuffle_iterator::operator*: Trying to subscript a shuffle_iterator past the end!");
    return static_cast<value_type>(__bijection_(static_cast<typename _Bijection::index_type>(__current_ + __n)));
  }

  _CCCL_API constexpr shuffle_iterator& operator++() noexcept
  {
    ++__current_;
    return *this;
  }

  _CCCL_API constexpr shuffle_iterator operator++(int) noexcept(::cuda::std::is_nothrow_copy_constructible_v<_Bijection>)
  {
    auto __tmp = *this;
    ++__current_;
    return __tmp;
  }

  _CCCL_API constexpr shuffle_iterator& operator--() noexcept
  {
    --__current_;
    return *this;
  }

  [[nodiscard]] _CCCL_API constexpr shuffle_iterator
  operator--(int) noexcept(::cuda::std::is_nothrow_copy_constructible_v<_Bijection>)
  {
    auto __tmp = *this;
    --__current_;
    return __tmp;
  }

  _CCCL_API constexpr shuffle_iterator& operator+=(difference_type __n) noexcept
  {
#if _CCCL_COMPILER(MSVC) // C4308: negative integral constant converted to unsigned type
    __current_ = static_cast<value_type>(static_cast<difference_type>(__current_) + __n);
#else // ^^^ _CCCL_COMPILER(MSVC) ^^^ / vvv !_CCCL_COMPILER(MSVC) vvv
    __current_ += __n;
#endif // !_CCCL_COMPILER(MSVC)
    return *this;
  }

  _CCCL_API constexpr shuffle_iterator& operator-=(difference_type __n) noexcept
  {
#if _CCCL_COMPILER(MSVC) // C4308: negative integral constant converted to unsigned type
    __current_ = static_cast<value_type>(static_cast<difference_type>(__current_) - __n);
#else // ^^^ _CCCL_COMPILER(MSVC) ^^^ / vvv !_CCCL_COMPILER(MSVC) vvv
    __current_ -= __n;
#endif // !_CCCL_COMPILER(MSVC)
    return *this;
  }

  [[nodiscard]] _CCCL_API friend constexpr shuffle_iterator operator+(shuffle_iterator __i, difference_type __n) noexcept
  {
#if _CCCL_COMPILER(MSVC) // C4308: negative integral constant converted to unsigned type
    __i.__current_ = static_cast<value_type>(static_cast<difference_type>(__i.__current_) + __n);
#else // ^^^ _CCCL_COMPILER(MSVC) ^^^ / vvv !_CCCL_COMPILER(MSVC) vvv
    __i.__current_ += __n;
#endif // !_CCCL_COMPILER(MSVC)
    return __i;
  }

  [[nodiscard]] _CCCL_API friend constexpr shuffle_iterator operator+(difference_type __n, shuffle_iterator __i) noexcept
  {
#if _CCCL_COMPILER(MSVC) // C4308: negative integral constant converted to unsigned type
    __i.__current_ = static_cast<value_type>(static_cast<difference_type>(__i.__current_) + __n);
#else // ^^^ _CCCL_COMPILER(MSVC) ^^^ / vvv !_CCCL_COMPILER(MSVC) vvv
    __i.__current_ += __n;
#endif // !_CCCL_COMPILER(MSVC)
    return __i;
  }

  [[nodiscard]] _CCCL_API friend constexpr shuffle_iterator operator-(shuffle_iterator __i, difference_type __n) noexcept
  {
#if _CCCL_COMPILER(MSVC) // C4308: negative integral constant converted to unsigned type
    __i.__current_ = static_cast<value_type>(static_cast<difference_type>(__i.__current_) - __n);
#else // ^^^ _CCCL_COMPILER(MSVC) ^^^ / vvv !_CCCL_COMPILER(MSVC) vvv
    __i.__current_ -= __n;
#endif // !_CCCL_COMPILER(MSVC)
    return __i;
  }

  [[nodiscard]] _CCCL_API friend constexpr difference_type
  operator-(const shuffle_iterator& __x, const shuffle_iterator& __y) noexcept
  {
    return static_cast<difference_type>(__x.__current_ - __y.__current_);
  }

  [[nodiscard]] _CCCL_API friend constexpr bool
  operator==(const shuffle_iterator& __x, const shuffle_iterator& __y) noexcept
  {
    return __x.__current_ == __y.__current_;
  }

#if _CCCL_STD_VER <= 2017
  [[nodiscard]] _CCCL_API friend constexpr bool
  operator!=(const shuffle_iterator& __x, const shuffle_iterator& __y) noexcept
  {
    return __x.__current_ != __y.__current_;
  }
#endif // _CCCL_STD_VER <= 2017

#if _LIBCUDACXX_HAS_SPACESHIP_OPERATOR()
  [[nodiscard]] _CCCL_API friend constexpr ::cuda::std::strong_ordering
  operator<=>(const shuffle_iterator& __x, const shuffle_iterator& __y) noexcept
  {
    return __x.__current_ <=> __y.__current_;
  }
#else // ^^^ _LIBCUDACXX_HAS_NO_SPACESHIP_OPERATOR ^^^ / vvv !_LIBCUDACXX_HAS_NO_SPACESHIP_OPERATOR vvv

  [[nodiscard]] _CCCL_API friend constexpr bool
  operator<(const shuffle_iterator& __x, const shuffle_iterator& __y) noexcept
  {
    return __x.__current_ < __y.__current_;
  }

  [[nodiscard]] _CCCL_API friend constexpr bool
  operator>(const shuffle_iterator& __x, const shuffle_iterator& __y) noexcept
  {
    return __x.__current_ > __y.__current_;
  }

  [[nodiscard]] _CCCL_API friend constexpr bool
  operator<=(const shuffle_iterator& __x, const shuffle_iterator& __y) noexcept
  {
    return __x.__current_ <= __y.__current_;
  }

  [[nodiscard]] _CCCL_API friend constexpr bool
  operator>=(const shuffle_iterator& __x, const shuffle_iterator& __y) noexcept
  {
    return __x.__current_ >= __y.__current_;
  }
#endif // !_LIBCUDACXX_HAS_NO_SPACESHIP_OPERATOR
};

_CCCL_TEMPLATE(class _Bijection)
_CCCL_REQUIRES(__is_bijection<_Bijection>)
_CCCL_HOST_DEVICE shuffle_iterator(_Bijection) -> shuffle_iterator<typename _Bijection::index_type, _Bijection>;

_CCCL_TEMPLATE(class _Bijection, typename _Integral)
_CCCL_REQUIRES(__is_bijection<_Bijection> _CCCL_AND ::cuda::std::is_integral_v<_Integral>)
_CCCL_HOST_DEVICE shuffle_iterator(_Bijection, _Integral)
  -> shuffle_iterator<typename _Bijection::index_type, _Bijection>;

//! @brief make_shuffle_iterator creates a \p shuffle_iterator from an integer \p __num_elements and a bijection
//! function \p __bijection
//! @param __fun The bijection function used for shuffling
//! @param __start The starting position of the shuffle_iterator
template <class _Bijection, class _IndexType>
[[nodiscard]] _CCCL_API constexpr auto make_shuffle_iterator(_Bijection __fun, _IndexType __start = 0)
{
  return shuffle_iterator<_IndexType, _Bijection>{::cuda::std::move(__fun), __start};
}

_CCCL_END_NAMESPACE_CUDA

#include <uscl/std/__cccl/epilogue.h>

#endif // _CUDA___ITERATOR_TRANSFORM_ITERATOR_H
