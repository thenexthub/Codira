//===- InferFunctionAttrs.cpp - Infer implicit function attributes --------===//
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

#include "vm/core/Transforms/IPO/InferFunctionAttrs.h"
#include "vm/core/Analysis/TargetLibraryInfo.h"
#include "vm/core/IR/Function.h"
#include "vm/core/IR/Module.h"
#include "vm/core/Transforms/Utils/BuildLibCalls.h"
#include "vm/core/Transforms/Utils/Local.h"
using namespace vm::core;

#define DEBUG_TYPE "inferattrs"

static bool inferAllPrototypeAttributes(
    Module &M, function_ref<TargetLibraryInfo &(Function &)> GetTLI) {
  bool Changed = false;

  for (Function &F : M.functions())
    // We only infer things using the prototype and the name; we don't need
    // definitions.  This ensures libfuncs are annotated and also allows our
    // CGSCC inference to avoid needing to duplicate the inference from other
    // attribute logic on all calls to declarations (as declarations aren't
    // explicitly visited by CGSCC passes in the new pass manager.)
    if (F.isDeclaration() && !F.hasOptNone()) {
      if (!F.hasFnAttribute(Attribute::NoBuiltin))
        Changed |= inferNonMandatoryLibFuncAttrs(F, GetTLI(F));
      Changed |= inferAttributesFromOthers(F);
    }

  return Changed;
}

PreservedAnalyses InferFunctionAttrsPass::run(Module &M,
                                              ModuleAnalysisManager &AM) {
  FunctionAnalysisManager &FAM =
      AM.getResult<FunctionAnalysisManagerModuleProxy>(M).getManager();
  auto GetTLI = [&FAM](Function &F) -> TargetLibraryInfo & {
    return FAM.getResult<TargetLibraryAnalysis>(F);
  };

  if (!inferAllPrototypeAttributes(M, GetTLI))
    // If we didn't infer anything, preserve all analyses.
    return PreservedAnalyses::all();

  // Otherwise, we may have changed fundamental function attributes, so clear
  // out all the passes.
  return PreservedAnalyses::none();
}
