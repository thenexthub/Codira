//===--- MoveOnlyDiagnostics.cpp ------------------------------------------===//
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

#include "MoveOnlyDiagnostics.h"

#include "language/AST/Decl.h"
#include "language/AST/DiagnosticsSIL.h"
#include "language/AST/Stmt.h"
#include "language/Basic/Assertions.h"
#include "language/Basic/Defer.h"
#include "language/SIL/BasicBlockBits.h"
#include "language/SIL/BasicBlockDatastructures.h"
#include "language/SIL/DebugUtils.h"
#include "language/SIL/FieldSensitivePrunedLiveness.h"
#include "language/SIL/SILArgument.h"
#include "language/SILOptimizer/Utils/VariableNameUtils.h"
#include "toolchain/Support/Debug.h"

#include "MoveOnlyTypeUtils.h"
using namespace language;
using namespace language::siloptimizer;

static toolchain::cl::opt<bool> SilentlyEmitDiagnostics(
    "move-only-diagnostics-silently-emit-diagnostics",
    toolchain::cl::desc(
        "For testing purposes, emit the diagnostic silently so we can "
        "filecheck the result of emitting an error from the move checkers"),
    toolchain::cl::init(false));

//===----------------------------------------------------------------------===//
//                              MARK: Utilities
//===----------------------------------------------------------------------===//

template <typename... T, typename... U>
static void diagnose(ASTContext &context, SILInstruction *inst, Diag<T...> diag,
                     U &&...args) {
  // See if the consuming use is an owned moveonly_to_copyable whose only
  // user is a return. In that case, use the return loc instead. We do this
  // b/c it is illegal to put a return value location on a non-return value
  // instruction... so we have to hack around this slightly.
  auto loc = inst->getLoc();
  if (auto *mtc = dyn_cast<MoveOnlyWrapperToCopyableValueInst>(inst)) {
    if (auto *ri = mtc->getSingleUserOfType<ReturnInst>()) {
      loc = ri->getLoc();
    }
  }

  // If for testing reasons we want to return that we emitted an error but not
  // emit the actual error itself, return early.
  if (SilentlyEmitDiagnostics)
    return;

  auto sourceLoc = loc.getSourceLoc();
  auto diagKind = context.Diags.declaredDiagnosticKindFor(diag.ID);
  // Do not emit notes on invalid source locations.
  if (!sourceLoc && diagKind == DiagnosticKind::Note) {
    return;
  }
  context.Diags.diagnose(sourceLoc, diag, std::forward<U>(args)...);
}

template <typename... T, typename... U>
static void diagnose(ASTContext &context, SourceLoc loc, Diag<T...> diag,
                     U &&...args) {
  // If for testing reasons we want to return that we emitted an error but not
  // emit the actual error itself, return early.
  if (SilentlyEmitDiagnostics)
    return;

  context.Diags.diagnose(loc, diag, std::forward<U>(args)...);
}

static void getVariableNameForValue(MarkUnresolvedNonCopyableValueInst *mmci,
                                    SmallString<64> &resultingString) {
  VariableNameInferrer inferrer(mmci->getFunction(), resultingString);
  if (inferrer.tryInferNameFromUses(mmci))
    return;
  inferrer.inferByWalkingUsesToDefs(mmci->getOperand());
}

//===----------------------------------------------------------------------===//
//                           MARK: Misc Diagnostics
//===----------------------------------------------------------------------===//

void DiagnosticEmitter::emitCheckerDoesntUnderstandDiagnostic(
    MarkUnresolvedNonCopyableValueInst *markedValue) {
  // If we failed to canonicalize ownership, there was something in the SIL
  // that copy propagation did not understand. Emit a we did not understand
  // error.
  if (markedValue->getType().isMoveOnlyWrapped()) {
    diagnose(
        fn->getASTContext(), markedValue,
        diag::sil_movechecking_not_understand_consumable_and_assignable);
  } else {
    diagnose(fn->getASTContext(), markedValue,
             diag::sil_movechecking_not_understand_moveonly);
  }
  registerDiagnosticEmitted(markedValue);
  emittedCheckerDoesntUnderstandDiagnostic = true;
}

void DiagnosticEmitter::emitCheckedMissedCopyError(SILInstruction *copyInst) {
  diagnose(copyInst->getFunction()->getASTContext(), copyInst,
           diag::sil_movechecking_bug_missed_copy);
}

