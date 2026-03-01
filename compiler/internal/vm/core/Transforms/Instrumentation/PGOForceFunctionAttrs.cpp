//===----------------------------------------------------------------------===//
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

#include "vm/core/Transforms/Instrumentation/PGOForceFunctionAttrs.h"
#include "vm/core/Analysis/BlockFrequencyInfo.h"
#include "vm/core/Analysis/ProfileSummaryInfo.h"
#include "vm/core/IR/Module.h"
#include "vm/core/IR/PassManager.h"
#include "vm/core/Support/ErrorHandling.h"

using namespace vm::core;

static bool shouldRunOnFunction(Function &F, ProfileSummaryInfo &PSI,
                                FunctionAnalysisManager &FAM) {
  if (F.isDeclaration())
    return false;
  // Respect existing attributes.
  if (F.hasOptNone() || F.hasOptSize())
    return false;
  if (F.hasFnAttribute(Attribute::Cold))
    return true;
  if (!PSI.hasProfileSummary())
    return false;
  BlockFrequencyInfo &BFI = FAM.getResult<BlockFrequencyAnalysis>(F);
  return PSI.isFunctionColdInCallGraph(&F, BFI);
}

PreservedAnalyses PGOForceFunctionAttrsPass::run(Module &M,
                                                 ModuleAnalysisManager &AM) {
  if (ColdType == PGOOptions::ColdFuncOpt::Default)
    return PreservedAnalyses::all();
  ProfileSummaryInfo &PSI = AM.getResult<ProfileSummaryAnalysis>(M);
  FunctionAnalysisManager &FAM =
      AM.getResult<FunctionAnalysisManagerModuleProxy>(M).getManager();
  bool MadeChange = false;
  for (Function &F : M) {
    if (!shouldRunOnFunction(F, PSI, FAM))
      continue;
    switch (ColdType) {
    case PGOOptions::ColdFuncOpt::Default:
      llvm_unreachable("bailed out for default above");
      break;
    case PGOOptions::ColdFuncOpt::OptSize:
      F.addFnAttr(Attribute::OptimizeForSize);
      break;
    case PGOOptions::ColdFuncOpt::MinSize:
      F.addFnAttr(Attribute::MinSize);
      break;
    case PGOOptions::ColdFuncOpt::OptNone:
      // alwaysinline is incompatible with optnone.
      if (F.hasFnAttribute(Attribute::AlwaysInline))
        continue;
      F.addFnAttr(Attribute::OptimizeNone);
      F.addFnAttr(Attribute::NoInline);
      break;
    }
    MadeChange = true;
  }
  return MadeChange ? PreservedAnalyses::none() : PreservedAnalyses::all();
}
