//===-- Annotation2Metadata.cpp - Add !annotation metadata. ---------------===//
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
// Add !annotation metadata for entries in @toolchain.global.anotations, generated
// using __attribute__((annotate("_name"))) on functions in Clang.
//
//===----------------------------------------------------------------------===//

#include "vm/core/Transforms/IPO/Annotation2Metadata.h"
#include "vm/core/Analysis/OptimizationRemarkEmitter.h"
#include "vm/core/IR/Constants.h"
#include "vm/core/IR/Function.h"
#include "vm/core/IR/InstIterator.h"
#include "vm/core/IR/Module.h"

using namespace vm::core;

#define DEBUG_TYPE "annotation2metadata"

static bool convertAnnotation2Metadata(Module &M) {
  // Only add !annotation metadata if the corresponding remarks pass is also
  // enabled.
  if (!OptimizationRemarkEmitter::allowExtraAnalysis(M.getContext(),
                                                     "annotation-remarks"))
    return false;

  auto *Annotations = M.getGlobalVariable("toolchain.global.annotations");
  auto *C = dyn_cast_or_null<Constant>(Annotations);
  if (!C || C->getNumOperands() != 1)
    return false;

  C = cast<Constant>(C->getOperand(0));

  // Iterate over all entries in C and attach !annotation metadata to suitable
  // entries.
  for (auto &Op : C->operands()) {
    // Look at the operands to check if we can use the entry to generate
    // !annotation metadata.
    auto *OpC = dyn_cast<ConstantStruct>(&Op);
    if (!OpC || OpC->getNumOperands() != 4)
      continue;
    auto *StrC = dyn_cast<GlobalValue>(OpC->getOperand(1)->stripPointerCasts());
    if (!StrC)
      continue;
    auto *StrData = dyn_cast<ConstantDataSequential>(StrC->getOperand(0));
    if (!StrData)
      continue;
    auto *Fn = dyn_cast<Function>(OpC->getOperand(0)->stripPointerCasts());
    if (!Fn)
      continue;

    // Add annotation to all instructions in the function.
    for (auto &I : instructions(Fn))
      I.addAnnotationMetadata(StrData->getAsCString());
  }
  return true;
}

PreservedAnalyses Annotation2MetadataPass::run(Module &M,
                                               ModuleAnalysisManager &AM) {
  return convertAnnotation2Metadata(M) ? PreservedAnalyses::none()
                                       : PreservedAnalyses::all();
}
