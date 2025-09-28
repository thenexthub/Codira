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

#ifndef _CUDA_STD___COMPLEX_INVERSE_HYPERBOLIC_FUNCTIONS_H
#define _CUDA_STD___COMPLEX_INVERSE_HYPERBOLIC_FUNCTIONS_H

#include <uscl/std/detail/__config>

#if defined(_CCCL_IMPLICIT_SYSTEM_HEADER_GCC)
#  pragma GCC system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_CLANG)
#  pragma clang system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_MSVC)
#  pragma system_header
#endif // no system header

#include <uscl/std/__cmath/abs.h>
#include <uscl/std/__cmath/copysign.h>
#include <uscl/std/__cmath/isinf.h>
#include <uscl/std/__cmath/isnan.h>
#include <uscl/std/__cmath/trigonometric_functions.h>
#include <uscl/std/__complex/complex.h>
#include <uscl/std/__complex/exponential_functions.h>
#include <uscl/std/__complex/logarithms.h>
#include <uscl/std/__complex/nvbf16.h>
#include <uscl/std/__complex/nvfp16.h>
#include <uscl/std/__complex/roots.h>
#include <uscl/std/limits>
#include <uscl/std/numbers>

#include <uscl/std/__cccl/prologue.h>

_CCCL_BEGIN_NAMESPACE_CUDA_STD

// asinh

template <class _Tp>
[[nodiscard]] _CCCL_API inline complex<_Tp> asinh(const complex<_Tp>& __x)
{
  constexpr _Tp __pi = __numbers<_Tp>::__pi();
  if (::cuda::std::isinf(__x.real()))
  {
    if (::cuda::std::isnan(__x.imag()))
    {
      return __x;
    }
    if (::cuda::std::isinf(__x.imag()))
    {
      return complex<_Tp>(__x.real(), ::cuda::std::copysign(__pi * _Tp(0.25), __x.imag()));
    }
    return complex<_Tp>(__x.real(), ::cuda::std::copysign(_Tp(0), __x.imag()));
  }
  if (::cuda::std::isnan(__x.real()))
  {
    if (::cuda::std::isinf(__x.imag()))
    {
      return complex<_Tp>(__x.imag(), __x.real());
    }
    if (__x.imag() == _Tp(0))
    {
      return __x;
    }
    return complex<_Tp>(__x.real(), __x.real());
  }
  if (::cuda::std::isinf(__x.imag()))
  {
    return complex<_Tp>(::cuda::std::copysign(__x.imag(), __x.real()),
                        ::cuda::std::copysign(__pi / _Tp(2), __x.imag()));
  }
  complex<_Tp> __z = ::cuda::std::log(__x + ::cuda::std::sqrt(::cuda::std::__sqr(__x) + _Tp(1)));
  return complex<_Tp>(::cuda::std::copysign(__z.real(), __x.real()), ::cuda::std::copysign(__z.imag(), __x.imag()));
}

// We have performance issues with some trigonometric functions with extended floating point types
#if _LIBCUDACXX_HAS_NVBF16()
template <>
_CCCL_API inline complex<__nv_bfloat16> asinh(const complex<__nv_bfloat16>& __x)
{
  return complex<__nv_bfloat16>{::cuda::std::asinh(complex<float>{__x})};
}
#endif // _LIBCUDACXX_HAS_NVBF16()

#if _LIBCUDACXX_HAS_NVFP16()
template <>
_CCCL_API inline complex<__half> asinh(const complex<__half>& __x)
{
  return complex<__half>{::cuda::std::asinh(complex<float>{__x})};
}
#endif // _LIBCUDACXX_HAS_NVFP16()

// acosh

