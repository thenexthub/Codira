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

#ifndef __CUDAX_EXECUTION_STREAM_CONTEXT_IMPL
#define __CUDAX_EXECUTION_STREAM_CONTEXT_IMPL

#include <uscl/std/detail/__config>

#if defined(_CCCL_IMPLICIT_SYSTEM_HEADER_GCC)
#  pragma GCC system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_CLANG)
#  pragma clang system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_MSVC)
#  pragma system_header
#endif // no system header

#include <uscl/__device/device_ref.h>
#include <uscl/__stream/get_stream.h>
#include <uscl/__utility/immovable.h>

#include <uscl/experimental/__execution/stream/scheduler.cuh>
#include <uscl/experimental/stream.cuh>

#include <uscl/experimental/__execution/prologue.cuh>

namespace cuda::experimental::execution
{
//////////////////////////////////////////////////////////////////////////////////////////
// stream_context
struct _CCCL_TYPE_VISIBILITY_DEFAULT stream_context : private __immovable
{
  _CCCL_HOST_API explicit stream_context(device_ref __device)
      : __stream_{__device}
  {}

  _CCCL_HOST_API void sync() noexcept
  {
    __stream_.sync();
  }

  [[nodiscard]] _CCCL_HOST_API constexpr auto query(get_stream_t) const noexcept -> stream_ref
  {
    return __stream_;
  }

  [[nodiscard]] _CCCL_NODEBUG_HOST_API auto get_scheduler() noexcept -> stream_scheduler
  {
    return stream_scheduler{__stream_};
  }

private:
  stream __stream_;
};

} // namespace cuda::experimental::execution

#include <uscl/experimental/__execution/epilogue.cuh>

#endif // __CUDAX_EXECUTION_STREAM_CONTEXT_IMPL
