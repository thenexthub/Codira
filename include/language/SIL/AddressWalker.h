//===--- AddressWalker.h --------------------------------------------------===//
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
/// \file
///
/// This header defines a walker for SIL addresses that is guaranteed by the
/// language to be able to traverse the SIL from an address def to all of its
/// transitive uses. This is validated by the SIL optimizer.
///
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_SIL_ADDRESSWALKER_H
#define LANGUAGE_SIL_ADDRESSWALKER_H

#include "language/Basic/Defer.h"
#include "language/SIL/AddressUseKind.h"
#include "language/SIL/BasicBlockDatastructures.h"
#include "language/SIL/InstructionUtils.h"
#include "language/SIL/Projection.h"
#include "language/SIL/SILValue.h"

namespace language {

enum class TransitiveAddressWalkerTransitiveUseVisitation : uint8_t {
  OnlyUses,
  OnlyUser,
  BothUserAndUses,
};

/// A state structure for findTransitiveUsesForAddress. Intended to be only used
/// a single time. Please always use a new one for every call to
/// findTransitiveUsesForAddress.
///
/// Validated by the SIL verifier as always being able to visit all addresses
/// derived from alloc_stack, ref_element_addr, project_box, ref_tail_addr and
/// all other address roots.
template <typename Impl>
class TransitiveAddressWalker {
  /// Whether we could tell if this address use didn't escape, did have a
  /// pointer escape, or unknown if we failed to understand something.
  AddressUseKind result = AddressUseKind::NonEscaping;
  Operand *escapingUse = nullptr;

  unsigned didInvalidate = false;

  Impl &asImpl() { return *reinterpret_cast<Impl *>(this); }
  const Impl &asImpl() const { return *reinterpret_cast<const Impl *>(this); }

protected:
  /// Customization point for visiting uses. Returns true if we should continue
  /// visiting.
  ///
  /// NOTE: Do not call this directly from within
  /// findTransitiveUsesForAddress. Please call callVisitUse. This is intended
  /// just for subclasses to override.
  bool visitUse(Operand *use) { return true; }

  /// Customization point for visiting operands that we were unable to
  /// understand. These cause us to return AddressUseKind::Unknown.
  void onError(Operand *use) {}

  using TransitiveUseVisitation =
      TransitiveAddressWalkerTransitiveUseVisitation;

  /// Customization point that causes the walker to treat a specific transitive
  /// use as an end point use.
  ///
  /// Example: Visiting a mutable or immutable open_existential_addr.
  TransitiveUseVisitation visitTransitiveUseAsEndPointUse(Operand *use) {
    return TransitiveUseVisitation::OnlyUses;
  }

  void meet(AddressUseKind other) {
    assert(!didInvalidate);
    result = language::meet(result, other);
  }

  void recordEscape(Operand *op, AddressUseKind kind = AddressUseKind::PointerEscape) {
    meet(kind);
    escapingUse = op;
  }

private:
  /// Shim that actually calls visitUse and changes early exit.
  void callVisitUse(Operand *use) {
    assert(!didInvalidate);
    if (!asImpl().visitUse(use))
      result = AddressUseKind::Unknown;
  }

public:
  AddressUseKind walk(SILValue address);

