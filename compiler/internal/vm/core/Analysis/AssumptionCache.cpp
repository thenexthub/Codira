//===- AssumptionCache.cpp - Cache finding @toolchain.assume calls -------------===//
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
// This file contains a pass that keeps track of @toolchain.assume intrinsics in
// the functions of a module.
//
//===----------------------------------------------------------------------===//

#include "vm/core/Analysis/AssumptionCache.h"
#include "vm/core/ADT/STLExtras.h"
#include "vm/core/ADT/SmallPtrSet.h"
#include "vm/core/ADT/SmallVector.h"
#include "vm/core/Analysis/AssumeBundleQueries.h"
#include "vm/core/Analysis/TargetTransformInfo.h"
#include "vm/core/Analysis/ValueTracking.h"
#include "vm/core/IR/BasicBlock.h"
#include "vm/core/IR/Function.h"
#include "vm/core/IR/InstrTypes.h"
#include "vm/core/IR/Instruction.h"
#include "vm/core/IR/Instructions.h"
#include "vm/core/IR/PassManager.h"
#include "vm/core/IR/PatternMatch.h"
#include "vm/core/InitializePasses.h"
#include "vm/core/Pass.h"
#include "vm/core/Support/Casting.h"
#include "vm/core/Support/CommandLine.h"
#include "vm/core/Support/ErrorHandling.h"
#include "vm/core/Support/raw_ostream.h"
#include <cassert>

using namespace vm::core;
using namespace vm::core::PatternMatch;

static cl::opt<bool>
    VerifyAssumptionCache("verify-assumption-cache", cl::Hidden,
                          cl::desc("Enable verification of assumption cache"),
                          cl::init(false));

SmallVector<AssumptionCache::ResultElem, 1> &
AssumptionCache::getOrInsertAffectedValues(Value *V) {
  // Try using find_as first to avoid creating extra value handles just for the
  // purpose of doing the lookup.
  auto AVI = AffectedValues.find_as(V);
  if (AVI != AffectedValues.end())
    return AVI->second;

  return AffectedValues[AffectedValueCallbackVH(V, this)];
}

void AssumptionCache::findValuesAffectedByOperandBundle(
    OperandBundleUse Bundle, function_ref<void(Value *)> InsertAffected) {
  auto AddAffectedVal = [&](Value *V) {
    if (isa<Argument, GlobalValue, Instruction>(V))
      InsertAffected(V);
  };

  if (Bundle.getTagName() == "separate_storage") {
    assert(Bundle.Inputs.size() == 2 && "separate_storage must have two args");
    AddAffectedVal(getUnderlyingObject(Bundle.Inputs[0]));
    AddAffectedVal(getUnderlyingObject(Bundle.Inputs[1]));
  } else if (Bundle.Inputs.size() > ABA_WasOn &&
             Bundle.getTagName() != IgnoreBundleTag)
    AddAffectedVal(Bundle.Inputs[ABA_WasOn]);
}

static void
findAffectedValues(CallBase *CI, TargetTransformInfo *TTI,
                   SmallVectorImpl<AssumptionCache::ResultElem> &Affected) {
  // Note: This code must be kept in-sync with the code in
  // computeKnownBitsFromAssume in ValueTracking.

  auto InsertAffected = [&Affected](Value *V) {
    Affected.push_back({V, AssumptionCache::ExprResultIdx});
  };

  auto AddAffectedVal = [&Affected](Value *V, unsigned Idx) {
    if (isa<Argument>(V) || isa<GlobalValue>(V) || isa<Instruction>(V)) {
      Affected.push_back({V, Idx});
    }
  };

  for (unsigned Idx = 0; Idx != CI->getNumOperandBundles(); Idx++)
    AssumptionCache::findValuesAffectedByOperandBundle(
        CI->getOperandBundleAt(Idx),
        [&](Value *V) { Affected.push_back({V, Idx}); });

  Value *Cond = CI->getArgOperand(0);
  findValuesAffectedByCondition(Cond, /*IsAssume=*/true, InsertAffected);

  if (TTI) {
    const Value *Ptr;
    unsigned AS;
    std::tie(Ptr, AS) = TTI->getPredicatedAddrSpace(Cond);
    if (Ptr)
      AddAffectedVal(const_cast<Value *>(Ptr->stripInBoundsOffsets()),
                     AssumptionCache::ExprResultIdx);
  }
}

