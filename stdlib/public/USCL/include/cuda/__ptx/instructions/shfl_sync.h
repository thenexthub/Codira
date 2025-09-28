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

#ifndef _CUDA_PTX_SHFL_SYNC_H
#define _CUDA_PTX_SHFL_SYNC_H

#include <uscl/std/detail/__config>

#if defined(_CCCL_IMPLICIT_SYSTEM_HEADER_GCC)
#  pragma GCC system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_CLANG)
#  pragma clang system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_MSVC)
#  pragma system_header
#endif // no system header

#include <uscl/__ptx/instructions/get_sreg.h>
#include <uscl/__ptx/ptx_dot_variants.h>
#include <uscl/std/__bit/bit_cast.h>
#include <uscl/std/cstdint>

#include <nv/target> // __CUDA_MINIMUM_ARCH__ and friends

#include <uscl/std/__cccl/prologue.h>

_CCCL_BEGIN_NAMESPACE_CUDA_PTX

#if __cccl_ptx_isa >= 600

enum class __dot_shfl_mode
{
  __up,
  __down,
  __bfly,
  __idx
};

[[maybe_unused]]
_CCCL_DEVICE static inline uint32_t
__shfl_sync_dst_lane(__dot_shfl_mode __shfl_mode, uint32_t __lane_idx_offset, uint32_t __clamp_segmask)
{
  auto __lane     = ::cuda::ptx::get_sreg_laneid();
  auto __clamp    = __clamp_segmask & 0b11111;
  auto __segmask  = __clamp_segmask >> 8;
  auto __max_lane = (__lane & __segmask) | (__clamp & ~__segmask);
  uint32_t __j    = 0;
  if (__shfl_mode == __dot_shfl_mode::__idx)
  {
    auto __min_lane = __lane & __segmask;
    __j             = __min_lane | (__lane_idx_offset & ~__segmask);
  }
  else if (__shfl_mode == __dot_shfl_mode::__up)
  {
    __j = __lane_idx_offset >= __lane ? 0 : __lane - __lane_idx_offset;
  }
  else if (__shfl_mode == __dot_shfl_mode::__down)
  {
    __j = __lane + __lane_idx_offset;
  }
  else
  {
    __j = __lane ^ __lane_idx_offset;
  }
  auto __dst = __shfl_mode == __dot_shfl_mode::__up
               ? (__j >= __max_lane ? __j : __lane) //
               : (__j <= __max_lane ? __j : __lane);
  return (1u << __dst);
}

template <typename _Tp>
_CCCL_DEVICE static inline void __shfl_sync_checks(
  __dot_shfl_mode __shfl_mode,
  _Tp,
  [[maybe_unused]] uint32_t __lane_idx_offset,
  [[maybe_unused]] uint32_t __clamp_segmask,
  [[maybe_unused]] uint32_t __lane_mask)
{
  static_assert(sizeof(_Tp) == 4, "shfl.sync only accepts 4-byte data types");
  if (__shfl_mode != __dot_shfl_mode::__idx)
  {
    _CCCL_ASSERT(__lane_idx_offset < 32, "the lane index or offset must be less than the warp size");
  }
  _CCCL_ASSERT(__lane_mask != 0, "lane_mask must be non-zero");
  _CCCL_ASSERT((__clamp_segmask | 0b1111100011111) == 0b1111100011111,
               "clamp value + segmentation mask must use the bit positions [0:4] and [8:12]");
  _CCCL_ASSERT(::cuda::ptx::__shfl_sync_dst_lane(__shfl_mode, __lane_idx_offset, __clamp_segmask) & __lane_mask,
               "the destination lane must be a member of the lane mask");
}

