/*
 *
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 * Date: Friday, December 29, 2023.
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

#ifndef _CUDA_STD___MEMORY_ALIGN_H
#define _CUDA_STD___MEMORY_ALIGN_H

#include <uscl/std/detail/__config>

#if defined(_CCCL_IMPLICIT_SYSTEM_HEADER_GCC)
#  pragma GCC system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_CLANG)
#  pragma clang system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_MSVC)
#  pragma system_header
#endif // no system header

#include <uscl/__cmath/pow2.h>
#include <uscl/std/__memory/assume_aligned.h>
#include <uscl/std/__memory/runtime_assume_aligned.h>
#include <uscl/std/cstddef>
#include <uscl/std/cstdint>

#include <uscl/std/__cccl/prologue.h>

_CCCL_DIAG_PUSH
_CCCL_DIAG_SUPPRESS_MSVC(4146) // unary minus operator applied to unsigned type, result still unsigned

_CCCL_BEGIN_NAMESPACE_CUDA_STD

_CCCL_API inline void* align(size_t __alignment, size_t __size, void*& __ptr, size_t& __space)
{
  _CCCL_ASSERT(::cuda::is_power_of_two(__alignment), "cuda::std::align: alignment must be a power of two!");
  if (__space < __size)
  {
    return nullptr;
  }

  char* __char_ptr = static_cast<char*>(__ptr);
  char* __aligned_ptr =
    reinterpret_cast<char*>(reinterpret_cast<uintptr_t>(__char_ptr + (__alignment - 1)) & -__alignment);
  const size_t __diff = static_cast<size_t>(__aligned_ptr - __char_ptr);
  if (__diff > (__space - __size))
  {
    return nullptr;
  }

  //! We need to avoid using __aligned_ptr here, as nvcc looses track of the execution space otherwise
  __ptr = reinterpret_cast<void*>(__char_ptr + __diff);
  __space -= __diff;
  return ::cuda::std::__runtime_assume_aligned(__ptr, __alignment);
}

_CCCL_END_NAMESPACE_CUDA_STD

_CCCL_DIAG_POP

#include <uscl/std/__cccl/epilogue.h>

#endif // _CUDA_STD___MEMORY_ALIGN_H
