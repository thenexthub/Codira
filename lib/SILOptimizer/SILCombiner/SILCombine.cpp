//===--- SILCombine.cpp ---------------------------------------------------===//
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
// A port of LLVM's InstCombine pass to SIL. Its main purpose is for performing
// small combining operations/peepholes at the SIL level. It additionally
// performs dead code elimination when it initially adds instructions to the
// work queue in order to reduce compile time by not visiting trivially dead
// instructions.
//
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "sil-combine"

#include "SILCombiner.h"
#include "language/Basic/Assertions.h"
#include "language/SIL/BasicBlockDatastructures.h"
#include "language/SIL/DebugUtils.h"
#include "language/SIL/SILBuilder.h"
#include "language/SIL/SILVisitor.h"
#include "language/SIL/Test.h"
#include "language/SILOptimizer/Analysis/AliasAnalysis.h"
#include "language/SILOptimizer/Analysis/DominanceAnalysis.h"
#include "language/SILOptimizer/Analysis/NonLocalAccessBlockAnalysis.h"
#include "language/SILOptimizer/Analysis/SimplifyInstruction.h"
#include "language/SILOptimizer/PassManager/PassManager.h"
#include "language/SILOptimizer/PassManager/Transforms.h"
#include "language/SILOptimizer/Utils/CanonicalizeBorrowScope.h"
#include "language/SILOptimizer/Utils/CanonicalizeInstruction.h"
#include "language/SILOptimizer/Utils/CanonicalizeOSSALifetime.h"
#include "language/SILOptimizer/Utils/DebugOptUtils.h"
#include "language/SILOptimizer/Utils/InstOptUtils.h"
#include "language/SILOptimizer/Utils/SILOptFunctionBuilder.h"
#include "language/SILOptimizer/Utils/StackNesting.h"
#include "toolchain/ADT/SmallPtrSet.h"
#include "toolchain/ADT/SmallVector.h"
#include "toolchain/ADT/Statistic.h"
#include "toolchain/Support/CommandLine.h"
#include "toolchain/Support/Debug.h"
#include <fstream>
#include <set>

using namespace language;

STATISTIC(NumCombined, "Number of instructions combined");
STATISTIC(NumDeadInst, "Number of dead insts eliminated");

static toolchain::cl::opt<bool> EnableSinkingOwnedForwardingInstToUses(
    "silcombine-owned-code-sinking",
    toolchain::cl::desc("Enable sinking of owned forwarding insts"),
    toolchain::cl::init(true), toolchain::cl::Hidden);

// Allow disabling general optimization for targeted unit tests.
static toolchain::cl::opt<bool> EnableSILCombineCanonicalize(
    "sil-combine-canonicalize",
    toolchain::cl::desc("Canonicalization during sil-combine"), toolchain::cl::init(true),
    toolchain::cl::Hidden);

//===----------------------------------------------------------------------===//
//                              Utility Methods
//===----------------------------------------------------------------------===//

/// addReachableCodeToWorklist - Walk the function in depth-first order, adding
/// all reachable code to the worklist.
///
/// This has a couple of tricks to make the code faster and more powerful.  In
/// particular, we DCE instructions as we go, to avoid adding them to the
/// worklist (this significantly speeds up SILCombine on code where many
/// instructions are dead or constant).
void SILCombiner::addReachableCodeToWorklist(SILBasicBlock *BB) {
  BasicBlockWorklist Worklist(BB);
  toolchain::SmallVector<SILInstruction *, 128> InstrsForSILCombineWorklist;

  while (SILBasicBlock *BB = Worklist.pop()) {
    for (SILBasicBlock::iterator BBI = BB->begin(), E = BB->end(); BBI != E; ) {
      SILInstruction *Inst = &*BBI;
      ++BBI;

      // DCE instruction if trivially dead.
      if (isInstructionTriviallyDead(Inst)) {
        ++NumDeadInst;
        TOOLCHAIN_DEBUG(toolchain::dbgs() << "SC: DCE: " << *Inst << '\n');

        // We pass in false here since we need to signal to
        // eraseInstFromFunction to not add this instruction's operands to the
        // worklist since we have not initialized the worklist yet.
        //
        // The reason to just use a default argument here is that it allows us
        // to centralize all instruction removal in SILCombine into this one
        // function. This is important if we want to be able to update analyses
        // in a clean manner.
        eraseInstFromFunction(*Inst, BBI,
                              false /*Don't add operands to worklist*/);
        continue;
      }

      InstrsForSILCombineWorklist.push_back(Inst);
    }

    // Recursively visit successors.
    for (SILBasicBlock *Succ : BB->getSuccessors()) {
      Worklist.pushIfNotVisited(Succ);
    }
  }

  // Once we've found all of the instructions to add to the worklist, add them
  // in reverse order. This way SILCombine will visit from the top of the
  // function down. This jives well with the way that it adds all uses of
  // instructions to the worklist after doing a transformation, thus avoiding
  // some N^2 behavior in pathological cases.
  addInitialGroup(InstrsForSILCombineWorklist);
}

