//===-- M68kInstrBuilder.h - Functions to build M68k insts ------*- C++ -*-===//
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
/// \file
/// This file exposes functions that may be used with BuildMI from the
/// MachineInstrBuilder.h file to handle M68k'isms in a clean way.
///
/// TODO The BuildMem function may be used with the BuildMI function to add
/// entire memory references in a single, typed, function call.  M68k memory
/// references can be very complex expressions (described in the README), so
/// wrapping them up behind an easier to use interface makes sense.
/// Descriptions of the functions are included below.
///
/// For reference, the order of operands for memory references is:
/// (Operand), Base, Scale, Index, Displacement.
///
//===----------------------------------------------------------------------===//
//
#ifndef LLVM_LIB_TARGET_M68K_M68KINSTRBUILDER_H
#define LLVM_LIB_TARGET_M68K_M68KINSTRBUILDER_H

#include "vm/core/ADT/SmallVector.h"
#include "vm/core/CodeGen/MachineFrameInfo.h"
#include "vm/core/CodeGen/MachineFunction.h"
#include "vm/core/CodeGen/MachineInstr.h"
#include "vm/core/CodeGen/MachineInstrBuilder.h"
#include "vm/core/CodeGen/MachineMemOperand.h"
#include "vm/core/CodeGen/MachineOperand.h"
#include "vm/core/MC/MCInstrDesc.h"

#include <cassert>

namespace vm::core {
namespace M68k {
static inline const MachineInstrBuilder &
addOffset(const MachineInstrBuilder &MIB, int Offset) {
  return MIB.addImm(Offset);
}

/// addRegIndirectWithDisp - This function is used to add a memory reference
/// of the form (Offset, Base), i.e., one with no scale or index, but with a
/// displacement. An example is: (4,D0).
static inline const MachineInstrBuilder &
addRegIndirectWithDisp(const MachineInstrBuilder &MIB, Register Reg,
                       bool IsKill, int Offset) {
  return MIB.addImm(Offset).addReg(Reg, getKillRegState(IsKill));
}

/// addFrameReference - This function is used to add a reference to the base of
/// an abstract object on the stack frame of the current function.  This
/// reference has base register as the FrameIndex offset until it is resolved.
/// This allows a constant offset to be specified as well...
static inline const MachineInstrBuilder &
addFrameReference(const MachineInstrBuilder &MIB, int FI, int Offset = 0) {
  MachineInstr *MI = MIB;
  MachineFunction &MF = *MI->getParent()->getParent();
  MachineFrameInfo &MFI = MF.getFrameInfo();
  const MCInstrDesc &MCID = MI->getDesc();
  auto Flags = MachineMemOperand::MONone;
  if (MCID.mayLoad())
    Flags |= MachineMemOperand::MOLoad;
  if (MCID.mayStore())
    Flags |= MachineMemOperand::MOStore;
  MachineMemOperand *MMO = MF.getMachineMemOperand(
      MachinePointerInfo::getFixedStack(MF, FI, Offset), Flags,
      MFI.getObjectSize(FI), MFI.getObjectAlign(FI));
  return MIB.addImm(Offset).addFrameIndex(FI).addMemOperand(MMO);
}

static inline const MachineInstrBuilder &
addMemOperand(const MachineInstrBuilder &MIB, int FI, int Offset = 0) {
  MachineInstr *MI = MIB;
  MachineFunction &MF = *MI->getParent()->getParent();
  MachineFrameInfo &MFI = MF.getFrameInfo();
  const MCInstrDesc &MCID = MI->getDesc();
  auto Flags = MachineMemOperand::MONone;
  if (MCID.mayLoad())
    Flags |= MachineMemOperand::MOLoad;
  if (MCID.mayStore())
    Flags |= MachineMemOperand::MOStore;
  MachineMemOperand *MMO = MF.getMachineMemOperand(
      MachinePointerInfo::getFixedStack(MF, FI, Offset), Flags,
      MFI.getObjectSize(FI), MFI.getObjectAlign(FI));
  return MIB.addMemOperand(MMO);
}
} // end namespace M68k
} // end namespace vm::core

#endif // LLVM_LIB_TARGET_M68K_M68KINSTRBUILDER_H
