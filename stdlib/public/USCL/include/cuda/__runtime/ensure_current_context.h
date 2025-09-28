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

#ifndef _CUDA___RUNTIME_ENSURE_CURRENT_CONTEXT_H
#define _CUDA___RUNTIME_ENSURE_CURRENT_CONTEXT_H

#include <uscl/__cccl_config>

#if defined(_CCCL_IMPLICIT_SYSTEM_HEADER_GCC)
#  pragma GCC system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_CLANG)
#  pragma clang system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_MSVC)
#  pragma system_header
#endif // no system header

#if _CCCL_HAS_CTK() && !_CCCL_COMPILER(NVRTC)

#  include <cuda/__device/all_devices.h>
#  include <cuda/__driver/driver_api.h>

#  include <cuda/std/__cccl/prologue.h>

#  ifndef _CCCL_DOXYGEN_INVOKED // Do not document

_CCCL_BEGIN_NAMESPACE_CUDA
class stream_ref;

//! @brief RAII helper which on construction sets the current context to the specified one.
//! It sets the state back on destruction.
//!
struct [[maybe_unused]] __ensure_current_context
{
  //! @brief Construct a new `__ensure_current_context` object and switch to the primary context of the specified
  //!        device.
  //!
  //! @param new_device The device to switch the context to
  //!
  //! @throws cuda_error if the context switch fails
  explicit __ensure_current_context(device_ref __new_device)
  {
    auto __ctx = devices[__new_device.get()].primary_context();
    ::cuda::__driver::__ctxPush(__ctx);
  }

  //! @brief Construct a new `__ensure_current_context` object and switch to the specified
  //!        context.
  //!
  //! @param ctx The context to switch to
  //!
  //! @throws cuda_error if the context switch fails
  explicit __ensure_current_context(::CUcontext __ctx)
  {
    ::cuda::__driver::__ctxPush(__ctx);
  }

  //! @brief Construct a new `__ensure_current_context` object and switch to the context
  //!        under which the specified stream was created.
  //!
  //! @param stream Stream indicating the context to switch to
  //!
  //! @throws cuda_error if the context switch fails
  explicit __ensure_current_context(stream_ref __stream);

  __ensure_current_context(__ensure_current_context&&)                 = delete;
  __ensure_current_context(__ensure_current_context const&)            = delete;
  __ensure_current_context& operator=(__ensure_current_context&&)      = delete;
  __ensure_current_context& operator=(__ensure_current_context const&) = delete;

  //! @brief Destroy the `__ensure_current_context` object and switch back to the original
  //!        context.
  //!
  //! @throws cuda_error if the device switch fails. If the destructor is called
  //!         during stack unwinding, the program is automatically terminated.
  ~__ensure_current_context() noexcept(false)
  {
    // TODO would it make sense to assert here that we pushed and popped the same thing?
    ::cuda::__driver::__ctxPop();
  }
};

_CCCL_END_NAMESPACE_CUDA

#  endif // _CCCL_DOXYGEN_INVOKED

#  include <cuda/std/__cccl/epilogue.h>

#endif // _CCCL_HAS_CTK() && !_CCCL_COMPILER(NVRTC)

#endif // _CUDA___RUNTIME_ENSURE_CURRENT_CONTEXT_H
