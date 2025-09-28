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

#ifndef _CUDA_STD___NUMERIC_GCD_LCM_H
#define _CUDA_STD___NUMERIC_GCD_LCM_H

#include <uscl/std/detail/__config>

#if defined(_CCCL_IMPLICIT_SYSTEM_HEADER_GCC)
#  pragma GCC system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_CLANG)
#  pragma clang system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_MSVC)
#  pragma system_header
#endif // no system header

#include <uscl/__cmath/uabs.h>
#include <uscl/std/__type_traits/common_type.h>
#include <uscl/std/__type_traits/is_integral.h>
#include <uscl/std/__type_traits/is_same.h>
#include <uscl/std/__type_traits/is_signed.h>
#include <uscl/std/__type_traits/make_unsigned.h>
#include <uscl/std/limits>

#include <uscl/std/__cccl/prologue.h>

_CCCL_BEGIN_NAMESPACE_CUDA_STD

template <class _Tp>
constexpr _CCCL_API inline _Tp __gcd(_Tp __m, _Tp __n)
{
  static_assert((!is_signed_v<_Tp>), "");
  return __n == 0 ? __m : ::cuda::std::__gcd<_Tp>(__n, __m % __n);
}

template <class _Tp, class _Up>
constexpr _CCCL_API inline common_type_t<_Tp, _Up> gcd(_Tp __m, _Up __n)
{
  static_assert((is_integral_v<_Tp> && is_integral_v<_Up>), "Arguments to gcd must be integer types");
  static_assert((!is_same_v<remove_cv_t<_Tp>, bool>), "First argument to gcd cannot be bool");
  static_assert((!is_same_v<remove_cv_t<_Up>, bool>), "Second argument to gcd cannot be bool");
  using _Rp = common_type_t<_Tp, _Up>;
  using _Wp = make_unsigned_t<_Rp>;
  return static_cast<_Rp>(::cuda::std::__gcd(static_cast<_Wp>(::cuda::uabs(__m)), static_cast<_Wp>(::cuda::uabs(__n))));
}

template <class _Tp, class _Up>
constexpr _CCCL_API inline common_type_t<_Tp, _Up> lcm(_Tp __m, _Up __n)
{
  static_assert((is_integral_v<_Tp> && is_integral_v<_Up>), "Arguments to lcm must be integer types");
  static_assert((!is_same_v<remove_cv_t<_Tp>, bool>), "First argument to lcm cannot be bool");
  static_assert((!is_same_v<remove_cv_t<_Up>, bool>), "Second argument to lcm cannot be bool");
  if (__m == 0 || __n == 0)
  {
    return 0;
  }

  using _Rp         = common_type_t<_Tp, _Up>;
  using _Wp         = make_unsigned_t<_Rp>;
  const auto __val1 = ::cuda::uabs(__m) / ::cuda::std::gcd(__m, __n);
  const auto __val2 = ::cuda::uabs(__n);
  _CCCL_ASSERT((static_cast<_Wp>(numeric_limits<_Rp>::max()) / __val1 > __val2), "Overflow in lcm");
  return static_cast<_Rp>(__val1 * __val2);
}

_CCCL_END_NAMESPACE_CUDA_STD

#include <uscl/std/__cccl/epilogue.h>

#endif // _CUDA_STD___NUMERIC_GCD_LCM_H
