//===- LoongArchMatInt.h - Immediate materialisation -  --------*- C++ -*--===//
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

#ifndef LLVM_LIB_TARGET_LOONGARCH_MCTARGETDESC_MATINT_H
#define LLVM_LIB_TARGET_LOONGARCH_MCTARGETDESC_MATINT_H

#include "vm/core/ADT/SmallVector.h"
#include <cstdint>

namespace vm::core {
namespace LoongArchMatInt {
struct Inst {
  unsigned Opc;
  // Imm: Opc's imm operand, if Opc == BSTRINS_D, Imm = MSB << 32 | LSB.
  int64_t Imm;
  Inst(unsigned Opc, int64_t Imm) : Opc(Opc), Imm(Imm) {}
};
using InstSeq = SmallVector<Inst, 4>;

// Helper to generate an instruction sequence that will materialise the given
// immediate value into a register.
InstSeq generateInstSeq(int64_t Val);
} // end namespace LoongArchMatInt
} // end namespace vm::core

#endif