//===----------------------------------------------------------------------===//
//                               Implementation
//===----------------------------------------------------------------------===//

namespace language {

// Define a CanonicalizeInstruction subclass for use in SILCombine.
class SILCombineCanonicalize final : CanonicalizeInstruction {
  SmallSILInstructionWorklist<256> &Worklist;
  bool changed = false;

public:
  SILCombineCanonicalize(SmallSILInstructionWorklist<256> &Worklist,
                         DeadEndBlocks &deadEndBlocks)
      : CanonicalizeInstruction(DEBUG_TYPE, deadEndBlocks), Worklist(Worklist) {
  }

  void notifyNewInstruction(SILInstruction *inst) override {
    Worklist.add(inst);
    Worklist.addUsersOfAllResultsToWorklist(inst);
    changed = true;
  }

  // Just delete the given 'inst' and record its operands. The callback isn't
  // allowed to mutate any other instructions.
  void killInstruction(SILInstruction *inst) override {
    Worklist.eraseSingleInstFromFunction(*inst,
                                         /*AddOperandsToWorklist*/ true);
    changed = true;
  }

  void notifyHasNewUsers(SILValue value) override {
    if (Worklist.size() < 10000) {
      Worklist.addUsersToWorklist(value);
    }
    changed = true;
  }

  bool tryCanonicalize(SILInstruction *inst) {
    if (!EnableSILCombineCanonicalize)
      return false;

    changed = false;
    canonicalize(inst);
    return changed;
  }
};

} // end namespace language

SILCombiner::SILCombiner(SILFunctionTransform *trans,
                         bool removeCondFails, bool enableCopyPropagation) :
  parentTransform(trans),
  AA(trans->getPassManager()->getAnalysis<AliasAnalysis>(trans->getFunction())),
  CA(trans->getPassManager()->getAnalysis<BasicCalleeAnalysis>()),
  DA(trans->getPassManager()->getAnalysis<DominanceAnalysis>()),
  PCA(trans->getPassManager()->getAnalysis<ProtocolConformanceAnalysis>()),
  CHA(trans->getPassManager()->getAnalysis<ClassHierarchyAnalysis>()),
  NLABA(trans->getPassManager()->getAnalysis<NonLocalAccessBlockAnalysis>()),
  Worklist("SC"),
  deleter(InstModCallbacks()
              .onDelete([&](SILInstruction *instToDelete) {
                // We allow for users in SILCombine to perform 2 stage
                // deletion, so we need to split the erasing of
                // instructions from adding operands to the worklist.
                eraseInstFromFunction(*instToDelete,
                                      false /* don't add operands */);
              })
              .onNotifyWillBeDeleted(
                  [&](SILInstruction *instThatWillBeDeleted) {
                    Worklist.addOperandsToWorklist(
                      *instThatWillBeDeleted);
                  })
              .onCreateNewInst([&](SILInstruction *newlyCreatedInst) {
                Worklist.add(newlyCreatedInst);
              })
              .onSetUseValue([&](Operand *use, SILValue newValue) {
                use->set(newValue);
                Worklist.add(use->getUser());
              })),
  DEBA(trans->getPassManager()->getAnalysis<DeadEndBlocksAnalysis>()), 
  MadeChange(false), RemoveCondFails(removeCondFails),
  enableCopyPropagation(enableCopyPropagation), Iteration(0),
  Builder(*trans->getFunction(), &TrackingList),
  FuncBuilder(*trans),
  CastOpt(
      FuncBuilder, nullptr /*SILBuilderContext*/,
      /* ReplaceValueUsesAction */
      [&](SILValue Original, SILValue Replacement) {
        replaceValueUsesWith(Original, Replacement);
      },
      /* ReplaceInstUsesAction */
      [&](SingleValueInstruction *I, ValueBase *V) {
        replaceInstUsesWith(*I, V);
      },
      /* EraseAction */
      [&](SILInstruction *I) { eraseInstFromFunction(*I); }),
  deBlocks(trans->getFunction()),
  ownershipFixupContext(getInstModCallbacks(), deBlocks),
  languagePassInvocation(trans->getPassManager(),
                      trans->getFunction(), this) {}

