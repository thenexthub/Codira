//===------ riscv.cpp - Generic JITLink riscv edge kinds, utilities -------===//
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
//
// Generic utilities for graphs representing riscv objects.
//
//===----------------------------------------------------------------------===//

#include "vm/core/ExecutionEngine/JITLink/riscv.h"

#define DEBUG_TYPE "jitlink"

namespace vm::core {
namespace jitlink {
namespace riscv {

const char *getEdgeKindName(Edge::Kind K) {
  switch (K) {
  case R_RISCV_32:
    return "R_RISCV_32";
  case R_RISCV_64:
    return "R_RISCV_64";
  case R_RISCV_BRANCH:
    return "R_RISCV_BRANCH";
  case R_RISCV_JAL:
    return "R_RISCV_JAL";
  case R_RISCV_CALL:
    return "R_RISCV_CALL";
  case R_RISCV_CALL_PLT:
    return "R_RISCV_CALL_PLT";
  case R_RISCV_GOT_HI20:
    return "R_RISCV_GOT_HI20";
  case R_RISCV_PCREL_HI20:
    return "R_RISCV_PCREL_HI20";
  case R_RISCV_PCREL_LO12_I:
    return "R_RISCV_PCREL_LO12_I";
  case R_RISCV_PCREL_LO12_S:
    return "R_RISCV_PCREL_LO12_S";
  case R_RISCV_HI20:
    return "R_RISCV_HI20";
  case R_RISCV_LO12_I:
    return "R_RISCV_LO12_I";
  case R_RISCV_LO12_S:
    return "R_RISCV_LO12_S";
  case R_RISCV_ADD8:
    return "R_RISCV_ADD8";
  case R_RISCV_ADD16:
    return "R_RISCV_ADD16";
  case R_RISCV_ADD32:
    return "R_RISCV_ADD32";
  case R_RISCV_ADD64:
    return "R_RISCV_ADD64";
  case R_RISCV_SUB8:
    return "R_RISCV_SUB8";
  case R_RISCV_SUB16:
    return "R_RISCV_SUB16";
  case R_RISCV_SUB32:
    return "R_RISCV_SUB32";
  case R_RISCV_SUB64:
    return "R_RISCV_SUB64";
  case R_RISCV_RVC_BRANCH:
    return "R_RISCV_RVC_BRANCH";
  case R_RISCV_RVC_JUMP:
    return "R_RISCV_RVC_JUMP";
  case R_RISCV_SUB6:
    return "R_RISCV_SUB6";
  case R_RISCV_SET6:
    return "R_RISCV_SET6";
  case R_RISCV_SET8:
    return "R_RISCV_SET8";
  case R_RISCV_SET16:
    return "R_RISCV_SET16";
  case R_RISCV_SET32:
    return "R_RISCV_SET32";
  case R_RISCV_32_PCREL:
    return "R_RISCV_32_PCREL";
  case CallRelaxable:
    return "CallRelaxable";
  case AlignRelaxable:
    return "AlignRelaxable";
  case NegDelta32:
    return "NegDelta32";
  case R_RISCV_SET_ULEB128:
    return "R_RISCV_SET_ULEB128";
  case R_RISCV_SUB_ULEB128:
    return "R_RISCV_SUB_ULEB128";
  }
  return getGenericEdgeKindName(K);
}
} // namespace riscv
} // namespace jitlink
} // namespace vm::core
