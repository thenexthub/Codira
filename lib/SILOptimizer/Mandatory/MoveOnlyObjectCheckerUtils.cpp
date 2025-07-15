//===--- MoveOnlyChecker.cpp ----------------------------------------------===//
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

#define DEBUG_TYPE "sil-move-only-checker"

#include "language/AST/DiagnosticsSIL.h"
#include "language/AST/TypeCheckRequests.h"
#include "language/Basic/Assertions.h"
#include "language/Basic/FrozenMultiMap.h"
#include "language/Basic/STLExtras.h"
#include "language/SIL/BasicBlockBits.h"
#include "language/SIL/BasicBlockUtils.h"
#include "language/SIL/DebugUtils.h"
#include "language/SIL/FieldSensitivePrunedLiveness.h"
#include "language/SIL/InstructionUtils.h"
#include "language/SIL/NodeBits.h"
#include "language/SIL/OwnershipUtils.h"
#include "language/SIL/PostOrder.h"
#include "language/SIL/PrunedLiveness.h"
#include "language/SIL/SILArgument.h"
#include "language/SIL/SILBuilder.h"
#include "language/SIL/SILFunction.h"
#include "language/SIL/SILInstruction.h"
#include "language/SIL/SILLocation.h"
#include "language/SIL/SILUndef.h"
#include "language/SIL/SILValue.h"
#include "language/SIL/StackList.h"
#include "language/SILOptimizer/Analysis/ClosureScope.h"
#include "language/SILOptimizer/Analysis/DeadEndBlocksAnalysis.h"
#include "language/SILOptimizer/Analysis/DominanceAnalysis.h"
#include "language/SILOptimizer/Analysis/NonLocalAccessBlockAnalysis.h"
#include "language/SILOptimizer/Analysis/PostOrderAnalysis.h"
#include "language/SILOptimizer/PassManager/Transforms.h"
#include "language/SILOptimizer/Utils/CFGOptUtils.h"
#include "language/SILOptimizer/Utils/CanonicalizeOSSALifetime.h"
#include "language/SILOptimizer/Utils/InstructionDeleter.h"
#include "language/SILOptimizer/Utils/SILSSAUpdater.h"
#include "clang/AST/DeclTemplate.h"
#include "toolchain/ADT/ArrayRef.h"
#include "toolchain/ADT/IntervalMap.h"
#include "toolchain/ADT/PointerIntPair.h"
#include "toolchain/ADT/PointerUnion.h"
#include "toolchain/ADT/STLExtras.h"
#include "toolchain/ADT/SmallBitVector.h"
#include "toolchain/ADT/SmallVector.h"
#include "toolchain/Support/Allocator.h"
#include "toolchain/Support/Debug.h"
#include "toolchain/Support/ErrorHandling.h"
#include "toolchain/Support/RecyclingAllocator.h"

#include "MoveOnlyBorrowToDestructureUtils.h"
#include "MoveOnlyDiagnostics.h"
#include "MoveOnlyObjectCheckerUtils.h"

using namespace language;
using namespace language::siloptimizer;

//===----------------------------------------------------------------------===//
//                Mark Must Check Candidate Search for Objects
//===----------------------------------------------------------------------===//

bool language::siloptimizer::
    searchForCandidateObjectMarkUnresolvedNonCopyableValueInsts(
        SILFunction *fn,
        toolchain::SmallSetVector<MarkUnresolvedNonCopyableValueInst *, 32>
            &moveIntroducersToProcess,
        DiagnosticEmitter &emitter) {
  bool localChanged = false;
  for (auto &block : *fn) {
    for (auto ii = block.begin(), ie = block.end(); ii != ie;) {
      auto *mmci = dyn_cast<MarkUnresolvedNonCopyableValueInst>(&*ii);
      ++ii;

      if (!mmci || !mmci->hasMoveCheckerKind() || !mmci->getType().isObject())
        continue;

      moveIntroducersToProcess.insert(mmci);
    }
  }
  return localChanged;
}

//===----------------------------------------------------------------------===//
//                          MARK: OSSACanonicalizer
//===----------------------------------------------------------------------===//

