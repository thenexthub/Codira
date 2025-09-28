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

#ifndef _CUDA_STD___COMPLEX_HYPERBOLIC_FUNCTIONS_H
#define _CUDA_STD___COMPLEX_HYPERBOLIC_FUNCTIONS_H

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
#include <uscl/std/__cmath/hyperbolic_functions.h>
#include <uscl/std/__cmath/isfinite.h>
#include <uscl/std/__cmath/isinf.h>
#include <uscl/std/__cmath/isnan.h>
#include <uscl/std/__cmath/trigonometric_functions.h>
#include <uscl/std/__complex/complex.h>
#include <uscl/std/limits>

#include <uscl/std/__cccl/prologue.h>

_CCCL_BEGIN_NAMESPACE_CUDA_STD

// sinh

template <class _Tp>
[[nodiscard]] _CCCL_API inline complex<_Tp> sinh(const complex<_Tp>& __x)
{
  if (::cuda::std::isinf(__x.real()) && !::cuda::std::isfinite(__x.imag()))
  {
    return complex<_Tp>(__x.real(), numeric_limits<_Tp>::quiet_NaN());
  }
  if (__x.real() == _Tp(0) && !::cuda::std::isfinite(__x.imag()))
  {
    return complex<_Tp>(__x.real(), numeric_limits<_Tp>::quiet_NaN());
  }
  if (__x.imag() == _Tp(0) && !::cuda::std::isfinite(__x.real()))
  {
    return __x;
  }
  return complex<_Tp>(::cuda::std::sinh(__x.real()) * ::cuda::std::cos(__x.imag()),
                      ::cuda::std::cosh(__x.real()) * ::cuda::std::sin(__x.imag()));
}

// cosh

template <class _Tp>
[[nodiscard]] _CCCL_API inline complex<_Tp> cosh(const complex<_Tp>& __x)
{
  if (::cuda::std::isinf(__x.real()) && !::cuda::std::isfinite(__x.imag()))
  {
    return complex<_Tp>(::cuda::std::abs(__x.real()), numeric_limits<_Tp>::quiet_NaN());
  }
  if (__x.real() == _Tp(0) && !::cuda::std::isfinite(__x.imag()))
  {
    return complex<_Tp>(numeric_limits<_Tp>::quiet_NaN(), __x.real());
  }
  if (__x.real() == _Tp(0) && __x.imag() == _Tp(0))
  {
    return complex<_Tp>(_Tp(1), __x.imag());
  }
  if (__x.imag() == _Tp(0) && !::cuda::std::isfinite(__x.real()))
  {
    return complex<_Tp>(::cuda::std::abs(__x.real()), __x.imag());
  }
  return complex<_Tp>(::cuda::std::cosh(__x.real()) * ::cuda::std::cos(__x.imag()),
                      ::cuda::std::sinh(__x.real()) * ::cuda::std::sin(__x.imag()));
}

// tanh

template <class _Tp>
[[nodiscard]] _CCCL_API inline complex<_Tp> tanh(const complex<_Tp>& __x)
{
  if (::cuda::std::isinf(__x.real()))
  {
    if (!::cuda::std::isfinite(__x.imag()))
    {
      return complex<_Tp>(::cuda::std::copysign(_Tp(1), __x.real()), _Tp(0));
    }
    return complex<_Tp>(::cuda::std::copysign(_Tp(1), __x.real()),
                        ::cuda::std::copysign(_Tp(0), ::cuda::std::sin(_Tp(2) * __x.imag())));
  }
  if (::cuda::std::isnan(__x.real()) && __x.imag() == _Tp(0))
  {
    return __x;
  }
  _Tp __2r(_Tp(2) * __x.real());
  _Tp __2i(_Tp(2) * __x.imag());
  _Tp __d(::cuda::std::cosh(__2r) + ::cuda::std::cos(__2i));
  _Tp __2rsh(::cuda::std::sinh(__2r));
  if (::cuda::std::isinf(__2rsh) && ::cuda::std::isinf(__d))
  {
    return complex<_Tp>(__2rsh > _Tp(0) ? _Tp(1) : _Tp(-1), __2i > _Tp(0) ? _Tp(0) : _Tp(-0.));
  }
  return complex<_Tp>(__2rsh / __d, ::cuda::std::sin(__2i) / __d);
}

_CCCL_END_NAMESPACE_CUDA_STD

#include <uscl/std/__cccl/epilogue.h>

#endif // _CUDA_STD___COMPLEX_HYPERBOLIC_FUNCTIONS_H
