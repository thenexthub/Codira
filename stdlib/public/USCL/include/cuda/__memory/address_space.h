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

#ifndef _CUDA___MEMORY_ADDRESS_SPACE_H
#define _CUDA___MEMORY_ADDRESS_SPACE_H

#include <uscl/std/detail/__config>

#if defined(_CCCL_IMPLICIT_SYSTEM_HEADER_GCC)
#  pragma GCC system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_CLANG)
#  pragma clang system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_MSVC)
#  pragma system_header
#endif // no system header

#if _CCCL_CUDA_COMPILATION()

#  include <cuda/std/__memory/addressof.h>
#  include <cuda/std/__utility/to_underlying.h>

#  include <nv/target>

#  include <cuda/std/__cccl/prologue.h>

_CCCL_BEGIN_NAMESPACE_CUDA_DEVICE

//! @brief Address space enumeration for CUDA device code.
//!
//!        See https://docs.nvidia.com/cuda/parallel-thread-execution/#state-spaces for more details.
enum class address_space
{
  global, //!< Global state space
  shared, //!< Shared state space
  constant, //!< Constant state space
  local, //!< Local state space
  grid_constant, //!< Kernel function parameter in the parameter state space
  cluster_shared, //!< Cluster shared window within the shared state space
  __max,
};

[[nodiscard]] _CCCL_DEVICE_API constexpr bool __cccl_is_valid_address_space(address_space __space) noexcept
{
  const auto __v = ::cuda::std::to_underlying(__space);
  return __v >= 0 && __v < ::cuda::std::to_underlying(address_space::__max);
}

