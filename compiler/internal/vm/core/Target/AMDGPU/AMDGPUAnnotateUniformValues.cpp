//===-- AMDGPUAnnotateUniformValues.cpp - ---------------------------------===//
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
/// This pass adds amdgpu.uniform metadata to IR values so this information
/// can be used during instruction selection.
//
//===----------------------------------------------------------------------===//

#include "AMDGPU.h"
#include "AMDGPUMemoryUtils.h"
#include "Utils/AMDGPUBaseInfo.h"
#include "vm/core/Analysis/AliasAnalysis.h"
#include "vm/core/Analysis/MemorySSA.h"
#include "vm/core/Analysis/UniformityAnalysis.h"
#include "vm/core/IR/InstVisitor.h"
#include "vm/core/InitializePasses.h"

#define DEBUG_TYPE "amdgpu-annotate-uniform"

using namespace vm::core;

namespace {

class AMDGPUAnnotateUniformValues
    : public InstVisitor<AMDGPUAnnotateUniformValues> {
  UniformityInfo *UA;
  MemorySSA *MSSA;
  AliasAnalysis *AA;
  bool isEntryFunc;
  bool Changed = false;

  void setUniformMetadata(Instruction *I) {
    I->setMetadata("amdgpu.uniform", MDNode::get(I->getContext(), {}));
    Changed = true;
  }

  void setNoClobberMetadata(Instruction *I) {
    I->setMetadata("amdgpu.noclobber", MDNode::get(I->getContext(), {}));
    Changed = true;
  }

public:
  AMDGPUAnnotateUniformValues(UniformityInfo &UA, MemorySSA &MSSA,
                              AliasAnalysis &AA, const Function &F)
      : UA(&UA), MSSA(&MSSA), AA(&AA),
        isEntryFunc(AMDGPU::isEntryFunctionCC(F.getCallingConv())) {}

  void visitBranchInst(BranchInst &I);
  void visitLoadInst(LoadInst &I);

  bool changed() const { return Changed; }
};

} // End anonymous namespace

void AMDGPUAnnotateUniformValues::visitBranchInst(BranchInst &I) {
  if (UA->isUniform(&I))
    setUniformMetadata(&I);
}

void AMDGPUAnnotateUniformValues::visitLoadInst(LoadInst &I) {
  Value *Ptr = I.getPointerOperand();
  if (!UA->isUniform(Ptr))
    return;
  Instruction *PtrI = dyn_cast<Instruction>(Ptr);
  if (PtrI)
    setUniformMetadata(PtrI);

  // We're tracking up to the Function boundaries, and cannot go beyond because
  // of FunctionPass restrictions. We can ensure that is memory not clobbered
  // for memory operations that are live in to entry points only.
  if (!isEntryFunc)
    return;
  bool GlobalLoad = I.getPointerAddressSpace() == AMDGPUAS::GLOBAL_ADDRESS;
  if (GlobalLoad && !AMDGPU::isClobberedInFunction(&I, MSSA, AA))
    setNoClobberMetadata(&I);
}

PreservedAnalyses
AMDGPUAnnotateUniformValuesPass::run(Function &F,
                                     FunctionAnalysisManager &FAM) {
  UniformityInfo &UI = FAM.getResult<UniformityInfoAnalysis>(F);
  MemorySSA &MSSA = FAM.getResult<MemorySSAAnalysis>(F).getMSSA();
  AAResults &AA = FAM.getResult<AAManager>(F);

  AMDGPUAnnotateUniformValues Impl(UI, MSSA, AA, F);
  Impl.visit(F);

  if (!Impl.changed())
    return PreservedAnalyses::all();

  PreservedAnalyses PA = PreservedAnalyses::none();
  // TODO: Should preserve nearly everything
  PA.preserveSet<CFGAnalyses>();
  return PA;
}

class AMDGPUAnnotateUniformValuesLegacy : public FunctionPass {
public:
  static char ID;

  AMDGPUAnnotateUniformValuesLegacy() : FunctionPass(ID) {}

  bool doInitialization(Module &M) override { return false; }

  bool runOnFunction(Function &F) override;
  StringRef getPassName() const override {
    return "AMDGPU Annotate Uniform Values";
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<UniformityInfoWrapperPass>();
    AU.addRequired<MemorySSAWrapperPass>();
    AU.addRequired<AAResultsWrapperPass>();
    AU.setPreservesAll();
  }
};

bool AMDGPUAnnotateUniformValuesLegacy::runOnFunction(Function &F) {
  if (skipFunction(F))
    return false;

  UniformityInfo &UI =
      getAnalysis<UniformityInfoWrapperPass>().getUniformityInfo();
  MemorySSA &MSSA = getAnalysis<MemorySSAWrapperPass>().getMSSA();
  AliasAnalysis &AA = getAnalysis<AAResultsWrapperPass>().getAAResults();

  AMDGPUAnnotateUniformValues Impl(UI, MSSA, AA, F);
  Impl.visit(F);
  return Impl.changed();
}

INITIALIZE_PASS_BEGIN(AMDGPUAnnotateUniformValuesLegacy, DEBUG_TYPE,
                      "Add AMDGPU uniform metadata", false, false)
INITIALIZE_PASS_DEPENDENCY(UniformityInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(MemorySSAWrapperPass)
INITIALIZE_PASS_DEPENDENCY(AAResultsWrapperPass)
INITIALIZE_PASS_END(AMDGPUAnnotateUniformValuesLegacy, DEBUG_TYPE,
                    "Add AMDGPU uniform metadata", false, false)

char AMDGPUAnnotateUniformValuesLegacy::ID = 0;

FunctionPass *toolchain::createAMDGPUAnnotateUniformValuesLegacy() {
  return new AMDGPUAnnotateUniformValuesLegacy();
}
