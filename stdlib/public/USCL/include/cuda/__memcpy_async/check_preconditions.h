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

#ifndef _CUDA___MEMCPY_ASYNC_CHECK_PRECONDITIONS_H
#define _CUDA___MEMCPY_ASYNC_CHECK_PRECONDITIONS_H

#include <uscl/std/detail/__config>

#if defined(_CCCL_IMPLICIT_SYSTEM_HEADER_GCC)
#  pragma GCC system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_CLANG)
#  pragma clang system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_MSVC)
#  pragma system_header
#endif // no system header

#include <uscl/__memory/aligned_size.h>
#include <uscl/std/__algorithm/max.h>
#include <uscl/std/__cstddef/types.h>
#include <uscl/std/cstdint>

#include <uscl/std/__cccl/prologue.h>

_CCCL_BEGIN_NAMESPACE_CUDA

#ifndef _LIBCUDACXX_MEMCPY_ASYNC_PRE_TESTING
#  define _LIBCUDACXX_MEMCPY_ASYNC_PRE_ASSERT(_Cond, _Msg) _CCCL_ASSERT(_Cond, _Msg)
#else // ^^^ _LIBCUDACXX_MEMCPY_ASYNC_PRE_TESTING ^^^ / vvv !_LIBCUDACXX_MEMCPY_ASYNC_PRE_TESTING vvv
#  define _LIBCUDACXX_MEMCPY_ASYNC_PRE_ASSERT(_Cond, _Msg) \
    do                                                     \
    {                                                      \
      if (!(_Cond))                                        \
      {                                                    \
        return false;                                      \
      }                                                    \
    } while (false)
#endif // _LIBCUDACXX_MEMCPY_ASYNC_PRE_TESTING

// Check the memcpy_async preconditions, return value is intended for testing purposes exclusively
template <class _Tp, class _Size>
_CCCL_API inline bool __memcpy_async_check_pre(_Tp* __dst, const _Tp* __src, _Size __size)
{
  constexpr auto __align = ::cuda::std::max(alignof(_Tp), __get_size_align_v<_Size>);

  const auto __dst_val = reinterpret_cast<uintptr_t>(__dst);
  const auto __src_val = reinterpret_cast<uintptr_t>(__src);

  // check src and dst alignment
  _LIBCUDACXX_MEMCPY_ASYNC_PRE_ASSERT(
    __dst_val % __align == 0, "destination pointer must be aligned to the specified alignment");
  _LIBCUDACXX_MEMCPY_ASYNC_PRE_ASSERT(
    __src_val % __align == 0, "source pointer must be aligned to the specified alignment");

  // check src and dst overlap
  _LIBCUDACXX_MEMCPY_ASYNC_PRE_ASSERT(
    !((__dst_val <= __src_val && __src_val < __dst_val + __size)
      || (__src_val <= __dst_val && __dst_val < __src_val + __size)),
    "destination and source buffers must not overlap");
  return true;
}

template <class _Size>
_CCCL_API inline bool __memcpy_async_check_pre(void* __dst, const void* __src, _Size __size)
{
  return ::cuda::__memcpy_async_check_pre(reinterpret_cast<char*>(__dst), reinterpret_cast<const char*>(__src), __size);
}

_CCCL_END_NAMESPACE_CUDA

#include <uscl/std/__cccl/epilogue.h>

#endif // _CUDA___MEMCPY_ASYNC_CHECK_PRECONDITIONS_H
