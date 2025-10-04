/*
 *
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 * Date: Tuesday, June 11, 2024.
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

#ifndef _CUDA___BARRIER_BARRIER_H
#define _CUDA___BARRIER_BARRIER_H

#include <uscl/std/detail/__config>

#if defined(_CCCL_IMPLICIT_SYSTEM_HEADER_GCC)
#  pragma GCC system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_CLANG)
#  pragma clang system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_MSVC)
#  pragma system_header
#endif // no system header

#include <uscl/__fwd/barrier.h>
#include <uscl/std/__atomic/scopes.h>
#include <uscl/std/__barrier/barrier.h>
#include <uscl/std/__barrier/empty_completion.h>
#include <uscl/std/__new_>
#include <uscl/std/cstdint>

#include <uscl/std/__cccl/prologue.h>

_CCCL_BEGIN_NAMESPACE_CUDA

template <thread_scope _Sco, class _CompletionF>
class barrier : public ::cuda::std::__barrier_base<_CompletionF, _Sco>
{
public:
  _CCCL_HIDE_FROM_ABI barrier() = default;

  barrier(const barrier&)            = delete;
  barrier& operator=(const barrier&) = delete;

  _CCCL_API constexpr barrier(::cuda::std::ptrdiff_t __expected, _CompletionF __completion = _CompletionF())
      : ::cuda::std::__barrier_base<_CompletionF, _Sco>(__expected, __completion)
  {}

  _CCCL_API inline friend void init(barrier* __b, ::cuda::std::ptrdiff_t __expected)
  {
    _CCCL_ASSERT(__expected >= 0, "Cannot initialize barrier with negative arrival count");
    new (__b) barrier(__expected);
  }

  _CCCL_API inline friend void init(barrier* __b, ::cuda::std::ptrdiff_t __expected, _CompletionF __completion)
  {
    _CCCL_ASSERT(__expected >= 0, "Cannot initialize barrier with negative arrival count");
    new (__b) barrier(__expected, __completion);
  }
};

_CCCL_END_NAMESPACE_CUDA

#include <uscl/std/__cccl/epilogue.h>

#endif // _CUDA___BARRIER_BARRIER_H