template <class _Tp>
[[nodiscard]] _CCCL_API inline complex<_Tp> acosh(const complex<_Tp>& __x)
{
  constexpr _Tp __pi = __numbers<_Tp>::__pi();
  if (::cuda::std::isinf(__x.real()))
  {
    if (::cuda::std::isnan(__x.imag()))
    {
      return complex<_Tp>(::cuda::std::abs(__x.real()), __x.imag());
    }
    if (::cuda::std::isinf(__x.imag()))
    {
      if (__x.real() > _Tp(0))
      {
        return complex<_Tp>(__x.real(), ::cuda::std::copysign(__pi * _Tp(0.25), __x.imag()));
      }
      else
      {
        return complex<_Tp>(-__x.real(), ::cuda::std::copysign(__pi * _Tp(0.75), __x.imag()));
      }
    }
    if (__x.real() < _Tp(0))
    {
      return complex<_Tp>(-__x.real(), ::cuda::std::copysign(__pi, __x.imag()));
    }
    return complex<_Tp>(__x.real(), ::cuda::std::copysign(_Tp(0), __x.imag()));
  }
  if (::cuda::std::isnan(__x.real()))
  {
    if (::cuda::std::isinf(__x.imag()))
    {
      return complex<_Tp>(::cuda::std::abs(__x.imag()), __x.real());
    }
    return complex<_Tp>(__x.real(), __x.real());
  }
  if (::cuda::std::isinf(__x.imag()))
  {
    return complex<_Tp>(::cuda::std::abs(__x.imag()), ::cuda::std::copysign(__pi / _Tp(2), __x.imag()));
  }
  complex<_Tp> __z = ::cuda::std::log(__x + ::cuda::std::sqrt(::cuda::std::__sqr(__x) - _Tp(1)));
  return complex<_Tp>(::cuda::std::copysign(__z.real(), _Tp(0)), ::cuda::std::copysign(__z.imag(), __x.imag()));
}

// We have performance issues with some trigonometric functions with extended floating point types
#if _LIBCUDACXX_HAS_NVBF16()
template <>
_CCCL_API inline complex<__nv_bfloat16> acosh(const complex<__nv_bfloat16>& __x)
{
  return complex<__nv_bfloat16>{::cuda::std::acosh(complex<float>{__x})};
}
#endif // _LIBCUDACXX_HAS_NVBF16()

#if _LIBCUDACXX_HAS_NVFP16()
template <>
_CCCL_API inline complex<__half> acosh(const complex<__half>& __x)
{
  return complex<__half>{::cuda::std::acosh(complex<float>{__x})};
}
#endif // _LIBCUDACXX_HAS_NVFP16()

// atanh

template <class _Tp>
[[nodiscard]] _CCCL_API inline complex<_Tp> atanh(const complex<_Tp>& __x)
{
  constexpr _Tp __pi = __numbers<_Tp>::__pi();
  if (::cuda::std::isinf(__x.imag()))
  {
    return complex<_Tp>(::cuda::std::copysign(_Tp(0), __x.real()), ::cuda::std::copysign(__pi / _Tp(2), __x.imag()));
  }
  if (::cuda::std::isnan(__x.imag()))
  {
    if (::cuda::std::isinf(__x.real()) || __x.real() == _Tp(0))
    {
      return complex<_Tp>(::cuda::std::copysign(_Tp(0), __x.real()), __x.imag());
    }
    return complex<_Tp>(__x.imag(), __x.imag());
  }
  if (::cuda::std::isnan(__x.real()))
  {
    return complex<_Tp>(__x.real(), __x.real());
  }
  if (::cuda::std::isinf(__x.real()))
  {
    return complex<_Tp>(::cuda::std::copysign(_Tp(0), __x.real()), ::cuda::std::copysign(__pi / _Tp(2), __x.imag()));
  }
  if (::cuda::std::abs(__x.real()) == _Tp(1) && __x.imag() == _Tp(0))
  {
    return complex<_Tp>(::cuda::std::copysign(numeric_limits<_Tp>::infinity(), __x.real()),
                        ::cuda::std::copysign(_Tp(0), __x.imag()));
  }
  complex<_Tp> __z = ::cuda::std::log((_Tp(1) + __x) / (_Tp(1) - __x)) / _Tp(2);
  return complex<_Tp>(::cuda::std::copysign(__z.real(), __x.real()), ::cuda::std::copysign(__z.imag(), __x.imag()));
}

// We have performance issues with some trigonometric functions with extended floating point types
#if _LIBCUDACXX_HAS_NVBF16()
template <>
_CCCL_API inline complex<__nv_bfloat16> atanh(const complex<__nv_bfloat16>& __x)
{
  return complex<__nv_bfloat16>{::cuda::std::atanh(complex<float>{__x})};
}
#endif // _LIBCUDACXX_HAS_NVBF16()

#if _LIBCUDACXX_HAS_NVFP16()
template <>
_CCCL_API inline complex<__half> atanh(const complex<__half>& __x)
{
  return complex<__half>{::cuda::std::atanh(complex<float>{__x})};
}
#endif // _LIBCUDACXX_HAS_NVFP16()

_CCCL_END_NAMESPACE_CUDA_STD

#include <uscl/std/__cccl/epilogue.h>

#endif // _CUDA_STD___COMPLEX_INVERSE_HYPERBOLIC_FUNCTIONS_H