void DiagnosticEmitter::emitReinitAfterDiscardError(SILInstruction *badReinit,
                                                    SILInstruction *discard) {
  assert(isa<DropDeinitInst>(discard));
  assert(badReinit->getLoc() && "missing loc!");
  assert(discard->getLoc() && "missing loc!");

  diagnose(badReinit->getFunction()->getASTContext(),
           badReinit,
           diag::sil_movechecking_reinit_after_discard);

  diagnose(discard->getFunction()->getASTContext(), discard,
           diag::sil_movechecking_discard_self_here);
}

void DiagnosticEmitter::emitMissingConsumeInDiscardingContext(
    SILInstruction *leftoverDestroy,
    SILInstruction *discard) {
  assert(isa<DropDeinitInst>(discard));

  // A good location is one that has some connection with the original source
  // and corresponds to an exit of the function.
  auto hasGoodLocation = [](SILInstruction *si) -> bool {
    if (!si)
      return false;

    SILLocation loc = si->getLoc();
    if (loc.isNull())
      return false;

    switch (loc.getKind()) {
    case SILLocation::ReturnKind:
    case SILLocation::ImplicitReturnKind:
      return true;

    case SILLocation::RegularKind: {
      Decl *decl = loc.getAsASTNode<Decl>();
      if (decl && isa<AbstractFunctionDecl>(decl)) {
        // Having the function itself as a location results in a location at the
        // first line of the function.  Find another location.
        return false;
      }
      Stmt *stmt = loc.getAsASTNode<Stmt>();
      if (!stmt)
        return true; // For non-statements, assume it is exiting the fn.

      // Prefer statements that can possibly lead to an exit of the function.
      // This is determined by whether the statement causes an exit of a
      // lexical scope; so a 'break' counts but not a 'continue'.
      switch (stmt->getKind()) {
      case StmtKind::Throw:
      case StmtKind::Return:
      case StmtKind::Yield:
      case StmtKind::Break:
      case StmtKind::Then:
      case StmtKind::Fail:
      case StmtKind::PoundAssert:
        return true;

      case StmtKind::Continue:
      case StmtKind::Brace:
      case StmtKind::Defer:
      case StmtKind::If:
      case StmtKind::Guard:
      case StmtKind::While:
      case StmtKind::Do:
      case StmtKind::DoCatch:
      case StmtKind::RepeatWhile:
      case StmtKind::ForEach:
      case StmtKind::Switch:
      case StmtKind::Case:
      case StmtKind::Fallthrough:
      case StmtKind::Discard:
        return false;
      };
    }

    case SILLocation::InlinedKind:
    case SILLocation::MandatoryInlinedKind:
    case SILLocation::CleanupKind:
    case SILLocation::ArtificialUnreachableKind:
      return false;
    };
  };

  // An instruction corresponding to the logical place where the value is
  // destroyed. Ideally an exit point of the function reachable from here or
  // some relevant statement.
  SILInstruction *destroyPoint = leftoverDestroy;
  if (!hasGoodLocation(destroyPoint)) {
    // Search for a nearby function exit reachable from this destroy. We do this
    // because the move checker may have injected or hoisted an existing
    // destroy from leaf blocks to some earlier point. For example, if 'd'
    // represents a destroy of self, then we may have this CFG:
    //
    //        before:      after:
    //           .          d
    //          / \        / \
    //         d   d      .   .
    //
    BasicBlockWorkqueue bfsWorklist = {destroyPoint->getParent()};
    while (auto *bb = bfsWorklist.pop()) {
      TermInst *term = bb->getTerminator();

      // Looking for a block that exits the function or terminates the program.
      if (term->isFunctionExiting() || term->isProgramTerminating()) {
        SILInstruction *candidate = term;

        // Walk backwards until we find an instruction with any source location.
        // Sometimes a terminator like 'unreachable' may not have one, but one
        // of the preceding instructions will.
        while (candidate && candidate->getLoc().isNull())
          candidate = candidate->getPreviousInstruction();

        if (candidate && candidate->getLoc()) {
          destroyPoint = candidate;
          break;
        }
      }

      for (auto *nextBB : term->getSuccessorBlocks())
        bfsWorklist.pushIfNotVisited(nextBB);
    }
  }

  assert(destroyPoint->getLoc() && "missing loc!");
  assert(discard->getLoc() && "missing loc!");

  diagnose(leftoverDestroy->getFunction()->getASTContext(),
           destroyPoint,
           diag::sil_movechecking_discard_missing_consume_self);

  diagnose(discard->getFunction()->getASTContext(), discard,
            diag::sil_movechecking_discard_self_here);
}

