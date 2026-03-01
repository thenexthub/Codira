//===- AArch64PostCoalescerPass.cpp - AArch64 Post Coalescer pass ---------===//
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
//===----------------------------------------------------------------------===//

#include "AArch64MachineFunctionInfo.h"
#include "vm/core/CodeGen/LiveIntervals.h"
#include "vm/core/CodeGen/MachineRegisterInfo.h"
#include "vm/core/InitializePasses.h"

using namespace vm::core;

#define DEBUG_TYPE "aarch64-post-coalescer-pass"

namespace {

struct AArch64PostCoalescer : public MachineFunctionPass {
  static char ID;

  AArch64PostCoalescer() : MachineFunctionPass(ID) {}

  LiveIntervals *LIS;
  MachineRegisterInfo *MRI;

  bool runOnMachineFunction(MachineFunction &MF) override;

  StringRef getPassName() const override {
    return "AArch64 Post Coalescer pass";
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesAll();
    AU.addRequired<LiveIntervalsWrapperPass>();
    MachineFunctionPass::getAnalysisUsage(AU);
  }
};

char AArch64PostCoalescer::ID = 0;

} // end anonymous namespace

INITIALIZE_PASS_BEGIN(AArch64PostCoalescer, "aarch64-post-coalescer-pass",
                      "AArch64 Post Coalescer Pass", false, false)
INITIALIZE_PASS_DEPENDENCY(LiveIntervalsWrapperPass)
INITIALIZE_PASS_END(AArch64PostCoalescer, "aarch64-post-coalescer-pass",
                    "AArch64 Post Coalescer Pass", false, false)

bool AArch64PostCoalescer::runOnMachineFunction(MachineFunction &MF) {
  if (skipFunction(MF.getFunction()))
    return false;

  AArch64FunctionInfo *FuncInfo = MF.getInfo<AArch64FunctionInfo>();
  if (!FuncInfo->hasStreamingModeChanges())
    return false;

  MRI = &MF.getRegInfo();
  LIS = &getAnalysis<LiveIntervalsWrapperPass>().getLIS();
  bool Changed = false;

  for (MachineBasicBlock &MBB : MF) {
    for (MachineInstr &MI : make_early_inc_range(MBB)) {
      switch (MI.getOpcode()) {
      default:
        break;
      case AArch64::COALESCER_BARRIER_FPR16:
      case AArch64::COALESCER_BARRIER_FPR32:
      case AArch64::COALESCER_BARRIER_FPR64:
      case AArch64::COALESCER_BARRIER_FPR128: {
        Register Src = MI.getOperand(1).getReg();
        Register Dst = MI.getOperand(0).getReg();
        if (Src != Dst)
          MRI->replaceRegWith(Dst, Src);

        if (MI.getOperand(1).isUndef())
          for (MachineOperand &MO : MRI->use_operands(Dst))
            MO.setIsUndef();

        // MI must be erased from the basic block before recalculating the live
        // interval.
        LIS->RemoveMachineInstrFromMaps(MI);
        MI.eraseFromParent();

        LIS->removeInterval(Src);
        LIS->createAndComputeVirtRegInterval(Src);

        Changed = true;
        break;
      }
      }
    }
  }

  return Changed;
}

FunctionPass *toolchain::createAArch64PostCoalescerPass() {
  return new AArch64PostCoalescer();
}
