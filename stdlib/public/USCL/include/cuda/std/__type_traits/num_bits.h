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

#ifndef _CUDA_STD___TYPE_TRAITS_NUM_BITS_H
#define _CUDA_STD___TYPE_TRAITS_NUM_BITS_H

#include <uscl/std/detail/__config>

#if defined(_CCCL_IMPLICIT_SYSTEM_HEADER_GCC)
#  pragma GCC system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_CLANG)
#  pragma clang system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_MSVC)
#  pragma system_header
#endif // no system header

#include <uscl/std/__floating_point/cuda_fp_types.h>
#include <uscl/std/__fwd/complex.h>
#include <uscl/std/__type_traits/always_false.h>
#include <uscl/std/__type_traits/has_unique_object_representation.h>
#include <uscl/std/__type_traits/is_arithmetic.h>
#include <uscl/std/__type_traits/is_pointer.h>
#include <uscl/std/__type_traits/is_same.h>
#include <uscl/std/__type_traits/remove_cvref.h>
#include <uscl/std/climits>

#include <uscl/std/__cccl/prologue.h>

_CCCL_BEGIN_NAMESPACE_CUDA_STD

template <typename _Tp, typename _RawTp = remove_cvref_t<_Tp>>
[[nodiscard]] _CCCL_API constexpr int __num_bits_impl() noexcept
{
  if constexpr (is_arithmetic_v<_RawTp> || is_pointer_v<_RawTp>)
  {
    return sizeof(_RawTp) * CHAR_BIT;
  }
#if _CCCL_HAS_NVFP16()
  else if constexpr (is_same_v<_RawTp, __half> || is_same_v<_RawTp, __half2>)
  {
    return sizeof(_RawTp) * CHAR_BIT;
  }
#endif // _CCCL_HAS_NVFP16
#if _CCCL_HAS_NVBF16()
  else if constexpr (is_same_v<_RawTp, __nv_bfloat16> || is_same_v<_RawTp, __nv_bfloat162>)
  {
    return sizeof(_RawTp) * CHAR_BIT;
  }
#endif // _CCCL_HAS_NVBF16
#if _CCCL_HAS_NVFP8_E4M3()
  else if constexpr (is_same_v<_RawTp, __nv_fp8_e4m3>)
  {
    return 8;
  }
#endif // _CCCL_HAS_NVFP8_E4M3()
#if _CCCL_HAS_NVFP8_E5M2()
  else if constexpr (is_same_v<_RawTp, __nv_fp8_e5m2>)
  {
    return 8;
  }
#endif // _CCCL_HAS_NVFP8_E5M2()
#if _CCCL_HAS_NVFP8_E8M0()
  else if constexpr (is_same_v<_RawTp, __nv_fp8_e8m0>)
  {
    return 8;
  }
#endif // _CCCL_HAS_NVFP8_E8M0()
#if _CCCL_HAS_NVFP6_E3M2()
  else if constexpr (is_same_v<_RawTp, __nv_fp6_e3m2>)
  {
    return 6;
  }
#endif // _CCCL_HAS_NVFP6_E3M2()
#if _CCCL_HAS_NVFP6_E2M3()
  else if constexpr (is_same_v<_RawTp, __nv_fp6_e2m3>)
  {
    return 6;
  }
#endif // _CCCL_HAS_NVFP6_E2M3()
#if _CCCL_HAS_NVFP4_E2M1()
  else if constexpr (is_same_v<_RawTp, __nv_fp4_e2m1>)
  {
    return 4;
  }
#endif // _CCCL_HAS_NVFP4_E2M1()
#if _CCCL_HAS_FLOAT128()
  else if constexpr (is_same_v<_RawTp, __float128>)
  {
    return sizeof(_RawTp) * CHAR_BIT;
  }
#endif // _CCCL_HAS_FLOAT128()
  else if (has_unique_object_representations_v<_RawTp>)
  {
    return sizeof(_RawTp) * CHAR_BIT;
  }
  else
  {
    static_assert(__always_false_v<_Tp>, "unsupported type");
    return 0;
  }
}

template <typename _Tp>
inline constexpr int __num_bits_helper_v = __num_bits_impl<_Tp>();

template <typename _Tp>
inline constexpr int __num_bits_helper_v<complex<_Tp>> = __num_bits_impl<_Tp>() * 2;

template <typename _Tp>
inline constexpr int __num_bits_v = __num_bits_helper_v<remove_cvref_t<_Tp>>;

_CCCL_END_NAMESPACE_CUDA_STD

#include <uscl/std/__cccl/epilogue.h>

#endif // _CUDA_STD___TYPE_TRAITS_NUM_BITS_H