//===----------------------------------------------------------------------===//
//                          MARK: Object Diagnostics
//===----------------------------------------------------------------------===//

void DiagnosticEmitter::emitObjectGuaranteedDiagnostic(
    MarkUnresolvedNonCopyableValueInst *markedValue) {
  auto &astContext = fn->getASTContext();
  SmallString<64> varName;
  getVariableNameForValue(markedValue, varName);

  // See if we have any closure capture uses and emit a better diagnostic.
  if (getCanonicalizer().hasPartialApplyConsumingUse()) {
    diagnose(astContext, markedValue,
             diag::sil_movechecking_borrowed_parameter_captured_by_closure,
             varName);
    emitObjectDiagnosticsForPartialApplyUses(varName);
    registerDiagnosticEmitted(markedValue);
  }

  // If we do not have any non-partial apply consuming uses... just exit early.
  if (!getCanonicalizer().hasNonPartialApplyConsumingUse())
    return;

  registerDiagnosticEmitted(markedValue);

  // Check if this value is closure captured. In such a case, emit a special
  // error.
  if (auto *fArg = dyn_cast<SILFunctionArgument>(
          lookThroughCopyValueInsts(markedValue->getOperand()))) {
    if (fArg->isClosureCapture()) {
      diagnose(astContext, markedValue,
               diag::sil_movechecking_capture_consumed,
               varName);
      emitObjectDiagnosticsForGuaranteedUses(
          true /*ignore partial apply uses*/);
      registerDiagnosticEmitted(markedValue);
      return;
    }
  }

  diagnose(astContext, markedValue,
           diag::sil_movechecking_guaranteed_value_consumed, varName);

  emitObjectDiagnosticsForGuaranteedUses(true /*ignore partial apply uses*/);
}

void DiagnosticEmitter::emitObjectOwnedDiagnostic(
    MarkUnresolvedNonCopyableValueInst *markedValue) {
  registerDiagnosticEmitted(markedValue);

  auto &astContext = fn->getASTContext();
  SmallString<64> varName;
  getVariableNameForValue(markedValue, varName);

  // Ok we know that we are going to emit an error. Lets use a little more
  // compile time to emit a nice error.
  InstructionSet consumingUserSet(markedValue->getFunction());
  InstructionSet nonConsumingUserSet(markedValue->getFunction());
  toolchain::SmallDenseMap<SILBasicBlock *, SILInstruction *, 8>
      consumingBlockToUserMap;
  toolchain::SmallDenseMap<SILBasicBlock *, SILInstruction *, 8>
      nonConsumingBlockToUserMap;

  // NOTE: We use all lifetime ending and non-lifetime ending users to ensure
  // that we properly identify cases where the actual boundary use is in a loop
  // further down the loop nest from our original use. In such a case, it will
  // not be identified as part of the boundary and instead we will identify a
  // boundary edge which does not provide us with something that we want to
  // error upon.
  for (auto *user : getCanonicalizer().canonicalizer.getLifetimeEndingUsers()) {
    consumingUserSet.insert(user);
    consumingBlockToUserMap.try_emplace(user->getParent(), user);
  }
  for (auto *user :
       getCanonicalizer().canonicalizer.getNonLifetimeEndingUsers()) {
    nonConsumingUserSet.insert(user);
    nonConsumingBlockToUserMap.try_emplace(user->getParent(), user);
  }

  // Now for each consuming use that needs a copy...
  for (auto *user : getCanonicalizer().consumingUsesNeedingCopy) {
    // First search from user to the end of the block for one of our boundary
    // uses and if it is in the block, emit an error and continue.
    bool foundSingleBlockError = false;
    for (auto ii = std::next(user->getIterator()),
              ie = user->getParent()->end();
         ii != ie; ++ii) {
      if (consumingUserSet.contains(&*ii)) {
        foundSingleBlockError = true;
        diagnose(astContext, markedValue,
                 diag::sil_movechecking_owned_value_consumed_more_than_once,
                 varName);
        diagnose(astContext, user,
                 diag::sil_movechecking_consuming_use_here);
        diagnose(astContext, &*ii,
                 diag::sil_movechecking_consumed_again_here);
        break;
      }

      if (nonConsumingUserSet.contains(&*ii)) {
        foundSingleBlockError = true;
        diagnose(astContext, markedValue,
                 diag::sil_movechecking_value_used_after_consume, varName);
        diagnose(astContext, user,
                 diag::sil_movechecking_consuming_use_here);
        diagnose(astContext, &*ii,
                 diag::sil_movechecking_nonconsuming_use_here);
        break;
      }
    }

    // If we found a single block error for this user, continue.
    if (foundSingleBlockError)
      continue;

    // Otherwise, the reason why the consuming use needs to be copied is in a
    // successor block. Lets go look for that user.
    BasicBlockWorklist worklist(markedValue->getFunction());
    for (auto *succBlock : user->getParent()->getSuccessorBlocks())
      worklist.push(succBlock);
    while (auto *nextBlock = worklist.pop()) {
      // First, check if we are visiting the same block as our user block. In
      // such a case, we found a consuming use within a loop.
      if (nextBlock == user->getParent()) {
        diagnose(astContext, markedValue,
                 diag::sil_movechecking_value_consumed_in_a_loop, varName);
        auto d =
            diag::sil_movechecking_consumed_in_loop_here;
        diagnose(astContext, user, d);
        break;
      }

      {
        auto iter = consumingBlockToUserMap.find(nextBlock);
        if (iter != consumingBlockToUserMap.end()) {
          // We found it... emit the error and break.
          diagnose(
              astContext, markedValue,
              diag::sil_movechecking_owned_value_consumed_more_than_once,
              varName);
          diagnose(astContext, user,
                   diag::sil_movechecking_consuming_use_here);
          diagnose(astContext, iter->second,
                   diag::sil_movechecking_consumed_again_here);
          break;
        }
      }

      {
        auto iter = nonConsumingBlockToUserMap.find(nextBlock);
        if (iter != nonConsumingBlockToUserMap.end()) {
          // We found it... emit the error and break.
          diagnose(astContext, markedValue,
                   diag::sil_movechecking_value_used_after_consume, varName);
          diagnose(astContext, user,
                   diag::sil_movechecking_consuming_use_here);
          diagnose(astContext, iter->second,
                   diag::sil_movechecking_nonconsuming_use_here);
          break;
        }
      }

      // If we didn't break, keep walking successors we haven't seen yet.
      for (auto *succBlock : nextBlock->getSuccessorBlocks()) {
        worklist.pushIfNotVisited(succBlock);
      }
    }
  }
}

