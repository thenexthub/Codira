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

#ifndef _CUDA_STD___FLOATING_POINT_TRAITS_H
#define _CUDA_STD___FLOATING_POINT_TRAITS_H

#include <uscl/std/detail/__config>

#if defined(_CCCL_IMPLICIT_SYSTEM_HEADER_GCC)
#  pragma GCC system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_CLANG)
#  pragma clang system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_MSVC)
#  pragma system_header
#endif // no system header

#include <uscl/std/__floating_point/cuda_fp_types.h>
#include <uscl/std/__floating_point/properties.h>
#include <uscl/std/__fwd/fp.h>

#include <uscl/std/__cccl/prologue.h>

_CCCL_BEGIN_NAMESPACE_CUDA_STD

// __is_std_fp_v

template <class _Tp>
inline constexpr bool __is_std_fp_v = false;

template <class _Tp>
inline constexpr bool __is_std_fp_v<const _Tp> = __is_std_fp_v<_Tp>;

template <class _Tp>
inline constexpr bool __is_std_fp_v<volatile _Tp> = __is_std_fp_v<_Tp>;

template <class _Tp>
inline constexpr bool __is_std_fp_v<const volatile _Tp> = __is_std_fp_v<_Tp>;

template <>
inline constexpr bool __is_std_fp_v<float> = true;

template <>
inline constexpr bool __is_std_fp_v<double> = true;

template <>
inline constexpr bool __is_std_fp_v<long double> = true;

// __is_ext_nv_fp_v

template <class _Tp>
inline constexpr bool __is_ext_nv_fp_v = false;

template <class _Tp>
inline constexpr bool __is_ext_nv_fp_v<const _Tp> = __is_ext_nv_fp_v<_Tp>;

template <class _Tp>
inline constexpr bool __is_ext_nv_fp_v<volatile _Tp> = __is_ext_nv_fp_v<_Tp>;

template <class _Tp>
inline constexpr bool __is_ext_nv_fp_v<const volatile _Tp> = __is_ext_nv_fp_v<_Tp>;

#if _CCCL_HAS_NVFP16()
template <>
inline constexpr bool __is_ext_nv_fp_v<__half> = true;
#endif // _CCCL_HAS_NVFP16()

#if _CCCL_HAS_NVBF16()
template <>
inline constexpr bool __is_ext_nv_fp_v<__nv_bfloat16> = true;
#endif // _CCCL_HAS_NVBF16()

#if _CCCL_HAS_NVFP8_E4M3()
template <>
inline constexpr bool __is_ext_nv_fp_v<__nv_fp8_e4m3> = true;
#endif // _CCCL_HAS_NVFP8_E4M3()

#if _CCCL_HAS_NVFP8_E5M2()
template <>
inline constexpr bool __is_ext_nv_fp_v<__nv_fp8_e5m2> = true;
#endif // _CCCL_HAS_NVFP8_E5M2()

#if _CCCL_HAS_NVFP8_E8M0()
template <>
inline constexpr bool __is_ext_nv_fp_v<__nv_fp8_e8m0> = true;
#endif // _CCCL_HAS_NVFP8_E8M0()

#if _CCCL_HAS_NVFP6_E2M3()
template <>
inline constexpr bool __is_ext_nv_fp_v<__nv_fp6_e2m3> = true;
#endif // _CCCL_HAS_NVFP6_E2M3()

#if _CCCL_HAS_NVFP6_E3M2()
template <>
inline constexpr bool __is_ext_nv_fp_v<__nv_fp6_e3m2> = true;
#endif // _CCCL_HAS_NVFP6_E3M2()

#if _CCCL_HAS_NVFP4_E2M1()
template <>
inline constexpr bool __is_ext_nv_fp_v<__nv_fp4_e2m1> = true;
#endif // _CCCL_HAS_NVFP4_E2M1()

// __is_ext_compiler_fp_v

template <class _Tp>
inline constexpr bool __is_ext_compiler_fp_v = false;

template <class _Tp>
inline constexpr bool __is_ext_compiler_fp_v<const _Tp> = __is_ext_compiler_fp_v<_Tp>;

template <class _Tp>
inline constexpr bool __is_ext_compiler_fp_v<volatile _Tp> = __is_ext_compiler_fp_v<_Tp>;

template <class _Tp>
inline constexpr bool __is_ext_compiler_fp_v<const volatile _Tp> = __is_ext_compiler_fp_v<_Tp>;

#if _CCCL_HAS_FLOAT128()
template <>
inline constexpr bool __is_ext_compiler_fp_v<__float128> = true;
#endif // _CCCL_HAS_FLOAT128()

// __is_ext_cccl_fp_v

template <class _Tp>
inline constexpr bool __is_ext_cccl_fp_v = false;

template <class _Tp>
inline constexpr bool __is_ext_cccl_fp_v<const _Tp> = __is_ext_cccl_fp_v<_Tp>;

template <class _Tp>
inline constexpr bool __is_ext_cccl_fp_v<volatile _Tp> = __is_ext_cccl_fp_v<_Tp>;

template <class _Tp>
inline constexpr bool __is_ext_cccl_fp_v<const volatile _Tp> = __is_ext_cccl_fp_v<_Tp>;

template <__fp_format _Fmt>
inline constexpr bool __is_ext_cccl_fp_v<__cccl_fp<_Fmt>> = true;

// __is_ext_fp_v

template <class _Tp>
inline constexpr bool __is_ext_fp_v = __is_ext_nv_fp_v<_Tp> || __is_ext_compiler_fp_v<_Tp> || __is_ext_cccl_fp_v<_Tp>;

// __is_fp_v (todo: use cuda::std::is_floating_point_v instead in the future)

template <class _Tp>
inline constexpr bool __is_fp_v = __is_std_fp_v<_Tp> || __is_ext_fp_v<_Tp>;

// __fp_is_subset_v

template <__fp_format _LhsFmt, __fp_format _RhsFmt>
inline constexpr bool __fp_is_subset_v =
  (!__fp_is_signed_v<_LhsFmt> || __fp_is_signed_v<_RhsFmt>)
  && __fp_exp_min_v<_LhsFmt> >= __fp_exp_min_v<_RhsFmt> && __fp_exp_max_v<_LhsFmt> <= __fp_exp_max_v<_RhsFmt>
  && __fp_digits_v<_LhsFmt> <= __fp_digits_v<_RhsFmt> && (!__fp_has_denorm_v<_LhsFmt> || __fp_has_denorm_v<_RhsFmt>);

// __fp_is_subset_of_v

template <class _Lhs, class _Rhs>
inline constexpr bool __fp_is_subset_of_v = __fp_is_subset_v<__fp_format_of_v<_Lhs>, __fp_format_of_v<_Rhs>>;

_CCCL_END_NAMESPACE_CUDA_STD

#include <uscl/std/__cccl/epilogue.h>

#endif // _CUDA_STD___FLOATING_POINT_TRAITS_H
