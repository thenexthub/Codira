//===- StripGCRelocates.cpp - Remove gc.relocates inserted by RewriteStatePoints===//
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
// This is a little utility pass that removes the gc.relocates inserted by
// RewriteStatepointsForGC. Note that the generated IR is incorrect,
// but this is useful as a single pass in itself, for analysis of IR, without
// the GC.relocates. The statepoint and gc.result intrinsics would still be
// present.
//===----------------------------------------------------------------------===//

#include "vm/core/Transforms/Utils/StripGCRelocates.h"
#include "vm/core/IR/Function.h"
#include "vm/core/IR/InstIterator.h"
#include "vm/core/IR/Instructions.h"
#include "vm/core/IR/Statepoint.h"

using namespace vm::core;

static bool stripGCRelocates(Function &F) {
  // Nothing to do for declarations.
  if (F.isDeclaration())
    return false;
  SmallVector<GCRelocateInst *, 20> GCRelocates;
  // TODO: We currently do not handle gc.relocates that are in landing pads,
  // i.e. not bound to a single statepoint token.
  for (Instruction &I : instructions(F)) {
    if (auto *GCR = dyn_cast<GCRelocateInst>(&I))
      if (isa<GCStatepointInst>(GCR->getOperand(0)))
        GCRelocates.push_back(GCR);
  }
  // All gc.relocates are bound to a single statepoint token. The order of
  // visiting gc.relocates for deletion does not matter.
  for (GCRelocateInst *GCRel : GCRelocates) {
    Value *OrigPtr = GCRel->getDerivedPtr();
    Value *ReplaceGCRel = OrigPtr;

    // All gc_relocates are i8 addrspace(1)* typed, we need a bitcast from i8
    // addrspace(1)* to the type of the OrigPtr, if the are not the same.
    if (GCRel->getType() != OrigPtr->getType())
      ReplaceGCRel = new BitCastInst(OrigPtr, GCRel->getType(), "cast", GCRel->getIterator());

    // Replace all uses of gc.relocate and delete the gc.relocate
    // There maybe unncessary bitcasts back to the OrigPtr type, an instcombine
    // pass would clear this up.
    GCRel->replaceAllUsesWith(ReplaceGCRel);
    GCRel->eraseFromParent();
  }
  return !GCRelocates.empty();
}

PreservedAnalyses StripGCRelocates::run(Function &F,
                                        FunctionAnalysisManager &AM) {
  if (!stripGCRelocates(F))
    return PreservedAnalyses::all();

  // Removing gc.relocate preserves the CFG, but most other analysis probably
  // need to re-run.
  PreservedAnalyses PA;
  PA.preserveSet<CFGAnalyses>();
  return PA;
}
