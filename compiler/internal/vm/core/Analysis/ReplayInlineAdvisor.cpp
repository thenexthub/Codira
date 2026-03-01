//===- ReplayInlineAdvisor.cpp - Replay InlineAdvisor ---------------------===//
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
// This file implements ReplayInlineAdvisor that replays inline decisions based
// on previous inline remarks from optimization remark log. This is a best
// effort approach useful for testing compiler/source changes while holding
// inlining steady.
//
//===----------------------------------------------------------------------===//

#include "vm/core/Analysis/ReplayInlineAdvisor.h"
#include "vm/core/Analysis/OptimizationRemarkEmitter.h"
#include "vm/core/Support/LineIterator.h"
#include "vm/core/Support/MemoryBuffer.h"
#include <memory>

using namespace vm::core;

#define DEBUG_TYPE "replay-inline"

ReplayInlineAdvisor::ReplayInlineAdvisor(
    Module &M, FunctionAnalysisManager &FAM, LLVMContext &Context,
    std::unique_ptr<InlineAdvisor> OriginalAdvisor,
    const ReplayInlinerSettings &ReplaySettings, bool EmitRemarks,
    InlineContext IC)
    : InlineAdvisor(M, FAM, IC), OriginalAdvisor(std::move(OriginalAdvisor)),
      ReplaySettings(ReplaySettings), EmitRemarks(EmitRemarks) {

  auto BufferOrErr = MemoryBuffer::getFileOrSTDIN(ReplaySettings.ReplayFile);
  std::error_code EC = BufferOrErr.getError();
  if (EC) {
    Context.emitError("Could not open remarks file: " + EC.message());
    return;
  }

  // Example for inline remarks to parse:
  //   main:3:1.1: '_Z3subii' inlined into 'main' at callsite sum:1 @
  //   main:3:1.1;
  // We use the callsite string after `at callsite` to replay inlining.
  line_iterator LineIt(*BufferOrErr.get(), /*SkipBlanks=*/true);
  const std::string PositiveRemark = "' inlined into '";
  const std::string NegativeRemark = "' will not be inlined into '";

  for (; !LineIt.is_at_eof(); ++LineIt) {
    StringRef Line = *LineIt;
    auto Pair = Line.split(" at callsite ");

    bool IsPositiveRemark = true;
    if (Pair.first.contains(NegativeRemark))
      IsPositiveRemark = false;

    auto CalleeCaller =
        Pair.first.split(IsPositiveRemark ? PositiveRemark : NegativeRemark);

    StringRef Callee = CalleeCaller.first.rsplit(": '").second;
    StringRef Caller = CalleeCaller.second.rsplit("'").first;

    auto CallSite = Pair.second.split(";").first;

    if (Callee.empty() || Caller.empty() || CallSite.empty()) {
      Context.emitError("Invalid remark format: " + Line);
      return;
    }

    std::string Combined = (Callee + CallSite).str();
    InlineSitesFromRemarks[Combined] = IsPositiveRemark;
    if (ReplaySettings.ReplayScope == ReplayInlinerSettings::Scope::Function)
      CallersToReplay.insert(Caller);
  }

  HasReplayRemarks = true;
}

std::unique_ptr<InlineAdvisor>
toolchain::getReplayInlineAdvisor(Module &M, FunctionAnalysisManager &FAM,
                             LLVMContext &Context,
                             std::unique_ptr<InlineAdvisor> OriginalAdvisor,
                             const ReplayInlinerSettings &ReplaySettings,
                             bool EmitRemarks, InlineContext IC) {
  auto Advisor = std::make_unique<ReplayInlineAdvisor>(
      M, FAM, Context, std::move(OriginalAdvisor), ReplaySettings, EmitRemarks,
      IC);
  if (!Advisor->areReplayRemarksLoaded())
    Advisor.reset();
  return Advisor;
}

std::unique_ptr<InlineAdvice> ReplayInlineAdvisor::getAdviceImpl(CallBase &CB) {
  assert(HasReplayRemarks);

  Function &Caller = *CB.getCaller();
  auto &ORE = FAM.getResult<OptimizationRemarkEmitterAnalysis>(Caller);

  // Decision not made by replay system
  if (!hasInlineAdvice(*CB.getFunction())) {
    // If there's a registered original advisor, return its decision
    if (OriginalAdvisor)
      return OriginalAdvisor->getAdvice(CB);

    // If no decision is made above, return non-decision
    return {};
  }

  std::string CallSiteLoc =
      formatCallSiteLocation(CB.getDebugLoc(), ReplaySettings.ReplayFormat);
  StringRef Callee = CB.getCalledFunction()->getName();
  std::string Combined = (Callee + CallSiteLoc).str();

  // Replay decision, if it has one
  auto Iter = InlineSitesFromRemarks.find(Combined);
  if (Iter != InlineSitesFromRemarks.end()) {
    if (Iter->second) {
      LLVM_DEBUG(dbgs() << "Replay Inliner: Inlined " << Callee << " @ "
                        << CallSiteLoc << "\n");
      return std::make_unique<DefaultInlineAdvice>(
          this, CB, toolchain::InlineCost::getAlways("previously inlined"), ORE,
          EmitRemarks);
    } else {
      LLVM_DEBUG(dbgs() << "Replay Inliner: Not Inlined " << Callee << " @ "
                        << CallSiteLoc << "\n");
      // A negative inline is conveyed by "None" std::optional<InlineCost>
      return std::make_unique<DefaultInlineAdvice>(this, CB, std::nullopt, ORE,
                                                   EmitRemarks);
    }
  }

  // Fallback decisions
  if (ReplaySettings.ReplayFallback ==
      ReplayInlinerSettings::Fallback::AlwaysInline)
    return std::make_unique<DefaultInlineAdvice>(
        this, CB, toolchain::InlineCost::getAlways("AlwaysInline Fallback"), ORE,
        EmitRemarks);
  else if (ReplaySettings.ReplayFallback ==
           ReplayInlinerSettings::Fallback::NeverInline)
    // A negative inline is conveyed by "None" std::optional<InlineCost>
    return std::make_unique<DefaultInlineAdvice>(this, CB, std::nullopt, ORE,
                                                 EmitRemarks);
  else {
    assert(ReplaySettings.ReplayFallback ==
           ReplayInlinerSettings::Fallback::Original);
    // If there's a registered original advisor, return its decision
    if (OriginalAdvisor)
      return OriginalAdvisor->getAdvice(CB);
  }

  // If no decision is made above, return non-decision
  return {};
}
