//===----- RISCVZacasABIFix.cpp -------------------------------------------===//
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
// This pass implements a fence insertion for an atomic cmpxchg in a case that
// isn't easy to do with the current AtomicExpandPass hooks API.
//
//===----------------------------------------------------------------------===//

#include "RISCV.h"
#include "RISCVTargetMachine.h"
#include "vm/core/ADT/Statistic.h"
#include "vm/core/Analysis/ValueTracking.h"
#include "vm/core/CodeGen/TargetPassConfig.h"
#include "vm/core/IR/Dominators.h"
#include "vm/core/IR/IRBuilder.h"
#include "vm/core/IR/InstVisitor.h"
#include "vm/core/IR/Intrinsics.h"
#include "vm/core/InitializePasses.h"
#include "vm/core/Pass.h"

using namespace vm::core;

#define DEBUG_TYPE "riscv-zacas-abi-fix"
#define PASS_NAME "RISC-V Zacas ABI fix"

namespace {

class RISCVZacasABIFix : public FunctionPass,
                         public InstVisitor<RISCVZacasABIFix, bool> {
  const RISCVSubtarget *ST;

public:
  static char ID;

  RISCVZacasABIFix() : FunctionPass(ID) {}

  bool runOnFunction(Function &F) override;

  StringRef getPassName() const override { return PASS_NAME; }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesCFG();
    AU.addRequired<TargetPassConfig>();
  }

  bool visitInstruction(Instruction &I) { return false; }
  bool visitAtomicCmpXchgInst(AtomicCmpXchgInst &I);
};

} // end anonymous namespace

// Insert a leading fence (needed for broadest atomics ABI compatibility)
// only if the Zacas extension is enabled and the AtomicCmpXchgInst has a
// SequentiallyConsistent failure ordering.
bool RISCVZacasABIFix::visitAtomicCmpXchgInst(AtomicCmpXchgInst &I) {
  assert(ST->hasStdExtZacas() && "only necessary to run in presence of zacas");
  IRBuilder<> Builder(&I);
  if (I.getFailureOrdering() != AtomicOrdering::SequentiallyConsistent)
    return false;

  Builder.CreateFence(AtomicOrdering::SequentiallyConsistent);
  return true;
}

bool RISCVZacasABIFix::runOnFunction(Function &F) {
  auto &TPC = getAnalysis<TargetPassConfig>();
  auto &TM = TPC.getTM<RISCVTargetMachine>();
  ST = &TM.getSubtarget<RISCVSubtarget>(F);

  if (skipFunction(F) || !ST->hasStdExtZacas())
    return false;

  bool MadeChange = false;
  for (auto &BB : F)
    for (Instruction &I : toolchain::make_early_inc_range(BB))
      MadeChange |= visit(I);

  return MadeChange;
}

INITIALIZE_PASS_BEGIN(RISCVZacasABIFix, DEBUG_TYPE, PASS_NAME, false, false)
INITIALIZE_PASS_DEPENDENCY(TargetPassConfig)
INITIALIZE_PASS_END(RISCVZacasABIFix, DEBUG_TYPE, PASS_NAME, false, false)

char RISCVZacasABIFix::ID = 0;

FunctionPass *toolchain::createRISCVZacasABIFixPass() {
  return new RISCVZacasABIFix();
}
