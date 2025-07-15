//===--- OwnershipLifetimeCompletion.h ------------------------------------===//
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
///
/// OSSA lifetime completion adds lifetime ending instructions to make
/// linear lifetimes complete.
///
/// Completion is bottom-up recursive over nested borrow scopes. Additionally,
/// this may be extended to support dependent owned lifetimes in the future to
/// handle owned non-escaping values.
///
/// Lexical lifetimes can only be incomplete as a result of dead-end blocks. In
/// this case, their lifetime ends immediately before the dead-end block.
///
/// Nonlexical lifetimes can be incomplete for any reason. Their lifetime ends
/// at the liveness boundary.
///
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_SILOPTIMIZER_UTILS_OSSSALIFETIMECOMPLETION_H
#define LANGUAGE_SILOPTIMIZER_UTILS_OSSSALIFETIMECOMPLETION_H

#include "language/SIL/NodeDatastructures.h"
#include "language/SIL/OwnershipLiveness.h"
#include "language/SIL/SILFunction.h"

namespace language {

class DeadEndBlocks;

enum class LifetimeCompletion { NoLifetime, AlreadyComplete, WasCompleted };

class OSSALifetimeCompletion {
public:
  enum HandleTrivialVariable_t { IgnoreTrivialVariable, ExtendTrivialVariable };

private:
  /// If domInfo is nullptr, then InteriorLiveness never assumes dominance. As a
  /// result it may report extra unenclosedPhis. In that case, any attempt to
  /// create a new phi would result in an immediately redundant phi.
  const DominanceInfo *domInfo = nullptr;

  DeadEndBlocks &deadEndBlocks;

  /// Cache intructions already handled by the recursive algorithm to avoid
  /// recomputing their lifetimes.
  ValueSet completedValues;

  /// Extend trivial variables for lifetime diagnostics (only in SILGenCleanup).
  HandleTrivialVariable_t handleTrivialVariable;

  /// Whether verification of the computed liveness should be run even when the
  /// global setting is off.
  /// TODO: Remove this option.
  bool ForceLivenessVerification;

public:
  OSSALifetimeCompletion(
      SILFunction *function, const DominanceInfo *domInfo,
      DeadEndBlocks &deadEndBlocks,
      HandleTrivialVariable_t handleTrivialVariable = IgnoreTrivialVariable,
      bool forceLivenessVerification = false)
      : domInfo(domInfo), deadEndBlocks(deadEndBlocks),
        completedValues(function), handleTrivialVariable(handleTrivialVariable),
        ForceLivenessVerification(forceLivenessVerification) {}

  /// The kind of boundary at which to complete the lifetime.
  ///
  /// Liveness: "As early as possible."  Consume the value after the last
  ///           non-consuming uses.
  /// Availability: "As late as possible."  Consume the value in the last blocks
  ///               beyond the non-consuming uses in which the value has been
  ///               consumed on no incoming paths.
  struct Boundary {
    enum Value : uint8_t {
      Liveness,
      Availability,
    };
    Value value;

    Boundary(Value value) : value(value){};
    operator Value() const { return value; }
  };

  /// Insert a lifetime-ending instruction on every path to complete the OSSA
  /// lifetime of \p value along \p boundary.
  ///
  /// If \p boundary is not specified, the following boundary will be used:
  ///   \p value is lexical -> Boundary::Availability
  ///   \p value is non-lexical -> Boundary::Liveness
  ///
  /// Lifetime completion is only relevant for owned
  /// values or borrow introducers.
  ///
  /// Currently \p boundary == {Boundary::Availability} is used by Mem2Reg and
  /// TempRValueOpt and PredicatbleMemOpt to complete lexical enum values on
  /// trivial paths.
  ///
  /// Returns true if any new instructions were created to complete the
  /// lifetime.
  LifetimeCompletion completeOSSALifetime(SILValue value, Boundary boundary) {
    switch (value->getOwnershipKind()) {
    case OwnershipKind::None: {
      if (auto scopedAddress = ScopedAddressValue(value)) {
        break;
      }
      // During SILGenCleanup, extend move_value [var_decl].
      if (handleTrivialVariable == ExtendTrivialVariable
          && value->isFromVarDecl()) {
        break;
      }
      return LifetimeCompletion::NoLifetime;
    }
    case OwnershipKind::Owned:
      break;
    case OwnershipKind::Any:
      toolchain::report_fatal_error("value with any ownership kind!?");
    case OwnershipKind::Guaranteed:
    case OwnershipKind::Unowned: {
      BorrowedValue borrowedValue(value);
      if (!borrowedValue)
        return LifetimeCompletion::NoLifetime;

      if (!borrowedValue.isLocalScope())
        return LifetimeCompletion::AlreadyComplete;
    }
    }

    if (!completedValues.insert(value))
      return LifetimeCompletion::AlreadyComplete;

    return analyzeAndUpdateLifetime(value, boundary)
               ? LifetimeCompletion::WasCompleted
               : LifetimeCompletion::AlreadyComplete;
  }

  enum class LifetimeEnd : uint8_t {
    /// The lifetime ends at the boundary.
    Boundary,
    /// The lifetime "ends" in a loop.
    Loop,
  };

  static void visitAvailabilityBoundary(
      SILValue value, const SSAPrunedLiveness &liveness,
      toolchain::function_ref<void(SILInstruction *, LifetimeEnd end)> visit);

protected:
  bool analyzeAndUpdateLifetime(SILValue value, Boundary boundary);
  bool analyzeAndUpdateLifetime(ScopedAddressValue value, Boundary boundary);
};

//===----------------------------------------------------------------------===//
// UnreachableLifetimeCompletion
//===----------------------------------------------------------------------===//

/// Fixup OSSA before deleting an unreachable code path.
///
/// Only needed when a code path reaches a no-return function, making the
/// path now partially unreachable. Conditional branch folding requires no fixup
/// because it causes the entire path to become unreachable.
class UnreachableLifetimeCompletion {
  SILFunction *function;

  // If domInfo is nullptr, lifetime completion may attempt to recreate
  // redundant phis, which should be immediately discarded.
  const DominanceInfo *domInfo = nullptr;
  DeadEndBlocks &deadEndBlocks;

  BasicBlockSetVector unreachableBlocks;
  InstructionSet unreachableInsts; // not including those in unreachableBlocks
  ValueSetVector incompleteValues;
  bool updatingLifetimes = false;

public:
  UnreachableLifetimeCompletion(SILFunction *function, DominanceInfo *domInfo,
                                DeadEndBlocks &deadEndBlocks)
      : function(function), domInfo(domInfo), deadEndBlocks(deadEndBlocks),
        unreachableBlocks(function), unreachableInsts(function),
        incompleteValues(function) {}

  /// Record information about this unreachable instruction and return true if
  /// ends any simple OSSA lifetimes.
  ///
  /// Note: this must be called in forward order so that lifetime completion
  /// runs from the inside out.
  void visitUnreachableInst(SILInstruction *instruction);

  void visitUnreachableBlock(SILBasicBlock *block) {
    unreachableBlocks.insert(block);
  }

  /// Complete the lifetime of any value defined outside of the unreachable
  /// region that was previously destroyed in the unreachable region.
  bool completeLifetimes();
};

inline toolchain::raw_ostream &
operator<<(toolchain::raw_ostream &OS, OSSALifetimeCompletion::Boundary boundary) {
  switch (boundary) {
  case OSSALifetimeCompletion::Boundary::Liveness:
    OS << "liveness";
    break;
  case OSSALifetimeCompletion::Boundary::Availability:
    OS << "availability";
    break;
  }
  return OS;
}

} // namespace language

#endif