void DiagnosticEmitter::emitObjectDiagnosticsForGuaranteedUses(
    bool ignorePartialApplyUses) const {
  auto &astContext = fn->getASTContext();

  for (auto *consumingUser : getCanonicalizer().consumingUsesNeedingCopy) {
    if (ignorePartialApplyUses &&
        OSSACanonicalizer::isPartialApplyUser(consumingUser))
      continue;
    diagnose(astContext, consumingUser,
             diag::sil_movechecking_consuming_use_here);
  }

  for (auto *user : getCanonicalizer().consumingBoundaryUsers) {
    if (ignorePartialApplyUses && OSSACanonicalizer::isPartialApplyUser(user))
      continue;

    diagnose(astContext, user, diag::sil_movechecking_consuming_use_here);
  }
}

void DiagnosticEmitter::emitObjectDiagnosticsForPartialApplyUses(
    StringRef capturedVarName) const {
  auto &astContext = fn->getASTContext();

  for (auto *user : getCanonicalizer().consumingUsesNeedingCopy) {
    if (!OSSACanonicalizer::isPartialApplyUser(user))
      continue;
    diagnose(astContext,
             user,
             diag::sil_movechecking_consuming_closure_use_here,
             capturedVarName);
  }

  for (auto *user : getCanonicalizer().consumingBoundaryUsers) {
    if (!OSSACanonicalizer::isPartialApplyUser(user))
      continue;

    diagnose(astContext,
             user,
             diag::sil_movechecking_consuming_closure_use_here,
             capturedVarName);
  }
}

//===----------------------------------------------------------------------===//
//                         MARK: Address Diagnostics
//===----------------------------------------------------------------------===//

static bool isClosureCapture(MarkUnresolvedNonCopyableValueInst *markedValue) {
  SILValue val = markedValue->getOperand();

  // Sometimes we've mark-must-check'd a begin_access.
  val = stripAccessMarkers(val);

  // look past any project-box
  if (auto *pbi = dyn_cast<ProjectBoxInst>(val))
    val = pbi->getOperand();

  if (auto *fArg = dyn_cast<SILFunctionArgument>(val))
    return fArg->isClosureCapture();

  return false;
}

