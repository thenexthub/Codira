//===--- OwnershipUseVisitor.h -------------------------------*- C++ -*----===//
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
/// A visitor that classifies the uses of an OSSA value. The main entry points
/// are:
///
///     bool OwnershipUseVisitor::visitLifetimeEndingUses(SILValue ssaDef)
///
///     bool OwnershipUseVisitor::visitInteriorUses(SILValue ssaDef)
///
/// Extensions of the visitor determine how to handle pointer escapes,
/// reborrows, inner borrows, and scoped addresses.

#ifndef LANGUAGE_SIL_OWNERSHIPUSEVISITOR_H
#define LANGUAGE_SIL_OWNERSHIPUSEVISITOR_H

#include "language/SIL/NodeBits.h"
#include "language/SIL/OwnershipUtils.h"
#include "language/SIL/ScopedAddressUtils.h"
#include "language/SIL/SILArgument.h"
#include "language/SIL/SILBasicBlock.h"
#include "language/SIL/SILInstruction.h"
#include "language/SIL/SILValue.h"

namespace language {

enum class OwnershipUseKind { LifetimeEnding, NonLifetimeEnding };

/// Impl provides:
///
/// - NodeSet visited (if interior uses are visited)
///
/// - 'bool handleUsePoint(Operand *use, UseLifetimeConstraint)'
///
///   Where 'use->get()' is either the original SSA def, or the SILValue that
///   introduces an inner scope for which 'use' ends the scope.
///
/// Impl overrides:
///
/// - 'bool handlePointerEscape(Operand *use)' (default false)
///
/// -  bool handleOwnedPhi(Operand *phiOper) { return true; }
///
/// - 'bool handleOuterReborrow(Operand *phiOper)' (default true)
///
/// - 'bool handleGuaranteedForward(Operand *use)' (default true)
///
/// - 'bool handleInnerBorrow(BorrowingOperand)' (default true)
///
/// - 'bool handleScopedAddress(ScopedAddressValue)' (default true)
///
/// These may be overridden with a transformation that adds uses to the use's
/// instruction or phi, but it may not modify the use-list containing \p
/// use or \p phiOper or in any outer scopes.
template <typename Impl>
class OwnershipUseVisitorBase {
protected:
  /// If this returns true, then handleUsePoint will be called as if the pointer
  /// escape is an instantaneous use. The implementation may decide to track
  /// AddressUseKind::PointerEscaping. Bail-out by default for safety.
  bool handlePointerEscape(Operand *use) {
    return false;
  }

  /// Handle a owned forwarding phi of the original SSA def. Later, the phi
  /// will itself be visited as a use but its scope's uses will not be
  /// transitively visited. The implementation may track owned phis to
  /// continue processing extended liveness.
  ///
  /// Return true to continue visiting other uses.
  bool handleOwnedPhi(Operand *phiOper) { return true; }

  /// Handle a transitive reborrow of the original SSA def. Later, the reborrow
  /// will itself be visited as a use but its scope's uses will not be
  /// transitively visited. The implementation may track outer reborrows to
  /// continue processing extended liveness.
  ///
  /// Return true to continue visiting other uses.
  ///
  /// Note: this can lead to infinite recursion if the client does not maintain
  /// a visited set.
  bool handleOuterReborrow(Operand *phiOper) { return true; }

  /// Handle a guaranteed forwarding instruction. After the returns, this will
  /// itself be visited as a use, but its uses will not be transitively
  /// visited. The implementation may transtitively follow uses or track
  /// unenclosed guaranteed phis to insert missing reborrows.
  ///
  /// Return true to continue visiting other uses.
  bool handleGuaranteedForwardingPhi(Operand *phiOper) { return true; }

  /// If this returns true, then handleUsePoint will be called on the scope
  /// ending operands.
  ///
  /// Handles begin_borrow, load_borrow, store_borrow, begin_apply
  ///
  /// Allows the implementation to complete inner scopes before considering
  /// their scope ending operations as uses of the outer scope.
  bool handleInnerBorrow(BorrowingOperand borrowingOperand) {
    return true;
  }

