//===- EscapeEnumerator.cpp -----------------------------------------------===//
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
// Defines a helper class that enumerates all possible exits from a function,
// including exception handling.
//
//===----------------------------------------------------------------------===//

#include "vm/core/Transforms/Utils/EscapeEnumerator.h"
#include "vm/core/IR/EHPersonalities.h"
#include "vm/core/IR/Module.h"
#include "vm/core/TargetParser/Triple.h"
#include "vm/core/Transforms/Utils/Local.h"

using namespace vm::core;

static FunctionCallee getDefaultPersonalityFn(Module *M) {
  LLVMContext &C = M->getContext();
  Triple T(M->getTargetTriple());
  EHPersonality Pers = getDefaultEHPersonality(T);
  return M->getOrInsertFunction(getEHPersonalityName(Pers),
                                FunctionType::get(Type::getInt32Ty(C), true));
}

IRBuilder<> *EscapeEnumerator::Next() {
  if (Done)
    return nullptr;

  // Find all 'return', 'resume', and 'unwind' instructions.
  while (StateBB != StateE) {
    BasicBlock *CurBB = &*StateBB++;

    // Branches and invokes do not escape, only unwind, resume, and return
    // do.
    Instruction *TI = CurBB->getTerminator();
    if (!isa<ReturnInst>(TI) && !isa<ResumeInst>(TI))
      continue;

    if (CallInst *CI = CurBB->getTerminatingMustTailCall())
      TI = CI;
    Builder.SetInsertPoint(TI);
    return &Builder;
  }

  Done = true;

  if (!HandleExceptions)
    return nullptr;

  if (F.doesNotThrow())
    return nullptr;

  // Find all 'call' instructions that may throw.
  // We cannot tranform calls with musttail tag.
  SmallVector<Instruction *, 16> Calls;
  for (BasicBlock &BB : F)
    for (Instruction &II : BB)
      if (CallInst *CI = dyn_cast<CallInst>(&II))
        if (!CI->doesNotThrow() && !CI->isMustTailCall())
          Calls.push_back(CI);

  if (Calls.empty())
    return nullptr;

  // Create a cleanup block.
  LLVMContext &C = F.getContext();
  BasicBlock *CleanupBB = BasicBlock::Create(C, CleanupBBName, &F);
  Type *ExnTy = StructType::get(PointerType::getUnqual(C), Type::getInt32Ty(C));
  if (!F.hasPersonalityFn()) {
    FunctionCallee PersFn = getDefaultPersonalityFn(F.getParent());
    F.setPersonalityFn(cast<Constant>(PersFn.getCallee()));
  }

  if (isScopedEHPersonality(classifyEHPersonality(F.getPersonalityFn()))) {
    report_fatal_error("Scoped EH not supported");
  }

  LandingPadInst *LPad =
      LandingPadInst::Create(ExnTy, 1, "cleanup.lpad", CleanupBB);
  LPad->setCleanup(true);
  ResumeInst *RI = ResumeInst::Create(LPad, CleanupBB);

  // Transform the 'call' instructions into 'invoke's branching to the
  // cleanup block. Go in reverse order to make prettier BB names.
  for (unsigned I = Calls.size(); I != 0;) {
    CallInst *CI = cast<CallInst>(Calls[--I]);
    changeToInvokeAndSplitBasicBlock(CI, CleanupBB, DTU);
  }

  Builder.SetInsertPoint(RI);
  return &Builder;
}
