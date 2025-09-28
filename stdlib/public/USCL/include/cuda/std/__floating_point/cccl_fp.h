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

#ifndef _CUDA_STD___FLOATING_POINT_CCCL_FP_H
#define _CUDA_STD___FLOATING_POINT_CCCL_FP_H

#include <uscl/std/detail/__config>

#if defined(_CCCL_IMPLICIT_SYSTEM_HEADER_GCC)
#  pragma GCC system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_CLANG)
#  pragma clang system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_MSVC)
#  pragma system_header
#endif // no system header

#include <uscl/std/__concepts/concept_macros.h>
#include <uscl/std/__floating_point/arithmetic.h>
#include <uscl/std/__floating_point/conversion_rank_order.h>
#include <uscl/std/__floating_point/format.h>
#include <uscl/std/__floating_point/storage.h>
#include <uscl/std/__floating_point/traits.h>
#include <uscl/std/__type_traits/is_integral.h>

#include <uscl/std/__cccl/prologue.h>

_CCCL_BEGIN_NAMESPACE_CUDA_STD

template <__fp_format _Fmt>
class __cccl_fp
{
  static_assert(_Fmt != __fp_format::__invalid);

  using __storage_type = __fp_storage_t<_Fmt>;

  __storage_type __storage_;

public:
  _CCCL_HIDE_FROM_ABI constexpr __cccl_fp() noexcept = default;

  _CCCL_TEMPLATE(class _Tp)
  _CCCL_REQUIRES(__is_fp_v<_Tp> _CCCL_AND __fp_is_implicit_conversion_v<_Tp, __cccl_fp>)
  _CCCL_API constexpr __cccl_fp(const _Tp&) noexcept
      : __cccl_fp{}
  {
    // todo: implement construction from a floating-point type using __fp_cast
  }

  _CCCL_TEMPLATE(class _Tp)
  _CCCL_REQUIRES(__is_fp_v<_Tp> _CCCL_AND __fp_is_explicit_conversion_v<_Tp, __cccl_fp>)
  _CCCL_API explicit constexpr __cccl_fp(const _Tp&) noexcept
      : __cccl_fp{}
  {
    // todo: implement construction from a floating-point type using __fp_cast
  }

  _CCCL_TEMPLATE(class _Tp)
  _CCCL_REQUIRES(is_integral_v<_Tp>)
  _CCCL_API constexpr __cccl_fp(const _Tp&) noexcept
      : __cccl_fp{}
  {
    // todo: implement construction from an integral type using __fp_cast
  }

  _CCCL_HIDE_FROM_ABI constexpr __cccl_fp(const __cccl_fp&) noexcept = default;

  _CCCL_HIDE_FROM_ABI constexpr __cccl_fp& operator=(const __cccl_fp&) noexcept = default;

  _CCCL_TEMPLATE(class _Tp)
  _CCCL_REQUIRES(__is_fp_v<_Tp> _CCCL_AND(!__is_ext_cccl_fp_v<_Tp>)
                   _CCCL_AND __fp_is_implicit_conversion_v<__cccl_fp, _Tp>)
  _CCCL_API constexpr operator _Tp() const noexcept
  {
    // todo: implement conversion to a floating-point type using __fp_cast
    return _Tp{};
  }

  _CCCL_TEMPLATE(class _Tp)
  _CCCL_REQUIRES(__is_fp_v<_Tp> _CCCL_AND(!__is_ext_cccl_fp_v<_Tp>)
                   _CCCL_AND __fp_is_explicit_conversion_v<__cccl_fp, _Tp>)
  _CCCL_API explicit constexpr operator _Tp() const noexcept
  {
    // todo: implement conversion to a floating-point type using __fp_cast
    return _Tp{};
  }

  _CCCL_TEMPLATE(class _Tp)
  _CCCL_REQUIRES(is_integral_v<_Tp>)
  _CCCL_API constexpr operator _Tp() const noexcept
  {
    // todo: implement conversion to an integral type using __fp_cast
    return _Tp{};
  }

  template <class _Tp>
  friend _CCCL_API constexpr _Tp __fp_from_storage(__fp_storage_of_t<_Tp> __v) noexcept;
  template <class _Tp>
  friend _CCCL_API constexpr __fp_storage_of_t<_Tp> __fp_get_storage(_Tp __v) noexcept;
};

template <__fp_format _Fmt>
[[nodiscard]] _CCCL_API constexpr __cccl_fp<_Fmt> operator+(__cccl_fp<_Fmt> __v) noexcept
{
  return __v;
}

_CCCL_TEMPLATE(__fp_format _Fmt)
_CCCL_REQUIRES(__fp_is_signed_v<_Fmt>)
[[nodiscard]] _CCCL_API constexpr __cccl_fp<_Fmt> operator-(__cccl_fp<_Fmt> __v) noexcept
{
  return ::cuda::std::__fp_neg(__v);
}

_CCCL_END_NAMESPACE_CUDA_STD

#include <uscl/std/__cccl/epilogue.h>

#endif // _CUDA_STD___FLOATING_POINT_CCCL_FP_H
