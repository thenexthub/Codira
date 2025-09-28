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

#ifndef _CUDA___UTILITY_BASIC_ANY_DYNAMIC_ANY_CAST_H
#define _CUDA___UTILITY_BASIC_ANY_DYNAMIC_ANY_CAST_H

#include <uscl/std/detail/__config>

#if defined(_CCCL_IMPLICIT_SYSTEM_HEADER_GCC)
#  pragma GCC system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_CLANG)
#  pragma clang system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_MSVC)
#  pragma system_header
#endif // no system header

#include <uscl/__utility/__basic_any/access.h>
#include <uscl/__utility/__basic_any/basic_any_fwd.h>
#include <uscl/__utility/__basic_any/conversions.h>
#include <uscl/std/__concepts/concept_macros.h>
#include <uscl/std/__type_traits/is_pointer.h>
#include <uscl/std/__utility/move.h>

#include <uscl/std/__cccl/prologue.h>

_CCCL_BEGIN_NAMESPACE_CUDA

//! \brief Casts one __basic_any reference type to another __basic_any type using
//! runtime information to determine the validity of the conversion.
//!
//! \throws __bad_any_cast when \c __src cannot be dynamically cast to a
//! \c __basic_any<_DstInterface>.
_CCCL_TEMPLATE(class _DstInterface, class _SrcInterface)
_CCCL_REQUIRES(__any_castable_to<__basic_any<_SrcInterface>, __basic_any<_DstInterface>>)
[[nodiscard]] _CCCL_API auto __dynamic_any_cast(__basic_any<_SrcInterface>&& __src) -> __basic_any<_DstInterface>
{
  auto __dst = __basic_any_access::__make<_DstInterface>();
  __basic_any_access::__cast_to(::cuda::std::move(__src), __dst);
  return __dst;
}

//! \overload
_CCCL_TEMPLATE(class _DstInterface, class _SrcInterface)
_CCCL_REQUIRES(__any_castable_to<__basic_any<_SrcInterface>&, __basic_any<_DstInterface>>)
[[nodiscard]] _CCCL_API auto __dynamic_any_cast(__basic_any<_SrcInterface>& __src) -> __basic_any<_DstInterface>
{
  auto __dst = __basic_any_access::__make<_DstInterface>();
  __basic_any_access::__cast_to(__src, __dst);
  return __dst;
}

//! \overload
_CCCL_TEMPLATE(class _DstInterface, class _SrcInterface)
_CCCL_REQUIRES(__any_castable_to<__basic_any<_SrcInterface> const&, __basic_any<_DstInterface>>)
[[nodiscard]] _CCCL_API auto __dynamic_any_cast(__basic_any<_SrcInterface> const& __src) -> __basic_any<_DstInterface>
{
  auto __dst = __basic_any_access::__make<_DstInterface>();
  __basic_any_access::__cast_to(__src, __dst);
  return __dst;
}

//! \brief Casts a `__basic_any<_SrcInterface>*` into a `__basic_any<_DstInterface>`
//! using runtime information to determine the validity of the conversion.
//!
//! \pre \c _DstInterface must be a pointer type.
//!
//! \returns \c nullptr when \c __src cannot be dynamically cast to a
//! \c __basic_any<_DstInterface>.
_CCCL_TEMPLATE(class _DstInterface, class _SrcInterface)
_CCCL_REQUIRES(__any_castable_to<__basic_any<_SrcInterface>*, __basic_any<_DstInterface>>)
[[nodiscard]] _CCCL_API auto __dynamic_any_cast(__basic_any<_SrcInterface>* __src) -> __basic_any<_DstInterface>
{
  static_assert(
    ::cuda::std::is_pointer_v<_DstInterface>,
    "when __dynamic_any_cast-ing from a pointer to a __basic_any, the destination type must be a pointer to an "
    "interface type.");
  auto __dst = __basic_any_access::__make<_DstInterface>();
  __basic_any_access::__cast_to(__src, __dst);
  return __dst;
}

//! \overload
_CCCL_TEMPLATE(class _DstInterface, class _SrcInterface)
_CCCL_REQUIRES(__any_castable_to<__basic_any<_SrcInterface> const*, __basic_any<_DstInterface>>)
[[nodiscard]] _CCCL_API auto __dynamic_any_cast(__basic_any<_SrcInterface> const* __src) -> __basic_any<_DstInterface>
{
  static_assert(
    ::cuda::std::is_pointer_v<_DstInterface>,
    "when __dynamic_any_cast-ing from a pointer to a __basic_any, the destination type must be a pointer to an "
    "interface type.");
  auto __dst = __basic_any_access::__make<_DstInterface>();
  __basic_any_access::__cast_to(__src, __dst);
  return __dst;
}

_CCCL_END_NAMESPACE_CUDA

#include <uscl/std/__cccl/epilogue.h>

#endif // _CUDA___UTILITY_BASIC_ANY_DYNAMIC_ANY_CAST_H
