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

#ifndef _CUDA_STD___FLOATING_POINT_NATIVE_TYPE_H
#define _CUDA_STD___FLOATING_POINT_NATIVE_TYPE_H

#include <uscl/std/detail/__config>

#if defined(_CCCL_IMPLICIT_SYSTEM_HEADER_GCC)
#  pragma GCC system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_CLANG)
#  pragma clang system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_MSVC)
#  pragma system_header
#endif // no system header

#include <uscl/std/__floating_point/format.h>
#include <uscl/std/__floating_point/traits.h>
#include <uscl/std/__type_traits/is_void.h>
#include <uscl/std/cfloat>

#include <uscl/std/__cccl/prologue.h>

_CCCL_BEGIN_NAMESPACE_CUDA_STD

template <__fp_format _Fmt>
_CCCL_API constexpr auto __fp_native_type_impl()
{
  if constexpr (_Fmt == __fp_format::__binary32)
  {
    return float{};
  }
  else if constexpr (_Fmt == __fp_format::__binary64)
  {
    return double{};
  }
  else if constexpr (_Fmt == __fp_format::__binary128)
  {
#if _CCCL_HAS_FLOAT128()
    return __float128{0.0};
#elif _CCCL_HAS_LONG_DOUBLE() && LDBL_MIN_EXP == -16381 && LDBL_MAX_EXP == 16384 && LDBL_MANT_DIG == 113
    return (long double) {};
#else // ^^^ has native binary128 ^^^ / vvv no native binary128 vvv
    return;
#endif // ^^^ no native binary128 ^^^
  }
  else if constexpr (_Fmt == __fp_format::__fp80_x86)
  {
#if _CCCL_HAS_LONG_DOUBLE() && LDBL_MIN_EXP == -16381 && LDBL_MAX_EXP == 16384 && LDBL_MANT_DIG == 64
    return (long double) {};
#else // ^^^ has native x86 fp80 ^^^ / vvv no native x86 fp80 vvv
    return;
#endif // ^^^ no native x86 fp80 ^^^
  }
  else
  {
    return;
  }
}

template <__fp_format _Fmt>
using __fp_native_type_t = decltype(__fp_native_type_impl<_Fmt>());

template <__fp_format _Fmt>
inline constexpr bool __fp_has_native_type_v = !is_void_v<__fp_native_type_t<_Fmt>>;

template <class _Tp>
inline constexpr bool __fp_is_native_type_v = __is_std_fp_v<_Tp> || __is_ext_compiler_fp_v<_Tp>;

_CCCL_END_NAMESPACE_CUDA_STD

#include <uscl/std/__cccl/epilogue.h>

#endif // _CUDA_STD___FLOATING_POINT_NATIVE_TYPE_H
