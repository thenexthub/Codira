/*
 *
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 * Date: Wednesday, July 24, 2024.
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

#ifndef __CUDAX_EXECUTION_THREAD_CONTEXT
#define __CUDAX_EXECUTION_THREAD_CONTEXT

#include <uscl/std/detail/__config>

#if defined(_CCCL_IMPLICIT_SYSTEM_HEADER_GCC)
#  pragma GCC system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_CLANG)
#  pragma clang system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_MSVC)
#  pragma system_header
#endif // no system header

#include <uscl/experimental/__execution/run_loop.cuh>

#include <thread>

#include <uscl/experimental/__execution/prologue.cuh>

namespace cuda::experimental::execution
{
struct _CCCL_TYPE_VISIBILITY_DEFAULT thread_context
{
  _CCCL_HOST_API thread_context() noexcept
      : __thrd_{[this] {
        __loop_.run();
      }}
  {}

  _CCCL_HOST_API ~thread_context() noexcept
  {
    join();
  }

  _CCCL_HOST_API void join() noexcept
  {
    if (__thrd_.joinable())
    {
      __loop_.finish();
      __thrd_.join();
    }
  }

  _CCCL_HOST_API auto get_scheduler()
  {
    return __loop_.get_scheduler();
  }

private:
  run_loop __loop_;
  ::std::thread __thrd_;
};
} // namespace cuda::experimental::execution

#include <uscl/experimental/__execution/epilogue.cuh>

#endif // __CUDAX_EXECUTION_THREAD_CONTEXT