bool SILCombiner::trySinkOwnedForwardingInst(SingleValueInstruction *svi) {
  if (auto *consumingUse = svi->getSingleConsumingUse()) {
    auto *consumingUser = consumingUse->getUser();

    // If our user is already in the same block, we don't move it further.
    if (svi->getParent() == consumingUser->getParent())
      return false;

    // Otherwise, make sure our instruction does not have any non-debug uses
    // that are non-lifetime ending. If so, we return.
    if (toolchain::any_of(getNonDebugUses(svi),
                     [](Operand *use) { return !use->isLifetimeEnding(); }))
      return false;

    TOOLCHAIN_DEBUG(toolchain::dbgs() << "Sink forwarding: " << *svi << '\n');

    // Otherwise, delete all of the debug uses so we don't have to sink them as
    // well and then return true so we process svi in its new position.
    deleteAllDebugUses(svi, getInstModCallbacks());
    svi->moveBefore(consumingUser);
    MadeChange = true;

    // NOTE: We return false here so that our caller doesn't delete the
    // instruction and instead tries to simplify it.
    return false;
  }

  // If we have multiple consuming uses, then we know that our
  // forwarding inst must be live out of the current block and thus we
  // might be able to duplicate/sink.
  if (toolchain::any_of(getNonDebugUses(svi),
                   [](Operand *use) { return !use->isLifetimeEnding(); }))
    return false;

  while (!svi->use_empty()) {
    auto *sviUse = *svi->use_begin();
    auto *sviUser = sviUse->getUser();

    if (auto *dvi = dyn_cast<DestroyValueInst>(sviUser)) {
      dvi->setOperand(svi->getOperand(0));
      Worklist.add(dvi);
      continue;
    }

    if (sviUser->isDebugInstruction()) {
      eraseInstFromFunction(*sviUser);
      continue;
    }

    auto *newSVI = svi->clone(sviUser);

    TOOLCHAIN_DEBUG(toolchain::dbgs()
               << "Sink forwarding: " << *svi << " to " << *newSVI << '\n');

    Worklist.add(newSVI);
    sviUse->set(newSVI);
  }

  eraseInstFromFunction(*svi);
  MadeChange = true;
  return true;
}

