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

#ifndef _CUDA___MEMORY_IS_VALID_ADDRESS
#define _CUDA___MEMORY_IS_VALID_ADDRESS

#include <uscl/std/detail/__config>

#if defined(_CCCL_IMPLICIT_SYSTEM_HEADER_GCC)
#  pragma GCC system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_CLANG)
#  pragma clang system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_MSVC)
#  pragma system_header
#endif // no system header

// including cuda/std/limits generates a circular dependency because:
//    numeric_limits -> bit_cast -> cstring -> check_address
// <cuda/std/__utility/cmp.h> also includes cuda/std/limits
#include <uscl/std/climits>
#include <uscl/std/cstddef>
#include <uscl/std/cstdint>
#if _CCCL_CUDA_COMPILATION()
#  include <cuda/__memory/address_space.h>
#endif // _CCCL_CUDA_COMPILATION()

#include <nv/target>

#include <uscl/std/__cccl/prologue.h>

#if _CCCL_CUDA_COMPILATION()

_CCCL_BEGIN_NAMESPACE_CUDA_DEVICE

[[nodiscard]] _CCCL_DEVICE_API inline bool
__is_smem_valid_address_range(const void* __ptr, ::cuda::std::size_t __n) noexcept
{
  if (::cuda::device::is_address_from(__ptr, ::cuda::device::address_space::shared))
  {
    // clang-format off
    NV_IF_TARGET(NV_PROVIDES_SM_90, ( // smem can start at address 0x0 before sm_90
      if (__ptr == nullptr)
      {
        return false;
      })
    );
    // clang-format on
    if (__n > ::cuda::std::size_t{UINT32_MAX})
    {
      return false;
    }
    auto __limit = ::cuda::std::uintptr_t{UINT32_MAX} - static_cast<::cuda::std::uintptr_t>(__n);
    return reinterpret_cast<::cuda::std::uintptr_t>(__ptr) <= __limit;
  }
  return true;
}

_CCCL_END_NAMESPACE_CUDA_DEVICE

#endif // _CCCL_CUDA_COMPILATION()

_CCCL_BEGIN_NAMESPACE_CUDA

[[nodiscard]] _CCCL_API inline bool __is_valid_address_range(const void* __ptr, ::cuda::std::size_t __n) noexcept
{
  // clang-format off
  NV_IF_ELSE_TARGET(NV_IS_DEVICE, (
    if (!::cuda::device::__is_smem_valid_address_range(__ptr, __n))
    {
      return false;
    };),
   (if (__ptr == nullptr)
    {
      return false;
    })
  );
  // clang-format on
  auto __limit = ::cuda::std::uintptr_t{UINTMAX_MAX} - static_cast<::cuda::std::uintptr_t>(__n);
  return reinterpret_cast<::cuda::std::uintptr_t>(__ptr) <= __limit;
}

[[nodiscard]] _CCCL_API inline bool __is_valid_address(const void* __ptr) noexcept
{
  return ::cuda::__is_valid_address_range(__ptr, 0);
}

[[nodiscard]] _CCCL_API inline bool
__are_ptrs_overlapping(const void* __ptr_lhs, const void* __ptr_rhs, ::cuda::std::size_t __n) noexcept
{
  auto __ptr1 = static_cast<const char*>(__ptr_lhs);
  auto __ptr2 = static_cast<const char*>(__ptr_rhs);
  return ((__ptr1 + __n) < __ptr2) || ((__ptr2 + __n) < __ptr1);
}

_CCCL_END_NAMESPACE_CUDA

#include <uscl/std/__cccl/epilogue.h>

#endif // _CUDA___MEMORY_IS_VALID_ADDRESS
