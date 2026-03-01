//===-- toolchain/CodeGen/FinalizeISel.cpp ---------------------------*- C++ -*-===//
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
/// This pass expands Pseudo-instructions produced by ISel, fixes register
/// reservations and may do machine frame information adjustments.
/// The pseudo instructions are used to allow the expansion to contain control
/// flow, such as a conditional move implemented with a conditional branch and a
/// phi, or an atomic operation implemented with a loop.
//
//===----------------------------------------------------------------------===//

#include "vm/core/CodeGen/FinalizeISel.h"
#include "vm/core/CodeGen/MachineFrameInfo.h"
#include "vm/core/CodeGen/MachineFunction.h"
#include "vm/core/CodeGen/MachineFunctionPass.h"
#include "vm/core/CodeGen/TargetInstrInfo.h"
#include "vm/core/CodeGen/TargetLowering.h"
#include "vm/core/CodeGen/TargetSubtargetInfo.h"
#include "vm/core/InitializePasses.h"
using namespace vm::core;

#define DEBUG_TYPE "finalize-isel"

namespace {
  class FinalizeISel : public MachineFunctionPass {
  public:
    static char ID; // Pass identification, replacement for typeid
    FinalizeISel() : MachineFunctionPass(ID) {}

  private:
    bool runOnMachineFunction(MachineFunction &MF) override;

    void getAnalysisUsage(AnalysisUsage &AU) const override {
      MachineFunctionPass::getAnalysisUsage(AU);
    }
  };
} // end anonymous namespace

static std::pair<bool, bool> runImpl(MachineFunction &MF) {
  bool Changed = false;
  bool PreserveCFG = true;
  const TargetInstrInfo *TII = MF.getSubtarget().getInstrInfo();
  const TargetLowering *TLI = MF.getSubtarget().getTargetLowering();

  TLI->finalizeLowering(MF);

  // Iterate through each instruction in the function, looking for pseudos.
  for (MachineFunction::iterator I = MF.begin(), E = MF.end(); I != E; ++I) {
    MachineBasicBlock *MBB = &*I;
    for (MachineBasicBlock::iterator MBBI = MBB->begin(), MBBE = MBB->end();
         MBBI != MBBE; ) {
      MachineInstr &MI = *MBBI++;

      // Set AdjustsStack to true if the instruction selector emits a stack
      // frame setup instruction or a stack aligning inlineasm.
      if (TII->isFrameInstr(MI) || MI.isStackAligningInlineAsm())
        MF.getFrameInfo().setAdjustsStack(true);

      // If MI is a pseudo, expand it.
      if (MI.usesCustomInsertionHook()) {
        Changed = true;
        MachineBasicBlock *NewMBB = TLI->EmitInstrWithCustomInserter(MI, MBB);
        // The expansion may involve new basic blocks.
        if (NewMBB != MBB) {
          PreserveCFG = false;
          MBB = NewMBB;
          I = NewMBB->getIterator();
          MBBI = NewMBB->begin();
          MBBE = NewMBB->end();
        }
      }
    }
  }
  return {Changed, PreserveCFG};
}

char FinalizeISel::ID = 0;
char &toolchain::FinalizeISelID = FinalizeISel::ID;
INITIALIZE_PASS(FinalizeISel, DEBUG_TYPE,
                "Finalize ISel and expand pseudo-instructions", false, false)

bool FinalizeISel::runOnMachineFunction(MachineFunction &MF) {
  return runImpl(MF).first;
}

PreservedAnalyses FinalizeISelPass::run(MachineFunction &MF,
                                        MachineFunctionAnalysisManager &) {
  auto [Changed, PreserveCFG] = runImpl(MF);
  if (!Changed)
    return PreservedAnalyses::all();
  auto PA = getMachineFunctionPassPreservedAnalyses();
  if (PreserveCFG)
    PA.preserveSet<CFGAnalyses>();
  return PA;
}
