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

#ifndef _CUDAX__MEMORY_RESOURCE_RESOURCE_CUH
#define _CUDAX__MEMORY_RESOURCE_RESOURCE_CUH

#include <uscl/std/detail/__config>

#if defined(_CCCL_IMPLICIT_SYSTEM_HEADER_GCC)
#  pragma GCC system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_CLANG)
#  pragma clang system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_MSVC)
#  pragma system_header
#endif // no system header

#include <uscl/__memory_resource/resource.h>
#include <uscl/__utility/__basic_any/semiregular.h>
#include <uscl/std/__type_traits/is_same.h>

#include <uscl/std/__cccl/prologue.h>

namespace cuda::experimental
{

template <class _Resource, class _OtherResource>
_CCCL_CONCEPT __non_polymorphic_resources = _CCCL_REQUIRES_EXPR((_Resource, _OtherResource))(
  requires(::cuda::mr::synchronous_resource<_Resource>),
  requires(::cuda::mr::synchronous_resource<_OtherResource>),
  requires(__non_polymorphic<_Resource>),
  requires(__non_polymorphic<_OtherResource>));

//! @brief Equality comparison between two resources of different types. Always returns false.
_CCCL_TEMPLATE(class _Resource, class _OtherResource)
_CCCL_REQUIRES(
  (!::cuda::std::is_same_v<_Resource, _OtherResource>) _CCCL_AND __non_polymorphic_resources<_Resource, _OtherResource>)
[[nodiscard]] bool operator==(_Resource const&, _OtherResource const&) noexcept
{
  return false;
}

#if _CCCL_STD_VER <= 2017
//! @brief Inequality comparison between two resources of different types. Always returns true.
_CCCL_TEMPLATE(class _Resource, class _OtherResource)
_CCCL_REQUIRES(
  (!::cuda::std::is_same_v<_Resource, _OtherResource>) _CCCL_AND __non_polymorphic_resources<_Resource, _OtherResource>)
[[nodiscard]] bool operator!=(_Resource const&, _OtherResource const&) noexcept
{
  return true;
}
#endif // _CCCL_STD_VER <= 2017

} // namespace cuda::experimental

#include <uscl/std/__cccl/epilogue.h>

#endif //_CUDAX__MEMORY_RESOURCE_RESOURCE_CUH
