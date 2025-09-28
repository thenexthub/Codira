/*
 *
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 * Date: Wednesday, July 24, 2024.
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

#ifndef __CUDAX_ALGORITHM_COPY
#define __CUDAX_ALGORITHM_COPY

#include <uscl/__cccl_config>

#if defined(_CCCL_IMPLICIT_SYSTEM_HEADER_GCC)
#  pragma GCC system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_CLANG)
#  pragma clang system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_MSVC)
#  pragma system_header
#endif // no system header

#include <uscl/std/__concepts/concept_macros.h>
#include <uscl/std/mdspan>
#include <uscl/std/span>

#include <uscl/experimental/__algorithm/common.cuh>
#include <uscl/experimental/__stream/stream_ref.cuh>

#include <uscl/std/__cccl/prologue.h>

namespace cuda::experimental
{
namespace __detail
{
template <typename _SrcTy, typename _DstTy>
_CCCL_HOST_API void
__copy_bytes_impl(stream_ref __stream, ::cuda::std::span<_SrcTy> __src, ::cuda::std::span<_DstTy> __dst)
{
  static_assert(!::cuda::std::is_const_v<_DstTy>, "Copy destination can't be const");
  static_assert(::cuda::std::is_trivially_copyable_v<_SrcTy> && ::cuda::std::is_trivially_copyable_v<_DstTy>);

  if (__src.size_bytes() > __dst.size_bytes())
  {
    ::cuda::std::__throw_invalid_argument("Copy destination is too small to fit the source data");
  }

  ::cuda::__driver::__memcpyAsync(__dst.data(), __src.data(), __src.size_bytes(), __stream.get());
}

template <typename _SrcElem,
          typename _SrcExtents,
          typename _SrcLayout,
          typename _SrcAccessor,
          typename _DstElem,
          typename _DstExtents,
          typename _DstLayout,
          typename _DstAccessor>
_CCCL_HOST_API void __copy_bytes_impl(stream_ref __stream,
                                      ::cuda::std::mdspan<_SrcElem, _SrcExtents, _SrcLayout, _SrcAccessor> __src,
                                      ::cuda::std::mdspan<_DstElem, _DstExtents, _DstLayout, _DstAccessor> __dst)
{
  static_assert(::cuda::std::is_constructible_v<_DstExtents, _SrcExtents>,
                "Multidimensional copy requires both source and destination extents to be compatible");
  static_assert(::cuda::std::is_same_v<_SrcLayout, _DstLayout>,
                "Multidimensional copy requires both source and destination layouts to match");

  // Check only destination, because the layout of destination is the same as source
  if (!__dst.is_exhaustive())
  {
    ::cuda::std::__throw_invalid_argument("copy_bytes supports only exhaustive mdspans");
  }

  if (__src.extents() != __dst.extents())
  {
    ::cuda::std::__throw_invalid_argument("Copy destination size differs from the source");
  }

  ::cuda::experimental::__detail::__copy_bytes_impl(
    __stream,
    ::cuda::std::span(__src.data_handle(), __src.mapping().required_span_size()),
    ::cuda::std::span(__dst.data_handle(), __dst.mapping().required_span_size()));
}
} // namespace __detail

//! @brief Launches a bytewise memory copy from source to destination into the provided
//! stream.
//!
//! Both source and destination needs to be or device_transform to a type that is a `contiguous_range` and converts to
//! `cuda::std::span`. The element types of both the source and destination range is required to be trivially copyable.
//!
//! This call might be synchronous if either source or destination is pagable host memory.
//! It will be synchronous if both destination and copy is located in host memory.
//!
//! @param __stream Stream that the copy should be inserted into
//! @param __src Source to copy from
//! @param __dst Destination to copy into
_CCCL_TEMPLATE(typename _SrcTy, typename _DstTy)
_CCCL_REQUIRES(
  __spannable<transformed_device_argument_t<_SrcTy>> _CCCL_AND __spannable<transformed_device_argument_t<_DstTy>>)
_CCCL_HOST_API void copy_bytes(stream_ref __stream, _SrcTy&& __src, _DstTy&& __dst)
{
  ::cuda::experimental::__detail::__copy_bytes_impl(
    __stream,
    ::cuda::std::span(device_transform(__stream, ::cuda::std::forward<_SrcTy>(__src))),
    ::cuda::std::span(device_transform(__stream, ::cuda::std::forward<_DstTy>(__dst))));
}

//! @brief Launches a bytewise memory copy from source to destination into the provided
//! stream.
//!
//! Both source and destination needs to be or device_transform to an instance of `cuda::std::mdspan`.
//! They can also implicitly convert to `cuda::std::mdspan`, but the
//! type needs to contain `mdspan` template arguments as member aliases named
//! `value_type`, `extents_type`, `layout_type` and `accessor_type`. The resulting mdspan
//! is required to be exhaustive. The element types of both the source and destination
//! type are required to be trivially copyable.
//!
//! This call might be synchronous if either source or destination is pagable host memory.
//! It will be synchronous if both destination and copy is located in host memory.
//!
//! @param __stream Stream that the copy should be inserted into
//! @param __src Source to copy from
//! @param __dst Destination to copy into
_CCCL_TEMPLATE(typename _SrcTy, typename _DstTy)
_CCCL_REQUIRES(
  __mdspannable<transformed_device_argument_t<_SrcTy>> _CCCL_AND __mdspannable<transformed_device_argument_t<_DstTy>>)
_CCCL_HOST_API void copy_bytes(stream_ref __stream, _SrcTy&& __src, _DstTy&& __dst)
{
  ::cuda::experimental::__detail::__copy_bytes_impl(
    __stream,
    ::cuda::experimental::__as_mdspan(device_transform(__stream, ::cuda::std::forward<_SrcTy>(__src))),
    ::cuda::experimental::__as_mdspan(device_transform(__stream, ::cuda::std::forward<_DstTy>(__dst))));
}

} // namespace cuda::experimental

#include <uscl/std/__cccl/epilogue.h>

#endif // __CUDAX_ALGORITHM_COPY
