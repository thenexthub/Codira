//===- MachineCheckDebugify.cpp - Check debug info ------------------------===//
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
/// \file This checks debug info after mir-debugify (+ pass-to-test). Currently
/// it simply checks the integrity of line info in DILocation and
/// DILocalVariable which mir-debugifiy generated before.
//===----------------------------------------------------------------------===//

#include "vm/core/ADT/StringExtras.h"
#include "vm/core/CodeGen/MachineBasicBlock.h"
#include "vm/core/CodeGen/MachineFunction.h"
#include "vm/core/CodeGen/MachineModuleInfo.h"
#include "vm/core/CodeGen/Passes.h"
#include "vm/core/IR/Constants.h"
#include "vm/core/IR/DebugInfoMetadata.h"
#include "vm/core/IR/Module.h"
#include "vm/core/InitializePasses.h"
#include "vm/core/Pass.h"

#define DEBUG_TYPE "mir-check-debugify"

using namespace vm::core;

namespace {

struct CheckDebugMachineModule : public ModulePass {
  bool runOnModule(Module &M) override {
    NamedMDNode *NMD = M.getNamedMetadata("toolchain.mir.debugify");
    if (!NMD) {
      errs() << "WARNING: Please run mir-debugify to generate "
                "toolchain.mir.debugify metadata first.\n";
      return false;
    }

    MachineModuleInfo &MMI =
        getAnalysis<MachineModuleInfoWrapperPass>().getMMI();

    auto getDebugifyOperand = [&](unsigned Idx) -> unsigned {
      return mdconst::extract<ConstantInt>(NMD->getOperand(Idx)->getOperand(0))
          ->getZExtValue();
    };
    assert(NMD->getNumOperands() == 2 &&
           "toolchain.mir.debugify should have exactly 2 operands!");
    unsigned NumLines = getDebugifyOperand(0);
    unsigned NumVars = getDebugifyOperand(1);
    BitVector MissingLines{NumLines, true};
    BitVector MissingVars{NumVars, true};

    for (Function &F : M.functions()) {
      MachineFunction *MF = MMI.getMachineFunction(F);
      if (!MF)
        continue;
      for (MachineBasicBlock &MBB : *MF) {
        // Find missing lines.
        // TODO: Avoid meta instructions other than dbg_val.
        for (MachineInstr &MI : MBB) {
          if (MI.isDebugValue())
            continue;
          const DebugLoc DL = MI.getDebugLoc();
          if (DL && DL.getLine() != 0) {
            MissingLines.reset(DL.getLine() - 1);
            continue;
          }

          if (!DL) {
            errs() << "WARNING: Instruction with empty DebugLoc in function ";
            errs() << F.getName() << " --";
            MI.print(errs());
          }
        }

        // Find missing variables.
        // TODO: Handle DBG_INSTR_REF which is under an experimental option now.
        for (MachineInstr &MI : MBB) {
          if (!MI.isDebugValue())
            continue;
          const DILocalVariable *LocalVar = MI.getDebugVariable();
          unsigned Var = ~0U;

          (void)to_integer(LocalVar->getName(), Var, 10);
          assert(Var <= NumVars && "Unexpected name for DILocalVariable");
          MissingVars.reset(Var - 1);
        }
      }
    }

    bool Fail = false;
    for (unsigned Idx : MissingLines.set_bits()) {
      errs() << "WARNING: Missing line " << Idx + 1 << "\n";
      Fail = true;
    }

    for (unsigned Idx : MissingVars.set_bits()) {
      errs() << "WARNING: Missing variable " << Idx + 1 << "\n";
      Fail = true;
    }
    errs() << "Machine IR debug info check: ";
    errs() << (Fail ? "FAIL" : "PASS") << "\n";

    return false;
  }

  CheckDebugMachineModule() : ModulePass(ID) {}

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<MachineModuleInfoWrapperPass>();
    AU.setPreservesAll();
  }

  static char ID; // Pass identification.
};
char CheckDebugMachineModule::ID = 0;

} // end anonymous namespace

INITIALIZE_PASS_BEGIN(CheckDebugMachineModule, DEBUG_TYPE,
                      "Machine Check Debug Module", false, false)
INITIALIZE_PASS_END(CheckDebugMachineModule, DEBUG_TYPE,
                    "Machine Check Debug Module", false, false)

ModulePass *toolchain::createCheckDebugMachineModulePass() {
  return new CheckDebugMachineModule();
}
