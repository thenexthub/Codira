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

#ifndef _CUDA___ANNOTATED_PTR_APPLY_ACCESS_PROPERTY_H
#define _CUDA___ANNOTATED_PTR_APPLY_ACCESS_PROPERTY_H

#include <uscl/std/detail/__config>

#if defined(_CCCL_IMPLICIT_SYSTEM_HEADER_GCC)
#  pragma GCC system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_CLANG)
#  pragma clang system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_MSVC)
#  pragma system_header
#endif // no system header

#include <uscl/__annotated_ptr/access_property.h>
#include <uscl/__memory/address_space.h>

#include <uscl/std/__cccl/prologue.h>

_CCCL_BEGIN_NAMESPACE_CUDA

template <typename _Shape>
_CCCL_API inline void apply_access_property(
  [[maybe_unused]] const volatile void* __ptr,
  [[maybe_unused]] _Shape __shape,
  [[maybe_unused]] access_property::persisting __prop) noexcept
{
  // clang-format off
  NV_IF_TARGET(
    NV_PROVIDES_SM_80,
    (_CCCL_ASSERT(__ptr != nullptr, "null pointer");
     if (!::cuda::device::is_address_from(__ptr, ::cuda::device::address_space::global))
     {
       return;
     }
     constexpr size_t __line_size = 128;
     auto __p                     = reinterpret_cast<uint8_t*>(const_cast<void*>(__ptr));
     auto __nbytes                = static_cast<size_t>(__shape);
     // Apply to all 128 bytes aligned cache lines inclusive of __p
     for (size_t __i = 0; __i < __nbytes; __i += __line_size) {
       asm volatile("prefetch.global.L2::evict_last [%0];" ::"l"(__p + __i) :);
     }))
  // clang-format on
}

template <typename _Shape>
_CCCL_API inline void apply_access_property(
  [[maybe_unused]] const volatile void* __ptr,
  [[maybe_unused]] _Shape __shape,
  [[maybe_unused]] access_property::normal __prop) noexcept
{
  // clang-format off
  NV_IF_TARGET(
    NV_PROVIDES_SM_80,
    (_CCCL_ASSERT(__ptr != nullptr, "null pointer");
     if (!::cuda::device::is_address_from(__ptr, ::cuda::device::address_space::global))
     {
       return;
     }
     constexpr size_t __line_size = 128;
     auto __p                     = reinterpret_cast<uint8_t*>(const_cast<void*>(__ptr));
     auto __nbytes                = static_cast<size_t>(__shape);
     // Apply to all 128 bytes aligned cache lines inclusive of __p
     for (size_t __i = 0; __i < __nbytes; __i += __line_size) {
       asm volatile("prefetch.global.L2::evict_normal [%0];" ::"l"(__p + __i) :);
     }))
  // clang-format on
}

_CCCL_END_NAMESPACE_CUDA

#include <uscl/std/__cccl/epilogue.h>

#endif // _CUDA___ANNOTATED_PTR_APPLY_ACCESS_PROPERTY_H
