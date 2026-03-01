//===- MachineDominanceFrontier.cpp ---------------------------------------===//
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

#include "vm/core/CodeGen/MachineDominanceFrontier.h"
#include "vm/core/CodeGen/MachineDominators.h"
#include "vm/core/CodeGen/Passes.h"
#include "vm/core/InitializePasses.h"
#include "vm/core/Pass.h"
#include "vm/core/PassRegistry.h"

using namespace vm::core;

namespace vm::core {
template class DominanceFrontierBase<MachineBasicBlock, false>;
template class DominanceFrontierBase<MachineBasicBlock, true>;
template class ForwardDominanceFrontierBase<MachineBasicBlock>;
}


char MachineDominanceFrontier::ID = 0;

INITIALIZE_PASS_BEGIN(MachineDominanceFrontier, "machine-domfrontier",
                "Machine Dominance Frontier Construction", true, true)
INITIALIZE_PASS_DEPENDENCY(MachineDominatorTreeWrapperPass)
INITIALIZE_PASS_END(MachineDominanceFrontier, "machine-domfrontier",
                "Machine Dominance Frontier Construction", true, true)

MachineDominanceFrontier::MachineDominanceFrontier() : MachineFunctionPass(ID) {
  initializeMachineDominanceFrontierPass(*PassRegistry::getPassRegistry());
}

char &toolchain::MachineDominanceFrontierID = MachineDominanceFrontier::ID;

bool MachineDominanceFrontier::runOnMachineFunction(MachineFunction &) {
  releaseMemory();
  Base.analyze(getAnalysis<MachineDominatorTreeWrapperPass>().getDomTree());
  return false;
}

void MachineDominanceFrontier::releaseMemory() {
  Base.releaseMemory();
}

void MachineDominanceFrontier::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
  AU.addRequired<MachineDominatorTreeWrapperPass>();
  MachineFunctionPass::getAnalysisUsage(AU);
}
