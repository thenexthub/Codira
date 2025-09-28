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

#ifndef _CUDA_STD___FUNCTIONAL_DEFAULT_SEARCHER_H
#define _CUDA_STD___FUNCTIONAL_DEFAULT_SEARCHER_H

#include <uscl/std/detail/__config>

#if defined(_CCCL_IMPLICIT_SYSTEM_HEADER_GCC)
#  pragma GCC system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_CLANG)
#  pragma clang system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_MSVC)
#  pragma system_header
#endif // no system header

#include <uscl/std/__algorithm/search.h>
#include <uscl/std/__functional/identity.h>
#include <uscl/std/__functional/operations.h>
#include <uscl/std/__iterator/iterator_traits.h>
#include <uscl/std/__utility/pair.h>

#include <uscl/std/__cccl/prologue.h>

_CCCL_BEGIN_NAMESPACE_CUDA_STD

#ifndef __cuda_std__

// default searcher
template <class _ForwardIterator, class _BinaryPredicate = equal_to<>>
class _CCCL_TYPE_VISIBILITY_DEFAULT default_searcher
{
public:
  _CCCL_API inline _CCCL_CONSTEXPR_CXX20
  default_searcher(_ForwardIterator __f, _ForwardIterator __l, _BinaryPredicate __p = _BinaryPredicate())
      : __first_(__f)
      , __last_(__l)
      , __pred_(__p)
  {}

  template <typename _ForwardIterator2>
  _CCCL_API inline _CCCL_CONSTEXPR_CXX20 pair<_ForwardIterator2, _ForwardIterator2>
  operator()(_ForwardIterator2 __f, _ForwardIterator2 __l) const
  {
    return ::cuda::std::__search(
      __f,
      __l,
      __first_,
      __last_,
      __pred_,
      typename ::cuda::std::iterator_traits<_ForwardIterator>::iterator_category(),
      typename ::cuda::std::iterator_traits<_ForwardIterator2>::iterator_category());
  }

private:
  _ForwardIterator __first_;
  _ForwardIterator __last_;
  _BinaryPredicate __pred_;
};
_LIBCUDACXX_CTAD_SUPPORTED_FOR_TYPE(default_searcher);

#endif // __cuda_std__

_CCCL_END_NAMESPACE_CUDA_STD

#include <uscl/std/__cccl/epilogue.h>

#endif // _CUDA_STD___FUNCTIONAL_DEFAULT_SEARCHER_H
