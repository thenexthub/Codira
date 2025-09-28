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

template <typename T>
struct cccl_type_info_mapping
{
  using Type = T;
};

#ifndef _CCCL_C_PARALLEL_JIT_TEMPLATES_PREPROCESS
#  include "../traits.h"

template <>
struct parameter_mapping<cccl_type_info>
{
  static const constexpr auto archetype = cccl_type_info_mapping<int>{};

  template <typename TplId>
  static std::string map(TplId, cccl_type_info arg)
  {
    return std::format("cccl_type_info_mapping<{}>{{}}", cccl_type_enum_to_name(arg.type));
  }

  template <typename TplId>
  static std::string aux(TplId, cccl_type_info)
  {
    return {};
  }
};
#endif
