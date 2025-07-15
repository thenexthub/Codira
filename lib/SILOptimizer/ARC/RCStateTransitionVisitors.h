//===--- RCStateTransitionVisitors.h ----------------------------*- C++ -*-===//
//
// Copyright (c) NeXTHub Corporation. All rights reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// This code is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// version 2 for more details (a copy is included in the LICENSE file that
// accompanied this code).
//
// Author(-s): Tunjay Akbarli
//

//===----------------------------------------------------------------------===//
//
// This file contains RCStateTransitionVisitors for performing ARC dataflow. It
// is necessary to prevent a circular dependency in between RefCountState.h and
// RCStateTransition.h
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_SILOPTIMIZER_PASSMANAGER_ARC_RCSTATETRANSITIONVISITORS_H
#define LANGUAGE_SILOPTIMIZER_PASSMANAGER_ARC_RCSTATETRANSITIONVISITORS_H

#include "ARCBBState.h"
#include "ARCRegionState.h"
#include "RCStateTransition.h"
#include "language/Basic/ImmutablePointerSet.h"
#include "language/Basic/BlotMapVector.h"

//===----------------------------------------------------------------------===//
//                          RCStateTransitionVisitor
//===----------------------------------------------------------------------===//

namespace language {

/// Define a visitor for visiting instructions according to their
/// RCStateTransitionKind.
template <typename ImplTy, typename ResultTy>
class RCStateTransitionKindVisitor {
  ImplTy &asImpl() { return *reinterpret_cast<ImplTy *>(this); }

public:
#define KIND(K) ResultTy visit ## K(SILNode *) { return ResultTy(); }
#include "RCStateTransition.def"

  ResultTy visit(SILNode *N) {
    switch (getRCStateTransitionKind(N)) {
#define KIND(K)                                 \
  case RCStateTransitionKind::K:                \
    return asImpl().visit ## K(N);
#include "RCStateTransition.def"
    }
    toolchain_unreachable("Covered switch isn't covered?!");
  }
};

} // end language namespace

//===----------------------------------------------------------------------===//
//                      RCStateTransitionDataflowResult
//===----------------------------------------------------------------------===//

namespace language {

enum class RCStateTransitionDataflowResultKind {
  /// Can this dataflow result have no further effects on any state. This means
  /// we can just early out and break early.
  NoEffects,

  /// Must we check for effects.
  CheckForEffects,
};

struct RCStateTransitionDataflowResult {
  using DataflowResultKind = RCStateTransitionDataflowResultKind;

  DataflowResultKind Kind;
  SILValue RCIdentity;
  bool NestingDetected = false;

  RCStateTransitionDataflowResult()
      : Kind(DataflowResultKind::CheckForEffects), RCIdentity() {}
  RCStateTransitionDataflowResult(DataflowResultKind Kind)
      : Kind(Kind), RCIdentity() {}
  RCStateTransitionDataflowResult(SILValue RCIdentity,
                                  bool NestingDetected = false)
      : Kind(DataflowResultKind::CheckForEffects), RCIdentity(RCIdentity),
        NestingDetected(NestingDetected) {}
  RCStateTransitionDataflowResult(const RCStateTransitionDataflowResult &) =
      default;
  ~RCStateTransitionDataflowResult() = default;
};

} // end language namespace

namespace toolchain {
raw_ostream &operator<<(raw_ostream &os,
                        language::RCStateTransitionDataflowResult Kind);
} // end toolchain namespace

//===----------------------------------------------------------------------===//
//                       BottomUpDataflowRCStateVisitor
//===----------------------------------------------------------------------===//

