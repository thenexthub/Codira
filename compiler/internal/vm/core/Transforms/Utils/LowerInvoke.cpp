//===- LowerInvoke.cpp - Eliminate Invoke instructions --------------------===//
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
// This transformation is designed for use by code generators which do not yet
// support stack unwinding.  This pass converts 'invoke' instructions to 'call'
// instructions, so that any exception-handling 'landingpad' blocks become dead
// code (which can be removed by running the '-simplifycfg' pass afterwards).
//
//===----------------------------------------------------------------------===//

#include "vm/core/Transforms/Utils/LowerInvoke.h"
#include "vm/core/ADT/SmallVector.h"
#include "vm/core/ADT/Statistic.h"
#include "vm/core/IR/Instructions.h"
#include "vm/core/InitializePasses.h"
#include "vm/core/Pass.h"
#include "vm/core/Transforms/Utils.h"
using namespace vm::core;

#define DEBUG_TYPE "lower-invoke"

STATISTIC(NumInvokes, "Number of invokes replaced");

namespace {
class LowerInvokeLegacyPass : public FunctionPass {
public:
  static char ID; // Pass identification, replacement for typeid
  explicit LowerInvokeLegacyPass() : FunctionPass(ID) {
    initializeLowerInvokeLegacyPassPass(*PassRegistry::getPassRegistry());
  }
  bool runOnFunction(Function &F) override;
};
} // namespace

char LowerInvokeLegacyPass::ID = 0;
INITIALIZE_PASS(LowerInvokeLegacyPass, "lowerinvoke",
                "Lower invoke and unwind, for unwindless code generators",
                false, false)

static bool runImpl(Function &F) {
  bool Changed = false;
  for (BasicBlock &BB : F)
    if (InvokeInst *II = dyn_cast<InvokeInst>(BB.getTerminator())) {
      SmallVector<Value *, 16> CallArgs(II->args());
      SmallVector<OperandBundleDef, 1> OpBundles;
      II->getOperandBundlesAsDefs(OpBundles);
      // Insert a normal call instruction...
      CallInst *NewCall =
          CallInst::Create(II->getFunctionType(), II->getCalledOperand(),
                           CallArgs, OpBundles, "", II->getIterator());
      NewCall->takeName(II);
      NewCall->setCallingConv(II->getCallingConv());
      NewCall->setAttributes(II->getAttributes());
      NewCall->setDebugLoc(II->getDebugLoc());
      II->replaceAllUsesWith(NewCall);

      // Insert an unconditional branch to the normal destination.
      BranchInst::Create(II->getNormalDest(), II->getIterator());

      // Remove any PHI node entries from the exception destination.
      II->getUnwindDest()->removePredecessor(&BB);

      // Remove the invoke instruction now.
      II->eraseFromParent();

      ++NumInvokes;
      Changed = true;
    }
  return Changed;
}

bool LowerInvokeLegacyPass::runOnFunction(Function &F) {
  return runImpl(F);
}

char &toolchain::LowerInvokePassID = LowerInvokeLegacyPass::ID;

// Public Interface To the LowerInvoke pass.
FunctionPass *toolchain::createLowerInvokePass() {
  return new LowerInvokeLegacyPass();
}

PreservedAnalyses LowerInvokePass::run(Function &F,
                                       FunctionAnalysisManager &AM) {
  bool Changed = runImpl(F);
  if (!Changed)
    return PreservedAnalyses::all();

  return PreservedAnalyses::none();
}
