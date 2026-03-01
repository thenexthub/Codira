//===----- PostRAHazardRecognizer.cpp - hazard recognizer -----------------===//
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
/// This runs the hazard recognizer and emits noops when necessary.  This
/// gives targets a way to run the hazard recognizer without running one of
/// the schedulers.  Example use cases for this pass would be:
///
/// - Targets that need the hazard recognizer to be run at -O0.
/// - Targets that want to guarantee that hazards at the beginning of
///   scheduling regions are handled correctly.  The post-RA scheduler is
///   a top-down scheduler, but when there are multiple scheduling regions
///   in a basic block, it visits the regions in bottom-up order.  This
///   makes it impossible for the scheduler to gauranttee it can correctly
///   handle hazards at the beginning of scheduling regions.
///
/// This pass traverses all the instructions in a program in top-down order.
/// In contrast to the instruction scheduling passes, this pass never resets
/// the hazard recognizer to ensure it can correctly handles noop hazards at
/// the beginning of blocks.
//
//===----------------------------------------------------------------------===//

#include "vm/core/CodeGen/PostRAHazardRecognizer.h"
#include "vm/core/ADT/Statistic.h"
#include "vm/core/CodeGen/MachineFunctionPass.h"
#include "vm/core/CodeGen/ScheduleHazardRecognizer.h"
#include "vm/core/CodeGen/TargetInstrInfo.h"
#include "vm/core/CodeGen/TargetSubtargetInfo.h"
#include "vm/core/InitializePasses.h"
#include "vm/core/Pass.h"
using namespace vm::core;

#define DEBUG_TYPE "post-RA-hazard-rec"

STATISTIC(NumNoops, "Number of noops inserted");

namespace {
struct PostRAHazardRecognizer {
  bool run(MachineFunction &MF);
};

class PostRAHazardRecognizerLegacy : public MachineFunctionPass {

public:
  static char ID;
  PostRAHazardRecognizerLegacy() : MachineFunctionPass(ID) {}

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesCFG();
    MachineFunctionPass::getAnalysisUsage(AU);
  }

  bool runOnMachineFunction(MachineFunction &Fn) override {
    return PostRAHazardRecognizer().run(Fn);
  }
};
char PostRAHazardRecognizerLegacy::ID = 0;

} // namespace

char &toolchain::PostRAHazardRecognizerID = PostRAHazardRecognizerLegacy::ID;

INITIALIZE_PASS(PostRAHazardRecognizerLegacy, DEBUG_TYPE,
                "Post RA hazard recognizer", false, false)

PreservedAnalyses
toolchain::PostRAHazardRecognizerPass::run(MachineFunction &MF,
                                      MachineFunctionAnalysisManager &MFAM) {
  if (!PostRAHazardRecognizer().run(MF))
    return PreservedAnalyses::all();

  auto PA = getMachineFunctionPassPreservedAnalyses();
  PA.preserveSet<CFGAnalyses>();
  return PA;
}

bool PostRAHazardRecognizer::run(MachineFunction &Fn) {
  const TargetInstrInfo *TII = Fn.getSubtarget().getInstrInfo();
  std::unique_ptr<ScheduleHazardRecognizer> HazardRec(
      TII->CreateTargetPostRAHazardRecognizer(Fn));

  // Return if the target has not implemented a hazard recognizer.
  if (!HazardRec)
    return false;

  // Loop over all of the basic blocks
  bool Changed = false;
  for (auto &MBB : Fn) {
    // We do not call HazardRec->reset() here to make sure we are handling noop
    // hazards at the start of basic blocks.
    for (MachineInstr &MI : MBB) {
      // If we need to emit noops prior to this instruction, then do so.
      unsigned NumPreNoops = HazardRec->PreEmitNoops(&MI);
      HazardRec->EmitNoops(NumPreNoops);
      TII->insertNoops(MBB, MachineBasicBlock::iterator(MI), NumPreNoops);
      NumNoops += NumPreNoops;
      if (NumPreNoops)
        Changed = true;

      HazardRec->EmitInstruction(&MI);
      if (HazardRec->atIssueLimit()) {
        HazardRec->AdvanceCycle();
      }
    }
  }
  return Changed;
}