void OSSACanonicalizer::computeBoundaryData(SILValue value) {
  // Now we have our liveness information. First compute the original boundary
  // (which ignores destroy_value).
  PrunedLivenessBoundary originalBoundary;
  canonicalizer.findOriginalBoundary(originalBoundary);

  // Then use that information to stash for our diagnostics the boundary
  // consuming/non-consuming users as well as enter the boundary consuming users
  // into the boundaryConsumignUserSet for quick set testing later.
  using IsInterestingUser = CanonicalizeOSSALifetime::IsInterestingUser;
  InstructionSet boundaryConsumingUserSet(value->getFunction());
  for (auto *lastUser : originalBoundary.lastUsers) {
    TOOLCHAIN_DEBUG(toolchain::dbgs() << "Looking at boundary use: " << *lastUser);
    switch (canonicalizer.isInterestingUser(lastUser)) {
    case IsInterestingUser::NonUser:
      toolchain_unreachable("Last user of original boundary should be a user?!");
    case IsInterestingUser::NonLifetimeEndingUse:
      TOOLCHAIN_DEBUG(toolchain::dbgs() << "    NonLifetimeEndingUse!\n");
      nonConsumingBoundaryUsers.push_back(lastUser);
      continue;
    case IsInterestingUser::LifetimeEndingUse:
      TOOLCHAIN_DEBUG(toolchain::dbgs() << "    LifetimeEndingUse!\n");
      consumingBoundaryUsers.push_back(lastUser);
      boundaryConsumingUserSet.insert(lastUser);
      continue;
    }
  }

  // Then go through any of the consuming interesting uses found by our liveness
  // and any that are not on the boundary are ones that we must error for.
  for (auto *consumingUser : canonicalizer.getLifetimeEndingUsers()) {
    bool isConsumingUseOnBoundary =
        boundaryConsumingUserSet.contains(consumingUser);
    TOOLCHAIN_DEBUG(toolchain::dbgs() << "Is consuming user on boundary "
                            << (isConsumingUseOnBoundary ? "yes" : "no") << ": "
                            << *consumingUser);
    if (!isConsumingUseOnBoundary) {
      consumingUsesNeedingCopy.push_back(consumingUser);
    }
  }
}

bool OSSACanonicalizer::canonicalize() {
  // First compute liveness. If we fail, bail.
  if (!computeLiveness()) {
    return false;
  }

  computeBoundaryData(canonicalizer.getCurrentDef());

  // Finally, rewrite lifetimes.
  rewriteLifetimes();

  return true;
}

//===----------------------------------------------------------------------===//
//                         MARK: Forward Declaration
//===----------------------------------------------------------------------===//

namespace {

struct MoveOnlyObjectCheckerPImpl {
  SILFunction *fn;
  borrowtodestructure::IntervalMapAllocator &allocator;
  DiagnosticEmitter &diagnosticEmitter;

  /// A set of mark_unresolved_non_copyable_value that we are actually going to
  /// process.
  toolchain::SmallSetVector<MarkUnresolvedNonCopyableValueInst *, 32>
      &moveIntroducersToProcess;

  bool changed = false;

  MoveOnlyObjectCheckerPImpl(
      SILFunction *fn, borrowtodestructure::IntervalMapAllocator &allocator,
      DiagnosticEmitter &diagnosticEmitter,
      toolchain::SmallSetVector<MarkUnresolvedNonCopyableValueInst *, 32>
          &moveIntroducersToProcess)
      : fn(fn), allocator(allocator), diagnosticEmitter(diagnosticEmitter),
        moveIntroducersToProcess(moveIntroducersToProcess) {}

  void check(DominanceInfo *domTree,
             DeadEndBlocksAnalysis *deadEndBlocksAnalysis,
             PostOrderAnalysis *poa);

  bool convertBorrowExtractsToOwnedDestructures(
      MarkUnresolvedNonCopyableValueInst *mmci, DominanceInfo *domTree,
      PostOrderAnalysis *poa);

  bool
  checkForSameInstMultipleUseErrors(MarkUnresolvedNonCopyableValueInst *base);