  /// If this returns true, then handleUsePoint will be called on the scope
  /// ending operands.
  ///
  /// Allows the implementation to complete inner scopes before considering
  /// their scope ending operations as uses of the outer scope.
  ///
  /// This may add uses to the inner scope, but it may not modify the use-list
  /// containing \p scopedAddress or in any outer scopes.
  bool handleScopedAddress(ScopedAddressValue scopedAddress) {
    return true;
  }
};

/// Visit uses relevant to liveness of a single OSSA value.
template <typename Impl>
class OwnershipUseVisitor : OwnershipUseVisitorBase<Impl> {
protected:
  Impl &asImpl() { return static_cast<Impl &>(*this); }

  bool handleUsePoint(Operand *use, UseLifetimeConstraint useConstraint) {
    asImpl().handleUsePoint(use, useConstraint);
    return true;
  }

public:
  /// Assumes that ssaDef's lifetime is complete (linear).
  bool visitLifetimeEndingUses(SILValue ssaDef);

  /// Does not assume that ssaDef's lifetime is complete.
  ///
  /// If ssaDef is either an owned phi or reborrow, then find inner adjacent
  /// phis and treat them just like inner borrow scopes.
  bool visitInteriorUses(SILValue ssaDef);

protected:
  bool visitConsumes(SILValue ssaDef);

  bool visitExtends(SILValue ssaDef);

  bool visitOuterBorrow(SILValue borrowBegin);

  bool visitOuterBorrowScopeEnd(Operand *borrowEnd);

  bool visitInnerBorrow(Operand *borrowingOperand);

  bool visitInnerBorrowScopeEnd(Operand *borrowEnd);

  bool visitOwnedUse(Operand *use);

  bool visitGuaranteedUses(SILValue guaranteedValue) {
    for (Operand *use : guaranteedValue->getUses()) {
      if (!visitGuaranteedUse(use))
        return false;
    }
    return true;
  }

  bool visitGuaranteedUse(Operand *use);

