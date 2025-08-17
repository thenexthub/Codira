//===- HLSLRuntime.h - HLSL Runtime -----------------------------*- C++ -*-===//
//
// Copyright (c) 2025, NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
// 
// Author: Tunjay Akbarli
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
// 
//     http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// 
// Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
// Middletown, DE 19709, New Castle County, USA.
//
//===----------------------------------------------------------------------===//
//
/// \file
/// Defines helper utilities for supporting the HLSL runtime environment.
//
//===----------------------------------------------------------------------===//

#ifndef CLANG_BASIC_HLSLRUNTIME_H
#define CLANG_BASIC_HLSLRUNTIME_H

#include "language/Core/Basic/LangOptions.h"
#include <cstdint>

namespace language::Core {
namespace hlsl {

constexpr ShaderStage
getStageFromEnvironment(const toolchain::Triple::EnvironmentType &E) {
  uint32_t Pipeline =
      static_cast<uint32_t>(E) - static_cast<uint32_t>(toolchain::Triple::Pixel);

  if (Pipeline > (uint32_t)ShaderStage::Invalid)
    return ShaderStage::Invalid;
  return static_cast<ShaderStage>(Pipeline);
}

#define ENUM_COMPARE_ASSERT(Value)                                             \
  static_assert(                                                               \
      getStageFromEnvironment(toolchain::Triple::Value) == ShaderStage::Value,      \
      "Mismatch between toolchain::Triple and language::Core::ShaderStage for " #Value);

ENUM_COMPARE_ASSERT(Pixel)
ENUM_COMPARE_ASSERT(Vertex)
ENUM_COMPARE_ASSERT(Geometry)
ENUM_COMPARE_ASSERT(Hull)
ENUM_COMPARE_ASSERT(Domain)
ENUM_COMPARE_ASSERT(Compute)
ENUM_COMPARE_ASSERT(Library)
ENUM_COMPARE_ASSERT(RayGeneration)
ENUM_COMPARE_ASSERT(Intersection)
ENUM_COMPARE_ASSERT(AnyHit)
ENUM_COMPARE_ASSERT(ClosestHit)
ENUM_COMPARE_ASSERT(Miss)
ENUM_COMPARE_ASSERT(Callable)
ENUM_COMPARE_ASSERT(Mesh)
ENUM_COMPARE_ASSERT(Amplification)

static_assert(getStageFromEnvironment(toolchain::Triple::UnknownEnvironment) ==
                  ShaderStage::Invalid,
              "Mismatch between toolchain::Triple and "
              "language::Core::ShaderStage for Invalid");
static_assert(getStageFromEnvironment(toolchain::Triple::MSVC) ==
                  ShaderStage::Invalid,
              "Mismatch between toolchain::Triple and "
              "language::Core::ShaderStage for Invalid");

} // namespace hlsl
} // namespace language::Core

#endif // CLANG_BASIC_HLSLRUNTIME_H
