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

#ifndef __CCCL_IS_NON_NARROWING_CONVERTIBLE_H
#define __CCCL_IS_NON_NARROWING_CONVERTIBLE_H

#include <uscl/std/__cccl/compiler.h>

//! There is compiler bug that results in incorrect results for the below `__is_non_narrowing_convertible` check.
//! This breaks some common functionality, so this *must* be included outside of a system header. See nvbug4867473.
#if defined(_CCCL_FORCE_SYSTEM_HEADER_GCC) || defined(_CCCL_FORCE_SYSTEM_HEADER_CLANG) \
  || defined(_CCCL_FORCE_SYSTEM_HEADER_MSVC)
#  error \
    "This header must be included only within the <cuda/std/__cccl/system_header>. This most likely means a mix and match of different versions of CCCL."
#endif // system header detected

namespace __cccl_internal
{

#if _CCCL_CUDA_COMPILATION()
template <class _Tp>
__host__ __device__ _Tp&& __cccl_declval(int);
template <class _Tp>
__host__ __device__ _Tp __cccl_declval(long);
template <class _Tp>
__host__ __device__ decltype(__cccl_internal::__cccl_declval<_Tp>(0)) __cccl_declval() noexcept;

// This requires a type to be implicitly convertible (also non-arithmetic)
template <class _Tp>
__host__ __device__ void __cccl_accepts_implicit_conversion(_Tp) noexcept;
#else // ^^^ CUDA compilation ^^^ / vvv no CUDA compilation
template <class _Tp>
_Tp&& __cccl_declval(int);
template <class _Tp>
_Tp __cccl_declval(long);
template <class _Tp>
decltype(__cccl_internal::__cccl_declval<_Tp>(0)) __cccl_declval() noexcept;

// This requires a type to be implicitly convertible (also non-arithmetic)
template <class _Tp>
void __cccl_accepts_implicit_conversion(_Tp) noexcept;
#endif // no CUDA compilation

template <class...>
using __cccl_void_t = void;

template <class _Dest, class _Source, class = void>
struct __is_non_narrowing_convertible
{
  static constexpr bool value = false;
};

// This also prohibits narrowing conversion in case of arithmetic types
template <class _Dest, class _Source>
struct __is_non_narrowing_convertible<_Dest,
                                      _Source,
                                      __cccl_void_t<decltype(__cccl_internal::__cccl_accepts_implicit_conversion<_Dest>(
                                                      __cccl_internal::__cccl_declval<_Source>())),
                                                    decltype(_Dest{__cccl_internal::__cccl_declval<_Source>()})>>
{
  static constexpr bool value = true;
};

} // namespace __cccl_internal

#endif // __CCCL_IS_NON_NARROWING_CONVERTIBLE_H
