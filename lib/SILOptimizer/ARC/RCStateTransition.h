//===--- RCStateTransition.h ------------------------------------*- C++ -*-===//
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

#ifndef LANGUAGE_SILOPTIMIZER_PASSMANAGER_ARC_RCSTATETRANSITION_H
#define LANGUAGE_SILOPTIMIZER_PASSMANAGER_ARC_RCSTATETRANSITION_H

#include "language/Basic/type_traits.h"
#include "language/Basic/ImmutablePointerSet.h"
#include "language/SIL/SILArgument.h"
#include "language/SIL/SILInstruction.h"
#include "toolchain/ADT/SmallPtrSet.h"
#include <cstdint>

namespace language {

class RCIdentityFunctionInfo;
class ConsumedArgToEpilogueReleaseMatcher;

} // end language namespace

//===----------------------------------------------------------------------===//
//                           RCStateTransitionKind
//===----------------------------------------------------------------------===//

namespace language {

/// The kind of an RCStateTransition.
enum class RCStateTransitionKind : uint8_t {
#define KIND(K) K,
#define ABSTRACT_VALUE(Name, StartKind, EndKind) \
  Name ## _Start = StartKind, Name ## _End = EndKind,
#include "RCStateTransition.def"
};

/// \returns the RCStateTransitionKind corresponding to \p N.
RCStateTransitionKind getRCStateTransitionKind(SILNode *N);

/// Define predicates to test for RCStateTransition abstract value kinds.
#define ABSTRACT_VALUE(Name, Start, End)                              \
  bool isRCStateTransition ## Name(RCStateTransitionKind Kind);       \
  static inline bool isRCStateTransition ## Name(SILNode *N) {        \
    return isRCStateTransition ## Name(getRCStateTransitionKind(N));  \
  }
#define KIND(Name)                                                      \
  static inline bool isRCStateTransition ## Name(SILNode *N) {          \
    return RCStateTransitionKind::Name == getRCStateTransitionKind(N);  \
  }
#include "RCStateTransition.def"

//===----------------------------------------------------------------------===//
//                             RCStateTransition
//===----------------------------------------------------------------------===//

class RefCountState;
class BottomUpRefCountState;
class TopDownRefCountState;

/// Represents a transition in the RC history of a ref count.
class RCStateTransition {
  friend class RefCountState;
  friend class BottomUpRefCountState;
  friend class TopDownRefCountState;

  /// An RCStateTransition can represent either an RC end point (i.e. an initial
  /// or terminal RC transition) or a ptr set of Mutators.
  SILNode *EndPoint;
  ImmutablePointerSet<SILInstruction *> *Mutators =
      ImmutablePointerSetFactory<SILInstruction *>::getEmptySet();
  RCStateTransitionKind Kind;

  // Should only be constructed be default RefCountState.
  RCStateTransition() = default;

public:
  ~RCStateTransition() = default;
  RCStateTransition(const RCStateTransition &R) = default;

  RCStateTransition(ImmutablePointerSet<SILInstruction *> *I) {
    assert(I->size() == 1);
    auto *Inst = *I->begin();
    Kind = getRCStateTransitionKind(Inst->asSILNode());
    if (isRCStateTransitionEndPoint(Kind)) {
      EndPoint = Inst->asSILNode();
      return;
    }

    if (isRCStateTransitionMutator(Kind)) {
      Mutators = I;
      return;
    }

    // Unknown kind.
  }

  RCStateTransition(SILFunctionArgument *A)
      : EndPoint(A), Kind(RCStateTransitionKind::StrongEntrance) {
    assert(A->hasConvention(SILArgumentConvention::Direct_Owned) &&
           "Expected owned argument");
  }

  RCStateTransitionKind getKind() const { return Kind; }

/// Define test functions for the various abstract categorizations we have.
#define ABSTRACT_VALUE(Name, StartKind, EndKind) bool is ## Name() const;
#include "RCStateTransition.def"

  /// Return true if this Transition is a mutator transition that contains I.
  bool containsMutator(SILInstruction *I) const {
    assert(isMutator() && "This should only be called if we are of mutator "
                          "kind");
    return Mutators->count(I);
  }

  using mutator_range =
      iterator_range<std::remove_pointer<decltype(Mutators)>::type::iterator>;

  /// Returns a Range of Mutators. Asserts if this transition is not a mutator
  /// transition.
  mutator_range getMutators() const {
    return {Mutators->begin(), Mutators->end()};
  }

  /// Return true if Inst is an instruction that causes a transition that can be
  /// paired with this transition.
  bool matchingInst(SILInstruction *Inst) const;

  /// Attempt to merge \p Other into \p this. Returns true if we succeeded,
  /// false otherwise.
  bool merge(const RCStateTransition &Other);

  /// Return true if the kind of this RCStateTransition is not 'Invalid'.
  bool isValid() const { return getKind() != RCStateTransitionKind::Invalid; }
};

// These static assert checks are here for performance reasons.
static_assert(IsTriviallyCopyable<RCStateTransition>::value,
              "RCStateTransitions must be trivially copyable");

} // end language namespace

namespace toolchain {
raw_ostream &operator<<(raw_ostream &os, language::RCStateTransitionKind Kind);
} // end toolchain namespace

#endif
