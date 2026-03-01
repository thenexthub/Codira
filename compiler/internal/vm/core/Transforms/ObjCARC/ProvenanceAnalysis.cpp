//===- ProvenanceAnalysis.cpp - ObjC ARC Optimization ---------------------===//
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
/// \file
///
/// This file defines a special form of Alias Analysis called ``Provenance
/// Analysis''. The word ``provenance'' refers to the history of the ownership
/// of an object. Thus ``Provenance Analysis'' is an analysis which attempts to
/// use various techniques to determine if locally
///
/// WARNING: This file knows about certain library functions. It recognizes them
/// by name, and hardwires knowledge of their semantics.
///
/// WARNING: This file knows about how certain Objective-C library functions are
/// used. Naive LLVM IR transformations which would otherwise be
/// behavior-preserving may break these assumptions.
//
//===----------------------------------------------------------------------===//

#include "ProvenanceAnalysis.h"
#include "vm/core/ADT/SmallPtrSet.h"
#include "vm/core/ADT/SmallVector.h"
#include "vm/core/Analysis/AliasAnalysis.h"
#include "vm/core/Analysis/ObjCARCAnalysisUtils.h"
#include "vm/core/IR/Instructions.h"
#include "vm/core/IR/Use.h"
#include "vm/core/IR/User.h"
#include "vm/core/IR/Value.h"
#include "vm/core/Support/Casting.h"
#include <utility>

using namespace vm::core;
using namespace vm::core::objcarc;

bool ProvenanceAnalysis::relatedSelect(const SelectInst *A,
                                       const Value *B) {
  // If the values are Selects with the same condition, we can do a more precise
  // check: just check for relations between the values on corresponding arms.
  if (const SelectInst *SB = dyn_cast<SelectInst>(B))
    if (A->getCondition() == SB->getCondition())
      return related(A->getTrueValue(), SB->getTrueValue()) ||
             related(A->getFalseValue(), SB->getFalseValue());

  // Check both arms of the Select node individually.
  return related(A->getTrueValue(), B) || related(A->getFalseValue(), B);
}

bool ProvenanceAnalysis::relatedPHI(const PHINode *A,
                                    const Value *B) {
  // If the values are PHIs in the same block, we can do a more precise as well
  // as efficient check: just check for relations between the values on
  // corresponding edges.
  if (const PHINode *PNB = dyn_cast<PHINode>(B))
    if (PNB->getParent() == A->getParent()) {
      for (unsigned i = 0, e = A->getNumIncomingValues(); i != e; ++i)
        if (related(A->getIncomingValue(i),
                    PNB->getIncomingValueForBlock(A->getIncomingBlock(i))))
          return true;
      return false;
    }

  // Check each unique source of the PHI node against B.
  SmallPtrSet<const Value *, 4> UniqueSrc;
  for (Value *PV1 : A->incoming_values()) {
    if (UniqueSrc.insert(PV1).second && related(PV1, B))
      return true;
  }

  // All of the arms checked out.
  return false;
}

/// Test if the value of P, or any value covered by its provenance, is ever
/// stored within the function (not counting callees).
static bool IsStoredObjCPointer(const Value *P) {
  if (!P->hasUseList())
    return true; // Assume the worst for a constant pointer.

  SmallPtrSet<const Value *, 8> Visited;
  SmallVector<const Value *, 8> Worklist;
  Worklist.push_back(P);
  Visited.insert(P);
  do {
    P = Worklist.pop_back_val();
    for (const Use &U : P->uses()) {
      const User *Ur = U.getUser();
      if (isa<StoreInst>(Ur)) {
        if (U.getOperandNo() == 0)
          // The pointer is stored.
          return true;
        // The pointed is stored through.
        continue;
      }
      if (isa<CallInst>(Ur))
        // The pointer is passed as an argument, ignore this.
        continue;
      if (isa<PtrToIntInst>(P))
        // Assume the worst.
        return true;
      if (Visited.insert(Ur).second)
        Worklist.push_back(Ur);
    }
  } while (!Worklist.empty());

  // Everything checked out.
  return false;
}

bool ProvenanceAnalysis::relatedCheck(const Value *A, const Value *B) {
  // Ask regular AliasAnalysis, for a first approximation.
  switch (AA->alias(A, B)) {
  case AliasResult::NoAlias:
    return false;
  case AliasResult::MustAlias:
  case AliasResult::PartialAlias:
    return true;
  case AliasResult::MayAlias:
    break;
  }

  bool AIsIdentified = IsObjCIdentifiedObject(A);
  bool BIsIdentified = IsObjCIdentifiedObject(B);

  // An ObjC-Identified object can't alias a load if it is never locally stored.
  if (AIsIdentified) {
    // Check for an obvious escape.
    if (isa<LoadInst>(B))
      return IsStoredObjCPointer(A);
    if (BIsIdentified) {
      // Check for an obvious escape.
      if (isa<LoadInst>(A))
        return IsStoredObjCPointer(B);
      // Both pointers are identified and escapes aren't an evident problem.
      return false;
    }
  } else if (BIsIdentified) {
    // Check for an obvious escape.
    if (isa<LoadInst>(A))
      return IsStoredObjCPointer(B);
  }

   // Special handling for PHI and Select.
  if (const PHINode *PN = dyn_cast<PHINode>(A))
    return relatedPHI(PN, B);
  if (const PHINode *PN = dyn_cast<PHINode>(B))
    return relatedPHI(PN, A);
  if (const SelectInst *S = dyn_cast<SelectInst>(A))
    return relatedSelect(S, B);
  if (const SelectInst *S = dyn_cast<SelectInst>(B))
    return relatedSelect(S, A);

  // Conservative.
  return true;
}

bool ProvenanceAnalysis::related(const Value *A, const Value *B) {
  A = GetUnderlyingObjCPtrCached(A, UnderlyingObjCPtrCache);
  B = GetUnderlyingObjCPtrCached(B, UnderlyingObjCPtrCache);

  // Quick check.
  if (A == B)
    return true;

  // Begin by inserting a conservative value into the map. If the insertion
  // fails, we have the answer already. If it succeeds, leave it there until we
  // compute the real answer to guard against recursive queries.
  std::pair<CachedResultsTy::iterator, bool> Pair =
    CachedResults.insert(std::make_pair(ValuePairTy(A, B), true));
  if (!Pair.second)
    return Pair.first->second;

  bool Result = relatedCheck(A, B);
  CachedResults[ValuePairTy(A, B)] = Result;
  return Result;
}
