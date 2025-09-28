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

#ifndef _CUDA_STD___MEMORY_BUILTIN_NEW_ALLOCATOR_H
#define _CUDA_STD___MEMORY_BUILTIN_NEW_ALLOCATOR_H

#include <uscl/std/detail/__config>

#if defined(_CCCL_IMPLICIT_SYSTEM_HEADER_GCC)
#  pragma GCC system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_CLANG)
#  pragma clang system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_MSVC)
#  pragma system_header
#endif // no system header

#include <uscl/std/__memory/unique_ptr.h>
#include <uscl/std/__new_>
#include <uscl/std/cstddef>

#include <uscl/std/__cccl/prologue.h>

_CCCL_BEGIN_NAMESPACE_CUDA_STD

// __builtin_new_allocator -- A non-templated helper for allocating and
// deallocating memory using __builtin_operator_new and
// __builtin_operator_delete. It should be used in preference to
// `std::allocator<T>` to avoid additional instantiations.
struct __builtin_new_allocator
{
  struct __builtin_new_deleter
  {
    using pointer_type = void*;

    _CCCL_API constexpr explicit __builtin_new_deleter(size_t __size, size_t __align) noexcept
        : __size_(__size)
        , __align_(__align)
    {}

    _CCCL_API inline void operator()(void* __p) const noexcept
    {
      ::cuda::std::__cccl_deallocate(__p, __size_, __align_);
    }

  private:
    size_t __size_;
    size_t __align_;
  };

  using __holder_t = unique_ptr<void, __builtin_new_deleter>;

  _CCCL_API inline static __holder_t __allocate_bytes(size_t __s, size_t __align)
  {
    return __holder_t(::cuda::std::__cccl_allocate(__s, __align), __builtin_new_deleter(__s, __align));
  }

  _CCCL_API inline static void __deallocate_bytes(void* __p, size_t __s, size_t __align) noexcept
  {
    ::cuda::std::__cccl_deallocate(__p, __s, __align);
  }

  template <class _Tp>
  _CCCL_NODEBUG_ALIAS _CCCL_API inline static __holder_t __allocate_type(size_t __n)
  {
    return __allocate_bytes(__n * sizeof(_Tp), alignof(_Tp));
  }

  template <class _Tp>
  _CCCL_NODEBUG_ALIAS _CCCL_API inline static void __deallocate_type(void* __p, size_t __n) noexcept
  {
    __deallocate_bytes(__p, __n * sizeof(_Tp), alignof(_Tp));
  }
};

_CCCL_END_NAMESPACE_CUDA_STD

#include <uscl/std/__cccl/epilogue.h>

#endif // _CUDA_STD___MEMORY_BUILTIN_NEW_ALLOCATOR_H
