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

#include <uscl/std/cstdint>

#include <string>

#include "errors.h"
#include <cccl/c/types.h>

struct storage_t;
struct input_storage_t;
struct output_storage_t;
struct items_storage_t; // Used in merge_sort

template <typename StorageT = storage_t>
std::string cccl_type_enum_to_name(cccl_type_enum type, bool is_pointer = false)
{
  std::string result;

  switch (type)
  {
    case cccl_type_enum::CCCL_INT8:
      result = "::cuda::std::int8_t";
      break;
    case cccl_type_enum::CCCL_INT16:
      result = "::cuda::std::int16_t";
      break;
    case cccl_type_enum::CCCL_INT32:
      result = "::cuda::std::int32_t";
      break;
    case cccl_type_enum::CCCL_INT64:
      result = "::cuda::std::int64_t";
      break;
    case cccl_type_enum::CCCL_UINT8:
      result = "::cuda::std::uint8_t";
      break;
    case cccl_type_enum::CCCL_UINT16:
      result = "::cuda::std::uint16_t";
      break;
    case cccl_type_enum::CCCL_UINT32:
      result = "::cuda::std::uint32_t";
      break;
    case cccl_type_enum::CCCL_UINT64:
      result = "::cuda::std::uint64_t";
      break;
    case cccl_type_enum::CCCL_FLOAT16:
#if _CCCL_HAS_NVFP16()
      result = "__half";
      break;
#else
      throw std::runtime_error("float16 is not supported");
#endif
    case cccl_type_enum::CCCL_FLOAT32:
      result = "float";
      break;
    case cccl_type_enum::CCCL_FLOAT64:
      result = "double";
      break;
    case cccl_type_enum::CCCL_STORAGE:
      check(nvrtcGetTypeName<StorageT>(&result));
      break;
    case cccl_type_enum::CCCL_BOOLEAN:
      result = "bool";
      break;
  }

  if (is_pointer)
  {
    result += "*";
  }

  return result;
}