  bool visitInteriorPointerUses(Operand *use);
};

/// Recursively visit all lifetime-ending uses that contribute to the ownership
/// live range of \p ssaDef. This assumes that ssaDef has a complete
/// lifetime and ignores non-lifetime-ending uses.
template <typename Impl>
bool OwnershipUseVisitor<Impl>::visitLifetimeEndingUses(SILValue ssaDef) {
  switch (ssaDef->getOwnershipKind()) {
  case OwnershipKind::Owned:
    return visitConsumes(ssaDef);

  case OwnershipKind::Guaranteed:
    return visitOuterBorrow(ssaDef);

  case OwnershipKind::None:
    if (ssaDef->isFromVarDecl()) {
      return visitExtends(ssaDef);
    }
    TOOLCHAIN_FALLTHROUGH;
  case OwnershipKind::Any:
  case OwnershipKind::Unowned:
    toolchain_unreachable("requires an owned or guaranteed orignalDef");
  }
  return true;
}

template <typename Impl>
bool OwnershipUseVisitor<Impl>::visitConsumes(SILValue ssaDef) {
  for (Operand *use : ssaDef->getUses()) {
    // extend_lifetime instructions are non-consuming but need to be visited
    // because together with consuming uses they enclose all users of the value.
    if (isa<ExtendLifetimeInst>(use->getUser())) {
      if (!handleUsePoint(use, UseLifetimeConstraint::NonLifetimeEnding))
        return false;
      continue;
    }
    if (use->isConsuming()) {
      if (PhiOperand(use) && !asImpl().handleOwnedPhi(use))
        return false;
        
      if (!handleUsePoint(use, UseLifetimeConstraint::LifetimeEnding))
        return false;
    }
  }
  return true;
}

template <typename Impl>
bool OwnershipUseVisitor<Impl>::visitExtends(SILValue ssaDef) {
  for (Operand *use : ssaDef->getUses()) {
    if (isa<ExtendLifetimeInst>(use->getUser())) {
      if (!handleUsePoint(use, UseLifetimeConstraint::NonLifetimeEnding))
        return false;
      continue;
    }
  }
  return true;
}

template <typename Impl>
bool OwnershipUseVisitor<Impl>::visitOuterBorrow(SILValue borrowBegin) {
  BorrowedValue borrow(borrowBegin);
  assert(borrow && "guaranteed values have no lifetime ending uses");
  return borrow.visitLocalScopeEndingUses([this](Operand *borrowEnd) {
    return visitOuterBorrowScopeEnd(borrowEnd);
  });
}

template <typename Impl>
bool OwnershipUseVisitor<Impl>::visitOuterBorrowScopeEnd(Operand *borrowEnd) {
  switch (borrowEnd->getOperandOwnership()) {
  case OperandOwnership::EndBorrow:
    return handleUsePoint(borrowEnd, UseLifetimeConstraint::LifetimeEnding);

  case OperandOwnership::Reborrow:
    if (!asImpl().handleOuterReborrow(borrowEnd))
      return false;

    return handleUsePoint(borrowEnd, UseLifetimeConstraint::LifetimeEnding);

  case OperandOwnership::InstantaneousUse:
    assert(isa<ExtendLifetimeInst>(borrowEnd->getUser()));
    return handleUsePoint(borrowEnd, UseLifetimeConstraint::NonLifetimeEnding);
  default:
    toolchain_unreachable("expected borrow scope end");
  }
}

/// Visit the lifetime-ending uses of borrow scope
/// (begin_borrow, load_borrow, or begin_apply).
template <typename Impl>
bool OwnershipUseVisitor<Impl>::visitInnerBorrow(Operand *borrowingOperand) {
  auto bo = BorrowingOperand(borrowingOperand);
  assert(bo && "unexpected Borrow operand ownership");
  if (bo.getScopeIntroducingUserResult()) {
    if (!asImpl().handleInnerBorrow(borrowingOperand))
      return false;

    return bo.visitScopeEndingUses(
      [&](Operand *borrowEnd) {
        return visitInnerBorrowScopeEnd(borrowEnd);
      },
      [&](Operand *unknownUse) {
        return asImpl().handlePointerEscape(unknownUse);
      });
  }
  if (auto dependentValue = bo.getDependentUserResult()) {
    switch (dependentValue->getOwnershipKind()) {
    case OwnershipKind::Guaranteed:
      if (!handleUsePoint(borrowingOperand,
                          UseLifetimeConstraint::NonLifetimeEnding)) {
        return false;
      }
      return visitGuaranteedUses(dependentValue);
    case OwnershipKind::Any:
    case OwnershipKind::None:
    case OwnershipKind::Owned: // non-escapable
    case OwnershipKind::Unowned:
      break;
    }
  }
  return asImpl().handlePointerEscape(borrowingOperand);
}

/// Note: borrowEnd->get() may be a borrow introducer for an inner scope, or a
/// borrow scopes that does not introduce a borrowed value (begin_apply).
template <typename Impl>
bool OwnershipUseVisitor<Impl>::visitInnerBorrowScopeEnd(Operand *borrowEnd) {
  switch (borrowEnd->getOperandOwnership()) {
  case OperandOwnership::EndBorrow:
  case OperandOwnership::Reborrow:
    return handleUsePoint(borrowEnd, UseLifetimeConstraint::NonLifetimeEnding);

  case OperandOwnership::DestroyingConsume:
  case OperandOwnership::ForwardingConsume: {
    // partial_apply [on_stack] and mark_dependence [nonescaping] can introduce
    // borrowing operand and can have destroy_value, return, or store consumes.
    //
    // TODO: When we have a C++ ForwardingUseDefWalker, walk the use-def
    // chain to ensure we have a partial_apply [on_stack] or mark_dependence
    // [nonescaping] def.
    return handleUsePoint(borrowEnd, UseLifetimeConstraint::NonLifetimeEnding);
  }
  case OperandOwnership::InstantaneousUse: {
    auto builtinUser = dyn_cast<BuiltinInst>(borrowEnd->getUser());
    if (builtinUser && builtinUser->getBuiltinKind() ==
                           BuiltinValueKind::EndAsyncLetLifetime) {
      return handleUsePoint(borrowEnd,
                            UseLifetimeConstraint::NonLifetimeEnding);
    }
    TOOLCHAIN_FALLTHROUGH;
  }
  default:
    toolchain_unreachable("expected borrow scope end");
  }
}

/// Recursively visit all uses that contribute to the ownership live range of \p
/// ssaDef. This does not assume that ssaDef has a complete lifetime
/// and visits non-lifetime-ending uses.
///
/// If ssaDef is a phi (owned or reborrowed), then find its inner adjacent phis
/// and treat them like inner borrows.
template <typename Impl>
bool OwnershipUseVisitor<Impl>::visitInteriorUses(SILValue ssaDef) {
  switch (ssaDef->getOwnershipKind()) {
  case OwnershipKind::Owned: {
    for (Operand *use : ssaDef->getUses()) {
      if (!visitOwnedUse(use))
        return false;
    }
    return true;
  }
  case OwnershipKind::Guaranteed: {
    return visitGuaranteedUses(ssaDef);
  }

  case OwnershipKind::Any:
  case OwnershipKind::None:
  case OwnershipKind::Unowned:
    toolchain_unreachable("requires an owned or guaranteed orignalDef");
  }
}

template <typename Impl>
bool OwnershipUseVisitor<Impl>::visitOwnedUse(Operand *use) {
  switch (use->getOperandOwnership()) {
  case OperandOwnership::NonUse:
    return true;

  case OperandOwnership::ForwardingConsume:
  case OperandOwnership::DestroyingConsume:
    if (auto phiOper = PhiOperand(use)) {
      if (!asImpl().handleOwnedPhi(use))
        return false;
    }
    return handleUsePoint(use, UseLifetimeConstraint::LifetimeEnding);

  case OperandOwnership::AnyInteriorPointer:
    return visitInteriorPointerUses(use);

  case OperandOwnership::PointerEscape:
    // TODO: Change ProjectBox ownership to InteriorPointer and allow them to
    // take owned values.
    if (isa<ProjectBoxInst>(use->getUser())) {
      return visitInteriorPointerUses(use);
    }
    if (!asImpl().handlePointerEscape(use))
      return false;

    TOOLCHAIN_FALLTHROUGH;
  case OperandOwnership::InstantaneousUse:
  case OperandOwnership::ForwardingUnowned:
  case OperandOwnership::UnownedInstantaneousUse:
  case OperandOwnership::BitwiseEscape:
    return handleUsePoint(use, UseLifetimeConstraint::NonLifetimeEnding);

  case OperandOwnership::Borrow:
    return visitInnerBorrow(use);

  // TODO: InteriorPointer should be handled like AnyInteriorPointer.
  case OperandOwnership::InteriorPointer:
  case OperandOwnership::TrivialUse:
  case OperandOwnership::EndBorrow:
  case OperandOwnership::Reborrow:
  case OperandOwnership::GuaranteedForwarding:
    toolchain_unreachable("ownership incompatible with an owned value");
  }
}

template <typename Impl>
bool OwnershipUseVisitor<Impl>::visitGuaranteedUse(Operand *use) {
  switch (use->getOperandOwnership()) {
  case OperandOwnership::NonUse:
    return true;

  case OperandOwnership::PointerEscape:
    // TODO: Change ProjectBox ownership to InteriorPointer and allow them to
    // take owned values.
    if (isa<ProjectBoxInst>(use->getUser())) {
      return visitInteriorPointerUses(use);
    }
    if (!asImpl().handlePointerEscape(use))
      return false;

    TOOLCHAIN_FALLTHROUGH;
  case OperandOwnership::InstantaneousUse:
  case OperandOwnership::ForwardingUnowned:
  case OperandOwnership::UnownedInstantaneousUse:
  case OperandOwnership::BitwiseEscape:
    return handleUsePoint(use, UseLifetimeConstraint::NonLifetimeEnding);
  case OperandOwnership::EndBorrow:
    return handleUsePoint(use, UseLifetimeConstraint::LifetimeEnding);

  case OperandOwnership::Reborrow:
    if (!asImpl().handleOuterReborrow(use))
      return false;

    return handleUsePoint(use, UseLifetimeConstraint::LifetimeEnding);

  case OperandOwnership::GuaranteedForwarding:
    if (!handleUsePoint(use, UseLifetimeConstraint::NonLifetimeEnding))
      return false;

    if (PhiOperand(use)) {
      return asImpl().handleGuaranteedForwardingPhi(use);
    }

    if (!asImpl().visited.insert(use->getUser()->asSILNode()))
      return true;

    return ForwardingOperand(use).visitForwardedValues([&](SILValue result) {
      // Do not include transitive uses with 'none' ownership
      if (result->getOwnershipKind() != OwnershipKind::None) {
        return visitGuaranteedUses(result);
      }
      return true;
    });

  case OperandOwnership::Borrow:
    return visitInnerBorrow(use);

  case OperandOwnership::InteriorPointer:
  case OperandOwnership::AnyInteriorPointer:
    return visitInteriorPointerUses(use);

  case OperandOwnership::TrivialUse:
  case OperandOwnership::ForwardingConsume:
  case OperandOwnership::DestroyingConsume:
    toolchain_unreachable("ownership incompatible with a guaranteed value");
  }
}

template <typename Impl>
bool OwnershipUseVisitor<Impl>::visitInteriorPointerUses(Operand *use) {
  assert(use->getOperandOwnership() == OperandOwnership::InteriorPointer
         || use->getOperandOwnership() == OperandOwnership::AnyInteriorPointer
         || isa<ProjectBoxInst>(use->getUser()));

  if (auto scopedAddress = ScopedAddressValue::forUse(use)) {
    // e.g. client may need to insert end_borrow if scopedAddress is a store_borrow.
    if (!asImpl().handleScopedAddress(scopedAddress))
      return false;

    return scopedAddress.visitScopeEndingUses([this](Operand *end) {
        return handleUsePoint(end, UseLifetimeConstraint::NonLifetimeEnding);
      });
  }
  handleUsePoint(use, UseLifetimeConstraint::NonLifetimeEnding);

  auto interiorPtrOp = InteriorPointerOperand(use);
  if (interiorPtrOp.getProjectedAddress()->use_empty()) {
    // findTransitiveUses reports PointerEscape for a dead interior address. But
    // the use-point was already recorded above. So it's safe to simply return.
    return true;
  }

  // TODO: findTransitiveUses should be a visitor so we're not recursively
  // allocating use vectors and potentially merging the use points.
  //
  // TODO: handleInnerBorrow needs to be called for any transitive load_borrow
  // uses to ensure their lifetimes are complete. For now, findTransitiveUses
  // just assumes that all scopes are incomplete.
  SmallVector<Operand *, 8> interiorUses;
  auto useKind = InteriorPointerOperand(use).findTransitiveUses(&interiorUses);
  if (useKind != AddressUseKind::NonEscaping) {
    if (!asImpl().handlePointerEscape(use))
      return false;
  }
  for (auto *interiorUse : interiorUses) {
    if (!handleUsePoint(interiorUse, UseLifetimeConstraint::NonLifetimeEnding))
      return false;
  }
  return true;
}

} // namespace language

#endif
