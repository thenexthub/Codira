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

#include <uscl/std/type_traits>

#include <format>
#include <string>

#include "../util/errors.h"

extern const char* jit_template_header_contents;

template <typename Arg>
struct parameter_mapping;

template <typename Tpl>
struct template_id
{};

struct specialization
{
  std::string type_name;
  std::string aux_code = "";
};

template <typename Tag,
          typename Traits,
          typename... Args,
          typename = Traits::template type<void, parameter_mapping<Args>::archetype...>>
specialization get_specialization(template_id<Traits> id, Args... args)
{
  if constexpr (requires { Traits::template special<Tag>(args...); })
  {
    if (auto result = Traits::template special<Tag>(args...))
    {
      return *result;
    }
  }

  std::string tag_name;
  check(nvrtcGetTypeName<Tag>(&tag_name));

  return {std::format("{}<{}{}>", Traits::name, tag_name, ((", " + parameter_mapping<Args>::map(id, args)) + ...)),
          std::format("struct {};", tag_name) + (parameter_mapping<Args>::aux(id, args) + ...)};
}
