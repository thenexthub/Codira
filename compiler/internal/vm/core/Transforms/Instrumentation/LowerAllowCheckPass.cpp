//===- LowerAllowCheckPass.cpp ----------------------------------*- C++ -*-===//
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

#include "vm/core/Transforms/Instrumentation/LowerAllowCheckPass.h"

#include "vm/core/ADT/SmallVector.h"
#include "vm/core/ADT/Statistic.h"
#include "vm/core/ADT/StringExtras.h"
#include "vm/core/ADT/StringRef.h"
#include "vm/core/Analysis/OptimizationRemarkEmitter.h"
#include "vm/core/Analysis/ProfileSummaryInfo.h"
#include "vm/core/IR/Constants.h"
#include "vm/core/IR/DiagnosticInfo.h"
#include "vm/core/IR/InstIterator.h"
#include "vm/core/IR/Instructions.h"
#include "vm/core/IR/IntrinsicInst.h"
#include "vm/core/IR/Intrinsics.h"
#include "vm/core/IR/Metadata.h"
#include "vm/core/IR/Module.h"
#include "vm/core/Support/Debug.h"
#include "vm/core/Support/RandomNumberGenerator.h"
#include <memory>
#include <optional>
#include <random>

using namespace vm::core;

#define DEBUG_TYPE "lower-allow-check"

static cl::opt<int>
    HotPercentileCutoff("lower-allow-check-percentile-cutoff-hot",
                        cl::desc("Hot percentile cutoff."));

static cl::opt<float>
    RandomRate("lower-allow-check-random-rate",
               cl::desc("Probability value in the range [0.0, 1.0] of "
                        "unconditional pseudo-random checks."));

STATISTIC(NumChecksTotal, "Number of checks");
STATISTIC(NumChecksRemoved, "Number of removed checks");

struct RemarkInfo {
  ore::NV Kind;
  ore::NV F;
  ore::NV BB;
  explicit RemarkInfo(IntrinsicInst *II)
      : Kind("Kind", II->getArgOperand(0)),
        F("Function", II->getParent()->getParent()),
        BB("Block", II->getParent()->getName()) {}
};

static void emitRemark(IntrinsicInst *II, OptimizationRemarkEmitter &ORE,
                       bool Removed) {
  if (Removed) {
    ORE.emit([&]() {
      RemarkInfo Info(II);
      return OptimizationRemark(DEBUG_TYPE, "Removed", II)
             << "Removed check: Kind=" << Info.Kind << " F=" << Info.F
             << " BB=" << Info.BB;
    });
  } else {
    ORE.emit([&]() {
      RemarkInfo Info(II);
      return OptimizationRemarkMissed(DEBUG_TYPE, "Allowed", II)
             << "Allowed check: Kind=" << Info.Kind << " F=" << Info.F
             << " BB=" << Info.BB;
    });
  }
}

