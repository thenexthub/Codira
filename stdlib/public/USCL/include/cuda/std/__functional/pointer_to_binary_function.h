/*
 *
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 * Date: Wednesday, November 8, 2023.
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

#ifndef _CUDA_STD___FUNCTIONAL_POINTER_TO_BINARY_FUNCTION_H
#define _CUDA_STD___FUNCTIONAL_POINTER_TO_BINARY_FUNCTION_H

#include <uscl/std/detail/__config>

#if defined(_CCCL_IMPLICIT_SYSTEM_HEADER_GCC)
#  pragma GCC system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_CLANG)
#  pragma clang system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_MSVC)
#  pragma system_header
#endif // no system header

#include <uscl/std/__functional/binary_function.h>

#include <uscl/std/__cccl/prologue.h>

_CCCL_BEGIN_NAMESPACE_CUDA_STD

#if defined(_LIBCUDACXX_ENABLE_CXX17_REMOVED_BINDERS)

_CCCL_SUPPRESS_DEPRECATED_PUSH

template <class _Arg1, class _Arg2, class _Result>
class _CCCL_TYPE_VISIBILITY_DEFAULT _LIBCUDACXX_DEPRECATED
pointer_to_binary_function : public __binary_function<_Arg1, _Arg2, _Result>
{
  _Result (*__f_)(_Arg1, _Arg2);

public:
  _CCCL_API inline explicit pointer_to_binary_function(_Result (*__f)(_Arg1, _Arg2))
      : __f_(__f)
  {}
  _CCCL_API inline _Result operator()(_Arg1 __x, _Arg2 __y) const
  {
    return __f_(__x, __y);
  }
};

template <class _Arg1, class _Arg2, class _Result>
_LIBCUDACXX_DEPRECATED _CCCL_API inline pointer_to_binary_function<_Arg1, _Arg2, _Result>
ptr_fun(_Result (*__f)(_Arg1, _Arg2))
{
  return pointer_to_binary_function<_Arg1, _Arg2, _Result>(__f);
}

_CCCL_SUPPRESS_DEPRECATED_POP

#endif // _LIBCUDACXX_ENABLE_CXX17_REMOVED_BINDERS

_CCCL_END_NAMESPACE_CUDA_STD

#include <uscl/std/__cccl/epilogue.h>

#endif // _CUDA_STD___FUNCTIONAL_POINTER_TO_BINARY_FUNCTION_H
