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

#include <cstdlib>
#include <string>
#include <variant>

#include <cccl/c/types.h>

// For each kernel accepts a user operator that contains both iterator and user operator state
// This declaration is used as blueprint for aligned_storage, but is only *valid* in the generated NVRTC program.
struct for_each_default
{
  // Defaults:
  void* iterator; // A pointer for iterator
  void* user_op; // A pointer for user data
};

struct for_each_kernel_state
{
  std::variant<for_each_default, std::unique_ptr<char[]>> for_each_arg;
  size_t user_op_offset;

  // Get address of argument for kernel
  void* get();
};

std::string get_for_kernel(cccl_op_t user_op, cccl_iterator_t iter);

for_each_kernel_state make_for_kernel_state(cccl_op_t user_op, cccl_iterator_t iterator);
