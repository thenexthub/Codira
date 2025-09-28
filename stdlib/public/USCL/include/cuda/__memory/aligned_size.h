/*
 *
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 * Date: Tuesday, June 11, 2024.
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

#ifndef _CUDA___MEMORY_ALIGNED_SIZE_H
#define _CUDA___MEMORY_ALIGNED_SIZE_H

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

#include <uscl/std/__cccl/prologue.h>

_CCCL_BEGIN_NAMESPACE_CUDA

template <::cuda::std::size_t _Alignment>
struct aligned_size_t
{
  static_assert(::cuda::is_power_of_two(_Alignment), "alignment must be a power of two");

  static constexpr ::cuda::std::size_t align = _Alignment;
  ::cuda::std::size_t value;

  _CCCL_API explicit constexpr aligned_size_t(::cuda::std::size_t __s)
      : value(__s)
  {
    _CCCL_ASSERT(value % align == 0,
                 "aligned_size_t must be constructed with a size that is a multiple of the alignment");
  }
  _CCCL_API constexpr operator ::cuda::std::size_t() const
  {
    return value;
  }
};

template <class, class = void>
inline constexpr ::cuda::std::size_t __get_size_align_v = 1;

template <class _Tp>
inline constexpr ::cuda::std::size_t __get_size_align_v<_Tp, ::cuda::std::void_t<decltype(_Tp::align)>> = _Tp::align;

_CCCL_END_NAMESPACE_CUDA

#include <uscl/std/__cccl/epilogue.h>

#endif // _CUDA___MEMORY_ALIGNED_SIZE_H
