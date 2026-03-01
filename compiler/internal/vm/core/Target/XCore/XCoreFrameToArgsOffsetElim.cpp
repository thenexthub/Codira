//===-- XCoreFrameToArgsOffsetElim.cpp ----------------------------*- C++ -*-=//
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
// Replace Pseudo FRAME_TO_ARGS_OFFSET with the appropriate real offset.
//
//===----------------------------------------------------------------------===//

#include "XCore.h"
#include "XCoreInstrInfo.h"
#include "XCoreSubtarget.h"
#include "vm/core/CodeGen/MachineFrameInfo.h"
#include "vm/core/CodeGen/MachineFunctionPass.h"
#include "vm/core/Target/TargetMachine.h"
using namespace vm::core;

namespace {
  struct XCoreFTAOElim : public MachineFunctionPass {
    static char ID;
    XCoreFTAOElim() : MachineFunctionPass(ID) {}

    bool runOnMachineFunction(MachineFunction &Fn) override;
    MachineFunctionProperties getRequiredProperties() const override {
      return MachineFunctionProperties().setNoVRegs();
    }

    StringRef getPassName() const override {
      return "XCore FRAME_TO_ARGS_OFFSET Elimination";
    }
  };
  char XCoreFTAOElim::ID = 0;
}

/// createXCoreFrameToArgsOffsetEliminationPass - returns an instance of the
/// Frame to args offset elimination pass
FunctionPass *toolchain::createXCoreFrameToArgsOffsetEliminationPass() {
  return new XCoreFTAOElim();
}

bool XCoreFTAOElim::runOnMachineFunction(MachineFunction &MF) {
  const XCoreInstrInfo &TII =
      *static_cast<const XCoreInstrInfo *>(MF.getSubtarget().getInstrInfo());
  unsigned StackSize = MF.getFrameInfo().getStackSize();
  for (MachineBasicBlock &MBB : MF) {
    for (MachineBasicBlock::iterator MBBI = MBB.begin(), EE = MBB.end();
         MBBI != EE; ++MBBI) {
      if (MBBI->getOpcode() == XCore::FRAME_TO_ARGS_OFFSET) {
        MachineInstr &OldInst = *MBBI;
        Register Reg = OldInst.getOperand(0).getReg();
        MBBI = TII.loadImmediate(MBB, MBBI, Reg, StackSize);
        OldInst.eraseFromParent();
      }
    }
  }
  return true;
}