/// Canonicalize each extended OSSA lifetime that contains an instruction newly
/// created during this SILCombine iteration.
///
/// \p currentInst is null if the current instruction was deleted during its
/// SILCombine.
///
/// Avoid endless worklist iteration as follows:
///
/// - Canonicalization only runs on the canonical definition of the visited
///   instruction if it was itself a copy or any new copies were inserted
///   as a result of optimization.
///
/// - Instructions are only added back to the SILCombine worklist when
///   canonicalization deletes an instruction. Only the canonical def being
///   processed and its uses are added rather than arbitrary operands of the
///   deleted instruction. This ensures that an instruction is only added back
///   to the worklist after SILCombine either directly optimized it or created a
///   new copy_value for which it is the canonical def or its use.
void SILCombiner::canonicalizeOSSALifetimes(SILInstruction *currentInst) {
  if (!enableCopyPropagation || !Builder.hasOwnership())
    return;

  toolchain::SmallSetVector<SILValue, 16> defsToCanonicalize;

  // copyInst was either optimized by a SILCombine visitor or is a copy_value
  // produced by the visitor. Find the canonical def.
  auto recordCopiedDef = [&defsToCanonicalize](CopyValueInst *copyInst) {
    SILValue def = CanonicalizeOSSALifetime::getCanonicalCopiedDef(copyInst);

    // getCanonicalCopiedDef returns a copy whenever that the copy's source is
    // guaranteed. In that case, find the root of the borrowed lifetime. If it
    // is a function argument, then a simple guaranteed canonicalization can be
    // performed. Canonicalizing other borrow scopes is not handled by
    // SILCombine because it's not a single-lifetime canonicalization.  Instead,
    // SILCombine treats a copy that uses a borrowed value as a separate owned
    // live range. Handling the compensation code across the borrow scope
    // boundary requires post processing in a particular order.  The copy
    // propagation pass knows how to handle that. To avoid complexity and ensure
    // fast convergence, rewriting borrow scopes should not be combined with
    // other unrelated transformations.
    if (auto *copyDef = dyn_cast<CopyValueInst>(def)) {
      if (SILValue borrowDef = CanonicalizeBorrowScope::getCanonicalBorrowedDef(
              copyDef->getOperand())) {
        if (isa<SILFunctionArgument>(borrowDef)) {
          def = borrowDef;
        }
      }
    }
    defsToCanonicalize.insert(def);
  };

  if (auto *copyInst = dyn_cast_or_null<CopyValueInst>(currentInst))
    recordCopiedDef(copyInst);

  for (auto *trackedInst : *Builder.getTrackingList()) {
    if (trackedInst->isDeleted())
      continue;
    if (auto *copyInst = dyn_cast<CopyValueInst>(trackedInst))
      recordCopiedDef(copyInst);
  }
  if (defsToCanonicalize.empty())
    return;

  // Remove instructions deleted during canonicalization from SILCombine's
  // worklist. CanonicalizeOSSALifetime invalidates operands before invoking
  // the deletion callback.
  auto canonicalizeCallbacks =
      InstModCallbacks().onDelete([this](SILInstruction *instToDelete) {
        eraseInstFromFunction(*instToDelete,
                              false /*do not add operands to the worklist*/);
      });
  InstructionDeleter deleter(std::move(canonicalizeCallbacks));

  DominanceInfo *domTree = DA->get(&Builder.getFunction());
  CanonicalizeOSSALifetime canonicalizer(
      DontPruneDebugInsts,
      MaximizeLifetime_t(!parentTransform->getFunction()->shouldOptimize()),
      parentTransform->getFunction(), NLABA, DEBA, domTree, CA, deleter);
  CanonicalizeBorrowScope borrowCanonicalizer(parentTransform->getFunction(),
                                              deleter);

  while (!defsToCanonicalize.empty()) {
    SILValue def = defsToCanonicalize.pop_back_val();

    deleter.getCallbacks().resetHadCallbackInvocation();

    auto canonicalized = [&]() {
      if (!deleter.getCallbacks().hadCallbackInvocation())
        return;

      if (auto *inst = def->getDefiningInstruction()) {
        Worklist.add(inst);
      }
      for (auto *use : def->getUses()) {
        Worklist.add(use->getUser());
      }
    };

    if (def->getOwnershipKind() == OwnershipKind::Guaranteed) {
      if (auto functionArg = dyn_cast<SILFunctionArgument>(def)) {
        if (borrowCanonicalizer.canonicalizeFunctionArgument(functionArg))
          canonicalized();
      }
      continue;
    }
    if (canonicalizer.canonicalizeValueLifetime(def)) {
      canonicalized();
    }
  }
}

bool SILCombiner::doOneIteration(SILFunction &F, unsigned Iteration) {
  MadeChange = false;

  TOOLCHAIN_DEBUG(toolchain::dbgs() << "\n\nSILCOMBINE ITERATION #" << Iteration << " on "
                          << F.getName() << "\n");

  // Add reachable instructions to our worklist.
  addReachableCodeToWorklist(&*F.begin());

  SILCombineCanonicalize scCanonicalize(Worklist, *DEBA->get(&F));

  // Process until we run out of items in our worklist.
  while (!Worklist.isEmpty()) {
    SILInstruction *I = Worklist.pop_back_val();

    // When we erase an instruction, we use the map in the worklist to check if
    // the instruction is in the worklist. If it is, we replace it with null
    // instead of shifting all members of the worklist towards the front. This
    // check makes sure that if we run into any such residual null pointers, we
    // skip them.
    if (I == nullptr)
      continue;

    if (!parentTransform->continueWithNextSubpassRun(I))
      return false;

    processInstruction(I, scCanonicalize, MadeChange);
  }

  Worklist.resetChecked();
  return MadeChange;
}

