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

#ifndef _CUDA_STD___FLOATING_POINT_STORAGE_H
#define _CUDA_STD___FLOATING_POINT_STORAGE_H

#include <uscl/std/detail/__config>

#if defined(_CCCL_IMPLICIT_SYSTEM_HEADER_GCC)
#  pragma GCC system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_CLANG)
#  pragma clang system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_MSVC)
#  pragma system_header
#endif // no system header

#include <uscl/std/__bit/bit_cast.h>
#include <uscl/std/__floating_point/cuda_fp_types.h>
#include <uscl/std/__floating_point/format.h>
#include <uscl/std/__floating_point/traits.h>
#include <uscl/std/__type_traits/always_false.h>
#include <uscl/std/__type_traits/is_same.h>
#include <uscl/std/cstdint>

#include <uscl/std/__cccl/prologue.h>

_CCCL_BEGIN_NAMESPACE_CUDA_STD

template <__fp_format _Fmt>
[[nodiscard]] _CCCL_API constexpr auto __fp_storage_type_impl() noexcept
{
  if constexpr (_Fmt == __fp_format::__fp8_nv_e4m3 || _Fmt == __fp_format::__fp8_nv_e5m2
                || _Fmt == __fp_format::__fp8_nv_e8m0 || _Fmt == __fp_format::__fp6_nv_e2m3
                || _Fmt == __fp_format::__fp6_nv_e3m2 || _Fmt == __fp_format::__fp4_nv_e2m1)
  {
    return uint8_t{};
  }
  else if constexpr (_Fmt == __fp_format::__binary16 || _Fmt == __fp_format::__bfloat16)
  {
    return uint16_t{};
  }
  else if constexpr (_Fmt == __fp_format::__binary32)
  {
    return uint32_t{};
  }
  else if constexpr (_Fmt == __fp_format::__binary64)
  {
    return uint64_t{};
  }
#if _CCCL_HAS_INT128()
  else if constexpr (_Fmt == __fp_format::__fp80_x86 || _Fmt == __fp_format::__binary128)
  {
    return __uint128_t{};
  }
#endif // _CCCL_HAS_INT128()
  else
  {
    static_assert(__always_false_v<decltype(_Fmt)>, "Unsupported floating point format");
  }
}

template <__fp_format _Fmt>
using __fp_storage_t = decltype(__fp_storage_type_impl<_Fmt>());

template <class _Tp>
using __fp_storage_of_t = __fp_storage_t<__fp_format_of_v<_Tp>>;

#if _CCCL_HAS_NVFP16()
struct __cccl_nvfp16_manip_helper : __half
{
  using __half::__x;
};
#endif // _CCCL_HAS_NVFP16()

#if _CCCL_HAS_NVBF16()
struct __cccl_nvbf16_manip_helper : __nv_bfloat16
{
  using __nv_bfloat16::__x;
};
#endif // _CCCL_HAS_NVBF16()

template <class _Tp>
[[nodiscard]] _CCCL_API constexpr _Tp __fp_from_storage(__fp_storage_of_t<_Tp> __v) noexcept
{
  if constexpr (__is_std_fp_v<_Tp> || __is_ext_compiler_fp_v<_Tp>)
  {
    return ::cuda::std::bit_cast<_Tp>(__v);
  }
  else if constexpr (__is_ext_cccl_fp_v<_Tp>)
  {
    _Tp __ret{};
    __ret.__storage_ = __v;
    return __ret;
  }
#if _CCCL_HAS_NVFP16()
  else if constexpr (is_same_v<_Tp, __half>)
  {
    __cccl_nvfp16_manip_helper __helper{};
    __helper.__x = __v;
    return __helper;
  }
#endif // _CCCL_HAS_NVFP16()
#if _CCCL_HAS_NVBF16()
  else if constexpr (is_same_v<_Tp, __nv_bfloat16>)
  {
    __cccl_nvbf16_manip_helper __helper{};
    __helper.__x = __v;
    return __helper;
  }
#endif // _CCCL_HAS_NVBF16()
#if _CCCL_HAS_NVFP8_E4M3()
  else if constexpr (is_same_v<_Tp, __nv_fp8_e4m3>)
  {
    __nv_fp8_e4m3 __ret{};
    __ret.__x = __v;
    return __ret;
  }
#endif // _CCCL_HAS_NVFP8_E4M3()
#if _CCCL_HAS_NVFP8_E5M2()
  else if constexpr (is_same_v<_Tp, __nv_fp8_e5m2>)
  {
    __nv_fp8_e5m2 __ret{};
    __ret.__x = __v;
    return __ret;
  }
#endif // _CCCL_HAS_NVFP8_E5M2()
#if _CCCL_HAS_NVFP8_E8M0()
  else if constexpr (is_same_v<_Tp, __nv_fp8_e8m0>)
  {
    __nv_fp8_e8m0 __ret{};
    __ret.__x = __v;
    return __ret;
  }
#endif // _CCCL_HAS_NVFP8_E8M0()
#if _CCCL_HAS_NVFP6_E2M3()
  else if constexpr (is_same_v<_Tp, __nv_fp6_e2m3>)
  {
    _CCCL_ASSERT((__v & 0xc0u) == 0u, "Invalid __nv_fp6_e2m3 storage value");
    __nv_fp6_e2m3 __ret{};
    __ret.__x = __v;
    return __ret;
  }
#endif // _CCCL_HAS_NVFP6_E2M3()
#if _CCCL_HAS_NVFP6_E3M2()
  else if constexpr (is_same_v<_Tp, __nv_fp6_e3m2>)
  {
    _CCCL_ASSERT((__v & 0xc0u) == 0u, "Invalid __nv_fp6_e3m2 storage value");
    __nv_fp6_e3m2 __ret{};
    __ret.__x = __v;
    return __ret;
  }
#endif // _CCCL_HAS_NVFP6_E3M2()
#if _CCCL_HAS_NVFP4_E2M1()
  else if constexpr (is_same_v<_Tp, __nv_fp4_e2m1>)
  {
    _CCCL_ASSERT((__v & 0xf0u) == 0u, "Invalid __nv_fp4_e2m1 storage value");
    __nv_fp4_e2m1 __ret{};
    __ret.__x = __v;
    return __ret;
  }
#endif // _CCCL_HAS_NVFP4_E2M1()
  else
  {
    static_assert(__always_false_v<_Tp>, "Unsupported floating point format");
  }
}

