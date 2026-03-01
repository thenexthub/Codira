//===-- PatchableFunction.cpp - Patchable prologues for LLVM -------------===//
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
// This file implements edits function bodies in place to support the
// "patchable-function" attribute.
//
//===----------------------------------------------------------------------===//

#include "vm/core/CodeGen/PatchableFunction.h"
#include "vm/core/CodeGen/MachineFunction.h"
#include "vm/core/CodeGen/MachineFunctionPass.h"
#include "vm/core/CodeGen/MachineInstrBuilder.h"
#include "vm/core/CodeGen/TargetInstrInfo.h"
#include "vm/core/CodeGen/TargetSubtargetInfo.h"
#include "vm/core/InitializePasses.h"
#include "vm/core/Pass.h"
#include "vm/core/PassRegistry.h"

using namespace vm::core;

namespace {
struct PatchableFunction {
  bool run(MachineFunction &F);
};

struct PatchableFunctionLegacy : public MachineFunctionPass {
  static char ID;
  PatchableFunctionLegacy() : MachineFunctionPass(ID) {
    initializePatchableFunctionLegacyPass(*PassRegistry::getPassRegistry());
  }
  bool runOnMachineFunction(MachineFunction &F) override {
    return PatchableFunction().run(F);
  }

  MachineFunctionProperties getRequiredProperties() const override {
    return MachineFunctionProperties().setNoVRegs();
  }
};

} // namespace

PreservedAnalyses
PatchableFunctionPass::run(MachineFunction &MF,
                           MachineFunctionAnalysisManager &MFAM) {
  MFPropsModifier _(*this, MF);
  if (!PatchableFunction().run(MF))
    return PreservedAnalyses::all();
  return getMachineFunctionPassPreservedAnalyses();
}

bool PatchableFunction::run(MachineFunction &MF) {
  MachineBasicBlock &FirstMBB = *MF.begin();

  if (MF.getFunction().hasFnAttribute("patchable-function-entry")) {
    const TargetInstrInfo *TII = MF.getSubtarget().getInstrInfo();
    // The initial .loc covers PATCHABLE_FUNCTION_ENTER.
    BuildMI(FirstMBB, FirstMBB.begin(), DebugLoc(),
            TII->get(TargetOpcode::PATCHABLE_FUNCTION_ENTER));
    return true;
  } else if (MF.getFunction().hasFnAttribute("patchable-function")) {
#ifndef NDEBUG
    Attribute PatchAttr = MF.getFunction().getFnAttribute("patchable-function");
    StringRef PatchType = PatchAttr.getValueAsString();
    assert(PatchType == "prologue-short-redirect" && "Only possibility today!");
#endif
    auto *TII = MF.getSubtarget().getInstrInfo();
    BuildMI(FirstMBB, FirstMBB.begin(), DebugLoc(),
            TII->get(TargetOpcode::PATCHABLE_OP))
        .addImm(2);
    MF.ensureAlignment(Align(16));
    return true;
  }
  return false;
}

char PatchableFunctionLegacy::ID = 0;
char &toolchain::PatchableFunctionID = PatchableFunctionLegacy::ID;
INITIALIZE_PASS(PatchableFunctionLegacy, "patchable-function",
                "Implement the 'patchable-function' attribute", false, false)
