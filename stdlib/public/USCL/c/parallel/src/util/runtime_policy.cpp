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

#include "runtime_policy.h"

#include <cub/detail/ptx-json-parser.h>

#include <format>

#include "../nvrtc/command_list.h"

nlohmann::json
get_policy(std::string_view policy_wrapper_expr, std::string_view translation_unit, std::span<const char*> args)
{
  std::vector<const char*> fixed_args(args.begin(), args.end());
  fixed_args.push_back("-DCUB_ENABLE_POLICY_PTX_JSON");
  fixed_args.push_back("-std=c++20");
  fixed_args.push_back("-default-device");

  std::string_view tag_name = "c_parallel_get_policy_tag";
  std::string fixed_source  = std::format(
    "{0}\n"
     "#if _CCCL_HAS_NVFP16()\n"
     "#include <cuda_fp16.h>\n"
     "#endif\n"
     "__global__ void ptx_json_emitting_kernel()\n"
     "{{\n"
     "  [[maybe_unused]] auto wrapped = {1};\n"
     "  ptx_json::id<ptx_json::string(\"{2}\")>() = wrapped.EncodedPolicy();\n"
     "}}\n",
    translation_unit,
    policy_wrapper_expr,
    tag_name);

  auto nvrtc_ptx =
    begin_linking_nvrtc_program(0, nullptr)
      ->add_program(nvrtc_translation_unit{fixed_source.c_str(), "runtime_policy.cu"})
      ->compile_program({fixed_args.data(), fixed_args.size()})
      ->get_program_ptx();

  return cub::detail::ptx_json::parse(tag_name, nvrtc_ptx.ptx.get());
}
