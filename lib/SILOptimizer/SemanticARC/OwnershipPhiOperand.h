//===--- OwnershipPhiOperand.h --------------------------------------------===//
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

#ifndef SWIFT_SILOPTIMIZER_SEMANTICARC_OWNERSHIPPHIOPERAND_H
#define SWIFT_SILOPTIMIZER_SEMANTICARC_OWNERSHIPPHIOPERAND_H

#include "language/Basic/STLExtras.h"
#include "language/SIL/SILArgument.h"
#include "language/SIL/SILBasicBlock.h"
#include "language/SIL/SILInstruction.h"
#include "language/SIL/SILUndef.h"
#include "language/SIL/SILValue.h"

namespace language {
namespace semanticarc {

/// The operand of a "phi" in the induced ownership graph of a def-use graph.
///
/// Some examples: br, struct, tuple.
class LLVM_LIBRARY_VISIBILITY OwnershipPhiOperand {
public:
  enum Kind {
    Branch,
    Struct,
    Tuple,
  };

private:
  Operand *op;

  OwnershipPhiOperand(Operand *op) : op(op) {}

public:
  static std::optional<OwnershipPhiOperand> get(const Operand *op) {
    switch (op->getUser()->getKind()) {
    case SILInstructionKind::BranchInst:
    case SILInstructionKind::StructInst:
    case SILInstructionKind::TupleInst:
      return {{const_cast<Operand *>(op)}};
    default:
      return std::nullopt;
    }
  }

  Kind getKind() const {
    switch (op->getUser()->getKind()) {
    case SILInstructionKind::BranchInst:
      return Kind::Branch;
    case SILInstructionKind::StructInst:
      return Kind::Struct;
    case SILInstructionKind::TupleInst:
      return Kind::Tuple;
    default:
      llvm_unreachable("unhandled case?!");
    }
  }

  operator const Operand *() const { return op; }
  operator Operand *() { return op; }

  Operand *getOperand() const { return op; }
  SILValue getValue() const { return op->get(); }
  SILType getType() const { return op->get()->getType(); }

  unsigned getOperandNumber() const { return op->getOperandNumber(); }

  void markUndef() & { op->set(SILUndef::get(getValue())); }

  SILInstruction *getInst() const { return op->getUser(); }

  /// Return true if this phi consumes a borrow.
  ///
  /// If so, we may need to insert an extra begin_borrow to balance the +1 when
  /// converting owned ownership phis to guaranteed ownership phis.
  bool isGuaranteedConsuming() const {
    switch (getKind()) {
    case Kind::Branch:
      return true;
    case Kind::Tuple:
    case Kind::Struct:
      return false;
    }
    llvm_unreachable("unhandled operand kind!");
  }

  bool operator<(const OwnershipPhiOperand &other) const {
    return op < other.op;
  }

  bool operator==(const OwnershipPhiOperand &other) const {
    return op == other.op;
  }

  bool visitResults(function_ref<bool(SILValue)> visitor) const {
    switch (getKind()) {
    case Kind::Struct:
      return visitor(cast<StructInst>(getInst()));
    case Kind::Tuple:
      return visitor(cast<TupleInst>(getInst()));
    case Kind::Branch: {
      auto *br = cast<BranchInst>(getInst());
      unsigned opNum = getOperandNumber();
      return llvm::all_of(
          br->getSuccessorBlocks(), [&](SILBasicBlock *succBlock) {
            return visitor(succBlock->getSILPhiArguments()[opNum]);
          });
    }
    }
    llvm_unreachable("unhandled operand kind!");
  }
};

} // namespace semanticarc
} // namespace language

#endif // SWIFT_SILOPTIMIZER_SEMANTICARC_OWNERSHIPPHIOPERAND_H
