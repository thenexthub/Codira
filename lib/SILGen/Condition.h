//===--- Condition.h - Defines the SILGen Condition class -------*- C++ -*-===//
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
// This file defines the Condition class, used by SIL Generation.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_SIL_LOWERING_CONDITION_H
#define LANGUAGE_SIL_LOWERING_CONDITION_H

#include "toolchain/ADT/ArrayRef.h"
#include "language/Basic/Assertions.h"
#include "language/SIL/SILLocation.h"
#include "language/SIL/SILValue.h"
#include "SILGenFunction.h"
#include "Scope.h"
#include "toolchain/Support/Compiler.h"

namespace language {
  class PatternBindingDecl;
  class SILBasicBlock;
  
namespace Lowering {

/// A condition is the result of evaluating a boolean expression as
/// control flow.
///
/// For each Condition instance, `enterTrue` must be called before `complete`.
/// If `enterFalse` is skipped, then an empty fall-through block is created.
class TOOLCHAIN_LIBRARY_VISIBILITY Condition {
  /// The blocks responsible for executing the true and false conditions. These
  /// are initialized non-null and set to null after being emitted.
  SILBasicBlock *TrueBB;
  SILBasicBlock *FalseBB;
  
  /// The continuation block if both branches are possible.
  SILBasicBlock *ContBB;

  /// The location wrapping the originator conditional expression.
  SILLocation Loc;
  
public:
  Condition(SILBasicBlock *TrueBB, SILBasicBlock *FalseBB,
            SILBasicBlock *ContBB,
            SILLocation L)
    : TrueBB(TrueBB), FalseBB(FalseBB), ContBB(ContBB), Loc(L)
  {
    assert((TrueBB != nullptr && FalseBB != nullptr) &&
           "Requires non-null block pointers.");
  }

  /// enterTrue - Begin the emission of the true block.
  void enterTrue(SILGenFunction &SGF) { enter(SGF, TrueBB); }

  /// exitTrue - End the emission of the true block.
  void exitTrue(SILGenFunction &SGF, ArrayRef<SILValue> Args = {}) {
    exit(SGF, TrueBB, Args);
    TrueBB = nullptr;
  }

  /// enterFalse - Begin the emission of the false block.
  void enterFalse(SILGenFunction &SGF) { enter(SGF, FalseBB); }

  /// exitFalse - End the emission of the true block.
  void exitFalse(SILGenFunction &SGF, ArrayRef<SILValue> Args = {}) {
    exit(SGF, FalseBB, Args);
    FalseBB = nullptr;
  }

  /// complete - Complete this conditional execution.  This should be called
  /// only after all other calls on this Condition have been made.
  /// This leaves SGF's SILGenBuilder at the continuation block.
  SILBasicBlock *complete(SILGenFunction &SGF);

protected:
  void enter(SILGenFunction &SGF, SILBasicBlock *destBB);

  void exit(SILGenFunction &SGF, SILBasicBlock *destBB,
            ArrayRef<SILValue> Args = {});
};

/// A conditional value is one that depends on conditional execution.
/// Depending on whether a type is address-only, it may be representable using
/// BB argument passing or by storing to a common result buffer.
class ConditionalValue {
  SILGenFunction &SGF;
  const TypeLowering &tl;
  
  /// The continuation block that receives the conditional value.
  SILBasicBlock *contBB;
  
  /// The location associated with the value.
  SILLocation loc;

  /// The buffer to receive an address-only result, or the BB argument that
  /// a loadable result is passed to.
  SILValue result;
  
  /// The Scope for the current branch.
  std::optional<Scope> scope;

  /// A place to hold conditional Initializations of our result.
  std::unique_ptr<Initialization> currentInitialization;
  
public:
  /// Begins a conditional computation of the type represented by the given
  /// type lowering. This potentially emits a temporary allocation for the
  /// result, so it must be called with the insertion point valid and dominating
  /// any branches that will be involved in the computation.
  ConditionalValue(SILGenFunction &SGF, SGFContext C, SILLocation loc,
                   const TypeLowering &valueTL);
  
  /// Enter a branch of the conditional value computation. Expression evaluation
  /// within this branch may use the returned SGFContext to potentially find a
  /// buffer to emit into. If a basic block is given, the insertion point must
  /// be invalid, and on return, the given basic block will be emitted, and the
  /// insertion point will be inside it. If the basic block is null, then
  /// codegen proceeds in the current basic block.
  SGFContext enterBranch(SILBasicBlock *bb = nullptr);
  
  /// Exit a branch of the conditional value computation, using the given value
  /// as the result of the computation on this branch. Branches to the
  /// continuation block for the conditional value. On return, the insertion
  /// point will be invalid.
  void exitBranch(RValue &&result);
  
  /// Complete the conditional computation. The insertion point must be invalid.
  /// On return, the continuation block for the conditional will be emitted, and
  /// the insertion point will be inside it. The result of the conditional
  /// computation will be returned.
  ManagedValue complete();
};
  
} // end namespace Lowering
} // end namespace language
  
#endif
