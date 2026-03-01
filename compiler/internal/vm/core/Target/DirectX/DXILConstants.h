//===- DXILConstants.h - Essential DXIL constants -------------------------===//
//
// Copyright (c) NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// Author: Tunjay Akbarli
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
//
// http://www.apache.org/licenses/LICENSE-2.0
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
///
/// \file This file contains essential DXIL constants.
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_DIRECTX_DXILCONSTANTS_H
#define LLVM_LIB_TARGET_DIRECTX_DXILCONSTANTS_H

namespace vm::core {
namespace dxil {

enum class OpCode : unsigned {
#define DXIL_OPCODE(Op, Name) Name = Op,
#include "DXILOperation.inc"
};

enum class OpCodeClass : unsigned {
#define DXIL_OPCLASS(Name) Name,
#include "DXILOperation.inc"
};

enum class OpParamType : unsigned {
#define DXIL_OP_PARAM_TYPE(Name) Name,
#include "DXILOperation.inc"
};

struct Attributes {
#define DXIL_ATTRIBUTE(Name) bool Name = false;
#include "DXILOperation.inc"
};

inline Attributes operator|(Attributes a, Attributes b) {
  Attributes c;
#define DXIL_ATTRIBUTE(Name) c.Name = a.Name | b.Name;
#include "DXILOperation.inc"
  return c;
}

inline Attributes &operator|=(Attributes &a, Attributes &b) {
  a = a | b;
  return a;
}

} // namespace dxil
} // namespace vm::core

#endif
