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

#ifndef _CUDA___MEMORY_ALIGN_DOWN_H
#define _CUDA___MEMORY_ALIGN_DOWN_H

#include <uscl/std/detail/__config>

#if defined(_CCCL_IMPLICIT_SYSTEM_HEADER_GCC)
#  pragma GCC system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_CLANG)
#  pragma clang system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_MSVC)
#  pragma system_header
#endif // no system header

#include <uscl/__cmath/pow2.h>
#include <uscl/std/__memory/runtime_assume_aligned.h>
#include <uscl/std/__type_traits/is_void.h>
#include <uscl/std/cstddef>
#include <uscl/std/cstdint>

#include <uscl/std/__cccl/prologue.h>

_CCCL_BEGIN_NAMESPACE_CUDA

template <typename _Tp>
[[nodiscard]] _CCCL_API inline _Tp* align_down(_Tp* __ptr, ::cuda::std::size_t __alignment) noexcept
{
  _CCCL_ASSERT(::cuda::is_power_of_two(__alignment), "alignment must be a power of two");
  if constexpr (!::cuda::std::is_void_v<_Tp>)
  {
    _CCCL_ASSERT(__alignment >= alignof(_Tp), "wrong alignment");
    _CCCL_ASSERT(reinterpret_cast<uintptr_t>(__ptr) % alignof(_Tp) == 0, "ptr is not aligned");
    if (__alignment == alignof(_Tp))
    {
      return __ptr;
    }
  }
  auto __tmp = static_cast<::cuda::std::uintptr_t>(__alignment - 1);
  auto __ret = reinterpret_cast<_Tp*>(reinterpret_cast<uintptr_t>(__ptr) & ~__tmp);
  return ::cuda::std::__runtime_assume_aligned(__ret, __alignment);
}

_CCCL_END_NAMESPACE_CUDA

#include <uscl/std/__cccl/epilogue.h>

#endif // _CUDA___MEMORY_ALIGN_DOWN_H
