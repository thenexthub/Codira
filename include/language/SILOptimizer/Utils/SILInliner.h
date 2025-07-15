//===--- SILInliner.h - Inlines SIL functions -------------------*- C++ -*-===//
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
// This file defines the SILInliner class, used for inlining SIL functions into
// function application sites
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_SIL_SILINLINER_H
#define LANGUAGE_SIL_SILINLINER_H

#include "language/AST/SubstitutionMap.h"
#include "language/SIL/ApplySite.h"
#include "language/SIL/SILInstruction.h"
#include <functional>

namespace language {

class SILOptFunctionBuilder;
class InstructionDeleter;

// For now Free is 0 and Expensive is 1. This can be changed in the future by
// adding more categories.
enum class InlineCost : unsigned {
  Free = 0,
  Expensive = 1
};

/// Return the 'cost' of one instruction. Instructions that are expected to
/// disappear at the LLVM IR level are assigned a cost of 'Free'.
InlineCost instructionInlineCost(SILInstruction &I);

class SILInliner {
public:
  enum class InlineKind { MandatoryInline, PerformanceInline };

  using DeletionFuncTy = std::function<void(SILInstruction *)>;

private:
  SILOptFunctionBuilder &FuncBuilder;
  InstructionDeleter &deleter;

  InlineKind IKind;
  SubstitutionMap ApplySubs;

public:
  SILInliner(SILOptFunctionBuilder &FuncBuilder, InstructionDeleter &deleter,
             InlineKind IKind, SubstitutionMap ApplySubs)
      : FuncBuilder(FuncBuilder), deleter(deleter), IKind(IKind),
        ApplySubs(ApplySubs) {}

  static bool canInlineBeginApply(BeginApplyInst *BA);

  /// Returns true if we are able to inline \arg AI.
  ///
  /// *NOTE* This must be checked before attempting to inline \arg AI. If one
  /// attempts to inline \arg AI and this returns false, an assert will fire.
  static bool canInlineApplySite(FullApplySite apply);

  /// Returns true if inlining \arg apply can result in improperly nested stack
  /// allocations.
  ///
  /// In this case stack nesting must be corrected after inlining with the
  /// StackNesting utility.
  static bool invalidatesStackNesting(FullApplySite apply) {
    // Inlining of coroutines can result in improperly nested stack
    // allocations.
    return isa<BeginApplyInst>(apply);
  }

  /// Inline a callee function at the given apply site with the given
  /// arguments. Delete the apply and any dead arguments. Return a valid
  /// iterator to the first inlined instruction (or first instruction after the
  /// call for an empty function).
  ///
  /// This only performs one step of inlining: it does not recursively
  /// inline functions called by the callee.
  ///
  /// This may split basic blocks and delete instructions anywhere.
  ///
  /// All inlined instructions must be either inside the original call block or
  /// inside new basic blocks laid out after the original call block.
  ///
  /// Any instructions in the original call block after the inlined call must be
  /// in a new basic block laid out after all inlined blocks.
  ///
  /// The above guarantees ensure that inlining is liner in the number of
  /// instructions and that inlined instructions are revisited exactly once.
  ///
  /// *NOTE*: This attempts to perform inlining unconditionally and thus asserts
  /// if inlining will fail. All users /must/ check that a function is allowed
  /// to be inlined using SILInliner::canInlineApplySite before calling this
  /// function.
  ///
  /// *NOTE*: Inlining can result in improperly nested stack allocations, which
  /// must be corrected after inlining. See invalidatesStackNesting().
  ///
  /// Returns the last block in function order containing inlined instructions
  /// (the original caller block for single-block functions).
  SILBasicBlock *
  inlineFunction(SILFunction *calleeFunction, FullApplySite apply,
                 ArrayRef<SILValue> appliedArgs);

  /// Inline the function called by the given full apply site. This creates
  /// an instance of SILInliner by constructing a substitution map from the
  /// given apply site, and invokes `inlineFunction` method on the SILInliner
  /// instance to inline the callee.
  /// This requires the full apply site to be a direct call i.e., the apply
  /// instruction must have a function ref.
  ///
  /// *NOTE*:This attempts to perform inlining unconditionally and thus asserts
  /// if inlining will fail. All users /must/ check that a function is allowed
  /// to be inlined using SILInliner::canInlineApplySite before calling this
  /// function.
  ///
  /// *NOTE*: Inlining can result in improperly nested stack allocations, which
  /// must be corrected after inlining. See invalidatesStackNesting().
  ///
  /// Returns the last block in function order containing inlined instructions
  /// (the original caller block for single-block functions).
  static SILBasicBlock *
  inlineFullApply(FullApplySite apply, SILInliner::InlineKind inlineKind,
                  SILOptFunctionBuilder &funcBuilder,
                  InstructionDeleter &deleter);
};

} // end namespace language

#endif