template <typename _Tp>
[[nodiscard]] _CCCL_DEVICE static inline _Tp shfl_sync_idx(
  _Tp __data, bool& __pred, uint32_t __lane_idx_offset, uint32_t __clamp_segmask, uint32_t __lane_mask) noexcept
{
  ::cuda::ptx::__shfl_sync_checks(__dot_shfl_mode::__idx, __data, __lane_idx_offset, __clamp_segmask, __lane_mask);
  auto __data1 = ::cuda::std::bit_cast<uint32_t>(__data);
  int __pred1;
  uint32_t __ret;
  asm volatile(
    "{                                                      \n\t\t"
    ".reg .pred p;                                          \n\t\t"
    "shfl.sync.idx.b32 %0|p, %2, %3, %4, %5;                \n\t\t"
    "selp.s32 %1, 1, 0, p;                                  \n\t"
    "}"
    : "=r"(__ret), "=r"(__pred1)
    : "r"(__data1), "r"(__lane_idx_offset), "r"(__clamp_segmask), "r"(__lane_mask));
  __pred = static_cast<bool>(__pred1);
  return ::cuda::std::bit_cast<uint32_t>(__ret);
}

template <typename _Tp>
[[nodiscard]] _CCCL_DEVICE static inline _Tp
shfl_sync_idx(_Tp __data, uint32_t __lane_idx_offset, uint32_t __clamp_segmask, uint32_t __lane_mask) noexcept
{
  ::cuda::ptx::__shfl_sync_checks(__dot_shfl_mode::__idx, __data, __lane_idx_offset, __clamp_segmask, __lane_mask);
  auto __data1 = ::cuda::std::bit_cast<uint32_t>(__data);
  uint32_t __ret;
  asm volatile("{                                                      \n\t\t"
               "shfl.sync.idx.b32 %0, %1, %2, %3, %4;                  \n\t\t"
               "}"
               : "=r"(__ret)
               : "r"(__data1), "r"(__lane_idx_offset), "r"(__clamp_segmask), "r"(__lane_mask));
  return ::cuda::std::bit_cast<uint32_t>(__ret);
}

template <typename _Tp>
[[nodiscard]] _CCCL_DEVICE static inline _Tp shfl_sync_up(
  _Tp __data, bool& __pred, uint32_t __lane_idx_offset, uint32_t __clamp_segmask, uint32_t __lane_mask) noexcept
{
  ::cuda::ptx::__shfl_sync_checks(__dot_shfl_mode::__up, __data, __lane_idx_offset, __clamp_segmask, __lane_mask);
  auto __data1 = ::cuda::std::bit_cast<uint32_t>(__data);
  int __pred1;
  uint32_t __ret;
  asm volatile(
    "{                                                      \n\t\t"
    ".reg .pred p;                                          \n\t\t"
    "shfl.sync.up.b32 %0|p, %2, %3, %4, %5;                 \n\t\t"
    "selp.s32 %1, 1, 0, p;                                  \n\t"
    "}"
    : "=r"(__ret), "=r"(__pred1)
    : "r"(__data1), "r"(__lane_idx_offset), "r"(__clamp_segmask), "r"(__lane_mask));
  __pred = static_cast<bool>(__pred1);
  return ::cuda::std::bit_cast<uint32_t>(__ret);
}

template <typename _Tp>
[[nodiscard]] _CCCL_DEVICE static inline _Tp
shfl_sync_up(_Tp __data, uint32_t __lane_idx_offset, uint32_t __clamp_segmask, uint32_t __lane_mask) noexcept
{
  ::cuda::ptx::__shfl_sync_checks(__dot_shfl_mode::__up, __data, __lane_idx_offset, __clamp_segmask, __lane_mask);
  auto __data1 = ::cuda::std::bit_cast<uint32_t>(__data);
  uint32_t __ret;
  asm volatile("{                                                      \n\t\t"
               "shfl.sync.up.b32 %0, %1, %2, %3, %4;                   \n\t\t"
               "}"
               : "=r"(__ret)
               : "r"(__data1), "r"(__lane_idx_offset), "r"(__clamp_segmask), "r"(__lane_mask));
  return ::cuda::std::bit_cast<uint32_t>(__ret);
}