void SILCombiner::processInstruction(SILInstruction *I,
                                     SILCombineCanonicalize &scCanonicalize,
                                     bool &MadeChange) {
  // Check to see if we can DCE the instruction.
  if (isInstructionTriviallyDead(I)) {
    TOOLCHAIN_DEBUG(toolchain::dbgs() << "SC: DCE: " << *I << '\n');
    eraseInstFromFunction(*I);
    ++NumDeadInst;
    MadeChange = true;
    return;
  }
#ifndef NDEBUG
  std::string OrigIStr;
#endif
  TOOLCHAIN_DEBUG(toolchain::raw_string_ostream SS(OrigIStr); I->print(SS);
             OrigIStr = SS.str(););
  TOOLCHAIN_DEBUG(toolchain::dbgs() << "SC: Visiting: " << OrigIStr << '\n');

  // Canonicalize the instruction.
  if (scCanonicalize.tryCanonicalize(I)) {
    MadeChange = true;
    return;
  }

  // If we have reached this point, all attempts to do simple simplifications
  // have failed. First if we have an owned forwarding value, we try to
  // sink. Otherwise, we perform the actual SILCombine operation.
  if (EnableSinkingOwnedForwardingInstToUses) {
    // If we have an ownership forwarding single value inst that forwards
    // through its first argument and it is trivially duplicatable, see if it
    // only has consuming uses. If so, we can duplicate the instruction into
    // the consuming use blocks and destroy any destroy_value uses of it that
    // we see. This makes it easier for SILCombine to fold instructions with
    // owned parameters since chains of these values will be in the same
    // block.
    if (auto *svi = dyn_cast<SingleValueInstruction>(I)) {
      if (auto fwdOp = ForwardingOperation(svi)) {
        if (fwdOp.getSingleForwardingOperand() &&
            SILValue(svi)->getOwnershipKind() == OwnershipKind::Owned) {
          // Try to sink the value. If we sank the value and deleted it,
          // return. If we didn't optimize or sank but we are still able to
          // optimize further, we fall through to SILCombine below.
          if (trySinkOwnedForwardingInst(svi)) {
            return;
          }
        }
      }
    }
  }

  // Then begin... SILCombine.
  Builder.setInsertionPoint(I);

  SILInstruction *currentInst = I;
  if (SILInstruction *Result = visit(I)) {
    ++NumCombined;
    // Should we replace the old instruction with a new one?
    Worklist.replaceInstructionWithInstruction(I, Result
#ifndef NDEBUG
                                               ,
                                               OrigIStr
#endif
    );
    currentInst = Result;
    MadeChange = true;
  }

  // Eliminate copies created that this SILCombine iteration may have
  // introduced during OSSA-RAUW.
  canonicalizeOSSALifetimes(currentInst->isDeleted() ? nullptr : currentInst);

  // Builder's tracking list has been accumulating instructions created by the
  // during this SILCombine iteration. To finish this iteration, go through
  // the tracking list and add its contents to the worklist and then clear
  // said list in preparation for the next iteration.
  for (SILInstruction *I : *Builder.getTrackingList()) {
    if (!I->isDeleted()) {
      TOOLCHAIN_DEBUG(toolchain::dbgs()
                 << "SC: add " << *I << " from tracking list to worklist\n");
      Worklist.add(I);
    }
  }
  Builder.getTrackingList()->clear();
}

