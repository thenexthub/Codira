//===--- GuaranteedPhiVerifierPrivate.h -----------------------------------===//
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

#ifndef LANGUAGE_SIL_GUARANTEEDPHIVERIFIER_H
#define LANGUAGE_SIL_GUARANTEEDPHIVERIFIER_H

#include "LinearLifetimeCheckerPrivate.h"

#include "language/SIL/OwnershipUtils.h"
#include "language/SIL/SILInstruction.h"
#include "language/SIL/SILValue.h"

namespace language {

class DeadEndBlocks;

/// GuaranteedPhiVerifier validates the lifetime of the Reborrow or
/// GuaranteedForwardingPhi lies within the lifetime of its base value. It uses
/// LinearLifetimeChecker for this.
///
/// A GuaranteedForwardingPhi can have different base values on different
/// control flow paths. Example:
///
/// sil [ossa] @test_forwarded_separate_base_values :
/// bb0(%0 : @owned $Wrapper1):
///   %1 = begin_borrow %0 : $Wrapper1
///   cond_br undef, bb1, bb2
///
/// bb1:
///   %3 = begin_borrow %0 : $Wrapper1
///   %4 = struct_extract %1 : $Wrapper1, #Wrapper1.val1
///   br bb3(%4 : $Wrapper2, %3 : $Wrapper1)
///
/// bb2:
///   %6 = begin_borrow %0 : $Wrapper1
///   %7 = struct_extract %6 : $Wrapper1, #Wrapper1.val1
///   br bb3(%7 : $Wrapper2, %6 : $Wrapper1)
///
/// bb3(%9 : @guaranteed $Wrapper2, %10 : @guaranteed $Wrapper1):
///   end_borrow %10 : $Wrapper1
///   end_borrow %1 : $Wrapper1
///   destroy_value %0 : $Wrapper1
///   %16 = tuple ()
///   return %16 : $()
/// }
///
/// Here %10 is the base value of %9 on one path and %1 is the base value of %9
/// on another path.
/// Similarly, a Reborrow can have different base values on different
/// control flow paths. Example:
/// sil [ossa] @test_reborrow_different_base_values :
/// bb0(%0 : @owned $Wrapper):
///   cond_br undef, bb1, bb2
///
/// bb1:
///   %2 = copy_value %0 : $Wrapper
///   %3 = begin_borrow %0 : $Wrapper
///   br bb3(%3 : $Wrapper, %2 : $Wrapper)
///
/// bb2:
///   %5 = copy_value %0 : $Wrapper
///   %6 = begin_borrow %5 : $Wrapper
///   br bb3(%6 : $Wrapper, %5 : $Wrapper)
///
/// bb3(%8 : @guaranteed $Wrapper, %9 : @owned $Wrapper):
///   end_borrow %8 : $Wrapper
///   destroy_value %9 : $Wrapper
///   destroy_value %0 : $Wrapper
///   %13 = tuple ()
///   return %13 : $()
/// }
///
/// Here %9 is the base value of %8 on path and %0 is the base value on another
/// path.
///
class GuaranteedPhiVerifier {
  /// A cache of dead-end basic blocks that we use to determine if we can
  /// ignore "leaks".
  DeadEndBlocks *deadEndBlocks = nullptr;
  /// The builder that the checker uses to emit error messages, crash if asked
  /// for, or supply back interesting info to the caller.
  LinearLifetimeChecker::ErrorBuilder errorBuilder;
  /// A map of reborrow phi/guaranteed forwarding phi to its base values.
  /// Note that a reborrow phi arg can have different base values based on
  /// different control flow paths.
  toolchain::DenseMap<SILPhiArgument *, SmallPtrSet<SILValue, 8>>
      dependentPhiToBaseValueMap;

public:
  GuaranteedPhiVerifier(const SILFunction *fn, DeadEndBlocks *deadEndBlocks,
                        LinearLifetimeChecker::ErrorBuilder errorBuilder)
      : deadEndBlocks(deadEndBlocks), errorBuilder(errorBuilder) {}

  /// Verify whether all reborrows of \p borrow are within the lifetime of the
  /// borrowed value.
  void verifyReborrows(BeginBorrowInst *borrow);
  /// Verify whether the GuaranteedForwardingPhi uses of \p borrow are within
  /// its lifetime.
  void verifyGuaranteedForwardingPhis(BorrowedValue borrow);

private:
  /// Verifies whether the \p phi's lifetime lies within the \p baseValue
  bool verifyDependentPhiLifetime(SILPhiArgument *phi, SILValue baseValue);
};

} // namespace language

#endif