void DiagnosticEmitter::emitAddressExclusivityHazardDiagnostic(
    MarkUnresolvedNonCopyableValueInst *markedValue,
    SILInstruction *consumingUser) {
  if (!useWithDiagnostic.insert(consumingUser).second)
    return;
  registerDiagnosticEmitted(markedValue);

  auto &astContext = markedValue->getFunction()->getASTContext();
  SmallString<64> varName;
  getVariableNameForValue(markedValue, varName);

  TOOLCHAIN_DEBUG(toolchain::dbgs() << "Emitting error for exclusivity!\n");
  TOOLCHAIN_DEBUG(toolchain::dbgs() << "    Mark: " << *markedValue);
  TOOLCHAIN_DEBUG(toolchain::dbgs() << "    Consuming use: " << *consumingUser);

  diagnose(astContext, markedValue,
           diag::sil_movechecking_bug_exclusivity_violation, varName);
  diagnose(astContext, consumingUser,
           diag::sil_movechecking_consuming_use_here);
}

void DiagnosticEmitter::emitAddressDiagnostic(
    MarkUnresolvedNonCopyableValueInst *markedValue,
    SILInstruction *lastLiveUser, SILInstruction *violatingUser,
    bool isUseConsuming, std::optional<ScopeRequiringFinalInit> scopeKind) {
  if (!useWithDiagnostic.insert(violatingUser).second)
    return;
  registerDiagnosticEmitted(markedValue);

  auto &astContext = markedValue->getFunction()->getASTContext();
  SmallString<64> varName;
  getVariableNameForValue(markedValue, varName);

  TOOLCHAIN_DEBUG(toolchain::dbgs() << "Emitting error!\n");
  TOOLCHAIN_DEBUG(toolchain::dbgs() << "    Mark: " << *markedValue);
  TOOLCHAIN_DEBUG(toolchain::dbgs() << "    Last Live Use: " << *lastLiveUser);
  TOOLCHAIN_DEBUG(toolchain::dbgs() << "    Last Live Use Is Consuming? "
                          << (isUseConsuming ? "yes" : "no") << '\n');
  TOOLCHAIN_DEBUG(toolchain::dbgs() << "    Violating Use: " << *violatingUser);

  // If our liveness use is the same as our violating use, then we know that we
  // had a loop. Give a better diagnostic.
  if (lastLiveUser == violatingUser) {
    diagnose(astContext, markedValue,
             diag::sil_movechecking_value_consumed_in_a_loop, varName);
    diagnose(astContext, violatingUser,
             diag::sil_movechecking_consuming_use_here);
    return;
  }

  if (scopeKind.has_value()) {
    switch (scopeKind.value()) {
    case ScopeRequiringFinalInit::InoutArgument:
      diagnose(astContext, markedValue,
               diag::sil_movechecking_not_reinitialized_before_end_of_function,
               varName, isClosureCapture(markedValue));
      break;
    case ScopeRequiringFinalInit::Coroutine:
      diagnose(astContext, markedValue,
               diag::sil_movechecking_not_reinitialized_before_end_of_coroutine,
               varName);
      break;
    case ScopeRequiringFinalInit::ModifyMemoryAccess:
      diagnose(astContext, markedValue,
               diag::sil_movechecking_not_reinitialized_before_end_of_access,
               varName, isClosureCapture(markedValue));
      break;
    }
    diagnose(astContext, violatingUser,
             diag::sil_movechecking_consuming_use_here);
    return;
  }

  // First if we are consuming emit an error for no implicit copy semantics.
  if (isUseConsuming) {
    diagnose(astContext, markedValue,
             diag::sil_movechecking_owned_value_consumed_more_than_once,
             varName);
    diagnose(astContext, violatingUser,
             diag::sil_movechecking_consuming_use_here);
    diagnose(astContext, lastLiveUser,
             diag::sil_movechecking_consumed_again_here);
    return;
  }

  // Otherwise, use the "used after consuming use" error.
  diagnose(astContext, markedValue,
           diag::sil_movechecking_value_used_after_consume, varName);
  diagnose(astContext, violatingUser,
           diag::sil_movechecking_consuming_use_here);
  diagnose(astContext, lastLiveUser,
           diag::sil_movechecking_nonconsuming_use_here);
}