namespace language::test {
struct SILCombinerProcessInstruction {
  void operator()(SILCombiner &combiner, SILInstruction *inst,
                  SILCombineCanonicalize &canonicalizer, bool &madeChange) {
    combiner.processInstruction(inst, canonicalizer, madeChange);
  }
};
// Arguments:
// - instruction: the instruction to be processed
// - bool: remove cond_fails
// - bool: enable lifetime canonicalization
// Dumps:
// - the function after the processing is attempted
static FunctionTest SILCombineProcessInstruction(
    "sil_combine_process_instruction",
    [](auto &function, auto &arguments, auto &test) {
      auto inst = arguments.takeInstruction();
      auto removeCondFails = arguments.takeBool();
      auto enableCopyPropagation = arguments.takeBool();
      SILCombiner combiner(test.getPass(), removeCondFails,
                           enableCopyPropagation);
      SILCombineCanonicalize canonicalizer(combiner.Worklist,
                                           *test.getDeadEndBlocks());
      bool madeChange = false;
      SILCombinerProcessInstruction()(combiner, inst, canonicalizer,
                                      madeChange);
      function.dump();
    });
} // end namespace language::test

namespace language::test {
// Arguments:
// - instruction: the instruction to be visited
// Dumps:
// - the function after the visitation is attempted
static FunctionTest SILCombineVisitInstruction(
    "sil_combine_visit_instruction",
    [](auto &function, auto &arguments, auto &test) {
      SILCombiner combiner(test.getPass(), false, false);
      auto inst = arguments.takeInstruction();
      combiner.Builder.setInsertionPoint(inst);
      auto *result = combiner.visit(inst);
      if (result) {
        combiner.Worklist.replaceInstructionWithInstruction(inst, result
#ifndef NDEBUG
                                                            ,
                                                            ""
#endif
        );
      }
      function.dump();
    });
} // end namespace language::test

bool SILCombiner::runOnFunction(SILFunction &F) {
  clear();

  bool Changed = false;
  // Perform iterations until we do not make any changes.
  while (doOneIteration(F, Iteration)) {
    Changed = true;
    ++Iteration;
  }

  if (invalidatedStackNesting) {
    StackNesting::fixNesting(&F);
  }

  assert(TrackingList.empty() && "TrackingList should be fully processed");
  return Changed;
}

void SILCombiner::eraseInstIncludingUsers(SILInstruction *inst) {
  for (SILValue result : inst->getResults()) {
    while (!result->use_empty()) {
      eraseInstIncludingUsers(result->use_begin()->getUser());
    }
  }
  eraseInstFromFunction(*inst);
}

/// Runs a Codira instruction pass.
void SILCombiner::runCodiraInstructionPass(SILInstruction *inst,
                              void (*runFunction)(BridgedInstructionPassCtxt)) {
  languagePassInvocation.startInstructionPassRun(inst);
  runFunction({ {inst->asSILNode()}, {&languagePassInvocation} });
  languagePassInvocation.finishedInstructionPassRun();
}

/// Registered briged instruction pass run functions.
static toolchain::StringMap<BridgedInstructionPassRunFn> languageInstPasses;
static bool passesRegistered = false;

// Called from initializeCodiraModules().
void SILCombine_registerInstructionPass(BridgedStringRef instClassName,
                                        BridgedInstructionPassRunFn runFn) {
  languageInstPasses[instClassName.unbridged()] = runFn;
  passesRegistered = true;
}

#define _RUN_LANGUAGE_SIMPLIFICATON(INST)                                         \
  static BridgedInstructionPassRunFn runFunction = nullptr;                    \
  static bool passDisabled = false;                                            \
  if (!runFunction) {                                                          \
    runFunction = languageInstPasses[#INST];                                      \
    if (!runFunction) {                                                        \
      if (passesRegistered) {                                                  \
        ABORT([&](auto &out) {                                                 \
          out << "Codira pass " << #INST << " is not registered";               \
        });                                                                    \
      } else {                                                                 \
        return nullptr;                                                        \
      }                                                                        \
    }                                                                          \
    StringRef instName = getSILInstructionName(SILInstructionKind::INST);      \
    passDisabled = SILPassManager::isInstructionPassDisabled(instName);        \
  }                                                                            \
  if (passDisabled &&                                                          \
      SILPassManager::disablePassesForFunction(inst->getFunction())) {         \
    return nullptr;                                                            \
  }                                                                            \
  runCodiraInstructionPass(inst, runFunction);                                  \
  return nullptr;

#define INSTRUCTION_SIMPLIFICATION(INST) \
SILInstruction *SILCombiner::visit##INST(INST *inst) {                     \
  _RUN_LANGUAGE_SIMPLIFICATON(INST)                                           \
}                                                                          \

