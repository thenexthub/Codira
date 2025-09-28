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

#ifndef _CUDA___EVENT_TIMED_EVENT_H
#define _CUDA___EVENT_TIMED_EVENT_H

#include <cuda_runtime_api.h>
// cuda_runtime_api needs to come first

#include <uscl/std/detail/__config>

#if defined(_CCCL_IMPLICIT_SYSTEM_HEADER_GCC)
#  pragma GCC system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_CLANG)
#  pragma clang system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_MSVC)
#  pragma system_header
#endif // no system header

#if _CCCL_HAS_CTK() && !_CCCL_COMPILER(NVRTC)

#  include <cuda/__event/event.h>
#  include <cuda/__utility/no_init.h>
#  include <cuda/std/__cuda/api_wrapper.h>
#  include <cuda/std/chrono>
#  include <cuda/std/cstddef>

#  include <cuda/std/__cccl/prologue.h>

_CCCL_BEGIN_NAMESPACE_CUDA

//! @brief An owning wrapper for a `cudaEvent_t` with timing enabled.
class timed_event : public event
{
public:
  //! @brief Construct a new `timed_event` object with the specified flags
  //!        and record the event on the specified stream.
  //!
  //! @throws cuda_error if the event creation fails.
  explicit timed_event(stream_ref __stream, flags __flags = flags::none);

  //! @brief Construct a new `timed_event` object with the specified flags. The event can only be recorded on streams
  //! from the specified device.
  //!
  //! @throws cuda_error if the event creation fails.
  explicit timed_event(device_ref __device, flags __flags = flags::none)
      : event(__device, static_cast<unsigned int>(__flags))
  {}

  //! @brief Construct a new `timed_event` object into the moved-from state.
  //!
  //! @post `get()` returns `cudaEvent_t()`.
  explicit constexpr timed_event(no_init_t) noexcept
      : event(no_init)
  {}

  timed_event(timed_event&&) noexcept            = default;
  timed_event(const timed_event&)                = delete;
  timed_event& operator=(timed_event&&) noexcept = default;
  timed_event& operator=(const timed_event&)     = delete;

  //! @brief Construct a `timed_event` object from a native `cudaEvent_t` handle.
  //!
  //! @param __evnt The native handle
  //!
  //! @return timed_event The constructed `timed_event` object
  //!
  //! @note The constructed `timed_event` object takes ownership of the native handle.
  [[nodiscard]] static timed_event from_native_handle(::cudaEvent_t __evnt) noexcept
  {
    return timed_event(__evnt);
  }

  // Disallow construction from an `int`, e.g., `0`.
  static timed_event from_native_handle(int) = delete;

  // Disallow construction from `nullptr`.
  static timed_event from_native_handle(::cuda::std::nullptr_t) = delete;

  //! @brief Compute the time elapsed between two `timed_event` objects.
  //!
  //! @throws cuda_error if the query for the elapsed time fails.
  //!
  //! @param __end The `timed_event` object representing the end time.
  //! @param __start The `timed_event` object representing the start time.
  //!
  //! @return cuda::std::chrono::nanoseconds The elapsed time in nanoseconds.
  //!
  //! @note The elapsed time has a resolution of approximately 0.5 microseconds.
  [[nodiscard]] friend ::cuda::std::chrono::nanoseconds operator-(const timed_event& __end, const timed_event& __start)
  {
    float __ms = 0.0f;
    ::cuda::__driver::__eventElapsedTime(__start.get(), __end.get(), &__ms);
    return ::cuda::std::chrono::nanoseconds(static_cast<::cuda::std::chrono::nanoseconds::rep>(__ms * 1'000'000.0));
  }

private:
  // Use `timed_event::from_native_handle(e)` to construct an owning `timed_event`
  // object from a `cudaEvent_t` handle.
  explicit constexpr timed_event(::cudaEvent_t __evnt) noexcept
      : event(__evnt)
  {}
};

_CCCL_END_NAMESPACE_CUDA

#  include <cuda/std/__cccl/epilogue.h>

#endif // _CCCL_HAS_CTK() && !_CCCL_COMPILER(NVRTC)

#endif // _CUDA___EVENT_TIMED_EVENT_H
