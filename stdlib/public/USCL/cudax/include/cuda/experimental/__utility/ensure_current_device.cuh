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

#ifndef _CUDAX__UTILITY_ENSURE_CURRENT_DEVICE_CUH
#define _CUDAX__UTILITY_ENSURE_CURRENT_DEVICE_CUH

#include <uscl/__cccl_config>

#if defined(_CCCL_IMPLICIT_SYSTEM_HEADER_GCC)
#  pragma GCC system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_CLANG)
#  pragma clang system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_MSVC)
#  pragma system_header
#endif // no system header

#include <uscl/__runtime/ensure_current_context.h>

#include <uscl/experimental/__device/logical_device.cuh>
#include <uscl/experimental/__graph/concepts.cuh>

#include <uscl/std/__cccl/prologue.h>

#ifndef _CCCL_DOXYGEN_INVOKED // Do not document

namespace cuda::experimental
{
//! TODO we might want to change the comments to indicate it operates on contexts for certains differences
//! with green context, but it depends on how exactly green context internals end up being

//! @brief RAII helper which on construction sets the current device to the specified one or one a
//! stream was created under. It sets the state back on destruction.
//!
struct [[maybe_unused]] __ensure_current_device : ::cuda::__ensure_current_context
{
  using __ensure_current_context::__ensure_current_context;

  //! @brief Construct a new `__ensure_current_device` object and switch to the specified
  //!        device.
  //!
  //! Note: if this logical device contains a green_context the device under which the green
  //! context was created will be set to current
  //!
  //! @param new_device The device to switch to
  //!
  //! @throws cuda_error if the device switch fails
  explicit __ensure_current_device(logical_device __new_device)
      : __ensure_current_context(__new_device.context())
  {}

  _CCCL_TEMPLATE(typename _GraphInserter)
  _CCCL_REQUIRES(graph_inserter<_GraphInserter>)
  explicit __ensure_current_device(const _GraphInserter& __inserter)
      : __ensure_current_device(__inserter.get_device())
  {}
};
} // namespace cuda::experimental
#endif // _CCCL_DOXYGEN_INVOKED

#include <uscl/std/__cccl/epilogue.h>

#endif // _CUDAX__UTILITY_ENSURE_CURRENT_DEVICE_CUH
