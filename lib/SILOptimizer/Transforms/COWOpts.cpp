//===--- COWOpts.cpp - Optimize COW operations ----------------------------===//
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
// This pass optimizes begin_cow_mutation and end_cow_mutation patterns.
//
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "cow-opts"
#include "language/SILOptimizer/PassManager/Transforms.h"
#include "language/SILOptimizer/Analysis/AliasAnalysis.h"
#include "language/SIL/NodeBits.h"
#include "language/SIL/SILFunction.h"
#include "language/SIL/SILBasicBlock.h"
#include "language/SIL/SILArgument.h"
#include "language/SIL/SILBuilder.h"
#include "language/SIL/StackList.h"
#include "toolchain/Support/Debug.h"

using namespace language;

namespace {

/// Constant folds the uniqueness result of begin_cow_mutation instructions.
///
/// If it can be proved that the buffer argument is uniquely referenced, the
/// uniqueness result is replaced with a constant boolean "true".
/// For example:
///
/// \code
///     %buffer = end_cow_mutation %mutable_buffer
///     // ...
///     // %buffer does not escape here
///     // ...
///     (%is_unique, %mutable_buffer2) = begin_cow_mutation %buffer
///     cond_br %is_unique, ...
/// \endcode
///
/// is replaced with
///
/// \code
///     %buffer = end_cow_mutation [keep_unique] %mutable_buffer
///     // ...
///     (%not_used, %mutable_buffer2) = begin_cow_mutation %buffer
///     %true = integer_literal 1
///     cond_br %true, ...
/// \endcode
///
/// Note that the keep_unique flag is set on the end_cow_mutation because the
/// code now relies on that the buffer is really uniquely referenced.
///
/// The optimization can also handle def-use chains between end_cow_mutation and
/// begin_cow_mutation which involve phi-arguments.
///
class COWOptsPass : public SILFunctionTransform {
public:
  COWOptsPass() {}

  void run() override;

private:
  AliasAnalysis *AA = nullptr;

  bool optimizeBeginCOW(BeginCOWMutationInst *BCM);

