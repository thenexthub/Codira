//===-- SIFixVGPRCopies.cpp - Fix VGPR Copies after regalloc --------------===//
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
/// \file
/// Add implicit use of exec to vector register copies.
///
//===----------------------------------------------------------------------===//

#include "SIFixVGPRCopies.h"
#include "AMDGPU.h"
#include "GCNSubtarget.h"
#include "MCTargetDesc/AMDGPUMCTargetDesc.h"
#include "vm/core/CodeGen/MachineFunctionPass.h"

using namespace vm::core;

#define DEBUG_TYPE "si-fix-vgpr-copies"

namespace {

class SIFixVGPRCopiesLegacy : public MachineFunctionPass {
public:
  static char ID;

  SIFixVGPRCopiesLegacy() : MachineFunctionPass(ID) {
    initializeSIFixVGPRCopiesLegacyPass(*PassRegistry::getPassRegistry());
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesAll();
    MachineFunctionPass::getAnalysisUsage(AU);
  }

  bool runOnMachineFunction(MachineFunction &MF) override;

  StringRef getPassName() const override { return "SI Fix VGPR copies"; }
};

class SIFixVGPRCopies {
public:
  bool run(MachineFunction &MF);
};

} // End anonymous namespace.

INITIALIZE_PASS(SIFixVGPRCopiesLegacy, DEBUG_TYPE, "SI Fix VGPR copies", false,
                false)

char SIFixVGPRCopiesLegacy::ID = 0;

char &toolchain::SIFixVGPRCopiesID = SIFixVGPRCopiesLegacy::ID;

PreservedAnalyses SIFixVGPRCopiesPass::run(MachineFunction &MF,
                                           MachineFunctionAnalysisManager &) {
  SIFixVGPRCopies().run(MF);
  return PreservedAnalyses::all();
}

bool SIFixVGPRCopiesLegacy::runOnMachineFunction(MachineFunction &MF) {
  return SIFixVGPRCopies().run(MF);
}

bool SIFixVGPRCopies::run(MachineFunction &MF) {
  const GCNSubtarget &ST = MF.getSubtarget<GCNSubtarget>();
  const SIRegisterInfo *TRI = ST.getRegisterInfo();
  const SIInstrInfo *TII = ST.getInstrInfo();
  bool Changed = false;

  for (MachineBasicBlock &MBB : MF) {
    for (MachineInstr &MI : MBB) {
      switch (MI.getOpcode()) {
      case AMDGPU::COPY:
        if (TII->isVGPRCopy(MI) && !MI.readsRegister(AMDGPU::EXEC, TRI)) {
          MI.addOperand(MF,
                        MachineOperand::CreateReg(AMDGPU::EXEC, false, true));
          LLVM_DEBUG(dbgs() << "Add exec use to " << MI);
          Changed = true;
        }
        break;
      default:
        break;
      }
    }
  }

  return Changed;
}