  // If the result of walk() is not NonEscaping, this returns a non-null
  // operand that caused the escape or unknown use.
  Operand *getEscapingUse() const { return escapingUse; }
};

template <typename Impl>
inline AddressUseKind
TransitiveAddressWalker<Impl>::walk(SILValue projectedAddress) {
  assert(!didInvalidate);

  // When we exit, set the result to be invalidated so we can't use this again.
  LANGUAGE_DEFER { didInvalidate = true; };

  StackList<Operand *> worklist(projectedAddress->getFunction());
  SmallPtrSet<Operand *, 32> visitedOperands;

  auto addToWorklist = [&](Operand *use) {
    if (visitedOperands.insert(use).second)
      worklist.push_back(use);
  };

  for (auto *use : projectedAddress->getUses()) {
    addToWorklist(use);
  }

  // Record all uses that aren't transitively followed. These are either
  // instantaneous uses of the address, or cause a pointer escape.
  auto transitiveResultUses = [&](Operand *use) {
    auto *svi = cast<SingleValueInstruction>(use->getUser());
    if (svi->use_empty()) {
      return callVisitUse(use);
    }

    auto visitation = asImpl().visitTransitiveUseAsEndPointUse(use);
    if (visitation != TransitiveUseVisitation::OnlyUses)
      callVisitUse(use);

    if (visitation == TransitiveUseVisitation::OnlyUser)
      return;

    for (auto *use : svi->getUses())
      addToWorklist(use);
  };

  while (!worklist.empty()) {
    if (result == AddressUseKind::Unknown)
      return AddressUseKind::Unknown;

    auto *op = worklist.pop_back_val();

    // Skip type dependent operands.
    if (op->isTypeDependent())
      continue;

    // Then update the worklist with new things to find if we recognize this
    // inst and then continue. If we fail, we emit an error at the bottom of the
    // loop that we didn't recognize the user.
    auto *user = op->getUser();

    if (auto *ti = dyn_cast<TermInst>(user)) {
      switch (ti->getTermKind()) {
      case TermKind::BranchInst:
      case TermKind::CondBranchInst:
        // We could have an address phi. To be conservative, just treat this as
        // a point escape.
        recordEscape(op);
        for (auto succBlockArgList : ti->getSuccessorBlockArgumentLists()) {
          auto *succ = succBlockArgList[op->getOperandNumber()];
          for (auto *use : succ->getUses())
            addToWorklist(use);
        }
        continue;

      case TermKind::UnreachableInst:
      case TermKind::UnwindInst:
      case TermKind::ThrowAddrInst:
        toolchain_unreachable("Should never be used");
      case TermKind::SwitchEnumInst:
      case TermKind::SwitchValueInst:
      case TermKind::DynamicMethodBranchInst:
      case TermKind::AwaitAsyncContinuationInst:
      case TermKind::CheckedCastBranchInst:
        toolchain_unreachable("Never takes an address");
      // Point uses.
      case TermKind::ReturnInst:
      case TermKind::ThrowInst:
      case TermKind::YieldInst:
      case TermKind::TryApplyInst:
      case TermKind::SwitchEnumAddrInst:
      case TermKind::CheckedCastAddrBranchInst:
        callVisitUse(op);
        continue;
      }
    }

    if (isa<PartialApplyInst>(user)) {
      recordEscape(op, AddressUseKind::Dependent);
      callVisitUse(op);
      continue;
    }
    if (isa<AddressToPointerInst>(user)) {
      recordEscape(op);
      callVisitUse(op);
      continue;
    }

    // First, eliminate "end point uses" that we just need to check liveness at
    // and do not need to check transitive uses of.
    if (isa<LoadInst>(user) || isa<CopyAddrInst>(user) ||
        isa<MarkUnresolvedMoveAddrInst>(user) || isIncidentalUse(user) ||
        isa<StoreInst>(user) || isa<DestroyAddrInst>(user) ||
        isa<AssignInst>(user) || isa<LoadUnownedInst>(user) ||
        isa<StoreUnownedInst>(user) || isa<EndApplyInst>(user) ||
        isa<LoadWeakInst>(user) || isa<StoreWeakInst>(user) ||
        isa<AssignByWrapperInst>(user) || isa<AssignOrInitInst>(user) ||
        isa<BeginUnpairedAccessInst>(user) ||
        isa<EndUnpairedAccessInst>(user) || isa<WitnessMethodInst>(user) ||
        isa<SelectEnumAddrInst>(user) || isa<InjectEnumAddrInst>(user) ||
        isa<IsUniqueInst>(user) || isa<ValueMetatypeInst>(user) ||
        isa<DebugValueInst>(user) || isa<EndBorrowInst>(user) ||
        isa<ExplicitCopyAddrInst>(user) || isa<DeallocStackInst>(user) ||
        isa<InitBlockStorageHeaderInst>(user) ||
        isa<GetAsyncContinuationAddrInst>(user) ||
        isa<ExistentialMetatypeInst>(user) ||
        isa<UncheckedRefCastAddrInst>(user) || isa<KeyPathInst>(user) ||
        isa<RetainValueAddrInst>(user) || isa<ReleaseValueAddrInst>(user) ||
        isa<PackElementSetInst>(user) || isa<PackElementGetInst>(user) ||
        isa<DeinitExistentialAddrInst>(user) || isa<LoadBorrowInst>(user) ||
        isa<TupleAddrConstructorInst>(user) || isa<DeallocPackInst>(user) ||
        isa<MergeIsolationRegionInst>(user) || isa<EndCOWMutationAddrInst>(user)) {
      callVisitUse(op);
      continue;
    }

    if (isa<UnconditionalCheckedCastAddrInst>(user) ||
        isa<MarkFunctionEscapeInst>(user)) {
      assert(!user->hasResults());
      callVisitUse(op);
      continue;
    }

    // Then handle users that we need to look at transitive uses of.
    if (Projection::isAddressProjection(user) ||
        isa<ProjectBlockStorageInst>(user) ||
        isa<OpenExistentialAddrInst>(user) ||
        isa<InitExistentialAddrInst>(user) || isa<InitEnumDataAddrInst>(user) ||
        isa<BeginAccessInst>(user) || isa<TailAddrInst>(user) ||
        isa<IndexAddrInst>(user) || isa<StoreBorrowInst>(user) ||
        isa<UncheckedAddrCastInst>(user) ||
        isa<VectorBaseAddrInst>(user) ||
        isa<MarkUnresolvedNonCopyableValueInst>(user) ||
        isa<MarkUninitializedInst>(user) || isa<DropDeinitInst>(user) ||
        isa<ProjectBlockStorageInst>(user) || isa<UpcastInst>(user) ||
        isa<TuplePackElementAddrInst>(user) ||
        isa<CopyableToMoveOnlyWrapperAddrInst>(user) ||
        isa<MoveOnlyWrapperToCopyableAddrInst>(user)) {
      transitiveResultUses(op);
      continue;
    }

    if (auto *builtin = dyn_cast<BuiltinInst>(user)) {
      if (auto kind = builtin->getBuiltinKind()) {
        switch (*kind) {
        case BuiltinValueKind::TSanInoutAccess:
        case BuiltinValueKind::ResumeThrowingContinuationReturning:
        case BuiltinValueKind::ResumeNonThrowingContinuationReturning:
        case BuiltinValueKind::GenericAdd:
        case BuiltinValueKind::GenericFAdd:
        case BuiltinValueKind::GenericAnd:
        case BuiltinValueKind::GenericAShr:
        case BuiltinValueKind::GenericLShr:
        case BuiltinValueKind::GenericOr:
        case BuiltinValueKind::GenericFDiv:
        case BuiltinValueKind::GenericMul:
        case BuiltinValueKind::GenericFMul:
        case BuiltinValueKind::GenericSDiv:
        case BuiltinValueKind::GenericExactSDiv:
        case BuiltinValueKind::GenericShl:
        case BuiltinValueKind::GenericSRem:
        case BuiltinValueKind::GenericSub:
        case BuiltinValueKind::GenericFSub:
        case BuiltinValueKind::GenericUDiv:
        case BuiltinValueKind::GenericExactUDiv:
        case BuiltinValueKind::GenericURem:
        case BuiltinValueKind::GenericFRem:
        case BuiltinValueKind::GenericXor:
        case BuiltinValueKind::TaskRunInline:
        case BuiltinValueKind::ZeroInitializer:
        case BuiltinValueKind::PrepareInitialization:
        case BuiltinValueKind::GetEnumTag:
        case BuiltinValueKind::InjectEnumTag:
        case BuiltinValueKind::AddressOfRawLayout:
        case BuiltinValueKind::FlowSensitiveSelfIsolation:
        case BuiltinValueKind::FlowSensitiveDistributedSelfIsolation:
          callVisitUse(op);
          continue;
        default:
          break;
        }
      }
    }

    if (auto fas = FullApplySite::isa(user)) {
      callVisitUse(op);
      continue;
    }

    if (auto mdi = MarkDependenceInstruction(user)) {
      // TODO: continue walking the dependent value, which may not be an
      // address. See AddressUtils.code. Until that is implemented, this must
      // be considered a pointer escape.
      if (op->get() == mdi.getBase()) {
        recordEscape(op, AddressUseKind::Dependent);
        callVisitUse(op);
        continue;
      }
      if (isa<MarkDependenceInst>(user)) {
        // If we are the value use of a forwarding markdep, look through it.
        transitiveResultUses(op);
        continue;
      }
      if (isa<MarkDependenceAddrInst>(user)) {
        // The address operand is simply a leaf use.
        callVisitUse(op);
        continue;
      }
    }

    // We were unable to recognize this user, so set AddressUseKind to unknown
    // and call onError with the specific user that caused the problem.
    asImpl().onError(op);
    return AddressUseKind::Unknown;
  }

  return result;
}

} // namespace language

#endif
