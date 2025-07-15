//===--- OwnershipLiveness.cpp --------------------------------------------===//
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

#include "language/SIL/OwnershipLiveness.h"
#include "language/Basic/Assertions.h"
#include "language/Basic/Debug.h"
#include "language/Basic/Defer.h"
#include "language/Basic/Toolchain.h"
#include "language/SIL/Dominance.h"
#include "language/SIL/PrunedLiveness.h"
#include "language/SIL/SILArgument.h"
#include "language/SIL/SILBasicBlock.h"
#include "language/SIL/SILInstruction.h"
#include "language/SIL/SILValue.h"
#include "language/SIL/Test.h"
#include "toolchain/ADT/SmallVector.h"

namespace language {

void OSSALiveness::print(toolchain::raw_ostream &OS) const { liveness.print(OS); }

void OSSALiveness::dump() const { print(toolchain::dbgs()); }

struct LinearLivenessVisitor :
  public OwnershipUseVisitor<LinearLivenessVisitor> {

  LinearLiveness &linearLiveness;

  LinearLivenessVisitor(LinearLiveness &linearLiveness):
    linearLiveness(linearLiveness){}

  bool handleUsePoint(Operand *use, UseLifetimeConstraint useConstraint) {
    if (!linearLiveness.includeExtensions &&
        isa<ExtendLifetimeInst>(use->getUser())) {
      return true;
    }
    linearLiveness.liveness.updateForUse(
      use->getUser(), useConstraint == UseLifetimeConstraint::LifetimeEnding);
    return true;
  }

  bool handlePointerEscape(Operand *use) {
    toolchain_unreachable("a pointer escape cannot end a linear lifetime");
  }

  // handleOwnedPhi and handleOuterReborrow ends the linear lifetime.
  // By default, they are treated like a normal lifetime-ending use.

  bool handleGuaranteedForwardingPhi(Operand *use) {
    toolchain_unreachable("guaranteed forwarding phi cannot end a linear lifetime");
  }

  bool handleInnerBorrow(BorrowingOperand borrowingOperand) {
    toolchain_unreachable("an inner borrow cannot end a linear lifetime");
  }

  bool handleInnerAdjacentReborrow(SILArgument *reborrow) {
    toolchain_unreachable("inner adjacent reborrows are not visited");
  }

  bool handleInnerReborrow(BorrowingOperand borrowingOperand) {
    toolchain_unreachable("an inner borrow cannot end a linear lifetime");
  }

  bool handleScopedAddress(ScopedAddressValue scopedAddress) {
    toolchain_unreachable("an scoped address cannot end a linear lifetime");
  }
};

LinearLiveness::LinearLiveness(SILValue def,
                               IncludeExtensions_t includeExtensions)
    : OSSALiveness(def), includeExtensions(includeExtensions) {
  switch (def->getOwnershipKind()) {
  case OwnershipKind::Owned:
    break;
  case OwnershipKind::Guaranteed: {
    BorrowedValue borrowedValue(def);
    assert(borrowedValue && borrowedValue.isLocalScope());
    (void)borrowedValue;
    break;
  }
  case OwnershipKind::None:
    assert(def->isFromVarDecl());
    break;
  case OwnershipKind::Unowned:
  case OwnershipKind::Any:
    toolchain_unreachable("bad ownership for LinearLiveness");
  }
}

void LinearLiveness::compute() {
  liveness.initializeDef(ownershipDef);
  LinearLivenessVisitor(*this).visitLifetimeEndingUses(ownershipDef);
}

/// Override OwnershipUseVisitor to callback to handleInnerScopeCallback for
/// nested borrow scopes. This supports lifetime completion from the inside-out.
///
/// By default, handleOwnedPhi and handleOuterReborrow are already treated like
/// a normal lifetime-ending use.
///
/// By default, handleGuaranteedForwardingPhi is already treated like a normal
/// non-lifetime-ending use.
struct InteriorLivenessVisitor :
  public OwnershipUseVisitor<InteriorLivenessVisitor> {

  InteriorLiveness &interiorLiveness;

  // If domInfo is nullptr, then InteriorLiveness never assumes dominance. As a
  // result it may report extra unenclosedPhis. In that case, any attempt to
  // create a new phi would result in an immediately redundant phi.
  const DominanceInfo *domInfo = nullptr;

  /// handleInnerScopeCallback may add uses to the inner scope, but it may not
  /// modify the use-list containing \p borrowingOperand. This callback can be
  /// used to ensure that the inner scope is complete before visiting its scope
  /// ending operands.
  ///
  /// An inner scope encapsulates any pointer escapes so visiting its interior
  /// uses is not necessary when visiting the outer scope's interior uses.
  InteriorLiveness::InnerScopeHandlerRef handleInnerScopeCallback;

  // State local to an invocation of
  // OwnershipUseVisitor::visitOwnershipUses().
  NodeSet visited;

  InteriorLivenessVisitor(
    InteriorLiveness &interiorLiveness,
    const DominanceInfo *domInfo,
    InteriorLiveness::InnerScopeHandlerRef handleInnerScope)
    : interiorLiveness(interiorLiveness),
      domInfo(domInfo),
      handleInnerScopeCallback(handleInnerScope),
      visited(interiorLiveness.ownershipDef->getFunction()) {}

  bool handleUsePoint(Operand *use, UseLifetimeConstraint useConstraint) {
    interiorLiveness.liveness.updateForUse(
        use->getUser(), useConstraint == UseLifetimeConstraint::LifetimeEnding);
    return true;
  }

  bool handlePointerEscape(Operand *use) {
    interiorLiveness.addressUseKind = AddressUseKind::PointerEscape;
    interiorLiveness.escapingUse = use;
    if (!handleUsePoint(use, UseLifetimeConstraint::NonLifetimeEnding))
      return false;

    return true;
  }

  /// After this returns true, handleUsePoint will be called on the scope
  /// ending operands.
  ///
  /// Handles begin_borrow, load_borrow, store_borrow, begin_apply.
  bool handleInnerBorrow(BorrowingOperand borrowingOperand) {
    if (handleInnerScopeCallback) {
      if (auto value = borrowingOperand.getScopeIntroducingUserResult()) {
        handleInnerScopeCallback(value);
      }
    }
    return true;
  }

  /// After this returns true, handleUsePoint will be called on the scope
  /// ending operands.
  ///
  /// Handles store_borrow, begin_access.
  bool handleScopedAddress(ScopedAddressValue scopedAddress) {
    if (handleInnerScopeCallback) {
      handleInnerScopeCallback(scopedAddress.value);
    }
    return true;
  }
};

void InteriorLiveness::compute(const DominanceInfo *domInfo, InnerScopeHandlerRef handleInnerScope) {
  liveness.initializeDef(ownershipDef);
  addressUseKind = AddressUseKind::NonEscaping;
  InteriorLivenessVisitor(*this, domInfo, handleInnerScope)
    .visitInteriorUses(ownershipDef);
}

void InteriorLiveness::print(toolchain::raw_ostream &OS) const {
  OSSALiveness::print(OS);

  switch (getAddressUseKind()) {
  case AddressUseKind::NonEscaping:
    OS << "Complete liveness\n";
    break;
  case AddressUseKind::PointerEscape:
    OS << "Incomplete liveness: Escaping address\n";
    break;
  case AddressUseKind::Dependent:
    OS << "Incomplete liveness: Dependent value\n";
    break;
  case AddressUseKind::Unknown:
    OS << "Incomplete liveness: Unknown address use\n";
    break;
  }
}

void InteriorLiveness::dump() const { print(toolchain::dbgs()); }

// =============================================================================
// ExtendedLinearLiveness
// =============================================================================
  
struct ExtendedLinearLivenessVisitor
    : public OwnershipUseVisitor<ExtendedLinearLivenessVisitor> {

  ExtendedLinearLiveness &extendedLiveness;

  // State local to an invocation of
  // OwnershipUseVisitor::visitOwnershipUses().
  InstructionSet visited;

  ExtendedLinearLivenessVisitor(ExtendedLinearLiveness &extendedLiveness)
    : extendedLiveness(extendedLiveness),
      visited(extendedLiveness.ownershipDef->getFunction()) {}

  bool handleUsePoint(Operand *use, UseLifetimeConstraint useConstraint) {
    extendedLiveness.liveness.updateForUse(
        use->getUser(), useConstraint == UseLifetimeConstraint::LifetimeEnding);
    return true;
  }

  bool handlePointerEscape(Operand *use) {
    toolchain_unreachable("a pointer escape cannot end a linear lifetime");
  }

  bool handleOwnedPhi(Operand *phiOper) {
    extendedLiveness.liveness.initializeDef(PhiOperand(phiOper).getValue());
    return true;
  }

  bool handleOuterReborrow(Operand *phiOper) {
    extendedLiveness.liveness.initializeDef(PhiOperand(phiOper).getValue());
    return true;
  }

  bool handleGuaranteedForwardingPhi(Operand *use) {
    toolchain_unreachable("guaranteed forwarding phi cannot end a linear lifetime");
  }

  bool handleInnerBorrow(BorrowingOperand borrowingOperand) {
    toolchain_unreachable("an inner borrow cannot end a linear lifetime");
  }

  bool handleInnerAdjacentReborrow(SILArgument *reborrow) {
    toolchain_unreachable("inner adjacent reborrows are not visited");
  }

  bool handleInnerReborrow(BorrowingOperand borrowingOperand) {
    toolchain_unreachable("an inner borrow cannot end a linear lifetime");
  }

  bool handleScopedAddress(ScopedAddressValue scopedAddress) {
    toolchain_unreachable("an scoped address cannot end a linear lifetime");
  }
};

ExtendedLinearLiveness::ExtendedLinearLiveness(SILValue def)
  : ownershipDef(def), liveness(def->getFunction(), &discoveredBlocks) {
  if (def->getOwnershipKind() != OwnershipKind::Owned) {
    BorrowedValue borrowedValue(def);
    assert(borrowedValue && borrowedValue.isLocalScope());
    (void)borrowedValue;
  }
}

void ExtendedLinearLiveness::compute() {
  liveness.initializeDef(ownershipDef);
  for (auto defIter = liveness.defBegin(); defIter != liveness.defEnd();
       ++defIter) {
    auto *def = cast<ValueBase>(*defIter);
    ExtendedLinearLivenessVisitor(*this).visitLifetimeEndingUses(def);
  }
}

void ExtendedLinearLiveness::print(toolchain::raw_ostream &OS) const {
  liveness.print(OS);
}

void ExtendedLinearLiveness::dump() const { print(toolchain::dbgs()); }

} // namespace language