static bool lowerAllowChecks(Function &F, FunctionAnalysisManager &AM,
                             const LowerAllowCheckPass::Options &Opts) {
  // Lazy analysis getters.
  auto GetBFI = [&AM, &F, BFI = (BlockFrequencyInfo *)nullptr]() mutable
      -> const BlockFrequencyInfo & {
    if (!BFI)
      BFI = &AM.getResult<BlockFrequencyAnalysis>(F);
    return *BFI;
  };
  auto GetPSI = [&AM, &F, PSI = std::optional<ProfileSummaryInfo *>()]() mutable
      -> const ProfileSummaryInfo * {
    if (!PSI.has_value()) {
      auto &MAMProxy = AM.getResult<ModuleAnalysisManagerFunctionProxy>(F);
      PSI = MAMProxy.getCachedResult<ProfileSummaryAnalysis>(*F.getParent());
    }
    return *PSI;
  };
  auto GetORE = [&AM, &F, ORE = (OptimizationRemarkEmitter *)nullptr]() mutable
      -> OptimizationRemarkEmitter & {
    if (!ORE)
      ORE = &AM.getResult<OptimizationRemarkEmitterAnalysis>(F);
    return *ORE;
  };

  // List of intrinsics and the constant value they should be lowered to.
  SmallVector<std::pair<IntrinsicInst *, bool>, 16> ReplaceWithValue;
  std::unique_ptr<RandomNumberGenerator> Rng;

  auto GetRng = [&]() -> RandomNumberGenerator & {
    if (!Rng)
      Rng = F.getParent()->createRNG(F.getName());
    return *Rng;
  };

  auto GetCutoff = [&](const IntrinsicInst *II) -> unsigned {
    if (HotPercentileCutoff.getNumOccurrences())
      return HotPercentileCutoff;
    else if (II->getIntrinsicID() == Intrinsic::allow_ubsan_check) {
      auto *Kind = cast<ConstantInt>(II->getArgOperand(0));
      if (Kind->getZExtValue() < Opts.cutoffs.size())
        return Opts.cutoffs[Kind->getZExtValue()];
    } else if (II->getIntrinsicID() == Intrinsic::allow_runtime_check) {
      return Opts.runtime_check;
    }

    return 0;
  };

  auto ShouldRemoveHot = [&](const BasicBlock &BB, unsigned int cutoff) {
    if (cutoff == 1000000)
      return true;
    const ProfileSummaryInfo *PSI = GetPSI();
    return PSI && PSI->isHotCountNthPercentile(
                      cutoff, GetBFI().getBlockProfileCount(&BB).value_or(0));
  };

  auto ShouldRemoveRandom = [&]() {
    return RandomRate.getNumOccurrences() &&
           !std::bernoulli_distribution(RandomRate)(GetRng());
  };

  auto ShouldRemove = [&](const IntrinsicInst *II) {
    unsigned int cutoff = GetCutoff(II);
    return ShouldRemoveRandom() || ShouldRemoveHot(*(II->getParent()), cutoff);
  };

  for (Instruction &I : instructions(F)) {
    IntrinsicInst *II = dyn_cast<IntrinsicInst>(&I);
    if (!II)
      continue;
    auto ID = II->getIntrinsicID();
    switch (ID) {
    case Intrinsic::allow_ubsan_check:
    case Intrinsic::allow_runtime_check: {
      bool ToRemove = ShouldRemove(II);

      ReplaceWithValue.push_back({
          II,
          !ToRemove,
      });
      emitRemark(II, GetORE(), ToRemove);
      break;
    }
    case Intrinsic::allow_sanitize_address:
      ReplaceWithValue.push_back(
          {II, F.hasFnAttribute(Attribute::SanitizeAddress)});
      break;
    case Intrinsic::allow_sanitize_thread:
      ReplaceWithValue.push_back(
          {II, F.hasFnAttribute(Attribute::SanitizeThread)});
      break;
    case Intrinsic::allow_sanitize_memory:
      ReplaceWithValue.push_back(
          {II, F.hasFnAttribute(Attribute::SanitizeMemory)});
      break;
    case Intrinsic::allow_sanitize_hwaddress:
      ReplaceWithValue.push_back(
          {II, F.hasFnAttribute(Attribute::SanitizeHWAddress)});
      break;
    default:
      break;
    }
  }

  for (auto [I, V] : ReplaceWithValue) {
    ++NumChecksTotal;
    if (!V) // If the final value is false, the check is considered removed.
      ++NumChecksRemoved;
    I->replaceAllUsesWith(ConstantInt::getBool(I->getType(), V));
    I->eraseFromParent();
  }

  return !ReplaceWithValue.empty();
}

PreservedAnalyses LowerAllowCheckPass::run(Function &F,
                                           FunctionAnalysisManager &AM) {
  if (F.isDeclaration())
    return PreservedAnalyses::all();

  return lowerAllowChecks(F, AM, Opts)
             // We do not change the CFG, we only replace the intrinsics with
             // true or false.
             ? PreservedAnalyses::none().preserveSet<CFGAnalyses>()
             : PreservedAnalyses::all();
}

bool LowerAllowCheckPass::IsRequested() {
  return RandomRate.getNumOccurrences() ||
         HotPercentileCutoff.getNumOccurrences();
}

void LowerAllowCheckPass::printPipeline(
    raw_ostream &OS, function_ref<StringRef(StringRef)> MapClassName2PassName) {
  static_cast<PassInfoMixin<LowerAllowCheckPass> *>(this)->printPipeline(
      OS, MapClassName2PassName);
  OS << "<";

  // Format is <cutoffs[0,1,2]=70000;cutoffs[5,6,8]=90000>
  // but it's equally valid to specify
  //   cutoffs[0]=70000;cutoffs[1]=70000;cutoffs[2]=70000;cutoffs[5]=90000;...
  // and that's what we do here. It is verbose but valid and easy to verify
  // correctness.
  // TODO: print shorter output by combining adjacent runs, etc.
  int i = 0;
  ListSeparator LS(";");
  for (unsigned int cutoff : Opts.cutoffs) {
    if (cutoff > 0)
      OS << LS << "cutoffs[" << i << "]=" << cutoff;
    i++;
  }
  if (Opts.runtime_check)
    OS << LS << "runtime_check=" << Opts.runtime_check;

  OS << '>';
}
