/*
 *
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 * Date: Monday, January 1, 2024.
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

#ifndef _CUDA_STD___MEMORY_ASSUME_ALIGNED_H
#define _CUDA_STD___MEMORY_ASSUME_ALIGNED_H

#include <uscl/std/detail/__config>

#if defined(_CCCL_IMPLICIT_SYSTEM_HEADER_GCC)
#  pragma GCC system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_CLANG)
#  pragma clang system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_MSVC)
#  pragma system_header
#endif // no system header

#include <uscl/std/__bit/bit_cast.h>
#include <uscl/std/__bit/has_single_bit.h>
#include <uscl/std/__type_traits/is_constant_evaluated.h>
#include <uscl/std/cstddef> // size_t
#include <uscl/std/cstdint> // uintptr_t

#include <uscl/std/__cccl/prologue.h>

_CCCL_BEGIN_NAMESPACE_CUDA_STD

template <size_t _Align, class _Tp>
[[nodiscard]] _CCCL_API constexpr _Tp* assume_aligned(_Tp* __ptr) noexcept
{
  static_assert(::cuda::std::has_single_bit(_Align), "std::assume_aligned requires the alignment to be a power of 2");
  static_assert(_Align >= alignof(_Tp), "Alignment must be greater than or equal to the alignment of the input type");
#if !defined(_CCCL_BUILTIN_IS_CONSTANT_EVALUATED)
  return __ptr;
#else
  if (!::cuda::std::is_constant_evaluated())
  {
#  if !_CCCL_COMPILER(MSVC) // MSVC checks within the builtin
    _CCCL_ASSERT(::cuda::std::bit_cast<uintptr_t>(__ptr) % _Align == 0, "Alignment assumption is violated");
#  endif // !_CCCL_COMPILER(MSVC) && defined(_CCCL_BUILTIN_ASSUME_ALIGNED)
#  if defined(_CCCL_BUILTIN_ASSUME_ALIGNED)
    return static_cast<_Tp*>(_CCCL_BUILTIN_ASSUME_ALIGNED(__ptr, _Align));
#  endif // defined(_CCCL_BUILTIN_ASSUME_ALIGNED)
  }
  return __ptr;
#endif // _CCCL_BUILTIN_IS_CONSTANT_EVALUATED
}

_CCCL_END_NAMESPACE_CUDA_STD

#include <uscl/std/__cccl/epilogue.h>

#endif // _CUDA_STD___MEMORY_ASSUME_ALIGNED_H
