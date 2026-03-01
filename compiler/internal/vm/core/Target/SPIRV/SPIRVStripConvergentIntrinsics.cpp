//===-- SPIRVStripConvergentIntrinsics.cpp ----------------------*- C++ -*-===//
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
// This pass trims convergence intrinsics as those were only useful when
// modifying the CFG during IR passes.
//
//===----------------------------------------------------------------------===//

#include "SPIRV.h"
#include "SPIRVSubtarget.h"
#include "SPIRVUtils.h"
#include "vm/core/IR/IntrinsicInst.h"
#include "vm/core/IR/Intrinsics.h"
#include "vm/core/Transforms/Utils/Cloning.h"
#include "vm/core/Transforms/Utils/LowerMemIntrinsics.h"

using namespace vm::core;

namespace {
class SPIRVStripConvergentIntrinsics : public FunctionPass {
public:
  static char ID;

  SPIRVStripConvergentIntrinsics() : FunctionPass(ID) {}

  bool runOnFunction(Function &F) override {
    DenseSet<Instruction *> ToRemove;

    // Is the instruction is a convergent intrinsic, add it to kill-list and
    // returns true. Returns false otherwise.
    auto CleanupIntrinsic = [&](IntrinsicInst *II) {
      if (II->getIntrinsicID() != Intrinsic::experimental_convergence_entry &&
          II->getIntrinsicID() != Intrinsic::experimental_convergence_loop &&
          II->getIntrinsicID() != Intrinsic::experimental_convergence_anchor)
        return false;

      II->replaceAllUsesWith(UndefValue::get(II->getType()));
      ToRemove.insert(II);
      return true;
    };

    // Replace the given CallInst by a similar CallInst with no convergencectrl
    // attribute.
    auto CleanupCall = [&](CallInst *CI) {
      auto OB = CI->getOperandBundle(LLVMContext::OB_convergencectrl);
      if (!OB.has_value())
        return;

      auto *NewCall = CallBase::removeOperandBundle(
          CI, LLVMContext::OB_convergencectrl, CI->getIterator());
      NewCall->copyMetadata(*CI);
      CI->replaceAllUsesWith(NewCall);
      ToRemove.insert(CI);
    };

    for (BasicBlock &BB : F) {
      for (Instruction &I : BB) {
        if (auto *II = dyn_cast<IntrinsicInst>(&I))
          if (CleanupIntrinsic(II))
            continue;
        if (auto *CI = dyn_cast<CallInst>(&I))
          CleanupCall(CI);
      }
    }

    // All usages must be removed before their definition is removed.
    for (Instruction *I : ToRemove)
      I->eraseFromParent();

    return ToRemove.size() != 0;
  }
};
} // namespace

char SPIRVStripConvergentIntrinsics::ID = 0;
INITIALIZE_PASS(SPIRVStripConvergentIntrinsics, "strip-convergent-intrinsics",
                "SPIRV strip convergent intrinsics", false, false)

FunctionPass *toolchain::createSPIRVStripConvergenceIntrinsicsPass() {
  return new SPIRVStripConvergentIntrinsics();
}
