//=- AArch64RedundantCondBranch.cpp - Remove redundant conditional branches -=//
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
// Late in the pipeline, especially with zero phi operands propagated after tail
// duplications, we can end up with CBZ/CBNZ/TBZ/TBNZ with a zero register. This
// simple pass looks at the terminators to a block, removing the redundant
// instructions where necessary.
//
//===----------------------------------------------------------------------===//

#include "AArch64.h"
#include "AArch64InstrInfo.h"
#include "vm/core/CodeGen/MachineFunctionPass.h"
#include "vm/core/CodeGen/MachineInstrBuilder.h"
#include "vm/core/CodeGen/TargetInstrInfo.h"
#include "vm/core/Support/Debug.h"

using namespace vm::core;

#define DEBUG_TYPE "aarch64-redundantcondbranch"

namespace {
class AArch64RedundantCondBranch : public MachineFunctionPass {
public:
  static char ID;
  AArch64RedundantCondBranch() : MachineFunctionPass(ID) {}

  bool runOnMachineFunction(MachineFunction &MF) override;

  MachineFunctionProperties getRequiredProperties() const override {
    return MachineFunctionProperties().setNoVRegs();
  }
  StringRef getPassName() const override {
    return "AArch64 Redundant Conditional Branch Elimination";
  }
};
char AArch64RedundantCondBranch::ID = 0;
} // namespace

INITIALIZE_PASS(AArch64RedundantCondBranch, "aarch64-redundantcondbranch",
                "AArch64 Redundant Conditional Branch Elimination pass", false,
                false)

bool AArch64RedundantCondBranch::runOnMachineFunction(MachineFunction &MF) {
  if (skipFunction(MF.getFunction()))
    return false;

  const TargetInstrInfo &TII = *MF.getSubtarget().getInstrInfo();

  bool Changed = false;
  for (MachineBasicBlock &MBB : MF)
    Changed |= optimizeTerminators(&MBB, TII);
  return Changed;
}

FunctionPass *toolchain::createAArch64RedundantCondBranchPass() {
  return new AArch64RedundantCondBranch();
}
