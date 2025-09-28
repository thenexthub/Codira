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

#ifndef _CUDAX__DEVICE_LOGICAL_DEVICE_CUH
#define _CUDAX__DEVICE_LOGICAL_DEVICE_CUH

#include <uscl/__cccl_config>

#if defined(_CCCL_IMPLICIT_SYSTEM_HEADER_GCC)
#  pragma GCC system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_CLANG)
#  pragma clang system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_MSVC)
#  pragma system_header
#endif // no system header

#include <uscl/__device/all_devices.h>

#include <uscl/experimental/__green_context/green_ctx.cuh>

#include <uscl/std/__cccl/prologue.h>

namespace cuda::experimental
{
struct __logical_device_access;

//! @brief A non-owning representation of a CUDA device or a green context
class logical_device
{
public:
  //! @brief Enum to indicate the kind of logical device stored
  enum class kinds
  {
    // Indicates logical device is a full device
    device,
    // Indicated logical device is a green context
    green_context
  };

  // We might want to make this private depending on how this type ends up looking like long term,
  // not documenting it for now
  [[nodiscard]] constexpr CUcontext context() const noexcept
  {
    return __ctx;
  }

  //! @brief Retrieve the device on which this logical device resides
  [[nodiscard]] constexpr device_ref underlying_device() const noexcept
  {
    return __dev_id;
  }

  //! @brief Retrieve the kind of logical device stored in this object
  //! The kind indicates if this logical_device holds a device or green_context
  [[nodiscard]] constexpr kinds kind() const noexcept
  {
    return __kind;
  }

  //! @brief Construct logical_device from a device ordinal
  //!
  //! Constructing a logical_device for a given device ordinal has a side effect of initializing that device
  explicit logical_device(int __id)
      : __dev_id(__id)
      , __kind(kinds::device)
      , __ctx(devices[__id].primary_context())
  {}

  //! @brief Construct logical_device from a device_ref
  //!
  //! Constructing a logical_device for a given device_ref has a side effect of initializing that device
  explicit logical_device(device_ref __dev)
      : logical_device(__dev.get())
  {}

  // More of a micro-optimization, we can also remove this (depending if we keep device_ref)
  //!
  //! Constructing a logical_device for a given device has a side effect of initializing that device
  logical_device(const ::cuda::physical_device& __dev)
      : __dev_id(__dev.get())
      , __kind(kinds::device)
      , __ctx(__dev.primary_context())
  {}

#if _CCCL_CTK_AT_LEAST(12, 5)
  //! @brief Construct logical_device from a green_context
  logical_device(const green_context& __gctx)
      : __dev_id(__gctx.__dev_id)
      , __kind(kinds::green_context)
      , __ctx(__gctx.__transformed)
  {}
#endif // _CCCL_CTK_AT_LEAST(12, 5)

  //! @brief Compares two logical_devices for equality
  //!
  //! @param __lhs The first `logical_device` to compare
  //! @param __rhs The second `logical_device` to compare
  //! @return true if `lhs` and `rhs` refer to the same logical device
  [[nodiscard]] friend bool operator==(logical_device __lhs, logical_device __rhs) noexcept
  {
    return __lhs.__ctx == __rhs.__ctx;
  }

#if _CCCL_STD_VER <= 2017
  //! @brief Compares two logical_devices for inequality
  //!
  //! @param __lhs The first `logical_device` to compare
  //! @param __rhs The second `logical_device` to compare
  //! @return true if `lhs` and `rhs` refer to the different logical device
  [[nodiscard]] friend bool operator!=(logical_device __lhs, logical_device __rhs) noexcept
  {
    return __lhs.__ctx != __rhs.__ctx;
  }
#endif // _CCCL_STD_VER <= 2017

private:
  friend __logical_device_access;
  // This might be a CUdevice as well
  int __dev_id = 0;
  kinds __kind;
  CUcontext __ctx = nullptr;

  logical_device(int __id, CUcontext __context, kinds __k)
      : __dev_id(__id)
      , __kind(__k)
      , __ctx(__context)
  {}
};

struct __logical_device_access
{
  static logical_device make_logical_device(int __id, CUcontext __context, logical_device::kinds __k)
  {
    return logical_device(__id, __context, __k);
  }
};

} // namespace cuda::experimental

#include <uscl/std/__cccl/epilogue.h>

#endif // _CUDAX__DEVICE_LOGICAL_DEVICE_CUH