void DiagnosticEmitter::emitInOutEndOfFunctionDiagnostic(
    MarkUnresolvedNonCopyableValueInst *markedValue,
    SILInstruction *violatingUser) {
  if (!useWithDiagnostic.insert(violatingUser).second)
    return;
  registerDiagnosticEmitted(markedValue);

  assert(cast<SILFunctionArgument>(markedValue->getOperand())
             ->getArgumentConvention()
             .isInoutConvention() &&
         "Expected markedValue to be on an inout");

  auto &astContext = markedValue->getFunction()->getASTContext();
  SmallString<64> varName;
  getVariableNameForValue(markedValue, varName);

  TOOLCHAIN_DEBUG(toolchain::dbgs() << "Emitting inout error error!\n");
  TOOLCHAIN_DEBUG(toolchain::dbgs() << "    Mark: " << *markedValue);
  TOOLCHAIN_DEBUG(toolchain::dbgs() << "    Violating Use: " << *violatingUser);

  // Otherwise, we need to do no implicit copy semantics. If our last use was
  // consuming message:
  diagnose(
      astContext, markedValue,
      diag::sil_movechecking_not_reinitialized_before_end_of_function,
      varName, isClosureCapture(markedValue));
  diagnose(astContext, violatingUser,
           diag::sil_movechecking_consuming_use_here);
}

void DiagnosticEmitter::emitAddressDiagnosticNoCopy(
    MarkUnresolvedNonCopyableValueInst *markedValue,
    SILInstruction *consumingUser) {
  if (!useWithDiagnostic.insert(consumingUser).second)
    return;

  auto &astContext = markedValue->getFunction()->getASTContext();
  SmallString<64> varName;
  getVariableNameForValue(markedValue, varName);

  TOOLCHAIN_DEBUG(toolchain::dbgs() << "Emitting no copy error!\n");
  TOOLCHAIN_DEBUG(toolchain::dbgs() << "    Mark: " << *markedValue);
  TOOLCHAIN_DEBUG(toolchain::dbgs() << "    Consuming Use: " << *consumingUser);

  // Otherwise, we need to do no implicit copy semantics. If our last use was
  // consuming message:
  diagnose(astContext, markedValue,
           diag::sil_movechecking_guaranteed_value_consumed, varName);
  diagnose(astContext, consumingUser,
           diag::sil_movechecking_consuming_use_here);
  registerDiagnosticEmitted(markedValue);
}

void DiagnosticEmitter::emitObjectDestructureNeededWithinBorrowBoundary(
    MarkUnresolvedNonCopyableValueInst *markedValue,
    SILInstruction *destructureNeedingUser,
    TypeTreeLeafTypeRange destructureSpan,
    FieldSensitivePrunedLivenessBoundary &boundary) {
  if (!useWithDiagnostic.insert(destructureNeedingUser).second)
    return;

  auto &astContext = markedValue->getFunction()->getASTContext();
  SmallString<64> varName;
  getVariableNameForValue(markedValue, varName);

  TOOLCHAIN_DEBUG(toolchain::dbgs() << "Emitting destructure can't be created error!\n");
  TOOLCHAIN_DEBUG(toolchain::dbgs() << "    Mark: " << *markedValue);
  TOOLCHAIN_DEBUG(toolchain::dbgs() << "    Destructure Needing Use: "
                          << *destructureNeedingUser);

  diagnose(astContext, markedValue,
           diag::sil_movechecking_use_after_partial_consume, varName);
  diagnose(astContext, destructureNeedingUser,
           diag::sil_movechecking_partial_consume_here);

  // Only emit errors for last users that overlap with our needed destructure
  // bits.
  for (auto pair : boundary.getLastUsers()) {
    if (toolchain::any_of(destructureSpan.getRange(),
                     [&](unsigned index) { return pair.second.test(index); })) {
      TOOLCHAIN_DEBUG(toolchain::dbgs()
                 << "    Destructure Boundary Use: " << *pair.first);
      diagnose(astContext, pair.first, diag::sil_movechecking_nonconsuming_use_here);
    }
  }
  registerDiagnosticEmitted(markedValue);
}

