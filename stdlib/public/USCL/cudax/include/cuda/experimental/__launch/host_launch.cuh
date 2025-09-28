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

#ifndef _CUDAX__LAUNCH_HOST_LAUNCH
#define _CUDAX__LAUNCH_HOST_LAUNCH

#include <uscl/__cccl_config>

#if defined(_CCCL_IMPLICIT_SYSTEM_HEADER_GCC)
#  pragma GCC system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_CLANG)
#  pragma clang system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_MSVC)
#  pragma system_header
#endif // no system header

#include <uscl/std/__functional/reference_wrapper.h>
#include <uscl/std/__type_traits/decay.h>
#include <uscl/std/__utility/forward.h>
#include <uscl/std/tuple>
#include <uscl/stream_ref>

#include <uscl/std/__cccl/prologue.h>

namespace cuda::experimental
{

template <typename _CallablePtr>
void __stream_callback_caller(cudaStream_t, cudaError_t __status, void* __callable_ptr)
{
  auto __casted_callable = static_cast<_CallablePtr>(__callable_ptr);
  if (__status == cudaSuccess)
  {
    (*__casted_callable)();
  }
  delete __casted_callable;
}

//! @brief Launches a host callable to be executed in stream order on the provided stream
//!
//! Callable and arguments are copied into an internal dynamic allocation to preserve them
//! until the asynchronous call happens. Lambda capture or reference_wrapper can be used if
//! there is a need to pass something by reference.
//!
//! Callable must not call any APIs from cuda, thrust or cub namespaces.
//! It must not call into CUDA Runtime or Driver APIs. It also can't depend on another
//! thread that might block on any asynchronous CUDA work.
//!
//! @param __stream Stream to launch the host function on
//! @param __callable Host function or callable object to call in stream order
//! @param __args Arguments to call the supplied callable with
template <typename _Callable, typename... _Args>
void host_launch(stream_ref __stream, _Callable __callable, _Args... __args)
{
  static_assert(::cuda::std::is_invocable_v<_Callable, _Args...>,
                "Callable can't be called with the supplied arguments");
  auto __lambda_ptr = new auto([__callable   = ::cuda::std::move(__callable),
                                __args_tuple = ::cuda::std::make_tuple(::cuda::std::move(__args)...)]() mutable {
    ::cuda::std::apply(__callable, __args_tuple);
  });

  // We use the callback here to have it execute even on stream error, because it needs to free the above allocation
  _CCCL_TRY_CUDA_API(
    cudaStreamAddCallback,
    "Failed to launch host function",
    __stream.get(),
    __stream_callback_caller<decltype(__lambda_ptr)>,
    static_cast<void*>(__lambda_ptr),
    0);
}

template <typename _CallablePtr>
void __host_func_launcher(void* __callable_ptr)
{
  auto __casted_callable = static_cast<_CallablePtr>(__callable_ptr);
  (*__casted_callable)();
}

//! @brief Launches a host callable to be executed in stream order on the provided stream
//!
//! Callable will be called using the supplied reference. If the callable was destroyed
//! or moved by the time it is asynchronously called the behavior is undefined.
//!
//! Callable can't take any arguments, if some additional state is required a lambda can be used
//! to capture it.
//!
//! Callable must not call any APIs from cuda, thrust or cub namespaces.
//! It must not call into CUDA Runtime or Driver APIs. It also can't depend on another
//! thread that might block on any asynchronous CUDA work.
//!
//! @param __stream Stream to launch the host function on
//! @param __callable A reference to a host function or callable object to call in stream order
template <typename _Callable, typename... _Args>
void host_launch(stream_ref __stream, ::cuda::std::reference_wrapper<_Callable> __callable)
{
  static_assert(::cuda::std::is_invocable_v<_Callable>, "Callable in reference_wrapper can't take any arguments");
  _CCCL_TRY_CUDA_API(
    cudaLaunchHostFunc,
    "Failed to launch host function",
    __stream.get(),
    __host_func_launcher<_Callable*>,
    ::cuda::std::addressof(__callable.get()));
}

} // namespace cuda::experimental

#include <uscl/std/__cccl/epilogue.h>

#endif // !_CUDAX__LAUNCH_HOST_LAUNCH
