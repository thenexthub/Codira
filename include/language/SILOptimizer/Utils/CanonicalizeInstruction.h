//===-- CanonicalizeInstruction.h - canonical SIL peepholes -----*- C++ -*-===//
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
/// SSA-peephole transformations that yield a more canonical SIL representation.
///
/// Unlike simplifyInstruction, these transformations may effect any
/// instruction, not only single-values, and may arbitrarily generate new SIL
/// instructions.
///
/// Unlike SILCombine, these peepholes must work on 'raw' SIL form and should be
/// limited to those necessary to aid in diagnostics and other mandatory
/// pipeline passes. Optimization may only be done to the extent that it
/// neither interferes with diagnostics nor increases compile time.
///
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_SILOPTIMIZER_UTILS_CANONICALIZEINSTRUCTION_H
#define LANGUAGE_SILOPTIMIZER_UTILS_CANONICALIZEINSTRUCTION_H

#include "language/SIL/BasicBlockUtils.h"
#include "language/SIL/SILBasicBlock.h"
#include "language/SIL/SILInstruction.h"
#include "language/SILOptimizer/Utils/InstOptUtils.h"
#include "toolchain/Support/Debug.h"

namespace language {

/// Abstract base class. Implements all canonicalization transforms. Extended by
/// passes to be notified of each SIL modification.
struct CanonicalizeInstruction {
  // May be overridden by passes.
  static constexpr const char *defaultDebugType = "sil-canonicalize";
  const char *debugType = defaultDebugType;
  DeadEndBlocks &deadEndBlocks;
  InstModCallbacks callbacks;
  bool preserveDebugInfo;

  CanonicalizeInstruction(const char *passDebugType,
                          DeadEndBlocks &deadEndBlocks)
      : deadEndBlocks(deadEndBlocks),
        callbacks() {

    preserveDebugInfo = getFunction()->getEffectiveOptimizationMode()
      <= OptimizationMode::NoOptimization;

#ifndef NDEBUG
    if (toolchain::DebugFlag && !toolchain::isCurrentDebugType(debugType))
      debugType = passDebugType;
#endif
    callbacks = InstModCallbacks()
      .onDelete([&](SILInstruction *toDelete) {
        killInstruction(toDelete);
      })
      .onCreateNewInst([&](SILInstruction *newInst) {
        notifyNewInstruction(newInst);
      })
      .onSetUseValue([&](Operand *use, SILValue newValue) {
        use->set(newValue);
        notifyHasNewUsers(newValue);
      });
  }

  virtual ~CanonicalizeInstruction();

  const SILFunction *getFunction() const { return deadEndBlocks.getFunction(); }

  // TODO: callbacks should come from the current InstructionDeleter.
  InstModCallbacks &getCallbacks() { return callbacks; }

  /// Rewrite this instruction, based on its operands and uses, into a more
  /// canonical representation.
  ///
  /// Return an iterator to the next instruction or to the end of the block.
  /// The returned iterator will follow any newly added or to-be-deleted
  /// instructions, regardless of whether the pass immediately deletes the
  /// instructions or simply records them for later deletion.
  ///
  /// To (re)visit new instructions, override notifyNewInstruction().
  ///
  /// To determine if any transformation at all occurred, override
  /// notifyNewInstruction(), killInstruction(), and notifyNewUsers().
  ///
  /// Warning: If the \p inst argument is killed and the client immediately
  /// erases \p inst, then it may be an invalid pointer upon return.
  SILBasicBlock::iterator canonicalize(SILInstruction *inst);

  /// Record a newly generated instruction.
  virtual void notifyNewInstruction(SILInstruction *inst) = 0;

  /// Kill an instruction that no longer has uses, or whose side effect is now
  /// represented by a different instruction. The client can defer erasing the
  /// instruction but must eventually erase all killed instructions to restore
  /// valid SIL.
  ///
  /// This callback should not mutate any other instructions. It may only delete
  /// the given argument. It will be called separately for each end-of-scope and
  /// debug use before being called on the instruction they use.
  virtual void killInstruction(SILInstruction *inst) = 0;

  /// Record a SIL value that has acquired new users.
  virtual void notifyHasNewUsers(SILValue value) = 0;
};

} // end namespace language

#endif // LANGUAGE_SILOPTIMIZER_UTILS_CANONICALIZEINSTRUCTION_H
