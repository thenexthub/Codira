//===--- RCStateTransition.cpp --------------------------------------------===//
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

#define DEBUG_TYPE "arc-sequence-opts"

#include "RCStateTransition.h"
#include "language/SIL/SILInstruction.h"
#include "language/SIL/SILFunction.h"
#include "toolchain/ADT/StringSwitch.h"
#include "toolchain/Support/Debug.h"

using namespace language;

//===----------------------------------------------------------------------===//
//                                  Utility
//===----------------------------------------------------------------------===//

static bool isAutoreleasePoolCall(SILInstruction *I) {
  auto *AI = dyn_cast<ApplyInst>(I);
  if (!AI)
    return false;

  auto *Fn = AI->getReferencedFunctionOrNull();
  if (!Fn)
    return false;

  return toolchain::StringSwitch<bool>(Fn->getName())
      .Case("objc_autoreleasePoolPush", true)
      .Case("objc_autoreleasePoolPop", true)
      .Default(false);
}

//===----------------------------------------------------------------------===//
//                           RCStateTransitionKind
//===----------------------------------------------------------------------===//

RCStateTransitionKind language::getRCStateTransitionKind(SILNode *N) {
  switch (N->getKind()) {
  case SILNodeKind::StrongRetainInst:
  case SILNodeKind::RetainValueInst:
    return RCStateTransitionKind::StrongIncrement;

  case SILNodeKind::StrongReleaseInst:
  case SILNodeKind::ReleaseValueInst:
    return RCStateTransitionKind::StrongDecrement;

  case SILNodeKind::SILFunctionArgument: {
    auto *Arg = cast<SILFunctionArgument>(N);
    if (Arg->hasConvention(SILArgumentConvention::Direct_Owned))
      return RCStateTransitionKind::StrongEntrance;
    return RCStateTransitionKind::Unknown;
  }

  case SILNodeKind::ApplyInst: {
    auto *AI = cast<ApplyInst>(N);
    if (isAutoreleasePoolCall(AI))
      return RCStateTransitionKind::AutoreleasePoolCall;

    // If we have an @owned return value. This AI is a strong entrance for its
    // return value.
    //
    // TODO: When we support pairing retains with @owned parameters, we will
    // need to be able to handle the potential of multiple state transition
    // kinds.
    for (auto result : AI->getSubstCalleeConv().getDirectSILResults()) {
      if (result.getConvention() == ResultConvention::Owned)
        return RCStateTransitionKind::StrongEntrance;
    }

    return RCStateTransitionKind::Unknown;
  }

  // Alloc* are always allocating new classes so they are introducing new
  // values at +1.
  case SILNodeKind::AllocRefInst:
  case SILNodeKind::AllocRefDynamicInst:
  case SILNodeKind::AllocBoxInst:
    return RCStateTransitionKind::StrongEntrance;

  case SILNodeKind::PartialApplyInst:
    // Partial apply boxes are introduced at +1.
    return RCStateTransitionKind::StrongEntrance;

  default:
    return RCStateTransitionKind::Unknown;
  }
}

/// Define test functions for all of our abstract value kinds.
#define ABSTRACT_VALUE(Name, StartKind, EndKind)                           \
  bool language::isRCStateTransition ## Name(RCStateTransitionKind Kind) {    \
    return unsigned(RCStateTransitionKind::StartKind) <= unsigned(Kind) && \
      unsigned(RCStateTransitionKind::EndKind) >= unsigned(Kind);          \
  }
#include "RCStateTransition.def"

raw_ostream &toolchain::operator<<(raw_ostream &os, RCStateTransitionKind Kind) {
  switch (Kind) {
#define KIND(K)                                 \
  case RCStateTransitionKind::K:                \
    return os << #K;
#include "RCStateTransition.def"
  }
  toolchain_unreachable("Covered switch isn't covered?!");
}

//===----------------------------------------------------------------------===//
//                             RCStateTransition
//===----------------------------------------------------------------------===//

#define ABSTRACT_VALUE(Name, Start, End)            \
  bool RCStateTransition::is ## Name() const {      \
    return isRCStateTransition ## Name(getKind());  \
  }
#include "RCStateTransition.def"

bool RCStateTransition::matchingInst(SILInstruction *Inst) const {
  // We only pair mutators for now.
  if (!isMutator())
    return false;

  if (Kind == RCStateTransitionKind::StrongIncrement) {
    auto InstTransKind = getRCStateTransitionKind(Inst->asSILNode());
    return InstTransKind == RCStateTransitionKind::StrongDecrement;
  }

  if (Kind == RCStateTransitionKind::StrongDecrement) {
    auto InstTransKind = getRCStateTransitionKind(Inst->asSILNode());
    return InstTransKind == RCStateTransitionKind::StrongIncrement;
  }

  return false;
}

bool RCStateTransition::merge(const RCStateTransition &Other) {
  // If our kinds do not match, bail. We don't cross the streams.
  if (Kind != Other.Kind)
    return false;

  // If we are not a mutator, there is nothing further to do here.
  if (!isMutator())
    return true;

  Mutators = Mutators->merge(Other.Mutators);

  return true;
}

