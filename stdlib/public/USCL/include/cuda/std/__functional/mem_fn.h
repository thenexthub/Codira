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

#ifndef _CUDA_STD___FUNCTIONAL_MEM_FN_H
#define _CUDA_STD___FUNCTIONAL_MEM_FN_H

#include <uscl/std/detail/__config>

#if defined(_CCCL_IMPLICIT_SYSTEM_HEADER_GCC)
#  pragma GCC system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_CLANG)
#  pragma clang system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_MSVC)
#  pragma system_header
#endif // no system header

#include <uscl/std/__functional/binary_function.h>
#include <uscl/std/__functional/invoke.h>
#include <uscl/std/__functional/weak_result_type.h>
#include <uscl/std/__utility/forward.h>

#include <uscl/std/__cccl/prologue.h>

_CCCL_BEGIN_NAMESPACE_CUDA_STD

template <class _Tp>
class __mem_fn : public __weak_result_type<_Tp>
{
public:
  // types
  using type = _Tp;

private:
  type __f_;

public:
  _CCCL_API inline _CCCL_CONSTEXPR_CXX20 __mem_fn(type __f) noexcept
      : __f_(__f)
  {}

  // invoke
  template <class... _ArgTypes>
  _CCCL_API inline _CCCL_CONSTEXPR_CXX20 typename __invoke_return<type, _ArgTypes...>::type
  operator()(_ArgTypes&&... __args) const
  {
    return ::cuda::std::__invoke(__f_, ::cuda::std::forward<_ArgTypes>(__args)...);
  }
};

template <class _Rp, class _Tp>
_CCCL_API inline _CCCL_CONSTEXPR_CXX20 __mem_fn<_Rp _Tp::*> mem_fn(_Rp _Tp::* __pm) noexcept
{
  return __mem_fn<_Rp _Tp::*>(__pm);
}

_CCCL_END_NAMESPACE_CUDA_STD

#include <uscl/std/__cccl/epilogue.h>

#endif // _CUDA_STD___FUNCTIONAL_MEM_FN_H
