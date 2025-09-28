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

#ifndef _CUDAX__STREAM_STREAM_REF_CUH
#define _CUDAX__STREAM_STREAM_REF_CUH

#include <uscl/std/detail/__config>
#if defined(_CCCL_IMPLICIT_SYSTEM_HEADER_GCC)
#  pragma GCC system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_CLANG)
#  pragma clang system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_MSVC)
#  pragma system_header
#endif // no system header

#include <uscl/__device/all_devices.h>
#include <uscl/__event/timed_event.h>
#include <uscl/std/__cuda/api_wrapper.h>
#include <uscl/stream_ref>

#include <uscl/experimental/__device/logical_device.cuh>
#include <uscl/experimental/__execution/fwd.cuh>
#include <uscl/experimental/__utility/ensure_current_device.cuh>

#include <cuda_runtime_api.h>

#include <uscl/std/__cccl/prologue.h>

namespace cuda::experimental
{
//! @brief A non-owning wrapper for cudaStream_t.
//!
//! @note It is undefined behavior to use a `stream_ref` object beyond the lifetime of the stream it was created from,
//! except for the `get()` member function.
struct stream_ref : ::cuda::stream_ref
{
  using scheduler_concept = execution::scheduler_t;

  stream_ref() = delete;

  //! @brief Converting constructor from \c ::cuda::stream_ref
  //!
  //! @post `*this == __other`
  _CCCL_HOST_API constexpr stream_ref(const ::cuda::stream_ref& __other) noexcept
      : ::cuda::stream_ref(__other)
  {}

  using ::cuda::stream_ref::stream_ref;

  //! @brief Deprecated. Use is_done() instead.
  [[deprecated("Use is_done() instead.")]] [[nodiscard]] bool ready() const
  {
    return is_done();
  }

  //! @brief Returns a \c execution::sender that completes on this stream.
  //!
  //! @note Equivalent to `execution::schedule(execution::stream_scheduler{*this})`.
  _CCCL_HOST_API auto schedule() const noexcept;

  //! @brief Get the logical device under which this stream was created.
  //!
  //! Compared to `device()` member function the returned \c logical_device will
  //! hold a green context for streams created under one.
  _CCCL_HOST_API logical_device logical_device() const
  {
    CUcontext __stream_ctx;
    ::cuda::experimental::logical_device::kinds __ctx_kind = ::cuda::experimental::logical_device::kinds::device;
#if _CCCL_CTK_AT_LEAST(12, 5)
    if (__driver::__getVersion() >= 12050)
    {
      auto __ctx = ::cuda::__driver::__streamGetCtx_v2(__stream);
      if (__ctx.__ctx_kind_ == ::cuda::__driver::__ctx_from_stream::__kind::__green)
      {
        __stream_ctx = ::cuda::__driver::__ctxFromGreenCtx(__ctx.__ctx_green_);
        __ctx_kind   = ::cuda::experimental::logical_device::kinds::green_context;
      }
      else
      {
        __stream_ctx = __ctx.__ctx_device_;
        __ctx_kind   = ::cuda::experimental::logical_device::kinds::device;
      }
    }
    else
#endif // _CCCL_CTK_AT_LEAST(12, 5)
    {
      __stream_ctx = ::cuda::__driver::__streamGetCtx(__stream);
      __ctx_kind   = ::cuda::experimental::logical_device::kinds::device;
    }
    // Because the stream can come from_native_handle, we can't just loop over devices comparing contexts,
    // lower to CUDART for this instead
    __ensure_current_device __setter(__stream_ctx);
    int __id;
    _CCCL_TRY_CUDA_API(cudaGetDevice, "Could not get device from a stream", &__id);
    return __logical_device_access::make_logical_device(__id, __stream_ctx, __ctx_kind);
  }

  [[nodiscard]] _CCCL_API constexpr auto query(const execution::get_forward_progress_guarantee_t&) const noexcept
    -> execution::forward_progress_guarantee
  {
    return execution::forward_progress_guarantee::weakly_parallel;
  }

  [[nodiscard]] _CCCL_API constexpr auto query(const execution::get_domain_t&) const noexcept
    -> execution::stream_domain;
};

} // namespace cuda::experimental

#include <uscl/std/__cccl/epilogue.h>

#endif // _CUDAX__STREAM_STREAM_REF_CUH