namespace language {

/// A visitor for performing the bottom up dataflow depending on the
/// RCState. Enables behavior to be cleanly customized depending on the
/// RCStateTransition associated with an instruction.
template <class ARCState>
class BottomUpDataflowRCStateVisitor
    : public RCStateTransitionKindVisitor<BottomUpDataflowRCStateVisitor<ARCState>,
                                          RCStateTransitionDataflowResult> {
  /// A local typedef to make things cleaner.
  using DataflowResult = RCStateTransitionDataflowResult;
  using IncToDecStateMapTy =
      BlotMapVector<SILInstruction *, BottomUpRefCountState>;

public:
  RCIdentityFunctionInfo *RCFI;
  EpilogueARCFunctionInfo *EAFI;
  ARCState &DataflowState;
  bool FreezeOwnedArgEpilogueReleases;
  BlotMapVector<SILInstruction *, BottomUpRefCountState> &IncToDecStateMap;
  ImmutablePointerSetFactory<SILInstruction *> &SetFactory;

public:
  BottomUpDataflowRCStateVisitor(
      RCIdentityFunctionInfo *RCFI, EpilogueARCFunctionInfo *EAFI,
      ARCState &DataflowState, bool FreezeOwnedArgEpilogueReleases,
      IncToDecStateMapTy &IncToDecStateMap,
      ImmutablePointerSetFactory<SILInstruction *> &SetFactory);
  DataflowResult visitAutoreleasePoolCall(SILNode *N);
  DataflowResult visitStrongDecrement(SILNode *N);
  DataflowResult visitStrongIncrement(SILNode *N);
};

} // end language namespace

//===----------------------------------------------------------------------===//
//                       TopDownDataflowRCStateVisitor
//===----------------------------------------------------------------------===//

namespace language {

/// A visitor for performing the bottom up dataflow depending on the
/// RCState. Enables behavior to be cleanly customized depending on the
/// RCStateTransition associated with an instruction.
template <class ARCState>
class TopDownDataflowRCStateVisitor
    : public RCStateTransitionKindVisitor<TopDownDataflowRCStateVisitor<ARCState>,
                                          RCStateTransitionDataflowResult> {
  /// A local typedef to make things cleaner.
  using DataflowResult = RCStateTransitionDataflowResult;
  using DecToIncStateMapTy =
      BlotMapVector<SILInstruction *, TopDownRefCountState>;

  RCIdentityFunctionInfo *RCFI;
  ARCState &DataflowState;
  DecToIncStateMapTy &DecToIncStateMap;
  ImmutablePointerSetFactory<SILInstruction *> &SetFactory;

public:
  TopDownDataflowRCStateVisitor(
      RCIdentityFunctionInfo *RCFI, ARCState &State,
      DecToIncStateMapTy &DecToIncStateMap,
      ImmutablePointerSetFactory<SILInstruction *> &SetFactory);
  DataflowResult visitAutoreleasePoolCall(SILNode *N);
  DataflowResult visitStrongDecrement(SILNode *N);
  DataflowResult visitStrongIncrement(SILNode *N);
  DataflowResult visitStrongEntrance(SILNode *N);

private:
  DataflowResult visitStrongEntranceApply(ApplyInst *AI);
  DataflowResult visitStrongEntrancePartialApply(PartialApplyInst *PAI);
  DataflowResult visitStrongEntranceArgument(SILFunctionArgument *Arg);
  DataflowResult visitStrongEntranceAllocRef(AllocRefInst *ARI);
  DataflowResult visitStrongEntranceAllocRefDynamic(AllocRefDynamicInst *ARI);
  DataflowResult visitStrongAllocBox(AllocBoxInst *ABI);
};

} // end language namespace

//===----------------------------------------------------------------------===//
//                       Forward Template Declarations
//===----------------------------------------------------------------------===//

namespace language {

extern template class BottomUpDataflowRCStateVisitor<
    ARCSequenceDataflowEvaluator::ARCBBState>;
extern template class BottomUpDataflowRCStateVisitor<ARCRegionState>;

extern template class TopDownDataflowRCStateVisitor<
    ARCSequenceDataflowEvaluator::ARCBBState>;
extern template class TopDownDataflowRCStateVisitor<ARCRegionState>;

} // end language namespace

#endif