template <typename _Tp>
[[nodiscard]] _CCCL_DEVICE static inline _Tp shfl_sync_down(
  _Tp __data, bool& __pred, uint32_t __lane_idx_offset, uint32_t __clamp_segmask, uint32_t __lane_mask) noexcept
{
  ::cuda::ptx::__shfl_sync_checks(__dot_shfl_mode::__down, __data, __lane_idx_offset, __clamp_segmask, __lane_mask);
  auto __data1 = ::cuda::std::bit_cast<uint32_t>(__data);
  int __pred1;
  uint32_t __ret;
  asm volatile(
    "{                                                      \n\t\t"
    ".reg .pred p;                                          \n\t\t"
    "shfl.sync.down.b32 %0|p, %2, %3, %4, %5;               \n\t\t"
    "selp.s32 %1, 1, 0, p;                                  \n\t"
    "}"
    : "=r"(__ret), "=r"(__pred1)
    : "r"(__data1), "r"(__lane_idx_offset), "r"(__clamp_segmask), "r"(__lane_mask));
  __pred = static_cast<bool>(__pred1);
  return ::cuda::std::bit_cast<uint32_t>(__ret);
}

template <typename _Tp>
[[nodiscard]] _CCCL_DEVICE static inline _Tp
shfl_sync_down(_Tp __data, uint32_t __lane_idx_offset, uint32_t __clamp_segmask, uint32_t __lane_mask) noexcept
{
  ::cuda::ptx::__shfl_sync_checks(__dot_shfl_mode::__down, __data, __lane_idx_offset, __clamp_segmask, __lane_mask);
  auto __data1 = ::cuda::std::bit_cast<uint32_t>(__data);
  uint32_t __ret;
  asm volatile("{                                                      \n\t\t"
               "shfl.sync.down.b32 %0, %1, %2, %3, %4;                 \n\t\t"
               "}"
               : "=r"(__ret)
               : "r"(__data1), "r"(__lane_idx_offset), "r"(__clamp_segmask), "r"(__lane_mask));
  return ::cuda::std::bit_cast<uint32_t>(__ret);
}

template <typename _Tp>
[[nodiscard]] _CCCL_DEVICE static inline _Tp shfl_sync_bfly(
  _Tp __data, bool& __pred, uint32_t __lane_idx_offset, uint32_t __clamp_segmask, uint32_t __lane_mask) noexcept
{
  ::cuda::ptx::__shfl_sync_checks(__dot_shfl_mode::__bfly, __data, __lane_idx_offset, __clamp_segmask, __lane_mask);
  auto __data1 = ::cuda::std::bit_cast<uint32_t>(__data);
  int __pred1;
  uint32_t __ret;
  asm volatile(
    "{                                                      \n\t\t"
    ".reg .pred p;                                          \n\t\t"
    "shfl.sync.bfly.b32 %0|p, %2, %3, %4, %5;               \n\t\t"
    "selp.s32 %1, 1, 0, p;                                  \n\t"
    "}"
    : "=r"(__ret), "=r"(__pred1)
    : "r"(__data1), "r"(__lane_idx_offset), "r"(__clamp_segmask), "r"(__lane_mask));
  __pred = static_cast<bool>(__pred1);
  return ::cuda::std::bit_cast<uint32_t>(__ret);
}

template <typename _Tp>
[[nodiscard]] _CCCL_DEVICE static inline _Tp
shfl_sync_bfly(_Tp __data, uint32_t __lane_idx_offset, uint32_t __clamp_segmask, uint32_t __lane_mask) noexcept
{
  ::cuda::ptx::__shfl_sync_checks(__dot_shfl_mode::__bfly, __data, __lane_idx_offset, __clamp_segmask, __lane_mask);
  auto __data1 = ::cuda::std::bit_cast<uint32_t>(__data);
  uint32_t __ret;
  asm volatile( //
    "{                                                      \n\t\t"
    "shfl.sync.bfly.b32 %0, %1, %2, %3, %4;                 \n\t\t"
    "}"
    : "=r"(__ret)
    : "r"(__data1), "r"(__lane_idx_offset), "r"(__clamp_segmask), "r"(__lane_mask));
  return ::cuda::std::bit_cast<uint32_t>(__ret);
}

#endif // __cccl_ptx_isa >= 600

_CCCL_END_NAMESPACE_CUDA_PTX

#include <uscl/std/__cccl/epilogue.h>

#endif // _CUDA_PTX_SHFL_SYNC_H