void DiagnosticEmitter::emitObjectInstConsumesValueTwice(
    MarkUnresolvedNonCopyableValueInst *markedValue, Operand *firstUse,
    Operand *secondUse) {
  assert(firstUse->getUser() == secondUse->getUser());
  assert(firstUse->isConsuming());
  assert(secondUse->isConsuming());

  TOOLCHAIN_DEBUG(toolchain::dbgs() << "Emitting object consumes value twice error!\n");
  TOOLCHAIN_DEBUG(toolchain::dbgs() << "    Mark: " << *markedValue);
  TOOLCHAIN_DEBUG(toolchain::dbgs() << "    User: " << *firstUse->getUser());
  TOOLCHAIN_DEBUG(toolchain::dbgs() << "    First Conflicting Operand: "
                          << firstUse->getOperandNumber() << '\n');
  TOOLCHAIN_DEBUG(toolchain::dbgs() << "    Second Conflicting Operand: "
                          << secondUse->getOperandNumber() << '\n');

  auto &astContext = markedValue->getModule().getASTContext();
  SmallString<64> varName;
  getVariableNameForValue(markedValue, varName);
  diagnose(astContext, markedValue,
           diag::sil_movechecking_owned_value_consumed_more_than_once,
           varName);
  diagnose(astContext, firstUse->getUser(),
           diag::sil_movechecking_two_consuming_uses_here);
  registerDiagnosticEmitted(markedValue);
}

void DiagnosticEmitter::emitObjectInstConsumesAndUsesValue(
    MarkUnresolvedNonCopyableValueInst *markedValue, Operand *consumingUse,
    Operand *nonConsumingUse) {
  assert(consumingUse->getUser() == nonConsumingUse->getUser());
  assert(consumingUse->isConsuming());
  assert(!nonConsumingUse->isConsuming());

  TOOLCHAIN_DEBUG(toolchain::dbgs() << "Emitting object consumeed and used error!\n");
  TOOLCHAIN_DEBUG(toolchain::dbgs() << "    Mark: " << *markedValue);
  TOOLCHAIN_DEBUG(toolchain::dbgs() << "    User: " << *consumingUse->getUser());
  TOOLCHAIN_DEBUG(toolchain::dbgs() << "    Consuming Operand: "
                          << consumingUse->getOperandNumber() << '\n');
  TOOLCHAIN_DEBUG(toolchain::dbgs() << "    Non Consuming Operand: "
                          << nonConsumingUse->getOperandNumber() << '\n');

  auto &astContext = markedValue->getModule().getASTContext();
  SmallString<64> varName;
  getVariableNameForValue(markedValue, varName);
  diagnose(astContext, markedValue,
           diag::sil_movechecking_owned_value_consumed_and_used_at_same_time,
           varName);
  diagnose(astContext, consumingUse->getUser(),
           diag::sil_movechecking_consuming_and_non_consuming_uses_here);
  registerDiagnosticEmitted(markedValue);
}

bool DiagnosticEmitter::emitGlobalOrClassFieldLoadedAndConsumed(
    MarkUnresolvedNonCopyableValueInst *markedValue) {
  SmallString<64> varName;
  getVariableNameForValue(markedValue, varName);

  SILValue operand = stripAccessMarkers(markedValue->getOperand());

  // is it a class?
  if (isa<RefElementAddrInst>(operand)) {
    diagnose(markedValue->getModule().getASTContext(),
             markedValue,
             diag::sil_movechecking_notconsumable_but_assignable_was_consumed,
             varName, /*isGlobal=*/false);
    registerDiagnosticEmitted(markedValue);
    return true;
  }

  // is it a global?
  if (isa<GlobalAddrInst>(operand)) {
    diagnose(markedValue->getModule().getASTContext(),
             markedValue,
             diag::sil_movechecking_notconsumable_but_assignable_was_consumed,
             varName, /*isGlobal=*/true);
    registerDiagnosticEmitted(markedValue);
    return true;
  }

  return false;
}

void DiagnosticEmitter::emitAddressEscapingClosureCaptureLoadedAndConsumed(
    MarkUnresolvedNonCopyableValueInst *markedValue) {
  SmallString<64> varName;
  getVariableNameForValue(markedValue, varName);
  diagnose(markedValue->getModule().getASTContext(),
           markedValue,
           diag::sil_movechecking_capture_consumed,
           varName);
  registerDiagnosticEmitted(markedValue);
}

