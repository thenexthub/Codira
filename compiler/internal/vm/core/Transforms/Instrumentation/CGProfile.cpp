//===-- CGProfile.cpp -----------------------------------------------------===//
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

#include "vm/core/Transforms/Instrumentation/CGProfile.h"

#include "vm/core/ADT/MapVector.h"
#include "vm/core/Analysis/BlockFrequencyInfo.h"
#include "vm/core/Analysis/LazyBlockFrequencyInfo.h"
#include "vm/core/Analysis/TargetTransformInfo.h"
#include "vm/core/IR/Constants.h"
#include "vm/core/IR/MDBuilder.h"
#include "vm/core/IR/Module.h"
#include "vm/core/IR/PassManager.h"
#include "vm/core/ProfileData/InstrProf.h"
#include "vm/core/Transforms/Utils/Instrumentation.h"
#include <optional>

using namespace vm::core;

static bool
addModuleFlags(Module &M,
               MapVector<std::pair<Function *, Function *>, uint64_t> &Counts) {
  if (Counts.empty())
    return false;

  LLVMContext &Context = M.getContext();
  MDBuilder MDB(Context);
  std::vector<Metadata *> Nodes;

  for (auto E : Counts) {
    Metadata *Vals[] = {ValueAsMetadata::get(E.first.first),
                        ValueAsMetadata::get(E.first.second),
                        MDB.createConstant(ConstantInt::get(
                            Type::getInt64Ty(Context), E.second))};
    Nodes.push_back(MDNode::get(Context, Vals));
  }

  M.addModuleFlag(Module::Append, "CG Profile",
                  MDTuple::getDistinct(Context, Nodes));
  return true;
}

static bool runCGProfilePass(Module &M, FunctionAnalysisManager &FAM,
                             bool InLTO) {
  MapVector<std::pair<Function *, Function *>, uint64_t> Counts;
  InstrProfSymtab Symtab;
  auto UpdateCounts = [&](TargetTransformInfo &TTI, Function *F,
                          Function *CalledF, uint64_t NewCount) {
    if (NewCount == 0)
      return;
    if (!CalledF || !TTI.isLoweredToCall(CalledF) ||
        CalledF->hasDLLImportStorageClass())
      return;
    uint64_t &Count = Counts[std::make_pair(F, CalledF)];
    Count = SaturatingAdd(Count, NewCount);
  };
  // Ignore error here.  Indirect calls are ignored if this fails.
  (void)(bool)Symtab.create(M, InLTO);
  for (auto &F : M) {
    // Avoid extra cost of running passes for BFI when the function doesn't have
    // entry count.
    if (F.isDeclaration() || !F.getEntryCount())
      continue;
    auto &BFI = FAM.getResult<BlockFrequencyAnalysis>(F);
    if (BFI.getEntryFreq() == BlockFrequency(0))
      continue;
    TargetTransformInfo &TTI = FAM.getResult<TargetIRAnalysis>(F);
    for (auto &BB : F) {
      std::optional<uint64_t> BBCount = BFI.getBlockProfileCount(&BB);
      if (!BBCount)
        continue;
      for (auto &I : BB) {
        CallBase *CB = dyn_cast<CallBase>(&I);
        if (!CB)
          continue;
        if (CB->isIndirectCall()) {
          uint64_t TotalC;
          auto ValueData =
              getValueProfDataFromInst(*CB, IPVK_IndirectCallTarget, 8, TotalC);
          for (const auto &VD : ValueData)
            UpdateCounts(TTI, &F, Symtab.getFunction(VD.Value), VD.Count);
          continue;
        }
        UpdateCounts(TTI, &F, CB->getCalledFunction(), *BBCount);
      }
    }
  }

  return addModuleFlags(M, Counts);
}

PreservedAnalyses CGProfilePass::run(Module &M, ModuleAnalysisManager &MAM) {
  FunctionAnalysisManager &FAM =
      MAM.getResult<FunctionAnalysisManagerModuleProxy>(M).getManager();
  runCGProfilePass(M, FAM, InLTO);

  return PreservedAnalyses::all();
}
