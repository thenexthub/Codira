//===--- InstWrappers.cpp ------------------------------------------------===//
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

#include "language/Basic/Assertions.h"
#include "language/SIL/InstWrappers.h"
#include "language/SIL/SILFunction.h"

using namespace language;

ForwardingOperation::ForwardingOperation(SILInstruction *inst) {
  if (ForwardingInstruction::isa(inst)) {
    this->forwardingInst = inst;
    return;
  }
}

ValueOwnershipKind ForwardingOperation::getForwardingOwnershipKind() {
  return ForwardingInstruction::get(forwardingInst)
      ->getForwardingOwnershipKind();
}

bool ForwardingOperation::preservesOwnership() {
  return ForwardingInstruction::get(forwardingInst)->preservesOwnership();
}

// See ForwardingInstruction.code preservesRepresentation().
bool ForwardingOperation::hasSameRepresentation() const {
  switch (forwardingInst->getKind()) {
  // Explicitly list instructions which definitely involve a representation
  // change.
  case SILInstructionKind::SwitchEnumInst:
  default:
    // Conservatively assume that a conversion changes representation.
    // Operations can be added as needed to participate in SIL opaque values.
    return false;

  case SILInstructionKind::ConvertFunctionInst:
  case SILInstructionKind::CopyableToMoveOnlyWrapperValueInst:
  case SILInstructionKind::DestructureTupleInst:
  case SILInstructionKind::DestructureStructInst:
  case SILInstructionKind::InitExistentialRefInst:
  case SILInstructionKind::ObjectInst:
  case SILInstructionKind::OpenExistentialBoxValueInst:
  case SILInstructionKind::OpenExistentialRefInst:
  case SILInstructionKind::OpenExistentialValueInst:
  case SILInstructionKind::MarkUnresolvedNonCopyableValueInst:
  case SILInstructionKind::MoveOnlyWrapperToCopyableValueInst:
  case SILInstructionKind::MarkUninitializedInst:
  case SILInstructionKind::StructExtractInst:
  case SILInstructionKind::TupleExtractInst:
  case SILInstructionKind::TuplePackExtractInst:
    return true;
  }
}

bool ForwardingOperation::isAddressOnly() const {
  if (auto *singleForwardingOp = getSingleForwardingOperand()) {
    return singleForwardingOp->get()->getType().isAddressOnly(
        *forwardingInst->getFunction());
  }
  // All ForwardingInstructions that forward all operands or no operands are
  // currently a single value instruction.
  auto *aggregate =
      cast<OwnershipForwardingSingleValueInstruction>(forwardingInst);
  // If any of the operands are address-only, then the aggregate must be.
  return aggregate->getType().isAddressOnly(*forwardingInst->getFunction());
}

bool ForwardingOperation::visitForwardedValues(
    function_ref<bool(SILValue)> visitor) {
  if (auto *svi = dyn_cast<SingleValueInstruction>(forwardingInst)) {
    return visitor(svi);
  }
  if (auto *mvri = dyn_cast<MultipleValueInstruction>(forwardingInst)) {
    return toolchain::all_of(mvri->getResults(), [&](SILValue value) {
      if (value->getOwnershipKind() == OwnershipKind::None)
        return true;
      return visitor(value);
    });
  }
  auto *ti = cast<TermInst>(forwardingInst);
  assert(ti->mayHaveTerminatorResult());
  return toolchain::all_of(ti->getSuccessorBlocks(), [&](SILBasicBlock *succBlock) {
    // If we do not have any arguments, then continue.
    if (succBlock->args_empty())
      return true;

    auto args = succBlock->getSILPhiArguments();
    assert(args.size() == 1 && "Transforming terminator with multiple args?!");
    return visitor(args[0]);
  });
}

bool language::isFixedStorageSemanticsCallKind(SILFunction *function) {
  for (auto &attr : function->getSemanticsAttrs()) {
    if (attr == "fixed_storage.check_index" ||
        attr == "fixed_storage.get_count") {
      return true;
    }
  }
  return false;
}