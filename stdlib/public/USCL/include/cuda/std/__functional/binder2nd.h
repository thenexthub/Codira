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

#ifndef _CUDA_STD___FUNCTIONAL_BINDER2ND_H
#define _CUDA_STD___FUNCTIONAL_BINDER2ND_H

#include <uscl/std/detail/__config>

#if defined(_CCCL_IMPLICIT_SYSTEM_HEADER_GCC)
#  pragma GCC system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_CLANG)
#  pragma clang system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_MSVC)
#  pragma system_header
#endif // no system header

#include <uscl/std/__functional/unary_function.h>

#include <uscl/std/__cccl/prologue.h>

_CCCL_BEGIN_NAMESPACE_CUDA_STD

#if defined(_LIBCUDACXX_ENABLE_CXX17_REMOVED_BINDERS)

_CCCL_SUPPRESS_DEPRECATED_PUSH

template <class __Operation>
class _CCCL_TYPE_VISIBILITY_DEFAULT _LIBCUDACXX_DEPRECATED
binder2nd : public __unary_function<typename __Operation::first_argument_type, typename __Operation::result_type>
{
protected:
  __Operation op;
  typename __Operation::second_argument_type value;

public:
  _CCCL_API inline binder2nd(const __Operation& __x, const typename __Operation::second_argument_type __y)
      : op(__x)
      , value(__y)
  {}
  _CCCL_EXEC_CHECK_DISABLE
  _CCCL_API inline typename __Operation::result_type operator()(typename __Operation::first_argument_type& __x) const
  {
    return op(__x, value);
  }
  _CCCL_EXEC_CHECK_DISABLE
  _CCCL_API inline typename __Operation::result_type
  operator()(const typename __Operation::first_argument_type& __x) const
  {
    return op(__x, value);
  }
};

template <class __Operation, class _Tp>
_LIBCUDACXX_DEPRECATED _CCCL_API inline binder2nd<__Operation> bind2nd(const __Operation& __op, const _Tp& __x)
{
  return binder2nd<__Operation>(__op, __x);
}

_CCCL_SUPPRESS_DEPRECATED_POP

#endif // defined(_LIBCUDACXX_ENABLE_CXX17_REMOVED_BINDERS)

_CCCL_END_NAMESPACE_CUDA_STD

#include <uscl/std/__cccl/epilogue.h>

#endif // _CUDA_STD___FUNCTIONAL_BINDER2ND_H
