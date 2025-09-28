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

#ifndef _CCCL_C_PARALLEL_JIT_TEMPLATES_PREPROCESS
#  include <cuda/std/optional>
#  include <cuda/std/span>

#  include "../traits.h"
#  include <cccl/c/types.h>
#endif

struct cccl_op_t_mapping
{
  bool is_stateless   = false;
  int size            = 1;
  int alignment       = 1;
  void (*operation)() = nullptr;
};

#ifndef _CCCL_C_PARALLEL_JIT_TEMPLATES_PREPROCESS
template <>
struct parameter_mapping<cccl_op_t>
{
  static const constexpr auto archetype = cccl_op_t_mapping{};

  template <typename Traits>
  static std::string map(template_id<Traits>, cccl_op_t op)
  {
    return std::format(
      "cccl_op_t_mapping{{.is_stateless = {}, .size = {}, .alignment = {}, .operation = {}}}",
      op.type != cccl_op_kind_t::CCCL_STATEFUL,
      op.size,
      op.alignment,
      op.name);
  }

  template <typename Traits>
  static std::string aux(template_id<Traits>, cccl_op_t op)
  {
    return std::format(R"(
        extern "C" __device__ void {}();
        )",
                       op.name);
  }
};
#endif
