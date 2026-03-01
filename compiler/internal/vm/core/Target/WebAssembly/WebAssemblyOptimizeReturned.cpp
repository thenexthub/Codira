//===-- WebAssemblyOptimizeReturned.cpp - Optimize "returned" attributes --===//
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
/// Optimize calls with "returned" attributes for WebAssembly.
///
//===----------------------------------------------------------------------===//

#include "WebAssembly.h"
#include "vm/core/IR/Dominators.h"
#include "vm/core/IR/InstVisitor.h"
#include "vm/core/Support/Debug.h"
#include "vm/core/Support/raw_ostream.h"
using namespace vm::core;

#define DEBUG_TYPE "wasm-optimize-returned"

namespace {
class OptimizeReturned final : public FunctionPass,
                               public InstVisitor<OptimizeReturned> {
  StringRef getPassName() const override {
    return "WebAssembly Optimize Returned";
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesCFG();
    AU.addRequired<DominatorTreeWrapperPass>();
    AU.addPreserved<DominatorTreeWrapperPass>();
    FunctionPass::getAnalysisUsage(AU);
  }

  bool runOnFunction(Function &F) override;

  DominatorTree *DT = nullptr;

public:
  static char ID;
  OptimizeReturned() : FunctionPass(ID) {}

  void visitCallBase(CallBase &CB);
};
} // End anonymous namespace

char OptimizeReturned::ID = 0;
INITIALIZE_PASS(OptimizeReturned, DEBUG_TYPE,
                "Optimize calls with \"returned\" attributes for WebAssembly",
                false, false)

FunctionPass *toolchain::createWebAssemblyOptimizeReturned() {
  return new OptimizeReturned();
}

void OptimizeReturned::visitCallBase(CallBase &CB) {
  for (unsigned I = 0, E = CB.arg_size(); I < E; ++I)
    if (CB.paramHasAttr(I, Attribute::Returned)) {
      Value *Arg = CB.getArgOperand(I);
      // Ignore constants, globals, undef, etc.
      if (isa<Constant>(Arg))
        continue;
      // Like replaceDominatedUsesWith but using Instruction/Use dominance.
      Arg->replaceUsesWithIf(&CB, [&](Use &U) {
        auto *I = cast<Instruction>(U.getUser());
        return !I->isLifetimeStartOrEnd() && DT->dominates(&CB, U);
      });
    }
}

bool OptimizeReturned::runOnFunction(Function &F) {
  LLVM_DEBUG(dbgs() << "********** Optimize returned Attributes **********\n"
                       "********** Function: "
                    << F.getName() << '\n');

  DT = &getAnalysis<DominatorTreeWrapperPass>().getDomTree();
  visit(F);
  return true;
}
