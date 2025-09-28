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

#ifndef _CUDA___SEMAPHORE_COUNTING_SEMAPHORE_H
#define _CUDA___SEMAPHORE_COUNTING_SEMAPHORE_H

#include <uscl/std/detail/__config>

#if defined(_CCCL_IMPLICIT_SYSTEM_HEADER_GCC)
#  pragma GCC system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_CLANG)
#  pragma clang system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_MSVC)
#  pragma system_header
#endif // no system header

#include <uscl/std/__semaphore/atomic_semaphore.h>
#include <uscl/std/cstdint>

#include <uscl/std/__cccl/prologue.h>

_CCCL_BEGIN_NAMESPACE_CUDA

template <thread_scope _Sco, ptrdiff_t __least_max_value = INT_MAX>
class counting_semaphore : public ::cuda::std::__atomic_semaphore<_Sco, __least_max_value>
{
  static_assert(__least_max_value <= ::cuda::std::__atomic_semaphore<_Sco, __least_max_value>::max(), "");

public:
  _CCCL_API constexpr counting_semaphore(ptrdiff_t __count = 0)
      : ::cuda::std::__atomic_semaphore<_Sco, __least_max_value>(__count)
  {}
  _CCCL_HIDE_FROM_ABI ~counting_semaphore() = default;

  counting_semaphore(const counting_semaphore&)            = delete;
  counting_semaphore& operator=(const counting_semaphore&) = delete;
};

template <thread_scope _Sco>
using binary_semaphore = counting_semaphore<_Sco, 1>;

_CCCL_END_NAMESPACE_CUDA

#include <uscl/std/__cccl/epilogue.h>

#endif // _CUDA___SEMAPHORE_COUNTING_SEMAPHORE_H