  static void collectEscapePoints(SILValue v,
                                  InstructionSetWithSize &escapePoints,
                                  ValueSet &handled);
};

void COWOptsPass::run() {
  SILFunction *F = getFunction();
  if (!F->shouldOptimize())
    return;

  TOOLCHAIN_DEBUG(toolchain::dbgs() << "*** COW optimization on function: "
                          << F->getName() << " ***\n");

  AA = PM->getAnalysis<AliasAnalysis>(F);

  bool changed = false;
  for (SILBasicBlock &block : *F) {
  
    for (SILInstruction &inst : block) {
      if (auto *beginCOW = dyn_cast<BeginCOWMutationInst>(&inst)) {
        if (optimizeBeginCOW(beginCOW))
          changed = true;
      }
    }
  }

  if (changed) {
    invalidateAnalysis(SILAnalysis::InvalidationKind::Instructions);
  }
}

static SILValue skipStructAndExtract(SILValue value) {
  while (true) {
    if (auto *si = dyn_cast<StructInst>(value)) {
      if (si->getNumOperands() != 1)
        return value;
      value = si->getOperand(0);
      continue;
    }
    if (auto *sei = dyn_cast<StructExtractInst>(value)) {
      value = sei->getOperand();
      continue;
    }
    return value;
  }
}

bool COWOptsPass::optimizeBeginCOW(BeginCOWMutationInst *BCM) {
  TOOLCHAIN_DEBUG(toolchain::dbgs() << "Looking at: ");
  TOOLCHAIN_DEBUG(BCM->dump());

  SILFunction *function = BCM->getFunction();
  StackList<EndCOWMutationInst *> endCOWMutationInsts(function);
  InstructionSet endCOWMutationsFound(function);

  {
    // Collect all end_cow_mutation instructions, used by the begin_cow_mutation,
    // looking through block phi-arguments.
    StackList<SILValue> workList(function);
    ValueSet handled(function);
    workList.push_back(BCM->getOperand());
    while (!workList.empty()) {
      SILValue v = skipStructAndExtract(workList.pop_back_val());
      if (SILPhiArgument *arg = dyn_cast<SILPhiArgument>(v)) {
        if (handled.insert(arg)) {
          SmallVector<SILValue, 4> incomingVals;
          if (!arg->getIncomingPhiValues(incomingVals))
            return false;
          for (SILValue incomingVal : incomingVals) {
            workList.push_back(incomingVal);
          }
        }
      } else if (auto *ECM = dyn_cast<EndCOWMutationInst>(v)) {
        if (endCOWMutationsFound.insert(ECM))
          endCOWMutationInsts.push_back(ECM);
      } else {
        return false;
      }
    }
  }

  // Collect all uses of the end_cow_instructions, where the buffer can
  // potentially escape.
  InstructionSetWithSize potentialEscapePoints(function);
  {
    ValueSet handled(function);
    for (EndCOWMutationInst *ECM : endCOWMutationInsts) {
      collectEscapePoints(ECM, potentialEscapePoints, handled);
    }
  }

  if (!potentialEscapePoints.empty()) {
    // Now, this is the complicated part: check if there is an escape point
    // within the liverange between the end_cow_mutation(s) and
    // begin_cow_mutation.
    //
    // For store instructions we do a little bit more: only count a store as an
    // escape if there is a (potential) load from the same address within the
    // liverange.
    StackList<SILInstruction *> instWorkList(function);
    StackList<SILInstruction *> potentialLoadInsts(function);
    StackList<SILValue> storeAddrs(function);
    ValueSet storeAddrsFound(function);
    BasicBlockSet handled(function);
    int numStoresFound = 0;
    int numLoadsFound = 0;
  
    // This is a simple worklist-based backward dataflow analysis.
    // Start at the initial begin_cow_mutation and go backward.
    instWorkList.push_back(BCM);

    while (!instWorkList.empty()) {
      SILInstruction *inst = instWorkList.pop_back_val();
      for (;;) {
        if (potentialEscapePoints.contains(inst)) {
          if (auto *store = dyn_cast<StoreInst>(inst)) {
            // Don't immediately bail on a store instruction. Instead, remember
            // it and check if it interferes with any (potential) load.
            if (storeAddrsFound.insert(store->getDest())) {
              TOOLCHAIN_DEBUG(toolchain::dbgs() << "Found store escape, record: ");
              TOOLCHAIN_DEBUG(inst->dump());
              storeAddrs.push_back(store->getDest());
              numStoresFound += 1;
            }
          } else {
            TOOLCHAIN_DEBUG(toolchain::dbgs() << "Found non-store escape, bailing out: ");
            TOOLCHAIN_DEBUG(inst->dump());
            return false;
          }
        }
        if (inst->mayReadFromMemory()) {
          TOOLCHAIN_DEBUG(toolchain::dbgs() << "Found a may read inst, record: ");
          TOOLCHAIN_DEBUG(inst->dump());
          potentialLoadInsts.push_back(inst);
          numLoadsFound += 1;
        }

        // An end_cow_mutation marks the begin of the liverange. It's the end
        // point of the dataflow analysis.
        auto *ECM = dyn_cast<EndCOWMutationInst>(inst);
        if (ECM && endCOWMutationsFound.contains(ECM))
          break;

        if (inst == &inst->getParent()->front()) {
          for (SILBasicBlock *pred : inst->getParent()->getPredecessorBlocks()) {
            if (handled.insert(pred))
              instWorkList.push_back(pred->getTerminator());
          }
          break;
        }

        inst = &*std::prev(inst->getIterator());
      }
    }
    
    // Check if there is any (potential) load from a memory location where the
    // buffer is stored to.
    if (numStoresFound != 0) {
      // Avoid quadratic behavior. Usually this limit is not exceeded.
      if (numStoresFound * numLoadsFound > 128)
        return false;
      for (SILInstruction *load : potentialLoadInsts) {
        for (SILValue storeAddr : storeAddrs) {
          if (!AA || AA->mayReadFromMemory(load, storeAddr)) {
            TOOLCHAIN_DEBUG(toolchain::dbgs() << "Found a store address aliasing with a load:");
            TOOLCHAIN_DEBUG(load->dump());
            TOOLCHAIN_DEBUG(storeAddr->dump());
            return false;
          }
        }
      }
    }
  }

  // Replace the uniqueness result of the begin_cow_mutation with an integer
  // literal of "true".
  SILBuilderWithScope B(BCM);
  auto *IL = B.createIntegerLiteral(BCM->getLoc(),
                                    BCM->getUniquenessResult()->getType(), 1);
  BCM->getUniquenessResult()->replaceAllUsesWith(IL);
  
  for (EndCOWMutationInst *ECM : endCOWMutationInsts) {
    // This is important for other optimizations: The code is now relying on
    // the buffer to be unique.
    ECM->setKeepUnique();
  }

  return true;
}

void COWOptsPass::collectEscapePoints(SILValue v,
                                      InstructionSetWithSize &escapePoints,
                                      ValueSet &handled) {
  if (!handled.insert(v))
    return;

  for (Operand *use : v->getUses()) {
    SILInstruction *user = use->getUser();
    switch (user->getKind()) {
      case SILInstructionKind::BeginCOWMutationInst:
      case SILInstructionKind::RefElementAddrInst:
      case SILInstructionKind::RefTailAddrInst:
      case SILInstructionKind::DebugValueInst:
        break;
      case SILInstructionKind::BranchInst:
        collectEscapePoints(cast<BranchInst>(user)->getArgForOperand(use),
                            escapePoints, handled);
        break;
      case SILInstructionKind::CondBranchInst:
        if (use->getOperandNumber() != CondBranchInst::ConditionIdx) {
          collectEscapePoints(cast<CondBranchInst>(user)->getArgForOperand(use),
                              escapePoints, handled);
        }
        break;
      case SILInstructionKind::StructInst:
      case SILInstructionKind::StructExtractInst:
      case SILInstructionKind::TupleInst:
      case SILInstructionKind::TupleExtractInst:
      case SILInstructionKind::UncheckedRefCastInst:
        collectEscapePoints(cast<SingleValueInstruction>(user),
                            escapePoints, handled);
        break;
      default:
        // Everything else is considered to be a potential escape of the buffer.
        escapePoints.insert(user);
    }
  }
}

} // end anonymous namespace

SILTransform *language::createCOWOpts() {
  return new COWOptsPass();
}