_CCCL_TEMPLATE(class _Tp, class _Up)
_CCCL_REQUIRES((!is_same_v<_Up, __fp_storage_of_t<_Tp>>) )
_CCCL_API constexpr _Tp __fp_from_storage(const _Up& __v) noexcept = delete;

template <class _Tp>
[[nodiscard]] _CCCL_API constexpr __fp_storage_of_t<_Tp> __fp_get_storage(_Tp __v) noexcept
{
  if constexpr (__is_std_fp_v<_Tp> || __is_ext_compiler_fp_v<_Tp>)
  {
    return ::cuda::std::bit_cast<__fp_storage_of_t<_Tp>>(__v);
  }
  else if constexpr (__is_ext_cccl_fp_v<_Tp>)
  {
    return __v.__storage_;
  }
#if _CCCL_HAS_NVFP16()
  else if constexpr (is_same_v<_Tp, __half>)
  {
    return __cccl_nvfp16_manip_helper{__v}.__x;
  }
#endif // _CCCL_HAS_NVFP16()
#if _CCCL_HAS_NVBF16()
  else if constexpr (is_same_v<_Tp, __nv_bfloat16>)
  {
    return __cccl_nvbf16_manip_helper{__v}.__x;
  }
#endif // _CCCL_HAS_NVBF16()
#if _CCCL_HAS_NVFP8_E4M3()
  else if constexpr (is_same_v<_Tp, __nv_fp8_e4m3>)
  {
    return __v.__x;
  }
#endif // _CCCL_HAS_NVFP8_E4M3()
#if _CCCL_HAS_NVFP8_E5M2()
  else if constexpr (is_same_v<_Tp, __nv_fp8_e5m2>)
  {
    return __v.__x;
  }
#endif // _CCCL_HAS_NVFP8_E5M2()
#if _CCCL_HAS_NVFP8_E8M0()
  else if constexpr (is_same_v<_Tp, __nv_fp8_e8m0>)
  {
    return __v.__x;
  }
#endif // _CCCL_HAS_NVFP8_E8M0()
#if _CCCL_HAS_NVFP6_E2M3()
  else if constexpr (is_same_v<_Tp, __nv_fp6_e2m3>)
  {
    return __v.__x;
  }
#endif // _CCCL_HAS_NVFP6_E2M3()
#if _CCCL_HAS_NVFP6_E3M2()
  else if constexpr (is_same_v<_Tp, __nv_fp6_e3m2>)
  {
    return __v.__x;
  }
#endif // _CCCL_HAS_NVFP6_E3M2()
#if _CCCL_HAS_NVFP4_E2M1()
  else if constexpr (is_same_v<_Tp, __nv_fp4_e2m1>)
  {
    return __v.__x;
  }
#endif // _CCCL_HAS_NVFP4_E2M1()
  else
  {
    static_assert(__always_false_v<_Tp>, "Unsupported floating point format");
  }
}

_CCCL_END_NAMESPACE_CUDA_STD

#include <uscl/std/__cccl/epilogue.h>

#endif // _CUDA_STD___FLOATING_POINT_STORAGE_H
