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

#ifndef _CUDA_STD___MDSPAN_SUBMDSPAN_HELPER_H
#define _CUDA_STD___MDSPAN_SUBMDSPAN_HELPER_H

#include <uscl/std/detail/__config>

#if defined(_CCCL_IMPLICIT_SYSTEM_HEADER_GCC)
#  pragma GCC system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_CLANG)
#  pragma clang system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_MSVC)
#  pragma system_header
#endif // no system header

#include <uscl/std/__concepts/concept_macros.h>
#include <uscl/std/__concepts/convertible_to.h>
#include <uscl/std/__fwd/mdspan.h>
#include <uscl/std/__mdspan/concepts.h>
#include <uscl/std/__mdspan/extents.h>
#include <uscl/std/__type_traits/is_integral.h>
#include <uscl/std/__type_traits/is_same.h>
#include <uscl/std/__type_traits/is_signed.h>
#include <uscl/std/__type_traits/is_unsigned.h>
#include <uscl/std/__utility/integer_sequence.h>
#include <uscl/std/array>
#include <uscl/std/tuple>

#include <uscl/std/__cccl/prologue.h>

_CCCL_BEGIN_NAMESPACE_CUDA_STD

// [mdspan.sub.overview]-2.5
template <class _IndexType, class... _SliceTypes>
[[nodiscard]] _CCCL_API constexpr array<size_t, sizeof...(_SliceTypes)> __map_rank(size_t __count = 0) noexcept
{
  return {(convertible_to<_SliceTypes, _IndexType> ? dynamic_extent : __count++)...};
}

// [mdspan.submdspan.strided.slice]
template <class _OffsetType, class _ExtentType, class _StrideType>
struct strided_slice
{
  using offset_type = _OffsetType;
  using extent_type = _ExtentType;
  using stride_type = _StrideType;

  static_assert(__index_like<offset_type>,
                "[mdspan.submdspan.strided.slice] cuda::std::strided_slice::offset_type must be signed or unsigned or "
                "integral-constant-like");
  static_assert(__index_like<extent_type>,
                "[mdspan.submdspan.strided.slice] cuda::std::strided_slice::extent_type must be signed or unsigned or "
                "integral-constant-like");
  static_assert(__index_like<stride_type>,
                "[mdspan.submdspan.strided.slice] cuda::std::strided_slice::stride_type must be signed or unsigned or "
                "integral-constant-like");

  _CCCL_NO_UNIQUE_ADDRESS offset_type offset{};
  _CCCL_NO_UNIQUE_ADDRESS extent_type extent{};
  _CCCL_NO_UNIQUE_ADDRESS stride_type stride{};
};

template <class _OffsetType, class _ExtentType, class _StrideType>
_CCCL_HOST_DEVICE strided_slice(_OffsetType, _ExtentType, _StrideType)
  -> strided_slice<_OffsetType, _ExtentType, _StrideType>;

template <typename>
inline constexpr bool __is_strided_slice = false;

template <class _OffsetType, class _ExtentType, class _StrideType>
inline constexpr bool __is_strided_slice<strided_slice<_OffsetType, _ExtentType, _StrideType>> = true;

struct full_extent_t
{
  _CCCL_HIDE_FROM_ABI explicit full_extent_t() = default;
};
inline constexpr full_extent_t full_extent{};

// [mdspan.submdspan.helpers]
_CCCL_TEMPLATE(class _Tp)
_CCCL_REQUIRES((!__integral_constant_like<_Tp>) )
[[nodiscard]] _CCCL_API constexpr _Tp __de_ice(_Tp __val) noexcept
{
  return __val;
}

_CCCL_TEMPLATE(class _Tp)
_CCCL_REQUIRES(__integral_constant_like<_Tp>)
[[nodiscard]] _CCCL_API constexpr auto __de_ice(_Tp) noexcept
{
  return _Tp::value;
}

template <class _IndexType, class _From>
[[nodiscard]] _CCCL_API constexpr auto __index_cast(_From&& __from) noexcept
{
  if constexpr (is_integral_v<_From> && !is_same_v<_From, bool>)
  {
    return __from;
  }
  else
  {
    return static_cast<_IndexType>(__from);
  }
}

template <size_t _Index, class... _Slices>
[[nodiscard]] _CCCL_API constexpr decltype(auto) __get_slice_at(_Slices&&... __slices) noexcept
{
  return ::cuda::std::get<_Index>(::cuda::std::forward_as_tuple(::cuda::std::forward<_Slices>(__slices)...));
}

template <size_t _Index, class... _Slices>
using __get_slice_type = tuple_element_t<_Index, __tuple_types<_Slices...>>;

template <class _IndexType, size_t _Index, class... _Slices>
[[nodiscard]] _CCCL_API constexpr _IndexType __first_extent_from_slice(_Slices... __slices) noexcept
{
  static_assert(is_signed_v<_IndexType> || is_unsigned_v<_IndexType>,
                "[mdspan.sub.helpers] mandates IndexType to be a signed or unsigned integral");
  using _SliceType                     = __get_slice_type<_Index, _Slices...>;
  [[maybe_unused]] _SliceType& __slice = ::cuda::std::__get_slice_at<_Index>(__slices...);
  if constexpr (convertible_to<_SliceType, _IndexType>)
  {
    return ::cuda::std::__index_cast<_IndexType>(__slice);
  }
  else
  {
    if constexpr (__index_pair_like<_SliceType, _IndexType>)
    {
      return ::cuda::std::__index_cast<_IndexType>(::cuda::std::get<0>(__slice));
    }
    else if constexpr (__is_strided_slice<_SliceType>)
    {
      return ::cuda::std::__index_cast<_IndexType>(::cuda::std::__de_ice(__slice.offset));
    }
    else
    {
      return 0;
    }
  }
  _CCCL_UNREACHABLE();
}

template <size_t _Index, class _Extents, class... _Slices>
[[nodiscard]] _CCCL_API constexpr typename _Extents::index_type
__last_extent_from_slice(const _Extents& __src, _Slices... __slices) noexcept
{
  static_assert(__mdspan_detail::__is_extents_v<_Extents>,
                "[mdspan.sub.helpers] mandates Extents to be a specialization of extents");
  using _IndexType                     = typename _Extents::index_type;
  using _SliceType                     = __get_slice_type<_Index, _Slices...>;
  [[maybe_unused]] _SliceType& __slice = ::cuda::std::__get_slice_at<_Index>(__slices...);
  if constexpr (convertible_to<_SliceType, _IndexType>)
  {
    return ::cuda::std::__index_cast<_IndexType>(::cuda::std::__de_ice(__slice) + 1);
  }
  else
  {
    if constexpr (__index_pair_like<_SliceType, _IndexType>)
    {
      return ::cuda::std::__index_cast<_IndexType>(::cuda::std::get<1>(__slice));
    }
    else if constexpr (__is_strided_slice<_SliceType>)
    {
      return ::cuda::std::__index_cast<_IndexType>(
        ::cuda::std::__de_ice(__slice.offset) * ::cuda::std::__de_ice(__slice.extent));
    }
    else
    {
      return ::cuda::std::__index_cast<_IndexType>(__src.extent(_Index));
    }
  }
  _CCCL_UNREACHABLE();
}

_CCCL_END_NAMESPACE_CUDA_STD

#include <uscl/std/__cccl/epilogue.h>

#endif // _CUDA_STD___MDSPAN_SUBMDSPAN_HELPER_H