#define INSTRUCTION_SIMPLIFICATION_WITH_LEGACY(INST) \
SILInstruction *SILCombiner::visit##INST(INST *inst) {                     \
  if (auto *result = legacyVisit##INST(inst))                              \
    return result;                                                         \
  if (!inst->isDeleted()) {                                                \
    _RUN_LANGUAGE_SIMPLIFICATON(INST)                                         \
  }                                                                        \
  return nullptr;                                                          \
}                                                                          \

#include "Simplifications.def"

#undef _RUN_LANGUAGE_SIMPLIFICATON

//===----------------------------------------------------------------------===//
//                                Entry Points
//===----------------------------------------------------------------------===//

namespace {

class SILCombine : public SILFunctionTransform {

  /// The entry point to the transformation.
  void run() override {
    bool enableCopyPropagation =
        getOptions().CopyPropagation == CopyPropagationOption::On;
    if (getOptions().EnableOSSAModules) {
      enableCopyPropagation =
          getOptions().CopyPropagation != CopyPropagationOption::Off;
    }

    SILCombiner Combiner(this, getOptions().RemoveRuntimeAsserts,
                         enableCopyPropagation);
    bool Changed = Combiner.runOnFunction(*getFunction());

    if (Changed) {
      updateAllGuaranteedPhis(getPassManager(), getFunction());
      // Invalidate everything.
      invalidateAnalysis(SILAnalysis::InvalidationKind::FunctionBody);
    }
  }
};

} // end anonymous namespace

SILTransform *language::createSILCombine() {
  return new SILCombine();
}

//===----------------------------------------------------------------------===//
//                          CodiraFunctionPassContext
//===----------------------------------------------------------------------===//

void CodiraPassInvocation::eraseInstruction(SILInstruction *inst, bool salvageDebugInfo) {
  if (silCombiner) {
    silCombiner->eraseInstFromFunction(*inst, /*addOperandsToWorklist=*/ true, salvageDebugInfo);
  } else {
    if (salvageDebugInfo) {
      language::salvageDebugInfo(inst);
    }
    if (inst->isStaticInitializerInst()) {
      inst->getParent()->erase(inst, *getPassManager()->getModule());
    } else {
      inst->eraseFromParent();
    }
  }
}

// cond_fail removal based on cond_fail message and containing function name.
//
// The standard library uses _precondition calls which have a message argument.
//
// Allow disabling the generated cond_fail by these message arguments.
//
// For example:
//
//  _precondition(source >= (0 as T), "Negative value is not representable")
// results in a cond_fail "Negative value is not representable".
//
// This commit allows for specifying a file that contains these messages on each
// line.
//
// /path/to/disable_cond_fails:
//
// ```
// Negative value is not representable
// Array index is out of range
// ```
//
// The optimizer will remove these cond_fails if the language frontend is invoked
// with -Xtoolchain -cond-fail-config-file=/path/to/disable_cond_fails.
//
// Additionally, also interpret the lines as function names and check whether
// the current cond_fail is contained in a listed function when considering
// whether to remove it.
static toolchain::cl::opt<std::string> CondFailConfigFile(
    "cond-fail-config-file", toolchain::cl::init(""),
    toolchain::cl::desc("read the cond_fail message strings to elimimate from file"));

static std::set<std::string> CondFailsToRemove;

bool SILCombiner::shouldRemoveCondFail(CondFailInst &CFI) {
  if (CondFailConfigFile.empty())
    return false;

  std::fstream fs(CondFailConfigFile);
  if (!fs) {
    toolchain::errs() << "cannot cond_fail disablement config file\n";
    exit(1);
  }
  if (CondFailsToRemove.empty()) {
    std::string line;
    while (std::getline(fs, line)) {
      CondFailsToRemove.insert(line);
    }
    fs.close();
  }
  // Check whether the cond_fail's containing function was listed in the config
  // file.
  if (CondFailsToRemove.find(CFI.getFunction()->getName().str()) !=
      CondFailsToRemove.end())
    return true;

  // Check whether the cond_fail's message was listed in the config file.
  auto message = CFI.getMessage();
  return CondFailsToRemove.find(message.str()) != CondFailsToRemove.end();
}
