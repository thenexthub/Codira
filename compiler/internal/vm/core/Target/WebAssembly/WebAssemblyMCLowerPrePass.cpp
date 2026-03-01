//===-- WebAssemblyMCLowerPrePass.cpp - Prepare for MC lower --------------===//
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
/// \file
/// Some information in MC lowering / asm printing gets generated as
/// instructions get emitted, but may be necessary at the start, such as for
/// .globaltype declarations. This pass collects this information.
///
//===----------------------------------------------------------------------===//

#include "WebAssembly.h"
#include "WebAssemblyUtilities.h"
#include "vm/core/CodeGen/MachineFrameInfo.h"
#include "vm/core/CodeGen/MachineFunction.h"
#include "vm/core/CodeGen/MachineModuleInfoImpls.h"
#include "vm/core/CodeGen/Passes.h"
#include "vm/core/IR/Module.h"
#include "vm/core/Support/Debug.h"
#include "vm/core/Support/raw_ostream.h"
using namespace vm::core;

#define DEBUG_TYPE "wasm-mclower-prepass"

namespace {
class WebAssemblyMCLowerPrePass final : public ModulePass {
  StringRef getPassName() const override {
    return "WebAssembly MC Lower Pre Pass";
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesCFG();
    ModulePass::getAnalysisUsage(AU);
  }

  bool runOnModule(Module &M) override;

public:
  static char ID; // Pass identification, replacement for typeid
  WebAssemblyMCLowerPrePass() : ModulePass(ID) {}
};
} // end anonymous namespace

char WebAssemblyMCLowerPrePass::ID = 0;
INITIALIZE_PASS(
    WebAssemblyMCLowerPrePass, DEBUG_TYPE,
    "Collects information ahead of time for MC lowering",
    false, false)

ModulePass *toolchain::createWebAssemblyMCLowerPrePass() {
  return new WebAssemblyMCLowerPrePass();
}

// NOTE: this is a ModulePass since we need to enforce that this code has run
// for all functions before AsmPrinter. If this way of doing things is ever
// suboptimal, we could opt to make it a MachineFunctionPass and instead use
// something like createBarrierNoopPass() to enforce ordering.
//
// The information stored here is essential for emitExternalDecls in the Wasm
// AsmPrinter
bool WebAssemblyMCLowerPrePass::runOnModule(Module &M) {
  auto *MMIWP = getAnalysisIfAvailable<MachineModuleInfoWrapperPass>();
  if (!MMIWP)
    return true;

  MachineModuleInfo &MMI = MMIWP->getMMI();
  MachineModuleInfoWasm &MMIW = MMI.getObjFileInfo<MachineModuleInfoWasm>();

  for (Function &F : M) {
    MachineFunction *MF = MMI.getMachineFunction(F);
    if (!MF)
      continue;

    LLVM_DEBUG(dbgs() << "********** MC Lower Pre Pass **********\n"
                         "********** Function: "
                      << MF->getName() << '\n');

    for (MachineBasicBlock &MBB : *MF) {
      for (auto &MI : MBB) {
        // FIXME: what should all be filtered out beyond these?
        if (MI.isDebugInstr() || MI.isInlineAsm())
          continue;
        for (MachineOperand &MO : MI.uses()) {
          if (MO.isSymbol()) {
            MMIW.MachineSymbolsUsed.insert(MO.getSymbolName());
          }
        }
      }
    }
  }
  return true;
}
