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

#pragma once

#include <cub/agent/single_pass_scan_operators.cuh>

#include "cccl/c/types.h"
#include <nvrtc/command_list.h>

struct scan_tile_state
{
  // scan_tile_state implements the same (host) interface as cub::ScanTileStateT, except
  // that it accepts the acummulator type as a runtime parameter rather than being
  // templated on it.
  //
  // Both specializations ScanTileStateT<T, true> and ScanTileStateT<T, false> - where the
  // bool parameter indicates whether `T` is primitive - are combined into a single type.

  void* d_tile_status; // d_tile_descriptors
  void* d_tile_partial;
  void* d_tile_inclusive;

  size_t description_bytes_per_tile;
  size_t payload_bytes_per_tile;

  scan_tile_state(size_t description_bytes_per_tile, size_t payload_bytes_per_tile)
      : d_tile_status(nullptr)
      , d_tile_partial(nullptr)
      , d_tile_inclusive(nullptr)
      , description_bytes_per_tile(description_bytes_per_tile)
      , payload_bytes_per_tile(payload_bytes_per_tile)
  {}

  cudaError_t Init(int num_tiles, void* d_temp_storage, size_t temp_storage_bytes)
  {
    void* allocations[3] = {};
    auto status          = cub::detail::tile_state_init(
      description_bytes_per_tile, payload_bytes_per_tile, num_tiles, d_temp_storage, temp_storage_bytes, allocations);
    if (status != cudaSuccess)
    {
      return status;
    }
    d_tile_status    = allocations[0];
    d_tile_partial   = allocations[1];
    d_tile_inclusive = allocations[2];
    return cudaSuccess;
  }

  cudaError_t AllocationSize(int num_tiles, size_t& temp_storage_bytes) const
  {
    temp_storage_bytes =
      cub::detail::tile_state_allocation_size(description_bytes_per_tile, payload_bytes_per_tile, num_tiles);
    return cudaSuccess;
  }
};

std::pair<size_t, size_t> get_tile_state_bytes_per_tile(
  cccl_type_info accum_t,
  const std::string& accum_cpp,
  const char** ptx_args,
  size_t num_ptx_args,
  const std::string& arch);
