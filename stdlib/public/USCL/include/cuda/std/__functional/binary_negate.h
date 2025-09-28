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

#ifndef _CUDA_STD___FUNCTIONAL_BINARY_NEGATE_H
#define _CUDA_STD___FUNCTIONAL_BINARY_NEGATE_H

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

#if _CCCL_STD_VER <= 2017 || defined(_LIBCUDACXX_ENABLE_CXX20_REMOVED_NEGATORS)

template <class _Predicate>
class _CCCL_TYPE_VISIBILITY_DEFAULT _LIBCUDACXX_DEPRECATED binary_negate
    : public __binary_function<typename _Predicate::first_argument_type, typename _Predicate::second_argument_type, bool>
{
  _Predicate __pred_;

public:
  _CCCL_API explicit constexpr binary_negate(const _Predicate& __pred)
      : __pred_(__pred)
  {}

  _CCCL_EXEC_CHECK_DISABLE
  constexpr _CCCL_API inline bool operator()(const typename _Predicate::first_argument_type& __x,
                                             const typename _Predicate::second_argument_type& __y) const
  {
    return !__pred_(__x, __y);
  }
};

_CCCL_SUPPRESS_DEPRECATED_PUSH
template <class _Predicate>
_LIBCUDACXX_DEPRECATED _CCCL_API constexpr binary_negate<_Predicate> not2(const _Predicate& __pred)
{
  return binary_negate<_Predicate>(__pred);
}
_CCCL_SUPPRESS_DEPRECATED_POP

#endif // _CCCL_STD_VER <= 2017 || defined(_LIBCUDACXX_ENABLE_CXX20_REMOVED_NEGATORS)

_CCCL_END_NAMESPACE_CUDA_STD

#include <uscl/std/__cccl/epilogue.h>

#endif // _CUDA_STD___FUNCTIONAL_BINARY_NEGATE_H
