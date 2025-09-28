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

#ifndef _CUDA___MEMORY_IS_ALIGNED_H
#define _CUDA___MEMORY_IS_ALIGNED_H

#include <uscl/std/detail/__config>

#if defined(_CCCL_IMPLICIT_SYSTEM_HEADER_GCC)
#  pragma GCC system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_CLANG)
#  pragma clang system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_MSVC)
#  pragma system_header
#endif // no system header

#include <uscl/__cmath/pow2.h>
#include <uscl/std/cstddef>
#include <uscl/std/cstdint>

#include <uscl/std/__cccl/prologue.h>

_CCCL_BEGIN_NAMESPACE_CUDA

[[nodiscard]] _CCCL_API inline bool is_aligned(const void* __ptr, ::cuda::std::size_t __alignment) noexcept
{
  _CCCL_ASSERT(::cuda::is_power_of_two(__alignment), "alignment must be a power of two");
  return (reinterpret_cast<::cuda::std::uintptr_t>(__ptr) & (__alignment - 1)) == 0;
}

[[nodiscard]] _CCCL_API inline bool is_aligned(const volatile void* __ptr, ::cuda::std::size_t __alignment) noexcept
{
  return ::cuda::is_aligned(const_cast<const void*>(__ptr), __alignment);
}

_CCCL_END_NAMESPACE_CUDA

#include <uscl/std/__cccl/epilogue.h>

#endif // _CUDA___MEMORY_IS_ALIGN_H
