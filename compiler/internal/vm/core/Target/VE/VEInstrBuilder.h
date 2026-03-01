//===-- VEInstrBuilder.h - Aides for building VE insts ----------*- C++ -*-===//
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
// This file exposes functions that may be used with BuildMI from the
// MachineInstrBuilder.h file to simplify generating frame and constant pool
// references.
//
// For reference, the order of operands for memory references is:
// (Operand), Dest Reg, Base Reg, and either Reg Index or Immediate
// Displacement.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_VE_VEINSTRBUILDER_H
#define LLVM_LIB_TARGET_VE_VEINSTRBUILDER_H

#include "vm/core/CodeGen/MachineInstrBuilder.h"

namespace vm::core {

/// addFrameReference - This function is used to add a reference to the base of
/// an abstract object on the stack frame of the current function.  This
/// reference has base register as the FrameIndex offset until it is resolved.
/// This allows a constant offset to be specified as well...
///
static inline const MachineInstrBuilder &
addFrameReference(const MachineInstrBuilder &MIB, int FI, int Offset = 0,
                  bool ThreeOp = true) {
  if (ThreeOp)
    return MIB.addFrameIndex(FI).addImm(0).addImm(Offset);
  return MIB.addFrameIndex(FI).addImm(Offset);
}

} // namespace vm::core

#endif
