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

#ifndef __CUDAX_EXECUTION_THREAD
#define __CUDAX_EXECUTION_THREAD

#include <uscl/std/detail/__config>

#if defined(_CCCL_IMPLICIT_SYSTEM_HEADER_GCC)
#  pragma GCC system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_CLANG)
#  pragma clang system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_MSVC)
#  pragma system_header
#endif // no system header

#include <thread>

#if _CCCL_CUDA_COMPILATION()
#  include <nv/target>
#  define _CUDAX_FOR_HOST_OR_DEVICE(_FOR_HOST, _FOR_DEVICE) NV_IF_TARGET(NV_IS_HOST, _FOR_HOST, _FOR_DEVICE)
#else // ^^^ _CCCL_CUDA_COMPILATION() ^^^ / vvv !_CCCL_CUDA_COMPILATION() vvv
#  define _CUDAX_FOR_HOST_OR_DEVICE(_FOR_HOST, _FOR_DEVICE) {_CCCL_PP_EXPAND _FOR_HOST}
#endif // ^^^ !_CCCL_CUDA_COMPILATION() ^^^

#include <uscl/experimental/__execution/prologue.cuh>

namespace cuda::experimental::execution
{
#if _CCCL_DEVICE_COMPILATION() && !_CCCL_CUDA_COMPILER(NVHPC)
using __thread_id _CCCL_NODEBUG_ALIAS = int;
#elif _CCCL_CUDA_COMPILER(NVHPC)
struct __thread_id
{
  union
  {
    ::std::thread::id __host_;
    int __device_;
  };

  _CCCL_API __thread_id() noexcept
      : __host_()
  {}
  _CCCL_API __thread_id(::std::thread::id __host) noexcept
      : __host_(__host)
  {}
  _CCCL_API __thread_id(int __device) noexcept
      : __device_(__device)
  {}

  _CCCL_API friend bool operator==(const __thread_id& __self, const __thread_id& __other) noexcept
  {
    _CUDAX_FOR_HOST_OR_DEVICE((return __self.__host_ == __other.__host_;),
                              (return __self.__device_ == __other.__device_;))
  }

  _CCCL_API friend bool operator!=(const __thread_id& __self, const __thread_id& __other) noexcept
  {
    return !(__self == __other);
  }
};
#else // ^^^ cuda device compilation ^^^ / vvv host compilation vvv
using __thread_id _CCCL_NODEBUG_ALIAS = ::std::thread::id;
#endif // ^^^ host compilation ^^^

inline _CCCL_API auto __this_thread_id() noexcept -> __thread_id
{
  _CUDAX_FOR_HOST_OR_DEVICE((return ::std::this_thread::get_id();),
                            (return static_cast<int>(threadIdx.x + blockIdx.x * blockDim.x);))
}

inline _CCCL_API void __this_thread_yield() noexcept
{
  _CUDAX_FOR_HOST_OR_DEVICE((::std::this_thread::yield();), (void();))
}
} // namespace cuda::experimental::execution

#include <uscl/experimental/__execution/epilogue.cuh>

#endif // __CUDAX_EXECUTION_THREAD
