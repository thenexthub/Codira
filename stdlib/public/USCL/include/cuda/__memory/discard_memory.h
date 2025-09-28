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

#ifndef _CUDA___MEMORY_DISCARD_MEMORY_H
#define _CUDA___MEMORY_DISCARD_MEMORY_H

#include <uscl/std/detail/__config>

#if defined(_CCCL_IMPLICIT_SYSTEM_HEADER_GCC)
#  pragma GCC system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_CLANG)
#  pragma clang system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_MSVC)
#  pragma system_header
#endif // no system header

#include <uscl/__memory/address_space.h>
#include <uscl/__memory/align_down.h>
#include <uscl/__memory/align_up.h>
#include <uscl/std/cstddef>

#include <uscl/std/__cccl/prologue.h>

_CCCL_BEGIN_NAMESPACE_CUDA

_CCCL_API inline void
discard_memory([[maybe_unused]] volatile void* __ptr, [[maybe_unused]] ::cuda::std::size_t __nbytes) noexcept {
// The discard PTX instruction is only available with PTX ISA 7.4 and later
#if __cccl_ptx_isa >= 740ULL
  NV_IF_TARGET(NV_PROVIDES_SM_80, ({
                 _CCCL_ASSERT(__ptr != nullptr, "null pointer passed to discard_memory");
                 if (!::cuda::device::is_address_from(__ptr, ::cuda::device::address_space::global))
                 {
                   return;
                 }

                 constexpr ::cuda::std::size_t __line_size = 128;

                 // Trim the first block and last block if they're not 128 bytes aligned
                 const auto __p             = static_cast<char*>(const_cast<void*>(__ptr));
                 const auto __end_p         = __p + __nbytes;
                 const auto __start_aligned = ::cuda::align_up(__p, __line_size);
                 const auto __end_aligned   = ::cuda::align_down(__end_p, __line_size);

                 for (auto __i = __start_aligned; __i < __end_aligned; __i += __line_size)
                 {
                   asm volatile("discard.global.L2 [%0], 128;" ::"l"(__i) :);
                 }
               }))
#endif // __cccl_ptx_isa >= 740ULL
}

_CCCL_END_NAMESPACE_CUDA

#include <uscl/std/__cccl/epilogue.h>

#endif // _CUDA___MEMORY_DISCARD_MEMORY_H
