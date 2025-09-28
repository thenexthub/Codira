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

#ifndef _CUDA_STD___COMPLEX_LITERALS_H
#define _CUDA_STD___COMPLEX_LITERALS_H

#include <uscl/std/detail/__config>

#if defined(_CCCL_IMPLICIT_SYSTEM_HEADER_GCC)
#  pragma GCC system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_CLANG)
#  pragma clang system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_MSVC)
#  pragma system_header
#endif // no system header

#include <uscl/std/__complex/complex.h>

#include <uscl/std/__cccl/prologue.h>

_CCCL_BEGIN_NAMESPACE_CUDA_STD

#ifdef _LIBCUDACXX_HAS_STL_LITERALS
// Literal suffix for complex number literals [complex.literals]

_CCCL_DIAG_PUSH
_CCCL_DIAG_SUPPRESS_GCC("-Wliteral-suffix")
_CCCL_DIAG_SUPPRESS_CLANG("-Wuser-defined-literals")
_CCCL_DIAG_SUPPRESS_MSVC(4455)

inline namespace literals
{
inline namespace complex_literals
{
#  if !_CCCL_CUDA_COMPILER(NVCC) && !_CCCL_COMPILER(NVRTC)
// NOTE: if you get a warning from GCC <7 here that "literal operator suffixes not preceded by ‘_’ are reserved for
// future standardization" then we are sorry. The warning was implemented before GCC 7, but can only be disabled since
// GCC 7. See also: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=69523
_CCCL_API constexpr complex<long double> operator""il(long double __im)
{
  return {0.0l, __im};
}
_CCCL_API constexpr complex<long double> operator""il(unsigned long long __im)
{
  return {0.0l, static_cast<long double>(__im)};
}

_CCCL_API constexpr complex<double> operator""i(long double __im)
{
  return {0.0, static_cast<double>(__im)};
}

_CCCL_API constexpr complex<double> operator""i(unsigned long long __im)
{
  return {0.0, static_cast<double>(__im)};
}

_CCCL_API constexpr complex<float> operator""if(long double __im)
{
  return {0.0f, static_cast<float>(__im)};
}

_CCCL_API constexpr complex<float> operator""if(unsigned long long __im)
{
  return {0.0f, static_cast<float>(__im)};
}
#  else // ^^^ !_CCCL_CUDA_COMPILER(NVCC) && !_CCCL_COMPILER(NVRTC) ^^^ / vvv other compilers vvv
_CCCL_API constexpr complex<double> operator""i(double __im)
{
  return {0.0, static_cast<double>(__im)};
}

_CCCL_API constexpr complex<double> operator""i(unsigned long long __im)
{
  return {0.0, static_cast<double>(__im)};
}

_CCCL_API constexpr complex<float> operator""if(double __im)
{
  return {0.0f, static_cast<float>(__im)};
}

_CCCL_API constexpr complex<float> operator""if(unsigned long long __im)
{
  return {0.0f, static_cast<float>(__im)};
}
#  endif // other compilers
} // namespace complex_literals
} // namespace literals

_CCCL_DIAG_POP

#endif // _LIBCUDACXX_HAS_STL_LITERALS

_CCCL_END_NAMESPACE_CUDA_STD

#include <uscl/std/__cccl/epilogue.h>

#endif // _CUDA_STD___COMPLEX_LITERALS_H