//! @brief Checks if the given pointer is from the specified address state space.
//! @param __ptr The address to check.
//! @param __space The address state space to check against.
//! @return `true` if the pointer is from the specified address space, `false` otherwise.
[[nodiscard]] _CCCL_DEVICE_API inline bool is_address_from(const void* __ptr, address_space __space) noexcept
{
  _CCCL_ASSERT(__ptr != nullptr, "invalid pointer");
  _CCCL_ASSERT(::cuda::device::__cccl_is_valid_address_space(__space), "invalid address space");

  // NVCC and NVRTC < 12.3 have problems tracking the address space of pointers, fallback to inline PTX for them
  switch (__space)
  {
    case address_space::global:
#  if _CCCL_CUDA_COMPILER(NVCC, <, 12, 3) || _CCCL_CUDA_COMPILER(NVRTC, <, 12, 3)
    {
      unsigned __ret;
      asm volatile(
        "{\n\t"
        "  .reg .pred p;\n\t"
        "  isspacep.global p, %1;\n\t"
        "  selp.u32 %0, 1, 0, p;\n\t"
        "}\n\t"
        : "=r"(__ret)
        : "l"(__ptr));
      return static_cast<bool>(__ret);
    }
#  else // ^^^ _CCCL_CUDA_COMPILER(NVCC, <, 12, 3) || _CCCL_CUDA_COMPILER(NVRTC, <, 12, 3) ^^^ /
        // vvv !_CCCL_CUDA_COMPILER(NVCC, <, 12, 3) && !_CCCL_CUDA_COMPILER(NVRTC, <, 12, 3) vvv
      return static_cast<bool>(::__isGlobal(__ptr));
#  endif // ^^^ !_CCCL_CUDA_COMPILER(NVCC, <, 12, 3) && !_CCCL_CUDA_COMPILER(NVRTC, <, 12, 3) ^^^
    case address_space::shared:
#  if _CCCL_CUDA_COMPILER(NVCC, <, 12, 3) || _CCCL_CUDA_COMPILER(NVRTC, <, 12, 3)
    {
      unsigned __ret;
      asm volatile(
        "{\n\t"
        "  .reg .pred p;\n\t"
        "  isspacep.shared p, %1;\n\t"
        "  selp.u32 %0, 1, 0, p;\n\t"
        "}\n\t"
        : "=r"(__ret)
        : "l"(__ptr));
      return static_cast<bool>(__ret);
    }
#  else // ^^^ _CCCL_CUDA_COMPILER(NVCC, <, 12, 3) || _CCCL_CUDA_COMPILER(NVRTC, <, 12, 3) ^^^ /
        // vvv !_CCCL_CUDA_COMPILER(NVCC, <, 12, 3) && !_CCCL_CUDA_COMPILER(NVRTC, <, 12, 3) vvv
      return static_cast<bool>(::__isShared(__ptr));
#  endif // ^^^ !_CCCL_CUDA_COMPILER(NVCC, <, 12, 3) && !_CCCL_CUDA_COMPILER(NVRTC, <, 12, 3) ^^^
    case address_space::constant:
#  if _CCCL_CUDA_COMPILER(NVCC, <, 12, 3) || _CCCL_CUDA_COMPILER(NVRTC, <, 12, 3)
    {
      unsigned __ret;
      asm volatile(
        "{\n\t"
        "  .reg .pred p;\n\t"
        "  isspacep.const p, %1;\n\t"
        "  selp.u32 %0, 1, 0, p;\n\t"
        "}\n\t"
        : "=r"(__ret)
        : "l"(__ptr));
      return static_cast<bool>(__ret);
    }
#  else // ^^^ _CCCL_CUDA_COMPILER(NVCC, <, 12, 3) || _CCCL_CUDA_COMPILER(NVRTC, <, 12, 3) ^^^ /
        // vvv !_CCCL_CUDA_COMPILER(NVCC, <, 12, 3) && !_CCCL_CUDA_COMPILER(NVRTC, <, 12, 3) vvv
      return static_cast<bool>(::__isConstant(__ptr));
#  endif // ^^^ !_CCCL_CUDA_COMPILER(NVCC, <, 12, 3) && !_CCCL_CUDA_COMPILER(NVRTC, <, 12, 3) ^^^
    case address_space::local:
      // __isLocal is buggy, see https://github.com/NVIDIA/cccl/pull/4866#discussion_r2121772829
      // let's always use the inline PTX instead of the intrinsic
#  if _CCCL_CUDA_COMPILER(NVCC) || _CCCL_CUDA_COMPILER(NVRTC)
    {
      unsigned __ret;
      asm volatile(
        "{\n\t"
        "  .reg .pred p;\n\t"
        "  isspacep.local p, %1;\n\t"
        "  selp.u32 %0, 1, 0, p;\n\t"
        "}\n\t"
        : "=r"(__ret)
        : "l"(__ptr));
      return static_cast<bool>(__ret);
    }
#  else // ^^^ _CCCL_CUDA_COMPILER(NVCC) || _CCCL_CUDA_COMPILER(NVRTC) ^^^ /
        // vvv !_CCCL_CUDA_COMPILER(NVCC) && !_CCCL_CUDA_COMPILER(NVRTC) vvv
      return static_cast<bool>(::__isLocal(__ptr));
#  endif // ^^^ !_CCCL_CUDA_COMPILER(NVCC) && !_CCCL_CUDA_COMPILER(NVRTC) ^^^
    case address_space::grid_constant:
#  if _CCCL_HAS_GRID_CONSTANT()
#    if _CCCL_CUDA_COMPILER(NVCC, <, 12, 3) || _CCCL_CUDA_COMPILER(NVRTC, <, 12, 3)
    {
      NV_IF_ELSE_TARGET(
        NV_PROVIDES_SM_70,
        (unsigned __ret; asm volatile(
           "{\n\t"
           "  .reg .pred p;\n\t"
           "  isspacep.param p, %1;\n\t"
           "  selp.u32 %0, 1, 0, p;\n\t"
           "}\n\t" : "=r"(__ret) : "l"(__ptr));
         return static_cast<bool>(__ret);),
        (return false;))
    }
#    else // ^^^ _CCCL_CUDA_COMPILER(NVCC, <, 12, 3) || _CCCL_CUDA_COMPILER(NVRTC, <, 12, 3) ^^^ /
          // vvv !_CCCL_CUDA_COMPILER(NVCC, <, 12, 3) && !_CCCL_CUDA_COMPILER(NVRTC, <, 12, 3) vvv
      NV_IF_ELSE_TARGET(NV_PROVIDES_SM_70, (return static_cast<bool>(::__isGridConstant(__ptr));), (return false;))
#    endif // ^^^ !_CCCL_CUDA_COMPILER(NVCC, <, 12, 3) && !_CCCL_CUDA_COMPILER(NVRTC, <, 12, 3) ^^^
#  else // ^^^ _CCCL_HAS_GRID_CONSTANT() ^^^ / vvv !_CCCL_HAS_GRID_CONSTANT() vvv
      return false;
#  endif // ^^^ !_CCCL_HAS_GRID_CONSTANT() ^^^
    case address_space::cluster_shared:
#  if _CCCL_CUDA_COMPILER(NVCC, <, 12, 3) || _CCCL_CUDA_COMPILER(NVRTC, <, 12, 3)
    {
      NV_IF_ELSE_TARGET(
        NV_PROVIDES_SM_90,
        (unsigned __ret; asm volatile(
           "{\n\t"
           "  .reg .pred p;\n\t"
           "  isspacep.shared::cluster p, %1;\n\t"
           "  selp.u32 %0, 1, 0, p;\n\t"
           "}\n\t" : "=r"(__ret) : "l"(__ptr));
         return static_cast<bool>(__ret);),
        (return false;))
    }
#  else // ^^^ _CCCL_CUDA_COMPILER(NVCC, <, 12, 3) || _CCCL_CUDA_COMPILER(NVRTC, <, 12, 3) ^^^ /
        // vvv !_CCCL_CUDA_COMPILER(NVCC, <, 12, 3) && !_CCCL_CUDA_COMPILER(NVRTC, <, 12, 3) vvv
      NV_IF_ELSE_TARGET(NV_PROVIDES_SM_90, (return static_cast<bool>(::__isClusterShared(__ptr));), (return false;))
#  endif // ^^^ !_CCCL_CUDA_COMPILER(NVCC, <, 12, 3) && !_CCCL_CUDA_COMPILER(NVRTC, <, 12, 3) ^^^
    default:
      return false;
  }
}

//! @brief Checks if the given pointer is from the specified address state space.
//! @param __ptr The address to check.
//! @param __space The address state space to check against.
//! @return `true` if the pointer is from the specified address space, `false` otherwise.
[[nodiscard]] _CCCL_DEVICE_API inline bool is_address_from(const volatile void* __ptr, address_space __space) noexcept
{
  return ::cuda::device::is_address_from(const_cast<const void*>(__ptr), __space);
}

//! @brief Checks if the given object is from the specified address state space.
//! @param __obj The object to check.
//! @param __space The address state space to check against.
//! @return `true` if the object is from the specified address space, `false` otherwise.
template <class _Tp>
[[nodiscard]] _CCCL_DEVICE_API inline bool is_object_from(_Tp& __obj, address_space __space) noexcept
{
  return ::cuda::device::is_address_from(::cuda::std::addressof(__obj), __space);
}

_CCCL_END_NAMESPACE_CUDA_DEVICE

#  include <cuda/std/__cccl/epilogue.h>

#endif // _CCCL_CUDA_COMPILATION()

#endif // _CUDA___MEMORY_ADDRESS_SPACE_H
