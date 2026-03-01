//===-- AnnotationRemarks.cpp - Generate remarks for annotated instrs. ----===//
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
// Generate remarks for instructions marked with !annotation.
//
//===----------------------------------------------------------------------===//

#include "vm/core/Transforms/Scalar/AnnotationRemarks.h"
#include "vm/core/ADT/MapVector.h"
#include "vm/core/Analysis/OptimizationRemarkEmitter.h"
#include "vm/core/Analysis/TargetLibraryInfo.h"
#include "vm/core/IR/Function.h"
#include "vm/core/IR/InstIterator.h"
#include "vm/core/Transforms/Utils/MemoryOpRemark.h"

using namespace vm::core;
using namespace vm::core::ore;

#define DEBUG_TYPE "annotation-remarks"
#define REMARK_PASS DEBUG_TYPE

static void tryEmitAutoInitRemark(ArrayRef<Instruction *> Instructions,
                                  OptimizationRemarkEmitter &ORE,
                                  const TargetLibraryInfo &TLI) {
  // For every auto-init annotation generate a separate remark.
  for (Instruction *I : Instructions) {
    if (!AutoInitRemark::canHandle(I))
      continue;

    Function &F = *I->getParent()->getParent();
    const DataLayout &DL = F.getDataLayout();
    AutoInitRemark Remark(ORE, REMARK_PASS, DL, TLI);
    Remark.visit(I);
  }
}

static void runImpl(Function &F, const TargetLibraryInfo &TLI) {
  if (!OptimizationRemarkEmitter::allowExtraAnalysis(F, REMARK_PASS))
    return;

  // Track all annotated instructions aggregated based on their debug location.
  DenseMap<MDNode *, SmallVector<Instruction *, 4>> DebugLoc2Annotated;

  OptimizationRemarkEmitter ORE(&F);
  // First, generate a summary of the annotated instructions.
  MapVector<StringRef, unsigned> Mapping;
  for (Instruction &I : instructions(F)) {
    if (!I.hasMetadata(LLVMContext::MD_annotation))
      continue;
    DebugLoc2Annotated[I.getDebugLoc().getAsMDNode()].push_back(&I);

    for (const MDOperand &Op :
         I.getMetadata(LLVMContext::MD_annotation)->operands()) {
      StringRef AnnotationStr =
          isa<MDString>(Op.get())
              ? cast<MDString>(Op.get())->getString()
              : cast<MDString>(cast<MDTuple>(Op.get())->getOperand(0).get())
                    ->getString();
      Mapping[AnnotationStr]++;
    }
  }

  for (const auto &KV : Mapping)
    ORE.emit(OptimizationRemarkAnalysis(REMARK_PASS, "AnnotationSummary",
                                        F.getSubprogram(), &F.front())
             << "Annotated " << NV("count", KV.second) << " instructions with "
             << NV("type", KV.first));

  // For each debug location, look for all the instructions with annotations and
  // generate more detailed remarks to be displayed at that location.
  for (auto &KV : DebugLoc2Annotated) {
    // Don't generate remarks with no debug location.
    if (!KV.first)
      continue;

    tryEmitAutoInitRemark(KV.second, ORE, TLI);
  }
}

PreservedAnalyses AnnotationRemarksPass::run(Function &F,
                                             FunctionAnalysisManager &AM) {
  auto &TLI = AM.getResult<TargetLibraryAnalysis>(F);
  runImpl(F, TLI);
  return PreservedAnalyses::all();
}
