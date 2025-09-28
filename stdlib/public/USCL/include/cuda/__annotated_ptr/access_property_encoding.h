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

#ifndef _CUDA___ANNOTATED_PTR_ACCESS_PROPERTY_ENCODING_H
#define _CUDA___ANNOTATED_PTR_ACCESS_PROPERTY_ENCODING_H

#include <uscl/std/detail/__config>

#if defined(_CCCL_IMPLICIT_SYSTEM_HEADER_GCC)
#  pragma GCC system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_CLANG)
#  pragma clang system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_MSVC)
#  pragma system_header
#endif // no system header

#include <uscl/__annotated_ptr/createpolicy.h>
#include <uscl/__cmath/ilog.h>
#include <uscl/std/__algorithm/clamp.h>
#include <uscl/std/__algorithm/max.h>
#include <uscl/std/__bit/bit_cast.h>
#include <uscl/std/__type_traits/is_constant_evaluated.h>
#include <uscl/std/__utility/to_underlying.h>
#include <uscl/std/cstddef>
#include <uscl/std/cstdint>
#include <uscl/std/limits>

#include <uscl/std/__cccl/prologue.h>

_CCCL_BEGIN_NAMESPACE_CUDA

enum class __l2_descriptor_mode_t : uint32_t
{
  _Desc_Implicit    = 0,
  _Desc_Interleaved = 2,
  _Desc_Block_Type  = 3
};

/***********************************************************************************************************************
 * Range Block Descriptor
 **********************************************************************************************************************/

// MemoryDescriptor:blockDesc_t  reference
//
// struct __block_desc_t // 64 bits
// {
//   uint64_t __reserved1       : 37;
//   uint32_t __block_count     : 7;
//   uint32_t __block_start     : 7;
//   uint32_t __reserved2       : 1;
//   uint32_t __block_size_enum : 4; // 56 bits
//
//   uint32_t __l2_cop_off             : 1;
//   uint32_t __l2_cop_on              : 2;
//   uint32_t __l2_descriptor_mode     : 2;
//   uint32_t __l1_inv_dont_allocate   : 1;
//   uint32_t __l2_sector_promote_256B : 1;
//   uint32_t __reserved3              : 1;
// };

#if !_CCCL_CUDA_COMPILER(NVRTC)

[[nodiscard]] _CCCL_HIDE_FROM_ABI uint64_t __block_encoding_host(
  __l2_evict_t __primary, __l2_evict_t __secondary, const void* __ptr, uint32_t __primary_bytes, uint32_t __total_bytes)
{
  _CCCL_ASSERT(__primary_bytes > 0, "primary_size must be greater than 0");
  _CCCL_ASSERT(__primary_bytes <= __total_bytes, "primary_size must be less than or equal to total_size");
  _CCCL_ASSERT(__secondary == __l2_evict_t::_L2_Evict_First || __secondary == __l2_evict_t::_L2_Evict_Unchanged,
               "secondary policy must be evict_first or evict_unchanged");
  auto __raw_ptr         = ::cuda::std::bit_cast<uintptr_t>(__ptr);
  auto __log2_total_size = ::cuda::ceil_ilog2(__total_bytes);
  // replace with ::cuda::std::add_sat when available PR #3449
  auto __block_size_enum = static_cast<uint32_t>(::cuda::std::max(__log2_total_size - 19, 0)); // min block size = 4K
  auto __log2_block_size = 12u + __block_size_enum;
  auto __block_size      = 1u << __log2_block_size;
  auto __block_start     = static_cast<uint32_t>(__raw_ptr >> __log2_block_size); // ptr / block_size
  // vvvv block_end = ceil_div(ptr + primary_size, block_size)
  auto __block_end = static_cast<uint32_t>((__raw_ptr + __primary_bytes + __block_size - 1) >> __log2_block_size);
  _CCCL_ASSERT(__block_end >= __block_start, "block_end < block_start");
  // NOTE: there is a bug in PTX createpolicy when __block_size_enum == 13. The *incorrect* behavior matches the
  //       following code:
  // auto __block_count        = (__block_size_enum == 13)
  //                            ? ((__block_end - __block_start <= 127u) ? (__block_end - __block_start) : 1)
  //                            : ::cuda::std::clamp(__block_end - __block_start, 1u, 127u);
  auto __block_count        = ::cuda::std::clamp(__block_end - __block_start, 1u, 127u);
  auto __l2_cop_off         = ::cuda::std::to_underlying(__secondary);
  auto __l2_cop_on          = ::cuda::std::to_underlying(__primary);
  auto __l2_descriptor_mode = ::cuda::std::to_underlying(__l2_descriptor_mode_t::_Desc_Block_Type);
  return static_cast<uint64_t>(__block_count) << 37 //
       | static_cast<uint64_t>(__block_start) << 44 //
       | static_cast<uint64_t>(__block_size_enum) << 52 //
       | static_cast<uint64_t>(__l2_cop_off) << 56 //
       | static_cast<uint64_t>(__l2_cop_on) << 57 //
       | static_cast<uint64_t>(__l2_descriptor_mode) << 59;
}

