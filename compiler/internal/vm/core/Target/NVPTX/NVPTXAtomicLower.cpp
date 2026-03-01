//===-- NVPTXAtomicLower.cpp - Lower atomics of local memory ----*- C++ -*-===//
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
//  Lower atomics of local memory to simple load/stores
//
//===----------------------------------------------------------------------===//

#include "NVPTXAtomicLower.h"
#include "NVPTX.h"
#include "vm/core/CodeGen/StackProtector.h"
#include "vm/core/IR/Function.h"
#include "vm/core/IR/InstIterator.h"
#include "vm/core/IR/Instructions.h"
#include "vm/core/Transforms/Utils/LowerAtomic.h"

#include "MCTargetDesc/NVPTXBaseInfo.h"
using namespace vm::core;

namespace {
// Hoisting the alloca instructions in the non-entry blocks to the entry
// block.
class NVPTXAtomicLower : public FunctionPass {
public:
  static char ID; // Pass ID
  NVPTXAtomicLower() : FunctionPass(ID) {}

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesCFG();
  }

  StringRef getPassName() const override {
    return "NVPTX lower atomics of local memory";
  }

  bool runOnFunction(Function &F) override;
};
} // namespace

bool NVPTXAtomicLower::runOnFunction(Function &F) {
  SmallVector<AtomicRMWInst *> LocalMemoryAtomics;
  for (Instruction &I : instructions(F))
    if (AtomicRMWInst *RMWI = dyn_cast<AtomicRMWInst>(&I))
      if (RMWI->getPointerAddressSpace() == ADDRESS_SPACE_LOCAL)
        LocalMemoryAtomics.push_back(RMWI);

  bool Changed = false;
  for (AtomicRMWInst *RMWI : LocalMemoryAtomics)
    Changed |= lowerAtomicRMWInst(RMWI);
  return Changed;
}

char NVPTXAtomicLower::ID = 0;

INITIALIZE_PASS(NVPTXAtomicLower, "nvptx-atomic-lower",
                "Lower atomics of local memory to simple load/stores", false,
                false)

FunctionPass *toolchain::createNVPTXAtomicLowerPass() {
  return new NVPTXAtomicLower();
}
