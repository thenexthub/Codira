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

#ifndef _CUDA___ANNOTATED_PTR_CREATEPOLICY_H
#define _CUDA___ANNOTATED_PTR_CREATEPOLICY_H

#include <uscl/std/detail/__config>

#if defined(_CCCL_IMPLICIT_SYSTEM_HEADER_GCC)
#  pragma GCC system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_CLANG)
#  pragma clang system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_MSVC)
#  pragma system_header
#endif // no system header

#include <uscl/__memory/address_space.h>
#include <uscl/std/cstddef>
#include <uscl/std/cstdint>

#include <uscl/std/__cccl/prologue.h>

_CCCL_BEGIN_NAMESPACE_CUDA

enum class __l2_evict_t : uint32_t
{
  _L2_Evict_Unchanged     = 0, // called "_L2_Evict_Normal" at lower level
  _L2_Evict_First         = 1,
  _L2_Evict_Last          = 2,
  _L2_Evict_Normal_Demote = 3
};

/***********************************************************************************************************************
 * PTX MAPPING
 **********************************************************************************************************************/

#if _CCCL_CUDA_COMPILATION()

template <typename = void>
[[nodiscard]] _CCCL_CONST _CCCL_HIDE_FROM_ABI _CCCL_DEVICE uint64_t __createpolicy_range_ptx(
  __l2_evict_t __primary, __l2_evict_t __secondary, size_t __gmem_ptr, uint32_t __primary_size, uint32_t __total_size)
{
  uint64_t __policy;
  if (__secondary == __l2_evict_t::_L2_Evict_Unchanged)
  {
    if (__primary == __l2_evict_t::_L2_Evict_Last)
    {
      asm("createpolicy.range.global.L2::evict_last.b64 %0, [%1], %2, %3;"
          : "=l"(__policy)
          : "l"(__gmem_ptr), "r"(__primary_size), "r"(__total_size));
    }
    else if (__primary == __l2_evict_t::_L2_Evict_Normal_Demote)
    {
      asm("createpolicy.range.global.L2::evict_normal.b64 %0, [%1], %2, %3;"
          : "=l"(__policy)
          : "l"(__gmem_ptr), "r"(__primary_size), "r"(__total_size));
    }
    else if (__primary == __l2_evict_t::_L2_Evict_First)
    {
      asm("createpolicy.range.global.L2::evict_first.b64 %0, [%1], %2, %3;"
          : "=l"(__policy)
          : "l"(__gmem_ptr), "r"(__primary_size), "r"(__total_size));
    }
    else if (__primary == __l2_evict_t::_L2_Evict_Unchanged)
    {
      asm("createpolicy.range.global.L2::evict_unchanged.b64 %0, [%1], %2, %3;"
          : "=l"(__policy)
          : "l"(__gmem_ptr), "r"(__primary_size), "r"(__total_size));
    }
    else
    {
      _CCCL_UNREACHABLE();
    }
  }
  else // __secondary == _L2_Evict_First
  {
    if (__primary == __l2_evict_t::_L2_Evict_Last)
    {
      asm("createpolicy.range.global.L2::evict_last.L2::evict_first.b64 %0, [%1], %2, %3;"
          : "=l"(__policy)
          : "l"(__gmem_ptr), "r"(__primary_size), "r"(__total_size));
    }
    else if (__primary == __l2_evict_t::_L2_Evict_Normal_Demote)
    {
      asm("createpolicy.range.global.L2::evict_normal.L2::evict_first.b64 %0, [%1], %2, %3;"
          : "=l"(__policy)
          : "l"(__gmem_ptr), "r"(__primary_size), "r"(__total_size));
    }
    else if (__primary == __l2_evict_t::_L2_Evict_First)
    {
      asm("createpolicy.range.global.L2::evict_first.L2::evict_first.b64 %0, [%1], %2, %3;"
          : "=l"(__policy)
          : "l"(__gmem_ptr), "r"(__primary_size), "r"(__total_size));
    }
    else if (__primary == __l2_evict_t::_L2_Evict_Unchanged)
    {
      asm("createpolicy.range.global.L2::evict_unchanged.L2::evict_first.b64 %0, [%1], %2, %3;"
          : "=l"(__policy)
          : "l"(__gmem_ptr), "r"(__primary_size), "r"(__total_size));
    }
    else
    {
      _CCCL_UNREACHABLE();
    }
  }
  return __policy;
}

