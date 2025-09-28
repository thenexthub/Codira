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

#ifndef __CUDA___ALGORITHM_COMMON
#define __CUDA___ALGORITHM_COMMON

#include <uscl/__cccl_config>

#if defined(_CCCL_IMPLICIT_SYSTEM_HEADER_GCC)
#  pragma GCC system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_CLANG)
#  pragma clang system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_MSVC)
#  pragma system_header
#endif // no system header

#include <uscl/std/__concepts/concept_macros.h>
#include <uscl/std/__concepts/convertible_to.h>
#include <uscl/std/__ranges/concepts.h>
#include <uscl/std/__type_traits/remove_reference.h>
#include <uscl/std/mdspan>
#include <uscl/std/span>

#include <uscl/std/__cccl/prologue.h>

_CCCL_BEGIN_NAMESPACE_CUDA

template <typename _Tp>
using __as_span_t = ::cuda::std::span<::cuda::std::remove_reference_t<::cuda::std::ranges::range_reference_t<_Tp>>>;

//! @brief A concept that checks if the type can be converted to a `cuda::std::span`.
//! The type must be a contiguous range.
template <typename _Tp>
_CCCL_CONCEPT __spannable = _CCCL_REQUIRES_EXPR((_Tp))( //
  requires(::cuda::std::ranges::contiguous_range<_Tp>), //
  requires(::cuda::std::convertible_to<_Tp, __as_span_t<_Tp>>));

template <typename _Tp>
using __as_mdspan_t =
  ::cuda::std::mdspan<typename ::cuda::std::decay_t<_Tp>::value_type,
                      typename ::cuda::std::decay_t<_Tp>::extents_type,
                      typename ::cuda::std::decay_t<_Tp>::layout_type,
                      typename ::cuda::std::decay_t<_Tp>::accessor_type>;

//! @brief A concept that checks if the type can be converted to a `cuda::std::mdspan`.
//! The type must have a conversion to `__as_mdspan_t<_Tp>`.
template <typename _Tp>
_CCCL_CONCEPT __mdspannable =
  _CCCL_REQUIRES_EXPR((_Tp))(requires(::cuda::std::convertible_to<_Tp, __as_mdspan_t<_Tp>>));

template <typename _Tp>
[[nodiscard]] _CCCL_HOST_API constexpr auto __as_mdspan(_Tp&& __value) noexcept -> __as_mdspan_t<_Tp>
{
  return ::cuda::std::forward<_Tp>(__value);
}

_CCCL_END_NAMESPACE_CUDA

#include <uscl/std/__cccl/epilogue.h>

#endif //__CUDA___ALGORITHM_COMMON
