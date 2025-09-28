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

#ifndef _CUDA_STD___CSTRING_MEMCPY
#define _CUDA_STD___CSTRING_MEMCPY

#include <uscl/std/detail/__config>

#if defined(_CCCL_IMPLICIT_SYSTEM_HEADER_GCC)
#  pragma GCC system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_CLANG)
#  pragma clang system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_MSVC)
#  pragma system_header
#endif // no system header

#include <uscl/__memory/check_address.h>

#if !_CCCL_COMPILER(NVRTC)
#  include <cstring>
#endif // !_CCCL_COMPILER(NVRTC)

#include <uscl/std/__cccl/prologue.h>

_CCCL_BEGIN_NAMESPACE_CUDA_STD

using ::size_t;

// We cannot redefine memcpy before CUDA 13.2 because of CUDA Math API nvbug 5452563.
// More in detail, some CUDA Math API functions call 'memcpy' without namespace qualification, causing name ambiguity.
#if _CCCL_CTK_AT_LEAST(13, 2)

_CCCL_API inline void* memcpy(void* __dest, const void* __src, size_t __count) noexcept
{
  _CCCL_ASSERT(::cuda::__is_valid_address_range(__src, __count), "memcpy: source range is invalid");
  _CCCL_ASSERT(::cuda::__is_valid_address_range(__dest, __count), "memcpy: destination range is invalid");
  _CCCL_ASSERT(::cuda::__are_ptrs_overlapping(__src, __dest, __count), "memcpy: source and destination overlap");
  return ::memcpy(__dest, __src, __count);
}

#else // < CUDA 13.2

using ::memcpy;

#endif // _CCCL_CTK_AT_LEAST(13, 2)

_CCCL_END_NAMESPACE_CUDA_STD

#include <uscl/std/__cccl/epilogue.h>

#endif // _CUDA_STD___CSTRING_MEMCPY