template <typename = void>
[[nodiscard]] _CCCL_CONST _CCCL_HIDE_FROM_ABI _CCCL_DEVICE uint64_t
__createpolicy_fraction_ptx(__l2_evict_t __primary, __l2_evict_t __secondary, float __fraction)
{
  uint64_t __policy;
  if (__secondary == __l2_evict_t::_L2_Evict_Unchanged)
  {
    if (__primary == __l2_evict_t::_L2_Evict_Last)
    {
      asm("createpolicy.fractional.L2::evict_last.b64 %0, %1;" : "=l"(__policy) : "f"(__fraction));
    }
    else if (__primary == __l2_evict_t::_L2_Evict_Normal_Demote)
    {
      asm("createpolicy.fractional.L2::evict_normal.b64 %0, %1;" : "=l"(__policy) : "f"(__fraction));
    }
    else if (__primary == __l2_evict_t::_L2_Evict_First)
    {
      asm("createpolicy.fractional.L2::evict_first.b64 %0, %1;" : "=l"(__policy) : "f"(__fraction));
    }
    else if (__primary == __l2_evict_t::_L2_Evict_Unchanged)
    {
      asm("createpolicy.fractional.L2::evict_unchanged.b64 %0, %1;" : "=l"(__policy) : "f"(__fraction));
    }
    else
    {
      _CCCL_UNREACHABLE();
    }
  }
  else // __secondary == _L2_Evict_First
  {
    if (__primary == __l2_evict_t::_L2_Evict_Last)
    {
      asm("createpolicy.fractional.L2::evict_last.L2::evict_first.b64 %0, %1;" : "=l"(__policy) : "f"(__fraction));
    }
    else if (__primary == __l2_evict_t::_L2_Evict_Normal_Demote)
    {
      asm("createpolicy.fractional.L2::evict_normal.L2::evict_first.b64 %0, %1;" : "=l"(__policy) : "f"(__fraction));
    }
    else if (__primary == __l2_evict_t::_L2_Evict_First)
    {
      asm("createpolicy.fractional.L2::evict_first.L2::evict_first.b64 %0, %1;" : "=l"(__policy) : "f"(__fraction));
    }
    else if (__primary == __l2_evict_t::_L2_Evict_Unchanged)
    {
      asm("createpolicy.fractional.L2::evict_unchanged.L2::evict_first.b64 %0, %1;" : "=l"(__policy) : "f"(__fraction));
    }
    else
    {
      _CCCL_UNREACHABLE();
    }
  }
  return __policy;
}

/***********************************************************************************************************************
 * C++ API
 **********************************************************************************************************************/

extern "C" _CCCL_DEVICE void __createpolicy_is_not_supported_before_SM_80();

template <typename T = void>
[[nodiscard]] _CCCL_CONST _CCCL_HIDE_FROM_ABI _CCCL_DEVICE uint64_t __createpolicy_range(
  __l2_evict_t __primary, __l2_evict_t __secondary, const void* __ptr, uint32_t __primary_size, uint32_t __total_size)
{
  _CCCL_ASSERT(::cuda::device::is_address_from(__ptr, ::cuda::device::address_space::global), "ptr must be global");
  _CCCL_ASSERT(__primary_size > 0, "primary_size  must be greater than zero");
  _CCCL_ASSERT(__primary_size <= __total_size, "primary_size must be less than or equal to total_size");
  _CCCL_ASSERT(__secondary == __l2_evict_t::_L2_Evict_First || __secondary == __l2_evict_t::_L2_Evict_Unchanged,
               "secondary policy must be evict_first or evict_unchanged");
  [[maybe_unused]] auto __gmem_ptr = ::__cvta_generic_to_global(__ptr);
  NV_IF_ELSE_TARGET(
    NV_PROVIDES_SM_80,
    (return ::cuda::__createpolicy_range_ptx(__primary, __secondary, __gmem_ptr, __primary_size, __total_size);),
    (::cuda::__createpolicy_is_not_supported_before_SM_80(); return 0;))
}

template <typename T = void>
[[nodiscard]] _CCCL_CONST _CCCL_HIDE_FROM_ABI _CCCL_DEVICE uint64_t
__createpolicy_fraction(__l2_evict_t __primary, __l2_evict_t __secondary, float __fraction = 1.0f)
{
  _CCCL_ASSERT(__fraction > 0.0f && __fraction <= 1.0f, "fraction must be between 0.0f and 1.0f");
  _CCCL_ASSERT(__secondary == __l2_evict_t::_L2_Evict_First || __secondary == __l2_evict_t::_L2_Evict_Unchanged,
               "secondary policy must be evict_first or evict_unchanged");
  NV_IF_ELSE_TARGET(NV_PROVIDES_SM_80,
                    (return ::cuda::__createpolicy_fraction_ptx(__primary, __secondary, __fraction);),
                    (::cuda::__createpolicy_is_not_supported_before_SM_80(); return 0;))
}

#endif // _CCCL_CUDA_COMPILATION()

_CCCL_END_NAMESPACE_CUDA

#include <uscl/std/__cccl/epilogue.h>

#endif // _CUDA___ANNOTATED_PTR_CREATEPOLICY_H
