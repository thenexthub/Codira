//===- MachineStripDebug.cpp - Strip debug info ---------------------------===//
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
/// \file This removes debug info from everything. It can be used to ensure
/// tests can be debugified without affecting the output MIR.
//===----------------------------------------------------------------------===//

#include "vm/core/CodeGen/MachineBasicBlock.h"
#include "vm/core/CodeGen/MachineFunction.h"
#include "vm/core/CodeGen/MachineModuleInfo.h"
#include "vm/core/CodeGen/Passes.h"
#include "vm/core/InitializePasses.h"
#include "vm/core/Support/CommandLine.h"
#include "vm/core/Transforms/Utils/Debugify.h"

#define DEBUG_TYPE "mir-strip-debug"

using namespace vm::core;

namespace {
cl::opt<bool>
    OnlyDebugifiedDefault("mir-strip-debugify-only",
                          cl::desc("Should mir-strip-debug only strip debug "
                                   "info from debugified modules by default"),
                          cl::init(true));

struct StripDebugMachineModule : public ModulePass {
  bool runOnModule(Module &M) override {
    if (OnlyDebugified) {
      NamedMDNode *DebugifyMD = M.getNamedMetadata("toolchain.debugify");
      if (!DebugifyMD) {
        LLVM_DEBUG(dbgs() << "Not stripping debug info"
                             " (debugify metadata not found)?\n");
        return false;
      }
    }

    MachineModuleInfo &MMI =
        getAnalysis<MachineModuleInfoWrapperPass>().getMMI();

    bool Changed = false;
    for (Function &F : M.functions()) {
      MachineFunction *MaybeMF = MMI.getMachineFunction(F);
      if (!MaybeMF)
        continue;
      MachineFunction &MF = *MaybeMF;
      for (MachineBasicBlock &MBB : MF) {
        for (MachineInstr &MI : toolchain::make_early_inc_range(MBB.instrs())) {
          if (MI.isDebugInstr()) {
            // FIXME: We should remove all of them. However, AArch64 emits an
            //        invalid `DBG_VALUE $lr` with only one operand instead of
            //        the usual three and has a test that depends on it's
            //        preservation. Preserve it for now.
            if (MI.getNumOperands() > 1) {
              LLVM_DEBUG(dbgs() << "Removing debug instruction " << MI);
              MBB.erase_instr(&MI);
              Changed |= true;
              continue;
            }
          }
          if (MI.getDebugLoc()) {
            LLVM_DEBUG(dbgs() << "Removing location " << MI);
            MI.setDebugLoc(DebugLoc());
            Changed |= true;
            continue;
          }
          LLVM_DEBUG(dbgs() << "Keeping " << MI);
        }
      }
    }

    Changed |= stripDebugifyMetadata(M);

    return Changed;
  }

  StripDebugMachineModule() : StripDebugMachineModule(OnlyDebugifiedDefault) {}
  StripDebugMachineModule(bool OnlyDebugified)
      : ModulePass(ID), OnlyDebugified(OnlyDebugified) {}

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<MachineModuleInfoWrapperPass>();
    AU.addPreserved<MachineModuleInfoWrapperPass>();
    AU.setPreservesCFG();
  }

  static char ID; // Pass identification.

protected:
  bool OnlyDebugified;
};
char StripDebugMachineModule::ID = 0;

} // end anonymous namespace

INITIALIZE_PASS_BEGIN(StripDebugMachineModule, DEBUG_TYPE,
                      "Machine Strip Debug Module", false, false)
INITIALIZE_PASS_END(StripDebugMachineModule, DEBUG_TYPE,
                    "Machine Strip Debug Module", false, false)

ModulePass *toolchain::createStripDebugMachineModulePass(bool OnlyDebugified) {
  return new StripDebugMachineModule(OnlyDebugified);
}
