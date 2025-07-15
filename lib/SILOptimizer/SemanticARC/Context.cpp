//===--- Context.cpp ------------------------------------------------------===//
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

#define DEBUG_TYPE "sil-semantic-arc-opts"

#include "Context.h"
#include "language/Basic/Assertions.h"
#include "language/SIL/MemAccessUtils.h"
#include "language/SIL/Projection.h"
#include "toolchain/Support/Debug.h"

using namespace language;
using namespace language::semanticarc;

static toolchain::cl::opt<bool>
    VerifyAfterTransformOption("sil-semantic-arc-opts-verify-after-transform",
                               toolchain::cl::Hidden, toolchain::cl::init(false));

void Context::verify() const {
  if (VerifyAfterTransformOption)
    fn.verify();
}

//===----------------------------------------------------------------------===//
//                        Well Behaved Write Analysis
//===----------------------------------------------------------------------===//

/// Returns true if we were able to ascertain that either the initialValue has
/// no write uses or all of the write uses were writes that we could understand.
bool Context::constructCacheValue(
    SILValue initialValue,
    SmallVectorImpl<Operand *> &wellBehavedWriteAccumulator) {
  SmallVector<Operand *, 8> worklist(initialValue->getNonTypeDependentUses());

  while (!worklist.empty()) {
    auto *op = worklist.pop_back_val();
    assert(!op->isTypeDependent() &&
           "Uses that are type dependent should have been filtered before "
           "being inserted into the worklist");
    SILInstruction *user = op->getUser();

    if (Projection::isAddressProjection(user) ||
        isa<ProjectBlockStorageInst>(user)) {
      for (SILValue r : user->getResults()) {
        toolchain::copy(r->getNonTypeDependentUses(),
                   std::back_inserter(worklist));
      }
      continue;
    }

    if (auto *oeai = dyn_cast<OpenExistentialAddrInst>(user)) {
      // Mutable access!
      if (oeai->getAccessKind() != OpenedExistentialAccess::Immutable) {
        wellBehavedWriteAccumulator.push_back(op);
      }

      //  Otherwise, look through it and continue.
      toolchain::copy(oeai->getNonTypeDependentUses(),
                 std::back_inserter(worklist));
      continue;
    }

    if (auto *si = dyn_cast<StoreInst>(user)) {
      // We must be the dest since addresses can not be stored.
      assert(si->getDest() == op->get());
      wellBehavedWriteAccumulator.push_back(op);
      continue;
    }

    // Add any destroy_addrs to the resultAccumulator.
    if (isa<DestroyAddrInst>(user)) {
      wellBehavedWriteAccumulator.push_back(op);
      continue;
    }

    // load_borrow and incidental uses are fine as well.
    if (isa<LoadBorrowInst>(user) || isIncidentalUse(user)) {
      continue;
    }

    // Look through begin_access and mark them/their end_borrow as users.
    if (auto *bai = dyn_cast<BeginAccessInst>(user)) {
      // If we do not have a read, mark this as a write. Also, insert our
      // end_access as well.
      if (bai->getAccessKind() != SILAccessKind::Read) {
        wellBehavedWriteAccumulator.push_back(op);
        transform(bai->getUsersOfType<EndAccessInst>(),
                  std::back_inserter(wellBehavedWriteAccumulator),
                  [](EndAccessInst *eai) { return &eai->getAllOperands()[0]; });
      }

      // And then add the users to the worklist and continue.
      toolchain::copy(bai->getNonTypeDependentUses(),
                 std::back_inserter(worklist));
      continue;
    }

    // If we have a load, we just need to mark the load [take] as a write.
    if (auto *li = dyn_cast<LoadInst>(user)) {
      if (li->getOwnershipQualifier() == LoadOwnershipQualifier::Take) {
        wellBehavedWriteAccumulator.push_back(op);
      }
      continue;
    }

    // If we have a FullApplySite, we need to do per convention/inst logic.
    if (auto fas = FullApplySite::isa(user)) {
      // Begin by seeing if we have an in_guaranteed use. If we do, we are done.
      if (fas.getArgumentConvention(*op) ==
          SILArgumentConvention::Indirect_In_Guaranteed) {
        continue;
      }

      // Then see if we have an apply site that is not a coroutine apply
      // site. In such a case, without further analysis, we can treat it like an
      // instantaneous write and validate that it doesn't overlap with our load
      // [copy].
      if (!fas.beginsCoroutineEvaluation() &&
          fas.getArgumentConvention(*op).isInoutConvention()) {
        wellBehavedWriteAccumulator.push_back(op);
        continue;
      }

      // Otherwise, be conservative and return that we had a write that we did
      // not understand.
      TOOLCHAIN_DEBUG(toolchain::dbgs()
                 << "Function: " << user->getFunction()->getName() << "\n");
      TOOLCHAIN_DEBUG(toolchain::dbgs() << "Value: " << op->get());
      TOOLCHAIN_DEBUG(toolchain::dbgs() << "Unhandled apply site!: " << *user);

      return false;
    }

    // Copy addr that read are just loads.
    if (auto *cai = dyn_cast<CopyAddrInst>(user)) {
      // If our value is the destination, this is a write.
      if (cai->getDest() == op->get()) {
        wellBehavedWriteAccumulator.push_back(op);
        continue;
      }

      // Ok, so we are Src by process of elimination. Make sure we are not being
      // taken.
      if (cai->isTakeOfSrc()) {
        wellBehavedWriteAccumulator.push_back(op);
        continue;
      }

      // Otherwise, we are safe and can continue.
      continue;
    }

    // If we did not recognize the user, just return conservatively that it was
    // written to in a way we did not understand.
    TOOLCHAIN_DEBUG(toolchain::dbgs()
               << "Function: " << user->getFunction()->getName() << "\n");
    TOOLCHAIN_DEBUG(toolchain::dbgs() << "Value: " << op->get());
    TOOLCHAIN_DEBUG(toolchain::dbgs() << "Unknown instruction!: " << *user);
    return false;
  }

  // Ok, we finished our worklist and this address is not being written to.
  return true;
}