namespace language::test {
// Arguments:
// - SILValue: value
// Dumps:
// - function
// - the computed pruned liveness
// - the liveness boundary
static FunctionTest LinearLivenessTest("linear_liveness", [](auto &function,
                                                             auto &arguments,
                                                             auto &test) {
  SILValue value = arguments.takeValue();
  function.print(toolchain::outs());
  toolchain::outs() << "Linear liveness: " << value;
  LinearLiveness liveness(value);
  liveness.compute();
  liveness.print(toolchain::outs());

  PrunedLivenessBoundary boundary;
  liveness.getLiveness().computeBoundary(boundary);
  boundary.print(toolchain::outs());
});

// Arguments:
// - SILValue: value
// Dumps:
// - function
// - the computed pruned liveness
// - the liveness boundary
static FunctionTest
    InteriorLivenessTest("interior_liveness",
                         [](auto &function, auto &arguments, auto &test) {
                           SILValue value = arguments.takeValue();
                           function.print(toolchain::outs());
                           toolchain::outs() << "Interior liveness: " << value;
                           auto *domTree = test.getDominanceInfo();
                           InteriorLiveness liveness(value);
                           auto handleInnerScope = [](SILValue innerBorrow) {
                             toolchain::outs() << "Inner scope: " << innerBorrow;
                           };
                           liveness.compute(domTree, handleInnerScope);
                           liveness.print(toolchain::outs());

                           PrunedLivenessBoundary boundary;
                           liveness.getLiveness().computeBoundary(boundary);
                           boundary.print(toolchain::outs());
                         });

// Arguments:
// - SILValue: value
// Dumps:
// - function
// - the computed pruned liveness
// - the liveness boundary
static FunctionTest ExtendedLinearLivenessTest(
    "extended-liveness", [](auto &function, auto &arguments, auto &test) {
      SILValue value = arguments.takeValue();
      function.print(toolchain::outs());
      toolchain::outs() << "Extended liveness: " << value;
      ExtendedLinearLiveness liveness(value);
      liveness.compute();
      liveness.print(toolchain::outs());

      PrunedLivenessBoundary boundary;
      liveness.getLiveness().computeBoundary(boundary);
      boundary.print(toolchain::outs());
    });
} // end namespace language::test
