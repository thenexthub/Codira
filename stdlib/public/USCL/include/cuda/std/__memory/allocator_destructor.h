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

#ifndef _CUDA_STD___MEMORY_ALLOCATOR_DESTRUCTOR_H
#define _CUDA_STD___MEMORY_ALLOCATOR_DESTRUCTOR_H

#include <uscl/std/detail/__config>

#if defined(_CCCL_IMPLICIT_SYSTEM_HEADER_GCC)
#  pragma GCC system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_CLANG)
#  pragma clang system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_MSVC)
#  pragma system_header
#endif // no system header

#include <uscl/std/__memory/allocator_traits.h>

#include <uscl/std/__cccl/prologue.h>

_CCCL_BEGIN_NAMESPACE_CUDA_STD

template <class _Alloc>
class __allocator_destructor
{
  using __alloc_traits _CCCL_NODEBUG_ALIAS = allocator_traits<_Alloc>;

public:
  using pointer _CCCL_NODEBUG_ALIAS   = typename __alloc_traits::pointer;
  using size_type _CCCL_NODEBUG_ALIAS = typename __alloc_traits::size_type;

private:
  _Alloc& __alloc_;
  size_type __s_;

public:
  _CCCL_API inline __allocator_destructor(_Alloc& __a, size_type __s) noexcept
      : __alloc_(__a)
      , __s_(__s)
  {}
  _CCCL_API inline void operator()(pointer __p) noexcept
  {
    __alloc_traits::deallocate(__alloc_, __p, __s_);
  }
};

_CCCL_END_NAMESPACE_CUDA_STD

#include <uscl/std/__cccl/epilogue.h>

#endif // _CUDA_STD___MEMORY_ALLOCATOR_DESTRUCTOR_H