  bool eraseMarkWithCopiedOperand(
    MarkUnresolvedNonCopyableValueInst *markedInst);
};

} // namespace

bool MoveOnlyObjectCheckerPImpl::convertBorrowExtractsToOwnedDestructures(
    MarkUnresolvedNonCopyableValueInst *mmci, DominanceInfo *domTree,
    PostOrderAnalysis *poa) {
  BorrowToDestructureTransform transform(allocator, mmci, mmci,
                                         diagnosticEmitter, poa);
  if (!transform.transform()) {
    TOOLCHAIN_DEBUG(toolchain::dbgs()
               << "Failed to perform borrow to destructure transform!\n");
    return false;
  }

  return true;
}

bool MoveOnlyObjectCheckerPImpl::checkForSameInstMultipleUseErrors(
    MarkUnresolvedNonCopyableValueInst *mmci) {
  TOOLCHAIN_DEBUG(toolchain::dbgs() << "Checking for same inst multiple use error!\n");

  SmallFrozenMultiMap<SILInstruction *, Operand *, 8> instToOperandsMap;
  StackList<Operand *> worklist(mmci->getFunction());
  for (auto *use : mmci->getUses())
    worklist.push_back(use);

  while (!worklist.empty()) {
    auto *nextUse = worklist.pop_back_val();

    switch (nextUse->getOperandOwnership()) {
    case OperandOwnership::NonUse:
      continue;

    case OperandOwnership::ForwardingUnowned:
    case OperandOwnership::PointerEscape:
    case OperandOwnership::BitwiseEscape:
      // None of the "unowned" uses can consume the original value. Simply
      // ignore them.
      continue;

    case OperandOwnership::TrivialUse:
    case OperandOwnership::InstantaneousUse:
    case OperandOwnership::UnownedInstantaneousUse:
      // Look through copy_value.
      if (auto *cvi = dyn_cast<CopyValueInst>(nextUse->getUser())) {
        for (auto *use : cvi->getUses())
          worklist.push_back(use);
        continue;
      }

      // Treat these as non-consuming uses that could have a consuming use as an
      // additional operand.
      TOOLCHAIN_DEBUG(toolchain::dbgs()
                 << "        Found non consuming use: " << *nextUse->getUser());
      instToOperandsMap.insert(nextUse->getUser(), nextUse);
      continue;
    case OperandOwnership::InteriorPointer:
    case OperandOwnership::AnyInteriorPointer:
      // We do not care about interior pointer uses since there aren't any
      // interior pointer using instructions that are also consuming uses.
      continue;

    case OperandOwnership::DestroyingConsume:
      if (isa<DestroyValueInst>(nextUse->getUser()))
        continue;
      [[fallthrough]];
    case OperandOwnership::ForwardingConsume:
      TOOLCHAIN_DEBUG(toolchain::dbgs()
                 << "        Found consuming use: " << *nextUse->getUser());

      instToOperandsMap.insert(nextUse->getUser(), nextUse);
      continue;

    case OperandOwnership::EndBorrow:
    case OperandOwnership::Reborrow:
    case OperandOwnership::GuaranteedForwarding:
      toolchain_unreachable(
          "We do not process borrows recursively so should never see this.");

    case OperandOwnership::Borrow:
      // We don't care about borrows so we don't process them recursively
      continue;
    }
  }

  // Ok, we have our list of potential uses. Sort the multi-map and then search
  // for errors.
  instToOperandsMap.setFrozen();
  for (auto pair : instToOperandsMap.getRange()) {
    TOOLCHAIN_DEBUG(toolchain::dbgs()
               << "Checking inst for multiple uses: " << *pair.first);
    Operand *foundConsumingUse = nullptr;
    Operand *foundNonConsumingUse = nullptr;
    for (auto *use : pair.second) {
      TOOLCHAIN_DEBUG(toolchain::dbgs()
                 << "    Visiting use: " << use->getOperandNumber() << '\n');
      if (use->isConsuming()) {
        TOOLCHAIN_DEBUG(toolchain::dbgs() << "        Is consuming!\n");
        if (foundConsumingUse) {
          // Emit error.
          TOOLCHAIN_DEBUG(
              toolchain::dbgs()
              << "        Had previous consuming use! Emitting error!\n");
          diagnosticEmitter.emitObjectInstConsumesValueTwice(
              mmci, foundConsumingUse, use);
          continue;
        }

        if (foundNonConsumingUse) {
          TOOLCHAIN_DEBUG(
              toolchain::dbgs()
              << "        Had previous non consuming use! Emitting error!\n");
          // Emit error.
          diagnosticEmitter.emitObjectInstConsumesAndUsesValue(
              mmci, use, foundNonConsumingUse);
          continue;
        }

        if (!foundConsumingUse)
          foundConsumingUse = use;
        continue;
      }

      TOOLCHAIN_DEBUG(toolchain::dbgs() << "        Is non consuming!\n");
      if (foundConsumingUse) {
        // Emit error.
        TOOLCHAIN_DEBUG(toolchain::dbgs()
                   << "        Had previous consuming use! Emitting error!\n");
        diagnosticEmitter.emitObjectInstConsumesAndUsesValue(
            mmci, foundConsumingUse, use);
        continue;
      }

      if (!foundNonConsumingUse)
        foundNonConsumingUse = use;
    }
  }

  return true;
}

