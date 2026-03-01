//===- PGOCtxProfFlattening.cpp - Contextual Instr. Flattening ------------===//
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
// Flattens the contextual profile and lowers it to MD_prof.
// This should happen after all IPO (which is assumed to have maintained the
// contextual profile) happened. Flattening consists of summing the values at
// the same index of the counters belonging to all the contexts of a function.
// The lowering consists of materializing the counter values to function
// entrypoint counts and branch probabilities.
//
// This pass also removes contextual instrumentation, which has been kept around
// to facilitate its functionality.
//
//===----------------------------------------------------------------------===//

#include "vm/core/Transforms/Instrumentation/PGOCtxProfFlattening.h"
#include "vm/core/ADT/STLExtras.h"
#include "vm/core/ADT/ScopeExit.h"
#include "vm/core/Analysis/CFG.h"
#include "vm/core/Analysis/CtxProfAnalysis.h"
#include "vm/core/Analysis/ProfileSummaryInfo.h"
#include "vm/core/IR/Analysis.h"
#include "vm/core/IR/CFG.h"
#include "vm/core/IR/Dominators.h"
#include "vm/core/IR/Instructions.h"
#include "vm/core/IR/IntrinsicInst.h"
#include "vm/core/IR/Module.h"
#include "vm/core/IR/PassManager.h"
#include "vm/core/IR/ProfileSummary.h"
#include "vm/core/ProfileData/ProfileCommon.h"
#include "vm/core/Transforms/Instrumentation/PGOInstrumentation.h"
#include "vm/core/Transforms/Scalar/DCE.h"
#include "vm/core/Transforms/Utils/BasicBlockUtils.h"

using namespace vm::core;

#define DEBUG_TYPE "ctx_prof_flatten"

namespace {

/// Assign branch weights and function entry count. Also update the PSI
/// builder.
void assignProfileData(Function &F, ArrayRef<uint64_t> RawCounters) {
  assert(!RawCounters.empty());
  ProfileAnnotator PA(F, RawCounters);

  F.setEntryCount(RawCounters[0]);
  SmallVector<uint64_t, 2> ProfileHolder;

  for (auto &BB : F) {
    for (auto &I : BB)
      if (auto *SI = dyn_cast<SelectInst>(&I)) {
        uint64_t TrueCount, FalseCount = 0;
        if (!PA.getSelectInstrProfile(*SI, TrueCount, FalseCount))
          continue;
        setProfMetadata(SI, {TrueCount, FalseCount},
                        std::max(TrueCount, FalseCount));
      }
    if (succ_size(&BB) < 2)
      continue;
    uint64_t MaxCount = 0;
    if (!PA.getOutgoingBranchWeights(BB, ProfileHolder, MaxCount))
      continue;
    assert(MaxCount > 0);
    setProfMetadata(BB.getTerminator(), ProfileHolder, MaxCount);
  }
}

[[maybe_unused]] bool areAllBBsReachable(const Function &F,
                                         FunctionAnalysisManager &FAM) {
  auto &DT = FAM.getResult<DominatorTreeAnalysis>(const_cast<Function &>(F));
  return toolchain::all_of(
      F, [&](const BasicBlock &BB) { return DT.isReachableFromEntry(&BB); });
}

void clearColdFunctionProfile(Function &F) {
  for (auto &BB : F)
    BB.getTerminator()->setMetadata(LLVMContext::MD_prof, nullptr);
  F.setEntryCount(0U);
}

void removeInstrumentation(Function &F) {
  for (auto &BB : F)
    for (auto &I : toolchain::make_early_inc_range(BB))
      if (isa<InstrProfCntrInstBase>(I))
        I.eraseFromParent();
}

void annotateIndirectCall(
    Module &M, CallBase &CB,
    const DenseMap<uint32_t, FlatIndirectTargets> &FlatProf,
    const InstrProfCallsite &Ins) {
  auto Idx = Ins.getIndex()->getZExtValue();
  auto FIt = FlatProf.find(Idx);
  if (FIt == FlatProf.end())
    return;
  const auto &Targets = FIt->second;
  SmallVector<InstrProfValueData, 2> Data;
  uint64_t Sum = 0;
  for (auto &[Guid, Count] : Targets) {
    Data.push_back({/*.Value=*/Guid, /*.Count=*/Count});
    Sum += Count;
  }

  toolchain::sort(Data,
             [](const InstrProfValueData &A, const InstrProfValueData &B) {
               return A.Count > B.Count;
             });
  toolchain::annotateValueSite(M, CB, Data, Sum,
                          InstrProfValueKind::IPVK_IndirectCallTarget,
                          Data.size());
  LLVM_DEBUG(dbgs() << "[ctxprof] flat indirect call prof: " << CB
                    << CB.getMetadata(LLVMContext::MD_prof) << "\n");
}

// We normally return a "Changed" bool, but the calling pass' run assumes
// something will change - some profile will be added - so this won't add much
// by returning false when applicable.
void annotateIndirectCalls(Module &M, const CtxProfAnalysis::Result &CtxProf) {
  const auto FlatIndCalls = CtxProf.flattenVirtCalls();
  for (auto &F : M) {
    if (F.isDeclaration())
      continue;
    auto FlatProfIter = FlatIndCalls.find(AssignGUIDPass::getGUID(F));
    if (FlatProfIter == FlatIndCalls.end())
      continue;
    const auto &FlatProf = FlatProfIter->second;
    for (auto &BB : F) {
      for (auto &I : BB) {
        auto *CB = dyn_cast<CallBase>(&I);
        if (!CB || !CB->isIndirectCall())
          continue;
        if (auto *Ins = CtxProfAnalysis::getCallsiteInstrumentation(*CB))
          annotateIndirectCall(M, *CB, FlatProf, *Ins);
      }
    }
  }
}

} // namespace

