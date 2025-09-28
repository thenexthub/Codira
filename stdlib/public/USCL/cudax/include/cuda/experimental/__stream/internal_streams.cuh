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

#ifndef _CUDAX__STREAM_INTERNAL_STREAMS_CUH
#define _CUDAX__STREAM_INTERNAL_STREAMS_CUH

#include <uscl/std/detail/__config>

#if defined(_CCCL_IMPLICIT_SYSTEM_HEADER_GCC)
#  pragma GCC system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_CLANG)
#  pragma clang system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_MSVC)
#  pragma system_header
#endif // no system header

#include <uscl/experimental/__stream/stream.cuh>

#include <cuda_runtime_api.h>

#include <uscl/std/__cccl/prologue.h>

namespace cuda::experimental
{
//! @brief internal stream used for memory allocations, no real blocking work
//! should ever be pushed into it
inline ::cuda::experimental::stream_ref __cccl_allocation_stream()
{
  static ::cuda::experimental::stream __stream{device_ref{0}};
  return __stream;
}

} // namespace cuda::experimental

#include <uscl/std/__cccl/epilogue.h>

#endif // _CUDAX__STREAM_INTERNAL_STREAMS_CUH
