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

#include <utility> // std::move

#include "command_list.h"
#include <cccl/c/types.h>

struct nvrtc_linkable_list_appender
{
  nvrtc_linkable_list& linkable_list;

  void append(nvrtc_linkable linkable)
  {
    std::visit(
      [&](auto&& l) {
        if (l.size)
        {
          linkable_list.push_back(std::move(l));
        }
      },
      linkable);
  }

  // New method that handles both types
  void append_operation(cccl_op_t op)
  {
    if (op.code_type == CCCL_OP_LTOIR)
    {
      // LTO-IR goes directly to the link list
      append(nvrtc_linkable{nvrtc_ltoir{op.code, op.code_size}});
    }
    else
    {
      append(nvrtc_linkable{nvrtc_code{op.code, op.code_size}});
    }
  }

  void add_iterator_definition(cccl_iterator_t it)
  {
    if (cccl_iterator_kind_t::CCCL_ITERATOR == it.type)
    {
      append_operation(it.advance); // Use new method
      append_operation(it.dereference); // Use new method
    }
  }
};