PreservedAnalyses PGOCtxProfFlatteningPass::run(Module &M,
                                                ModuleAnalysisManager &MAM) {
  // Ensure in all cases the instrumentation is removed: if this module had no
  // roots, the contextual profile would evaluate to false, but there would
  // still be instrumentation.
  // Note: in such cases we leave as-is any other profile info (if present -
  // e.g. synthetic weights, etc) because it wouldn't interfere with the
  // contextual - based one (which would be in other modules)
  toolchain::scope_exit OnExit([&]() {
    if (IsPreThinlink)
      return;
    for (auto &F : M)
      removeInstrumentation(F);
  });
  auto &CtxProf = MAM.getResult<CtxProfAnalysis>(M);
  // post-thinlink, we only reprocess for the module(s) containing the
  // contextual tree. For everything else, OnExit will just clean the
  // instrumentation.
  if (!IsPreThinlink && !CtxProf.isInSpecializedModule())
    return PreservedAnalyses::none();

  if (IsPreThinlink)
    annotateIndirectCalls(M, CtxProf);
  const auto FlattenedProfile = CtxProf.flatten();

  for (auto &F : M) {
    if (F.isDeclaration())
      continue;

    assert(areAllBBsReachable(
               F, MAM.getResult<FunctionAnalysisManagerModuleProxy>(M)
                      .getManager()) &&
           "Function has unreachable basic blocks. The expectation was that "
           "DCE was run before.");

    auto It = FlattenedProfile.find(AssignGUIDPass::getGUID(F));
    // If this function didn't appear in the contextual profile, it's cold.
    if (It == FlattenedProfile.end())
      clearColdFunctionProfile(F);
    else
      assignProfileData(F, It->second);
  }
  InstrProfSummaryBuilder PB(ProfileSummaryBuilder::DefaultCutoffs);
  // use here the flat profiles just so the importer doesn't complain about
  // how different the PSIs are between the module with the roots and the
  // various modules it imports.
  for (auto &C : FlattenedProfile) {
    PB.addEntryCount(C.second[0]);
    for (auto V : toolchain::drop_begin(C.second))
      PB.addInternalCount(V);
  }

  M.setProfileSummary(PB.getSummary()->getMD(M.getContext()),
                      ProfileSummary::Kind::PSK_Instr);
  PreservedAnalyses PA;
  PA.abandon<ProfileSummaryAnalysis>();
  MAM.invalidate(M, PA);
  auto &PSI = MAM.getResult<ProfileSummaryAnalysis>(M);
  PSI.refresh(PB.getSummary());
  return PreservedAnalyses::none();
}
