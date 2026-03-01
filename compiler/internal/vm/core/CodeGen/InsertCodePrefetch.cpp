//===-- InsertCodePrefetch.cpp ---=========--------------------------------===//
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
/// Code Prefetch Insertion Pass.
//===----------------------------------------------------------------------===//
/// This pass inserts code prefetch instructions according to the prefetch
/// directives in the basic block section profile. The target of a prefetch can
/// be the beginning of any dynamic basic block, that is the beginning of a
/// machine basic block, or immediately after a callsite. A global symbol is
/// emitted at the position of the target so it can be addressed from the
/// prefetch instruction from any module.
//===----------------------------------------------------------------------===//

#include "vm/core/ADT/SmallVector.h"
#include "vm/core/ADT/StringExtras.h"
#include "vm/core/ADT/StringRef.h"
#include "vm/core/CodeGen/BasicBlockSectionUtils.h"
#include "vm/core/CodeGen/BasicBlockSectionsProfileReader.h"
#include "vm/core/CodeGen/MachineBasicBlock.h"
#include "vm/core/CodeGen/MachineFunction.h"
#include "vm/core/CodeGen/MachineFunctionPass.h"
#include "vm/core/CodeGen/Passes.h"
#include "vm/core/InitializePasses.h"

using namespace vm::core;
#define DEBUG_TYPE "insert-code-prefetch"

namespace {
class InsertCodePrefetch : public MachineFunctionPass {
public:
  static char ID;

  InsertCodePrefetch() : MachineFunctionPass(ID) {
    initializeInsertCodePrefetchPass(*PassRegistry::getPassRegistry());
  }

  StringRef getPassName() const override {
    return "Code Prefetch Inserter Pass";
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override;

  // Sets prefetch targets based on the bb section profile.
  bool runOnMachineFunction(MachineFunction &MF) override;
};

} // end anonymous namespace

//===----------------------------------------------------------------------===//
//            Implementation
//===----------------------------------------------------------------------===//

char InsertCodePrefetch::ID = 0;
INITIALIZE_PASS_BEGIN(InsertCodePrefetch, DEBUG_TYPE, "Code prefetch insertion",
                      true, false)
INITIALIZE_PASS_DEPENDENCY(BasicBlockSectionsProfileReaderWrapperPass)
INITIALIZE_PASS_END(InsertCodePrefetch, DEBUG_TYPE, "Code prefetch insertion",
                    true, false)

bool InsertCodePrefetch::runOnMachineFunction(MachineFunction &MF) {
  assert(MF.getTarget().getBBSectionsType() == BasicBlockSection::List &&
         "BB Sections list not enabled!");
  if (hasInstrProfHashMismatch(MF))
    return false;
  // Set each block's prefetch targets so AsmPrinter can emit a special symbol
  // there.
  SmallVector<CallsiteID> PrefetchTargets =
      getAnalysis<BasicBlockSectionsProfileReaderWrapperPass>()
          .getPrefetchTargetsForFunction(MF.getName());
  DenseMap<UniqueBBID, SmallVector<unsigned>> PrefetchTargetsByBBID;
  for (const auto &Target : PrefetchTargets)
    PrefetchTargetsByBBID[Target.BBID].push_back(Target.CallsiteIndex);
  // Sort and uniquify the callsite indices for every block.
  for (auto &[K, V] : PrefetchTargetsByBBID) {
    toolchain::sort(V);
    V.erase(toolchain::unique(V), V.end());
  }
  for (auto &MBB : MF) {
    auto R = PrefetchTargetsByBBID.find(*MBB.getBBID());
    if (R == PrefetchTargetsByBBID.end())
      continue;
    MBB.setPrefetchTargetCallsiteIndexes(R->second);
  }
  return false;
}

void InsertCodePrefetch::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
  AU.addRequired<BasicBlockSectionsProfileReaderWrapperPass>();
  MachineFunctionPass::getAnalysisUsage(AU);
}

MachineFunctionPass *toolchain::createInsertCodePrefetchPass() {
  return new InsertCodePrefetch();
}