//===----------------------------------------------------------------------===//
//                          MARK: Main PImpl Routine
//===----------------------------------------------------------------------===//

void MoveOnlyObjectCheckerPImpl::check(
    DominanceInfo *domTree, DeadEndBlocksAnalysis *deadEndBlocksAnalysis,
    PostOrderAnalysis *poa) {
  auto callbacks =
      InstModCallbacks().onDelete([&](SILInstruction *instToDelete) {
        if (auto *mvi =
                dyn_cast<MarkUnresolvedNonCopyableValueInst>(instToDelete))
          moveIntroducersToProcess.remove(mvi);
        instToDelete->eraseFromParent();
      });
  InstructionDeleter deleter(std::move(callbacks));
  OSSACanonicalizer canonicalizer(fn, domTree, deadEndBlocksAnalysis, deleter);
  diagnosticEmitter.initCanonicalizer(&canonicalizer);

  unsigned initialDiagCount = diagnosticEmitter.getDiagnosticCount();

  auto moveIntroducers = toolchain::ArrayRef(moveIntroducersToProcess.begin(),
                                        moveIntroducersToProcess.end());
  while (!moveIntroducers.empty()) {
    MarkUnresolvedNonCopyableValueInst *markedValue = moveIntroducers.front();

    OSSACanonicalizer::LivenessState livenessState(canonicalizer, markedValue);

    moveIntroducers = moveIntroducers.drop_front(1);
    TOOLCHAIN_DEBUG(toolchain::dbgs() << "Visiting: " << *markedValue);

    // Before we do anything, we need to look for borrowed extracted values and
    // convert them to destructure operations.
    unsigned diagCount = diagnosticEmitter.getDiagnosticCount();
    if (!convertBorrowExtractsToOwnedDestructures(markedValue, domTree, poa)) {
      TOOLCHAIN_DEBUG(toolchain::dbgs()
                 << "Borrow extract to owned destructure transformation didn't "
                    "understand part of the SIL\n");
      diagnosticEmitter.emitCheckerDoesntUnderstandDiagnostic(markedValue);
      continue;
    }

    // If we emitted any non-exceptional diagnostics in
    // convertBorrowExtractsToOwnedDestructures, continue and process the next
    // instruction. The user can fix and re-compile. We want the OSSA
    // canonicalizer to be able to assume that all such borrow + struct_extract
    // uses were already handled.
    if (diagCount != diagnosticEmitter.getDiagnosticCount()) {
      TOOLCHAIN_DEBUG(toolchain::dbgs()
                 << "Emitting diagnostic in BorrowExtractToOwnedDestructure "
                    "transformation!\n");
      continue;
    }

    // First search for transitive consuming uses and prove that we do not have
    // any errors where a single instruction consumes the same value twice or
    // consumes and uses a value.
    if (!checkForSameInstMultipleUseErrors(markedValue)) {
      TOOLCHAIN_DEBUG(toolchain::dbgs() << "checkForSameInstMultipleUseError didn't "
                                 "understand part of the SIL\n");
      diagnosticEmitter.emitCheckerDoesntUnderstandDiagnostic(markedValue);
      continue;
    }

    if (diagCount != diagnosticEmitter.getDiagnosticCount()) {
      TOOLCHAIN_DEBUG(toolchain::dbgs() << "Found single inst multiple user error!\n");
      continue;
    }

    // Once that is complete, we then begin to canonicalize ownership, finding
    // our boundary and any uses that need a copy. We in this section only deal
    // with instructions due to our first step where we emitted errors for
    // instructions containing multiple operands.

    // Step 1. Compute liveness.
    if (!canonicalizer.computeLiveness()) {
      diagnosticEmitter.emitCheckerDoesntUnderstandDiagnostic(markedValue);
      TOOLCHAIN_DEBUG(
          toolchain::dbgs()
          << "Emitted checker doesnt understand diagnostic! Exiting early!\n");
      continue;
    } else {
      // Always set changed to true if we succeeded in canonicalizing since we
      // may have rewritten copies.
      changed = true;
    }

    // NOTE: In the following we only rewrite lifetimes once we have emitted
    // diagnostics. This ensures that we can emit diagnostics using the
    // liveness information before rewrite lifetimes has enriched the liveness
    // info with maximized liveness information.

    // Step 2. Compute our boundary non consuming, consuming uses, and consuming
    // uses that need copies.
    canonicalizer.computeBoundaryData(markedValue);

    // If we are asked to perform guaranteed checking, emit an error if we have
    // /any/ consuming boundary uses or uses that need copies and then rewrite
    // lifetimes.
    if (markedValue->getCheckKind() ==
        MarkUnresolvedNonCopyableValueInst::CheckKind::NoConsumeOrAssign) {
      if (canonicalizer.foundAnyConsumingUses()) {
        diagnosticEmitter.emitObjectGuaranteedDiagnostic(markedValue);
      }
      canonicalizer.rewriteLifetimes();
      continue;
    }

    if (!canonicalizer.foundConsumingUseRequiringCopy()) {
      // If we failed to understand how to perform the check or did not find
      // any targets... continue. In the former case we want to fail with a
      // checker did not understand diagnostic later and in the former, we
      // succeeded.
      canonicalizer.rewriteLifetimes();
      continue;
    }

    // Finally emit our object owned diagnostics and then rewrite lifetimes.
    diagnosticEmitter.emitObjectOwnedDiagnostic(markedValue);
    canonicalizer.rewriteLifetimes();
  }

  bool emittedDiagnostic =
      initialDiagCount != diagnosticEmitter.getDiagnosticCount();
  TOOLCHAIN_DEBUG(toolchain::dbgs() << "Emitting checker based diagnostic: "
                          << (emittedDiagnostic ? "yes" : "no") << '\n');

  // Ok, we have success. All of our marker instructions were proven as safe or
  // we emitted a diagnostic. Now we need to clean up the IR by eliminating our
  // marker instructions to signify that the checked SIL is correct. We also
  // perform some small cleanups for guaranteed values if we emitted a
  // diagnostic on them.
  //
  // NOTE: This is enforced in the verifier by only allowing
  // MarkUnresolvedNonCopyableValueInst in Raw SIL. This ensures we do not
  // miss any.
  //
  while (!moveIntroducersToProcess.empty()) {
    auto *markedInst = moveIntroducersToProcess.pop_back_val();

    // If we didn't emit a diagnostic on a non-trivial guaranteed argument,
    // eliminate the copy_value, destroy_values, and the
    // mark_unresolved_non_copyable_value.
    if (!diagnosticEmitter.emittedDiagnosticForValue(markedInst)) {
      if (markedInst->getCheckKind() ==
          MarkUnresolvedNonCopyableValueInst::CheckKind::NoConsumeOrAssign) {
        if (eraseMarkWithCopiedOperand(markedInst)) {
          changed = true;
          continue;
        }
      }
      markedInst->replaceAllUsesWith(markedInst->getOperand());
      markedInst->eraseFromParent();
      changed = true;
    }
  }
}