void AssumptionCache::updateAffectedValues(AssumeInst *CI) {
  SmallVector<AssumptionCache::ResultElem, 16> Affected;
  findAffectedValues(CI, TTI, Affected);

  for (auto &AV : Affected) {
    auto &AVV = getOrInsertAffectedValues(AV.Assume);
    if (toolchain::none_of(AVV, [&](ResultElem &Elem) {
          return Elem.Assume == CI && Elem.Index == AV.Index;
        }))
      AVV.push_back({CI, AV.Index});
  }
}

void AssumptionCache::unregisterAssumption(AssumeInst *CI) {
  SmallVector<AssumptionCache::ResultElem, 16> Affected;
  findAffectedValues(CI, TTI, Affected);

  for (auto &AV : Affected) {
    auto AVI = AffectedValues.find_as(AV.Assume);
    if (AVI == AffectedValues.end())
      continue;
    bool Found = false;
    bool HasNonnull = false;
    for (ResultElem &Elem : AVI->second) {
      if (Elem.Assume == CI) {
        Found = true;
        Elem.Assume = nullptr;
      }
      HasNonnull |= !!Elem.Assume;
      if (HasNonnull && Found)
        break;
    }
    assert(Found && "already unregistered or incorrect cache state");
    if (!HasNonnull)
      AffectedValues.erase(AVI);
  }

  toolchain::erase(AssumeHandles, CI);
}

void AssumptionCache::AffectedValueCallbackVH::deleted() {
  AC->AffectedValues.erase(getValPtr());
  // 'this' now dangles!
}

void AssumptionCache::transferAffectedValuesInCache(Value *OV, Value *NV) {
  auto &NAVV = getOrInsertAffectedValues(NV);
  auto AVI = AffectedValues.find(OV);
  if (AVI == AffectedValues.end())
    return;

  for (auto &A : AVI->second)
    if (!toolchain::is_contained(NAVV, A))
      NAVV.push_back(A);
  AffectedValues.erase(OV);
}

void AssumptionCache::AffectedValueCallbackVH::allUsesReplacedWith(Value *NV) {
  if (!isa<Instruction>(NV) && !isa<Argument>(NV))
    return;

  // Any assumptions that affected this value now affect the new value.

  AC->transferAffectedValuesInCache(getValPtr(), NV);
  // 'this' now might dangle! If the AffectedValues map was resized to add an
  // entry for NV then this object might have been destroyed in favor of some
  // copy in the grown map.
}

void AssumptionCache::scanFunction() {
  assert(!Scanned && "Tried to scan the function twice!");
  assert(AssumeHandles.empty() && "Already have assumes when scanning!");

  // Go through all instructions in all blocks, add all calls to @toolchain.assume
  // to this cache.
  for (BasicBlock &B : F)
    for (Instruction &I : B)
      if (isa<AssumeInst>(&I))
        AssumeHandles.push_back(&I);

  // Mark the scan as complete.
  Scanned = true;

  // Update affected values.
  for (auto &A : AssumeHandles)
    updateAffectedValues(cast<AssumeInst>(A));
}

void AssumptionCache::registerAssumption(AssumeInst *CI) {
  // If we haven't scanned the function yet, just drop this assumption. It will
  // be found when we scan later.
  if (!Scanned)
    return;

  AssumeHandles.push_back(CI);

#ifndef NDEBUG
  assert(CI->getParent() &&
         "Cannot register @toolchain.assume call not in a basic block");
  assert(&F == CI->getParent()->getParent() &&
         "Cannot register @toolchain.assume call not in this function");

  // We expect the number of assumptions to be small, so in an asserts build
  // check that we don't accumulate duplicates and that all assumptions point
  // to the same function.
  SmallPtrSet<Value *, 16> AssumptionSet;
  for (auto &VH : AssumeHandles) {
    if (!VH)
      continue;

    assert(&F == cast<Instruction>(VH)->getParent()->getParent() &&
           "Cached assumption not inside this function!");
    assert(match(cast<CallInst>(VH), m_Intrinsic<Intrinsic::assume>()) &&
           "Cached something other than a call to @toolchain.assume!");
    assert(AssumptionSet.insert(VH).second &&
           "Cache contains multiple copies of a call!");
  }
#endif

  updateAffectedValues(CI);
}

