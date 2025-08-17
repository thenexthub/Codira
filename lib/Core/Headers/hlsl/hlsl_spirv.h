//===----- hlsl_spirv.h - HLSL definitions for SPIR-V target --------------===//
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

#ifndef _HLSL_HLSL_SPIRV_H_
#define _HLSL_HLSL_SPIRV_H_

namespace hlsl {
namespace vk {
template <typename T, T v> struct integral_constant {
  static constexpr T value = v;
};

template <typename T> struct Literal {};

template <uint Opcode, uint Size, uint Alignment, typename... Operands>
using SpirvType = __hlsl_spirv_type<Opcode, Size, Alignment, Operands...>;

template <uint Opcode, typename... Operands>
using SpirvOpaqueType = __hlsl_spirv_type<Opcode, 0, 0, Operands...>;
} // namespace vk
} // namespace hlsl

#endif // _HLSL_HLSL_SPIRV_H_
