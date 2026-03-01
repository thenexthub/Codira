//===- LowerAtomic.cpp - Lower atomic intrinsics --------------------------===//
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
// This pass lowers atomic intrinsics to non-atomic form for use in a known
// non-preemptible environment.
//
//===----------------------------------------------------------------------===//

#include "vm/core/Transforms/Scalar/LowerAtomicPass.h"
#include "vm/core/IR/Function.h"
#include "vm/core/InitializePasses.h"
#include "vm/core/Pass.h"
#include "vm/core/Transforms/Scalar.h"
#include "vm/core/Transforms/Utils/LowerAtomic.h"
using namespace vm::core;

#define DEBUG_TYPE "lower-atomic"

static bool LowerFenceInst(FenceInst *FI) {
  FI->eraseFromParent();
  return true;
}

static bool LowerLoadInst(LoadInst *LI) {
  LI->setAtomic(AtomicOrdering::NotAtomic);
  return true;
}

static bool LowerStoreInst(StoreInst *SI) {
  SI->setAtomic(AtomicOrdering::NotAtomic);
  return true;
}

static bool runOnBasicBlock(BasicBlock &BB) {
  bool Changed = false;
  for (Instruction &Inst : make_early_inc_range(BB)) {
    if (FenceInst *FI = dyn_cast<FenceInst>(&Inst))
      Changed |= LowerFenceInst(FI);
    else if (AtomicCmpXchgInst *CXI = dyn_cast<AtomicCmpXchgInst>(&Inst))
      Changed |= lowerAtomicCmpXchgInst(CXI);
    else if (AtomicRMWInst *RMWI = dyn_cast<AtomicRMWInst>(&Inst))
      Changed |= lowerAtomicRMWInst(RMWI);
    else if (LoadInst *LI = dyn_cast<LoadInst>(&Inst)) {
      if (LI->isAtomic())
        LowerLoadInst(LI);
    } else if (StoreInst *SI = dyn_cast<StoreInst>(&Inst)) {
      if (SI->isAtomic())
        LowerStoreInst(SI);
    }
  }
  return Changed;
}

static bool lowerAtomics(Function &F) {
  bool Changed = false;
  for (BasicBlock &BB : F) {
    Changed |= runOnBasicBlock(BB);
  }
  return Changed;
}

PreservedAnalyses LowerAtomicPass::run(Function &F, FunctionAnalysisManager &) {
  if (lowerAtomics(F))
    return PreservedAnalyses::none();
  return PreservedAnalyses::all();
}

namespace {
class LowerAtomicLegacyPass : public FunctionPass {
public:
  static char ID;

  LowerAtomicLegacyPass() : FunctionPass(ID) {
    initializeLowerAtomicLegacyPassPass(*PassRegistry::getPassRegistry());
  }

  bool runOnFunction(Function &F) override {
    // Don't skip optnone functions; atomics still need to be lowered.
    FunctionAnalysisManager DummyFAM;
    auto PA = Impl.run(F, DummyFAM);
    return !PA.areAllPreserved();
  }

private:
  LowerAtomicPass Impl;
  };
}

char LowerAtomicLegacyPass::ID = 0;
INITIALIZE_PASS(LowerAtomicLegacyPass, "loweratomic",
                "Lower atomic intrinsics to non-atomic form", false, false)

Pass *toolchain::createLowerAtomicPass() { return new LowerAtomicLegacyPass(); }
