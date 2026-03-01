//-  X86Insertwait.cpp - Strict-Fp:Insert wait instruction X87 instructions --//
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
// This file defines the pass which insert x86 wait instructions after each
// X87 instructions when strict float is enabled.
//
// The logic to insert a wait instruction after an X87 instruction is as below:
// 1. If the X87 instruction don't raise float exception nor is a load/store
//    instruction, or is a x87 control instruction, don't insert wait.
// 2. If the X87 instruction is an instruction which the following instruction
//    is an X87 exception synchronizing X87 instruction, don't insert wait.
// 3. For other situations, insert wait instruction.
//
//===----------------------------------------------------------------------===//

#include "X86.h"
#include "X86InstrInfo.h"
#include "X86Subtarget.h"
#include "vm/core/CodeGen/MachineBasicBlock.h"
#include "vm/core/CodeGen/MachineFunction.h"
#include "vm/core/CodeGen/MachineFunctionPass.h"
#include "vm/core/CodeGen/MachineInstr.h"
#include "vm/core/CodeGen/MachineInstrBuilder.h"
#include "vm/core/CodeGen/MachineOperand.h"
#include "vm/core/IR/DebugLoc.h"
#include "vm/core/Support/Debug.h"

using namespace vm::core;

#define DEBUG_TYPE "x86-insert-wait"

namespace {

class WaitInsert : public MachineFunctionPass {
public:
  static char ID;

  WaitInsert() : MachineFunctionPass(ID) {}

  bool runOnMachineFunction(MachineFunction &MF) override;

  StringRef getPassName() const override {
    return "X86 insert wait instruction";
  }
};

} // namespace

char WaitInsert::ID = 0;

FunctionPass *toolchain::createX86InsertX87waitPass() { return new WaitInsert(); }

static bool isX87ControlInstruction(MachineInstr &MI) {
  switch (MI.getOpcode()) {
  case X86::FNINIT:
  case X86::FLDCW16m:
  case X86::FNSTCW16m:
  case X86::FNSTSW16r:
  case X86::FNSTSWm:
  case X86::FNCLEX:
  case X86::FLDENVm:
  case X86::FSTENVm:
  case X86::FRSTORm:
  case X86::FSAVEm:
  case X86::FINCSTP:
  case X86::FDECSTP:
  case X86::FFREE:
  case X86::FFREEP:
  case X86::FNOP:
  case X86::WAIT:
    return true;
  default:
    return false;
  }
}

static bool isX87NonWaitingControlInstruction(MachineInstr &MI) {
  // a few special control instructions don't perform a wait operation
  switch (MI.getOpcode()) {
  case X86::FNINIT:
  case X86::FNSTSW16r:
  case X86::FNSTSWm:
  case X86::FNSTCW16m:
  case X86::FNCLEX:
    return true;
  default:
    return false;
  }
}

bool WaitInsert::runOnMachineFunction(MachineFunction &MF) {
  if (!MF.getFunction().hasFnAttribute(Attribute::StrictFP))
    return false;

  const X86Subtarget &ST = MF.getSubtarget<X86Subtarget>();
  const X86InstrInfo *TII = ST.getInstrInfo();
  bool Changed = false;

  for (MachineBasicBlock &MBB : MF) {
    for (MachineBasicBlock::iterator MI = MBB.begin(); MI != MBB.end(); ++MI) {
      // Jump non X87 instruction.
      if (!X86::isX87Instruction(*MI))
        continue;
      // If the instruction instruction neither has float exception nor is
      // a load/store instruction, or the instruction is x87 control
      // instruction, do not insert wait.
      if (!(MI->mayRaiseFPException() || MI->mayLoadOrStore()) ||
          isX87ControlInstruction(*MI))
        continue;
      // If the following instruction is an X87 instruction and isn't an X87
      // non-waiting control instruction, we can omit insert wait instruction.
      MachineBasicBlock::iterator AfterMI = std::next(MI);
      if (AfterMI != MBB.end() && X86::isX87Instruction(*AfterMI) &&
          !isX87NonWaitingControlInstruction(*AfterMI))
        continue;

      BuildMI(MBB, AfterMI, MI->getDebugLoc(), TII->get(X86::WAIT));
      LLVM_DEBUG(dbgs() << "\nInsert wait after:\t" << *MI);
      // Jump the newly inserting wait
      ++MI;
      Changed = true;
    }
  }
  return Changed;
}