/// Erase a copy_value operand of MarkUnresolvedNonCopyableValueInst.
bool MoveOnlyObjectCheckerPImpl::eraseMarkWithCopiedOperand(
  MarkUnresolvedNonCopyableValueInst *markedInst)
{
  auto *cvi = dyn_cast<CopyValueInst>(markedInst->getOperand());
  if (!cvi)
    return false;

  auto replacement = cvi->getOperand();
  auto orig = replacement;
  while (true) {
    if (auto *copyToMoveOnly =
        dyn_cast<CopyableToMoveOnlyWrapperValueInst>(orig)) {
      orig = copyToMoveOnly->getOperand();
      continue;
    }
    if (auto *markDep = dyn_cast<MarkDependenceInst>(orig)) {
      orig = markDep->getValue();
      continue;
    }
    break;
  }

  // TODO: Instead of pattern matching specific code generation patterns,
  // we should be able to eliminate any `copy_value` whose canonical
  // lifetime fits within the borrow scope of the borrowed value being
  // copied from.

  // NOTE: destroys is a separate array that we use to avoid iterator
  // invalidation when cleaning up destroy_value of guaranteed checked values.
  SmallVector<DestroyValueInst *, 8> destroys;

  // Handle:
  //
  // bb(%arg : @guaranteed $Type):
  //   %copy = copy_value %arg
  //   %mark = mark_unresolved_non_copyable_value [no_consume_or_assign]
  //   %copy
  if (auto *arg = dyn_cast<SILArgument>(orig)) {
    if (arg->getOwnershipKind() == OwnershipKind::Guaranteed) {
      for (auto *use : markedInst->getConsumingUses()) {
        destroys.push_back(cast<DestroyValueInst>(use->getUser()));
      }
      while (!destroys.empty())
        destroys.pop_back_val()->eraseFromParent();
      markedInst->replaceAllUsesWith(replacement);
      markedInst->eraseFromParent();
      cvi->eraseFromParent();
      return true;
    }
  }

  // Handle:
  //
  //   %1 = load_borrow %0
  //   %2 = copy_value %1
  //   %3 = mark_unresolved_non_copyable_value [no_consume_or_assign] %2
  if (isa<LoadBorrowInst>(orig)) {
    for (auto *use : markedInst->getConsumingUses()) {
      destroys.push_back(cast<DestroyValueInst>(use->getUser()));
    }
    while (!destroys.empty())
      destroys.pop_back_val()->eraseFromParent();
    markedInst->replaceAllUsesWith(replacement);
    markedInst->eraseFromParent();
    cvi->eraseFromParent();
    return true;
  }

  // Handle:
  //   (%yield, ..., %handle) = begin_apply
  //   %copy = copy_value %yield
  //   %mark = mark_unresolved_noncopyable_value [no_consume_or_assign]
  //   %copy
  if (isa_and_nonnull<BeginApplyInst>(orig->getDefiningInstruction())) {
    if (orig->getOwnershipKind() == OwnershipKind::Guaranteed) {
      for (auto *use : markedInst->getConsumingUses()) {
        destroys.push_back(cast<DestroyValueInst>(use->getUser()));
      }
      while (!destroys.empty())
        destroys.pop_back_val()->eraseFromParent();
      markedInst->replaceAllUsesWith(replacement);
      markedInst->eraseFromParent();
      cvi->eraseFromParent();
      return true;
    }
  }
  return false;
}

//===----------------------------------------------------------------------===//
//                            MARK: Driver Routine
//===----------------------------------------------------------------------===//

bool MoveOnlyObjectChecker::check(
    toolchain::SmallSetVector<MarkUnresolvedNonCopyableValueInst *, 32>
        &instsToCheck) {
  assert(instsToCheck.size() &&
         "Should only call this with actual insts to check?!");
  MoveOnlyObjectCheckerPImpl checker(instsToCheck[0]->getFunction(), allocator,
                                     diagnosticEmitter, instsToCheck);
  checker.check(domTree, deadEndBlocksAnalysis, poa);
  return checker.changed;
}
