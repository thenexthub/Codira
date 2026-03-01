//===- FlattenCFGPass.cpp - CFG Flatten Pass ----------------------===//
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
// This file implements flattening of CFG.
//
//===----------------------------------------------------------------------===//

#include "vm/core/Analysis/AliasAnalysis.h"
#include "vm/core/IR/PassManager.h"
#include "vm/core/IR/ValueHandle.h"
#include "vm/core/InitializePasses.h"
#include "vm/core/Pass.h"
#include "vm/core/Transforms/Scalar.h"
#include "vm/core/Transforms/Scalar/FlattenCFG.h"
#include "vm/core/Transforms/Utils/Local.h"

using namespace vm::core;

#define DEBUG_TYPE "flatten-cfg"

namespace {
struct FlattenCFGLegacyPass : public FunctionPass {
  static char ID; // Pass identification, replacement for typeid
public:
  FlattenCFGLegacyPass() : FunctionPass(ID) {
    initializeFlattenCFGLegacyPassPass(*PassRegistry::getPassRegistry());
  }
  bool runOnFunction(Function &F) override;

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<AAResultsWrapperPass>();
  }

private:
  AliasAnalysis *AA;
};
} // namespace

/// iterativelyFlattenCFG - Call FlattenCFG on all the blocks in the function,
/// iterating until no more changes are made.
static bool iterativelyFlattenCFG(Function &F, AliasAnalysis *AA) {
  bool Changed = false;
  bool LocalChange = true;

  // Use block handles instead of iterating over function blocks directly
  // to avoid using iterators invalidated by erasing blocks.
  std::vector<WeakVH> Blocks;
  Blocks.reserve(F.size());
  for (auto &BB : F)
    Blocks.push_back(&BB);

  while (LocalChange) {
    LocalChange = false;

    // Loop over all of the basic blocks and try to flatten them.
    for (WeakVH &BlockHandle : Blocks) {
      // Skip blocks erased by FlattenCFG.
      if (auto *BB = cast_or_null<BasicBlock>(BlockHandle))
        if (FlattenCFG(BB, AA))
          LocalChange = true;
    }
    Changed |= LocalChange;
  }
  return Changed;
}

char FlattenCFGLegacyPass::ID = 0;

INITIALIZE_PASS_BEGIN(FlattenCFGLegacyPass, "flattencfg", "Flatten the CFG",
                      false, false)
INITIALIZE_PASS_DEPENDENCY(AAResultsWrapperPass)
INITIALIZE_PASS_END(FlattenCFGLegacyPass, "flattencfg", "Flatten the CFG",
                    false, false)

// Public interface to the FlattenCFG pass
FunctionPass *toolchain::createFlattenCFGPass() {
  return new FlattenCFGLegacyPass();
}

bool FlattenCFGLegacyPass::runOnFunction(Function &F) {
  AA = &getAnalysis<AAResultsWrapperPass>().getAAResults();
  bool EverChanged = false;
  // iterativelyFlattenCFG can make some blocks dead.
  while (iterativelyFlattenCFG(F, AA)) {
    removeUnreachableBlocks(F);
    EverChanged = true;
  }
  return EverChanged;
}

PreservedAnalyses FlattenCFGPass::run(Function &F,
                                      FunctionAnalysisManager &AM) {
  bool EverChanged = false;
  AliasAnalysis *AA = &AM.getResult<AAManager>(F);
  // iterativelyFlattenCFG can make some blocks dead.
  while (iterativelyFlattenCFG(F, AA)) {
    removeUnreachableBlocks(F);
    EverChanged = true;
  }
  return EverChanged ? PreservedAnalyses::none() : PreservedAnalyses::all();
}