void DiagnosticEmitter::emitPromotedBoxArgumentError(
    MarkUnresolvedNonCopyableValueInst *markedValue, SILFunctionArgument *arg) {
  auto &astContext = fn->getASTContext();
  SmallString<64> varName;
  getVariableNameForValue(markedValue, varName);

  registerDiagnosticEmitted(markedValue);

  // diagnose consume of capture within a closure
  diagnose(astContext,
           arg->getDecl()->getLoc(),
           diag::sil_movechecking_capture_consumed,
           varName);

  // Now for each consuming use that needs a copy...
  for (auto *user : getCanonicalizer().consumingUsesNeedingCopy) {
    diagnose(astContext, user, diag::sil_movechecking_consuming_use_here);
  }

  for (auto *user : getCanonicalizer().consumingBoundaryUsers) {
    diagnose(astContext, user, diag::sil_movechecking_consuming_use_here);
  }
}

void DiagnosticEmitter::emitCannotPartiallyMutateError(
    MarkUnresolvedNonCopyableValueInst *address, PartialMutationError error,
    SILInstruction *user, TypeTreeLeafTypeRange usedBits,
    PartialMutation kind) {

  TypeOffsetSizePair pair(usedBits);

  SmallString<128> pathString;
  auto rootType = address->getType();
  if (error.type != rootType) {
    toolchain::raw_svector_ostream os(pathString);
    auto *fn = address->getFunction();
    pair.constructPathString(error.type, {rootType, fn}, rootType, fn, os);
  }

  auto &astContext = fn->getASTContext();

  SmallString<64> varName;
  getVariableNameForValue(address, varName);

  if (!pathString.empty())
    varName.append(pathString);

  switch (error) {
  case PartialMutationError::Kind::FeatureDisabled: {
    auto feature = partialMutationFeature(error.getKind());
    assert(feature);
    assert(!astContext.LangOpts.hasFeature(*feature));

    switch (kind) {
    case PartialMutation::Kind::Consume:
      diagnose(astContext, user, diag::sil_movechecking_cannot_destructure,
               varName);
      break;
    case PartialMutation::Kind::Reinit:
      diagnose(astContext, user, diag::sil_movechecking_cannot_partially_reinit,
               varName);
      diagnose(astContext, &kind.getEarlierConsumingUse(),
               diag::sil_movechecking_consuming_use_here);
      break;
    }
    registerDiagnosticEmitted(address);
    return;
  }
  case PartialMutationError::Kind::HasDeinit: {
    auto diagnostic = [&]() {
      switch (kind) {
      case PartialMutation::Kind::Consume:
        return diag::sil_movechecking_cannot_destructure_has_deinit;
      case PartialMutation::Kind::Reinit:
        return diag::sil_movechecking_cannot_partially_reinit_has_deinit;
      }
    }();
    diagnose(astContext, user, diagnostic, varName);
    registerDiagnosticEmitted(address);
    auto deinitLoc =
        error.getDeinitingNominal().getValueTypeDestructor()->getLoc(
            /*SerializedOK=*/false);
    if (!deinitLoc)
      return;
    astContext.Diags.diagnose(deinitLoc, diag::sil_movechecking_deinit_here);
    return;
  }
  case PartialMutationError::Kind::NonfrozenImportedType: {
    auto &nominal = error.getNonfrozenImportedNominal();
    auto diagnostic = [&]() {
      switch (kind) {
      case PartialMutation::Kind::Consume:
        return diag::sil_movechecking_cannot_destructure_imported_nonfrozen;
      case PartialMutation::Kind::Reinit:
        return diag::sil_movechecking_cannot_partially_reinit_nonfrozen;
      }
    }();
    diagnose(astContext, user, diagnostic, varName,
             nominal.getDeclaredInterfaceType(), nominal.getModuleContext());
    registerDiagnosticEmitted(address);
    return;
  }
  case PartialMutationError::Kind::NonfrozenUsableFromInlineType: {
    auto &nominal = error.getNonfrozenUsableFromInlineNominal();
    auto diagnostic = [&]() {
      switch (kind) {
      case PartialMutation::Kind::Consume:
        return diag::
            sil_movechecking_cannot_destructure_exported_usableFromInline_alwaysEmitIntoClient;
      case PartialMutation::Kind::Reinit:
        return diag::
            sil_movechecking_cannot_partially_reinit_exported_usableFromInline_alwaysEmitIntoClient;
      }
    }();
    diagnose(astContext, user, diagnostic, varName,
             nominal.getDeclaredInterfaceType());
    registerDiagnosticEmitted(address);
    return;
  }
  case PartialMutationError::Kind::ConsumeDuringDeinit: {
    astContext.Diags.diagnose(user->getLoc().getSourceLoc(),
                              diag::sil_movechecking_consume_during_deinit);
    return;
  }
  }
  toolchain_unreachable("unhandled case");
}
