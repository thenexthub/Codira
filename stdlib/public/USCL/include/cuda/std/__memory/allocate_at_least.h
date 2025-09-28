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

#ifndef _CUDA_STD___MEMORY_ALLOCATE_AT_LEAST_H
#define _CUDA_STD___MEMORY_ALLOCATE_AT_LEAST_H

#include <uscl/std/detail/__config>

#if defined(_CCCL_IMPLICIT_SYSTEM_HEADER_GCC)
#  pragma GCC system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_CLANG)
#  pragma clang system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_MSVC)
#  pragma system_header
#endif // no system header

#include <uscl/std/__memory/allocator_traits.h>
#include <uscl/std/cstddef>

#include <uscl/std/__cccl/prologue.h>

_CCCL_BEGIN_NAMESPACE_CUDA_STD

#if _CCCL_STD_VER >= 2023
template <class _Pointer>
struct allocation_result
{
  _Pointer ptr;
  size_t count;
};
_LIBCUDACXX_CTAD_SUPPORTED_FOR_TYPE(allocation_result);

template <class _Alloc>
[[nodiscard]] _CCCL_API constexpr allocation_result<typename allocator_traits<_Alloc>::pointer>
allocate_at_least(_Alloc& __alloc, size_t __n)
{
  if constexpr (requires { __alloc.allocate_at_least(__n); })
  {
    return __alloc.allocate_at_least(__n);
  }
  else
  {
    return {__alloc.allocate(__n), __n};
  }
}

template <class _Alloc>
[[nodiscard]] _CCCL_API constexpr auto __allocate_at_least(_Alloc& __alloc, size_t __n)
{
  return ::cuda::std::allocate_at_least(__alloc, __n);
}
#else // ^^^ _CCCL_STD_VER >= 2023 ^^^ / vvv _CCCL_STD_VER < 2023 vvv
template <class _Pointer>
struct __allocation_result
{
  _Pointer ptr;
  size_t count;
};

template <class _Alloc>
[[nodiscard]] _CCCL_API constexpr __allocation_result<typename allocator_traits<_Alloc>::pointer>
__allocate_at_least(_Alloc& __alloc, size_t __n)
{
  return {__alloc.allocate(__n), __n};
}

#endif // _CCCL_STD_VER >= 2023

_CCCL_END_NAMESPACE_CUDA_STD

#include <uscl/std/__cccl/epilogue.h>

#endif // _CUDA_STD___MEMORY_ALLOCATE_AT_LEAST_H
