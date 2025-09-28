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

#ifndef _CUDA_STD___MEMORY_RUNTIME_ASSUME_ALIGNED_H
#define _CUDA_STD___MEMORY_RUNTIME_ASSUME_ALIGNED_H

#include <uscl/std/detail/__config>

#if defined(_CCCL_IMPLICIT_SYSTEM_HEADER_GCC)
#  pragma GCC system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_CLANG)
#  pragma clang system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_MSVC)
#  pragma system_header
#endif // no system header

#include <uscl/std/__type_traits/remove_volatile.h>
#include <uscl/std/cstddef>
#include <uscl/std/cstdint>

#include <uscl/std/__cccl/prologue.h>

_CCCL_BEGIN_NAMESPACE_CUDA_STD

template <typename _Tp>
[[nodiscard]] _CCCL_API _Tp* __runtime_assume_aligned(_Tp* __ptr, ::cuda::std::size_t __alignment) noexcept
{
#if defined(_CCCL_BUILTIN_ASSUME_ALIGNED)
  using _Up = remove_volatile_t<_Tp>;
  switch (__alignment)
  {
    case 1:
      return static_cast<_Tp*>(_CCCL_BUILTIN_ASSUME_ALIGNED(const_cast<_Up*>(__ptr), 1));
    case 2:
      return static_cast<_Tp*>(_CCCL_BUILTIN_ASSUME_ALIGNED(const_cast<_Up*>(__ptr), 2));
    case 4:
      return static_cast<_Tp*>(_CCCL_BUILTIN_ASSUME_ALIGNED(const_cast<_Up*>(__ptr), 4));
    case 8:
      return static_cast<_Tp*>(_CCCL_BUILTIN_ASSUME_ALIGNED(const_cast<_Up*>(__ptr), 8));
    case 16:
      return static_cast<_Tp*>(_CCCL_BUILTIN_ASSUME_ALIGNED(const_cast<_Up*>(__ptr), 16));
    default:
      return static_cast<_Tp*>(_CCCL_BUILTIN_ASSUME_ALIGNED(const_cast<_Up*>(__ptr), 32));
  }
#else
  _CCCL_ASSUME(reinterpret_cast<uintptr_t>(__ptr) % __alignment == 0);
  return __ptr;
#endif // defined(_CCCL_BUILTIN_ASSUME_ALIGNED)
}

_CCCL_END_NAMESPACE_CUDA

#include <uscl/std/__cccl/epilogue.h>

#endif // _CUDA_STD___MEMORY_RUNTIME_ASSUME_ALIGNED_H