AssumptionCache AssumptionAnalysis::run(Function &F,
                                        FunctionAnalysisManager &FAM) {
  auto &TTI = FAM.getResult<TargetIRAnalysis>(F);
  return AssumptionCache(F, &TTI);
}

AnalysisKey AssumptionAnalysis::Key;

PreservedAnalyses AssumptionPrinterPass::run(Function &F,
                                             FunctionAnalysisManager &AM) {
  AssumptionCache &AC = AM.getResult<AssumptionAnalysis>(F);

  OS << "Cached assumptions for function: " << F.getName() << "\n";
  for (auto &VH : AC.assumptions())
    if (VH)
      OS << "  " << *cast<CallInst>(VH)->getArgOperand(0) << "\n";

  return PreservedAnalyses::all();
}

void AssumptionCacheTracker::FunctionCallbackVH::deleted() {
  auto I = ACT->AssumptionCaches.find_as(cast<Function>(getValPtr()));
  if (I != ACT->AssumptionCaches.end())
    ACT->AssumptionCaches.erase(I);
  // 'this' now dangles!
}

AssumptionCache &AssumptionCacheTracker::getAssumptionCache(Function &F) {
  // We probe the function map twice to try and avoid creating a value handle
  // around the function in common cases. This makes insertion a bit slower,
  // but if we have to insert we're going to scan the whole function so that
  // shouldn't matter.
  auto I = AssumptionCaches.find_as(&F);
  if (I != AssumptionCaches.end())
    return *I->second;

  auto *TTIWP = getAnalysisIfAvailable<TargetTransformInfoWrapperPass>();
  auto *TTI = TTIWP ? &TTIWP->getTTI(F) : nullptr;

  // Ok, build a new cache by scanning the function, insert it and the value
  // handle into our map, and return the newly populated cache.
  auto IP = AssumptionCaches.insert(std::make_pair(
      FunctionCallbackVH(&F, this), std::make_unique<AssumptionCache>(F, TTI)));
  assert(IP.second && "Scanning function already in the map?");
  return *IP.first->second;
}

AssumptionCache *AssumptionCacheTracker::lookupAssumptionCache(Function &F) {
  auto I = AssumptionCaches.find_as(&F);
  if (I != AssumptionCaches.end())
    return I->second.get();
  return nullptr;
}

void AssumptionCacheTracker::verifyAnalysis() const {
  // FIXME: In the long term the verifier should not be controllable with a
  // flag. We should either fix all passes to correctly update the assumption
  // cache and enable the verifier unconditionally or somehow arrange for the
  // assumption list to be updated automatically by passes.
  if (!VerifyAssumptionCache)
    return;

  SmallPtrSet<const CallInst *, 4> AssumptionSet;
  for (const auto &I : AssumptionCaches) {
    for (auto &VH : I.second->assumptions())
      if (VH)
        AssumptionSet.insert(cast<CallInst>(VH));

    for (const BasicBlock &B : cast<Function>(*I.first))
      for (const Instruction &II : B)
        if (match(&II, m_Intrinsic<Intrinsic::assume>()) &&
            !AssumptionSet.count(cast<CallInst>(&II)))
          report_fatal_error("Assumption in scanned function not in cache");
  }
}

AssumptionCacheTracker::AssumptionCacheTracker() : ImmutablePass(ID) {}

AssumptionCacheTracker::~AssumptionCacheTracker() = default;

char AssumptionCacheTracker::ID = 0;

INITIALIZE_PASS(AssumptionCacheTracker, "assumption-cache-tracker",
                "Assumption Cache Tracker", false, true)
