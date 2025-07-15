//===--- LinearLifetimeChecker.h ------------------------------------------===//
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

#ifndef LANGUAGE_SIL_LINEARLIFETIMECHECKER_H
#define LANGUAGE_SIL_LINEARLIFETIMECHECKER_H

#include "language/Basic/Debug.h"
#include "language/Basic/Toolchain.h"
#include "language/SIL/SILArgument.h"
#include "language/SIL/SILInstruction.h"
#include "language/SIL/SILBasicBlock.h"
#include "language/SIL/SILFunction.h"
#include "language/SIL/SILValue.h"
#include "language/SIL/BasicBlockUtils.h"
#include "language/SIL/BasicBlockBits.h"
#include "toolchain/ADT/SmallPtrSet.h"

namespace language {

class SILBasicBlock;
class SILInstruction;
class SILModule;
class SILValue;
class SILOwnershipVerifier;
class SILValueOwnershipChecker;

/// A class used to validate linear lifetime with respect to an SSA-like
/// definition.
///
/// This class is able to both validate that a linear lifetime has been properly
/// constructed (for verification and safety purposes) as well as return to the
/// caller upon failure, what the failure was. In certain cases (for instance if
/// there exists a path without a non-consuming use), the class will report back
/// the specific insertion points needed to insert these compensating releases.
///
/// DISCUSSION: A linear lifetime consists of a starting block or instruction
/// and a list of non-consuming uses and a set of consuming uses. The consuming
/// uses must not be reachable from each other and jointly post-dominate all
/// consuming uses as well as the defining block/instruction.
class LinearLifetimeChecker {
public:
  class Error;
  struct ErrorBehaviorKind;
  class ErrorBuilder;

private:
  friend class GuaranteedPhiVerifier;
  friend class SILOwnershipVerifier;
  friend class SILValueOwnershipChecker;

  // TODO: migrate away from using dead end blocks for OSSA values. end_borrow
  // or destroy_value should ideally exist on all paths. However, deadEndBlocks
  // may still be useful for checking memory lifetime for address uses.
  DeadEndBlocks *deadEndBlocks;

public:
  /// \p deadEndBlocks should be provided for lifetimes that do not require
  /// consuming uses on dead-end paths, which end in an unreachable terminator.
  /// OSSA values require consumes on all paths, so \p deadEndBlocks are *not*
  /// required for OSSA lifetimes. Memory lifetimes and access scopes only
  /// require destroys on non-dead-end paths.
  ///
  /// TODO: The verifier currently requires OSSA borrow scopes to end on all
  /// paths. Owned OSSA lifetimes may still be missing destroys on dead-end
  /// paths. Once owned values are fully enforced, the same invariant will hold
  /// for all OSSA values.
  LinearLifetimeChecker(DeadEndBlocks *deadEndBlocks = nullptr)
      : deadEndBlocks(deadEndBlocks) {}

  /// Returns true that \p value forms a linear lifetime with consuming uses \p
  /// consumingUses, non consuming uses \p nonConsumingUses. Returns false
  /// otherwise.
  bool validateLifetime(SILValue value, ArrayRef<Operand *> consumingUses,
                        ArrayRef<Operand *> nonConsumingUses);

  /// Given a value and a consuming use of that value, compute a non-unique
  /// minimal set of insertion points that together with \p consumingUse
  /// post-dominate and end the lifetime of \p value.
  ///
  /// Returns true if we completed the consuming use set and discovered that \p
  /// consumingUse is not strongly control equivalent to value (meaning
  /// consumingUse is not in the same loop in the loop nest as value).
  bool completeConsumingUseSet(
      SILValue value, Operand *consumingUse,
      function_ref<void(SILBasicBlock::iterator insertPt)> visitor);

  /// Given a linear lifetime defined by \p value and \p consumingUses, return
  /// true if all uses in \p usesToTest are strictly not contained within the
  /// region where the Linear Lifetime defined by \p value and \p consumingUses
  /// is live. Otherwise, returns false.
  bool usesNotContainedWithinLifetime(SILValue value,
                                      ArrayRef<Operand *> consumingUses,
                                      ArrayRef<Operand *> usesToTest);

private:
  /// Returns true if:
  ///
  /// 1. No consuming uses are reachable from any other consuming use, from any
  /// non-consuming uses, or from the producer instruction.
  /// 2. The consuming use set jointly post dominates producers and all non
  /// consuming uses.
  ///
  /// Returns false otherwise.
  ///
  /// \p value The value whose lifetime we are checking.
  /// \p consumingUses the array of users that destroy or consume a value.
  /// \p nonConsumingUses regular uses
  /// \p errorBehavior If we detect an error, should we return false or hard
  /// error.
  /// \p leakingBlocks If non-null a list of blocks where the value was detected
  /// to leak. Can be used to insert missing destroys.
  Error checkValue(SILValue value, ArrayRef<Operand *> consumingUses,
                   ArrayRef<Operand *> nonConsumingUses,
                   ErrorBuilder &errorBuilder);

  Error checkValue(SILValue value, ArrayRef<Operand *> consumingUses,
                   ArrayRef<Operand *> nonConsumingUses,
                   ErrorBuilder &errorBuilder,
                   function_ref<void(SILBasicBlock *)> leakingBlockCallback);

  Error checkValueImpl(
      SILValue value, ArrayRef<Operand *> consumingUses,
      ArrayRef<Operand *> nonConsumingUses, ErrorBuilder &errorBuilder,
      std::optional<function_ref<void(SILBasicBlock *)>> leakingBlockCallback,
      std::optional<function_ref<void(Operand *)>>
          nonConsumingUsesOutsideLifetimeCallback);
};

} // namespace language

#endif