#endif // !_CCCL_CUDA_COMPILER(NVRTC)

[[nodiscard]] _CCCL_API inline uint64_t __block_encoding(
  __l2_evict_t __primary, __l2_evict_t __secondary, const void* __ptr, size_t __primary_bytes, size_t __total_bytes)
{
  _CCCL_ASSERT(__primary_bytes <= size_t{0xFFFFFFFF}, "primary size must be less than 4GB");
  _CCCL_ASSERT(__total_bytes <= size_t{0xFFFFFFFF}, "total size must be less than 4GB");
  auto __primary_bytes1 = static_cast<uint32_t>(__primary_bytes);
  auto __total_bytes1   = static_cast<uint32_t>(__total_bytes);
  NV_IF_ELSE_TARGET(
    NV_IS_HOST,
    (return ::cuda::__block_encoding_host(__primary, __secondary, __ptr, __primary_bytes1, __total_bytes1);),
    (return ::cuda::__createpolicy_range(__primary, __secondary, __ptr, __primary_bytes1, __total_bytes1);))
}

/***********************************************************************************************************************
 * Interleaved Descriptor
 **********************************************************************************************************************/

// MemoryDescriptor:interleaveDesc_t reference
//
// struct __interleaved_desc_t // 64 bits
// {
//   uint64_t            : 52;
//   uint32_t __fraction : 4; // 56 bits
//
//   uint32_t __l2_cop_off             : 1;
//   uint32_t __l2_cop_on              : 2;
//   uint32_t __l2_descriptor_mode     : 2;
//   uint32_t __l1_inv_dont_allocate   : 1;
//   uint32_t __l2_sector_promote_256B : 1;
//   uint32_t                          : 1;
// };

[[nodiscard]] _CCCL_API constexpr uint64_t
__l2_interleave(__l2_evict_t __primary, __l2_evict_t __secondary, float __fraction)
{
  if (!::cuda::std::__cccl_default_is_constant_evaluated())
  {
    NV_IF_ELSE_TARGET(
      NV_PROVIDES_SM_80, (return ::cuda::__createpolicy_fraction(__primary, __secondary, __fraction);), (return 0;))
  }
  _CCCL_ASSERT(__fraction > 0.0f && __fraction <= 1.0f, "fraction must be between 0.0f and 1.0f");
  _CCCL_ASSERT(__secondary == __l2_evict_t::_L2_Evict_First || __secondary == __l2_evict_t::_L2_Evict_Unchanged,
               "secondary policy must be evict_first or evict_unchanged");
  constexpr auto __epsilon  = ::cuda::std::numeric_limits<float>::epsilon();
  auto __num                = static_cast<uint32_t>((__fraction - __epsilon) * 16.0f); // fraction = num / 16
  auto __l2_cop_off         = ::cuda::std::to_underlying(__secondary);
  auto __l2_cop_on          = ::cuda::std::to_underlying(__primary);
  auto __l2_descriptor_mode = ::cuda::std::to_underlying(__l2_descriptor_mode_t::_Desc_Interleaved);
  return static_cast<uint64_t>(__num) << 52 //
       | static_cast<uint64_t>(__l2_cop_off) << 56 //
       | static_cast<uint64_t>(__l2_cop_on) << 57 //
       | static_cast<uint64_t>(__l2_descriptor_mode) << 59;
}

inline constexpr auto __l2_interleave_normal = uint64_t{0x10F0000000000000};

inline constexpr auto __l2_interleave_streaming = uint64_t{0x12F0000000000000};

inline constexpr auto __l2_interleave_persisting = uint64_t{0x14F0000000000000};

inline constexpr auto __l2_interleave_normal_demote = uint64_t{0x16F0000000000000};

_CCCL_END_NAMESPACE_CUDA

#include <uscl/std/__cccl/epilogue.h>

#endif // _CUDA___ANNOTATED_PTR_ACCESS_PROPERTY_ENCODING_H
