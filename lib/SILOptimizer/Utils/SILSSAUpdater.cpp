//===--- SILSSAUpdater.cpp - Unstructured SSA Update Tool -----------------===//
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

#include "language/SILOptimizer/Utils/SILSSAUpdater.h"
#include "language/Basic/Assertions.h"
#include "language/Basic/Malloc.h"
#include "language/SIL/OwnershipUtils.h"
#include "language/SIL/SILArgument.h"
#include "language/SIL/SILBasicBlock.h"
#include "language/SIL/SILBuilder.h"
#include "language/SIL/SILModule.h"
#include "language/SIL/SILUndef.h"
#include "language/SIL/Test.h"
#include "language/SILOptimizer/Utils/CFGOptUtils.h"
#include "toolchain/ADT/DenseMap.h"
#include "toolchain/ADT/SmallVector.h"
#include "toolchain/Support/raw_ostream.h"
#include "toolchain/Transforms/Utils/SSAUpdaterImpl.h"

using namespace language;

void *SILSSAUpdater::allocate(unsigned size, unsigned align) const {
  return AlignedAlloc(size, align);
}

void SILSSAUpdater::deallocateSentinel(SILUndef *undef) {
  AlignedFree(undef);
}

SILSSAUpdater::SILSSAUpdater(SmallVectorImpl<SILPhiArgument *> *phis)
    : blockToAvailableValueMap(nullptr), ownershipKind(OwnershipKind::None),
      phiSentinel(nullptr, deallocateSentinel), insertedPhis(phis) {}

SILSSAUpdater::~SILSSAUpdater() = default;

void SILSSAUpdater::initialize(SILFunction *fn, SILType inputType,
                               ValueOwnershipKind kind) {
  type = inputType;
  ownershipKind = kind;

  phiSentinel = std::unique_ptr<SILUndef, void (*)(SILUndef *)>(
      SILUndef::getSentinelValue(fn, this, inputType),
      SILSSAUpdater::deallocateSentinel);

  if (!blockToAvailableValueMap)
    blockToAvailableValueMap.reset(new AvailableValsTy());
  else
    blockToAvailableValueMap->clear();
}

bool SILSSAUpdater::hasValueForBlock(SILBasicBlock *block) const {
  return blockToAvailableValueMap->count(block);
}

/// Indicate that a rewritten value is available in the specified block with the
/// specified value.
void SILSSAUpdater::addAvailableValue(SILBasicBlock *block, SILValue value) {
  assert(value->getOwnershipKind().isCompatibleWith(ownershipKind));
  (*blockToAvailableValueMap)[block] = value;
}

/// Construct SSA form, materializing a value that is live at the end of the
/// specified block.
SILValue SILSSAUpdater::getValueAtEndOfBlock(SILBasicBlock *block) {
  return getValueAtEndOfBlockInternal(block);
}

/// Are all available values identicalTo each other.
static bool
areIdentical(toolchain::DenseMap<SILBasicBlock *, SILValue> &availableValues) {
  if (auto *firstInst =
          dyn_cast<SingleValueInstruction>(availableValues.begin()->second)) {
    for (auto value : availableValues) {
      auto *svi = dyn_cast<SingleValueInstruction>(value.second);
      if (!svi)
        return false;
      if (!svi->isIdenticalTo(firstInst))
        return false;
    }
    return true;
  }

  if (auto *mvir = dyn_cast<MultipleValueInstructionResult>(
          availableValues.begin()->second)) {
    for (auto value : availableValues) {
      auto *result = dyn_cast<MultipleValueInstructionResult>(value.second);
      if (!result)
        return false;
      if (!result->getParent()->isIdenticalTo(mvir->getParent()) ||
          result->getIndex() != mvir->getIndex()) {
        return false;
      }
    }
    return true;
  }

  auto *firstArg = cast<SILArgument>(availableValues.begin()->second);
  for (auto value : availableValues) {
    auto *arg = dyn_cast<SILArgument>(value.second);
    if (!arg)
      return false;
    if (arg != firstArg) {
      return false;
    }
  }

  return true;
}

/// This should be called in top-down order of each def that needs its uses
/// rewritten. The order that we visit uses for a given def is irrelevant.
void SILSSAUpdater::rewriteUse(Operand &use) {
  // Replicate function_refs to their uses. SILGen can't build phi nodes for
  // them and it would not make much sense anyways.
  if (auto *fri = dyn_cast<FunctionRefInst>(use.get())) {
    assert(areIdentical(*blockToAvailableValueMap) &&
           "The function_refs need to have the same value");
    SILInstruction *user = use.getUser();
    use.set(cast<FunctionRefInst>(fri->clone(user)));
    return;
  } else if (auto *pdfri =
                 dyn_cast<PreviousDynamicFunctionRefInst>(use.get())) {
    assert(areIdentical(*blockToAvailableValueMap) &&
           "The function_refs need to have the same value");
    SILInstruction *user = use.getUser();
    use.set(cast<PreviousDynamicFunctionRefInst>(pdfri->clone(user)));
    return;
  } else if (auto *dfri = dyn_cast<DynamicFunctionRefInst>(use.get())) {
    assert(areIdentical(*blockToAvailableValueMap) &&
           "The function_refs need to have the same value");
    SILInstruction *user = use.getUser();
    use.set(cast<DynamicFunctionRefInst>(dfri->clone(user)));
    return;
  } else if (auto *ili = dyn_cast<IntegerLiteralInst>(use.get()))
    if (areIdentical(*blockToAvailableValueMap)) {
      // Some toolchain intrinsics don't like phi nodes as their constant inputs (e.g
      // ctlz).
      SILInstruction *user = use.getUser();
      use.set(cast<IntegerLiteralInst>(ili->clone(user)));
      return;
    }

  // Again we need to be careful here, because ssa construction (with the
  // existing representation) can change the operand from under us.
  UseWrapper useWrapper(&use);

  SILInstruction *user = use.getUser();
  SILValue newVal = getValueInMiddleOfBlock(user->getParent());
  assert(newVal && "Need a valid value");
  static_cast<Operand *>(useWrapper)->set(newVal);
}

/// Get the edge values from the terminator to the destination basic block.
static OperandValueArrayRef getEdgeValuesForTerminator(TermInst *ti,
                                                       SILBasicBlock *toBlock) {
  if (auto *br = dyn_cast<BranchInst>(ti)) {
    assert(br->getDestBB() == toBlock &&
           "Incoming edge block and phi block mismatch");
    return br->getArgs();
  }
  if (auto *cbi = dyn_cast<CondBranchInst>(ti)) {
    bool isTrueEdge = cbi->getTrueBB() == toBlock;
    assert(((isTrueEdge && cbi->getTrueBB() == toBlock) ||
            cbi->getFalseBB() == toBlock) &&
           "Incoming edge block and phi block mismatch");
    return isTrueEdge ? cbi->getTrueArgs() : cbi->getFalseArgs();
  }

  // We need a predecessor who is capable of holding outgoing branch
  // arguments.
  toolchain_unreachable("Unrecognized terminator leading to phi block");
}

/// Check that the argument has the same incoming edge values as the value
/// map.
static bool
isEquivalentPHI(SILPhiArgument *phi,
                toolchain::SmallDenseMap<SILBasicBlock *, SILValue, 8> &valueMap) {
  SILBasicBlock *phiBlock = phi->getParent();
  size_t phiArgEdgeIndex = phi->getIndex();
  for (auto *predBlock : phiBlock->getPredecessorBlocks()) {
    auto desiredVal = valueMap[predBlock];
    OperandValueArrayRef edgeValues =
        getEdgeValuesForTerminator(predBlock->getTerminator(), phiBlock);
    if (edgeValues[phiArgEdgeIndex] != desiredVal)
      return false;
  }
  return true;
}

SILValue SILSSAUpdater::getValueInMiddleOfBlock(SILBasicBlock *block) {
  // If this basic block does not define a value we can just use the value
  // live at the end of the block.
  if (!hasValueForBlock(block))
    return getValueAtEndOfBlock(block);

  /// Otherwise, we have to build SSA for the value defined in this block and
  /// this block's predecessors.
  SILValue singularValue;
  SmallVector<std::pair<SILBasicBlock *, SILValue>, 4> predVals;
  bool firstPred = true;

  // SSAUpdater can modify TerminatorInst and therefore invalidate the
  // predecessor iterator. Find all the predecessors before the SSA update.
  SmallVector<SILBasicBlock *, 4> preds;
  for (auto *predBlock : block->getPredecessorBlocks()) {
    preds.push_back(predBlock);
  }

  for (auto *predBlock : preds) {
    SILValue predVal = getValueAtEndOfBlock(predBlock);
    predVals.push_back(std::make_pair(predBlock, predVal));
    if (firstPred) {
      singularValue = predVal;
      firstPred = false;
    } else if (singularValue != predVal)
      singularValue = SILValue();
  }

  // Return undef for blocks without predecessor.
  if (predVals.empty())
    return SILUndef::get(block->getParent(), type);

  // Check if we already have an equivalent phi.
  if (!block->getArguments().empty()) {
    toolchain::SmallDenseMap<SILBasicBlock *, SILValue, 8> valueMap(predVals.begin(),
                                                               predVals.end());
    for (auto *arg : block->getSILPhiArguments()) {
      if (arg->isPhi() && isEquivalentPHI(arg, valueMap))
        return arg;
    }
  }

  if (singularValue)
    return singularValue;

  // Create a new phi node.
  SILPhiArgument *phiArg = block->createPhiArgument(type, ownershipKind);
  for (auto &pair : predVals) {
    addNewEdgeValueToBranch(pair.first->getTerminator(), block, pair.second,
                            deleter);
  }

  if (insertedPhis)
    insertedPhis->push_back(phiArg);

  return phiArg;
}

namespace toolchain {

/// Traits for the SSAUpdaterImpl specialized for SIL and the SILSSAUpdater.
template <>
class SSAUpdaterTraits<SILSSAUpdater> {
public:
  using BlkT = SILBasicBlock;
  using ValT = SILValue;
  using PhiT = SILPhiArgument;

  using BlkSucc_iterator = SILBasicBlock::succ_iterator;
  static BlkSucc_iterator BlkSucc_begin(BlkT *block) {
    return block->succ_begin();
  }
  static BlkSucc_iterator BlkSucc_end(BlkT *block) { return block->succ_end(); }

  /// Iterator for PHI operands.
  class PHI_iterator {
  private:
    SILBasicBlock::pred_iterator predBlockIter;
    SILBasicBlock *phiBlock;
    size_t phiArgEdgeIndex;

  public:
    explicit PHI_iterator(SILPhiArgument *phiArg) // begin iterator
        : predBlockIter(phiArg->getParent()->pred_begin()),
          phiBlock(phiArg->getParent()), phiArgEdgeIndex(phiArg->getIndex()) {}
    PHI_iterator(SILPhiArgument *phiArg, bool) // end iterator
        : predBlockIter(phiArg->getParent()->pred_end()),
          phiBlock(phiArg->getParent()), phiArgEdgeIndex(phiArg->getIndex()) {}

    PHI_iterator &operator++() {
      ++predBlockIter;
      return *this;
    }

    bool operator==(const PHI_iterator &x) const {
      return predBlockIter == x.predBlockIter;
    }

    bool operator!=(const PHI_iterator& x) const { return !operator==(x); }

    SILValue getValueForBlock(size_t inputArgIndex, SILBasicBlock *block,
                              TermInst *ti) {
      OperandValueArrayRef args = getEdgeValuesForTerminator(ti, block);
      assert(inputArgIndex < args.size() &&
             "Not enough values on incoming edge");
      return args[inputArgIndex];
    }

    SILValue getIncomingValue() {
      return getValueForBlock(phiArgEdgeIndex, phiBlock,
                              (*predBlockIter)->getTerminator());
    }

    SILBasicBlock *getIncomingBlock() { return *predBlockIter; }
  };

  static inline PHI_iterator PHI_begin(PhiT *phi) { return PHI_iterator(phi); }
  static inline PHI_iterator PHI_end(PhiT *phi) {
    return PHI_iterator(phi, true);
  }

  /// Put the predecessors of BB into the Preds vector.
  static void
  FindPredecessorBlocks(SILBasicBlock *block,
                        SmallVectorImpl<SILBasicBlock *> *predBlocks) {
    toolchain::copy(block->getPredecessorBlocks(), std::back_inserter(*predBlocks));
  }

  static SILValue GetPoisonVal(SILBasicBlock *block, SILSSAUpdater *ssaUpdater) {
    return SILUndef::get(block->getParent(), ssaUpdater->type);
  }

  /// Add an Argument to the basic block.
  static SILValue CreateEmptyPHI(SILBasicBlock *block, unsigned numPreds,
                                 SILSSAUpdater *ssaUpdater) {
    // Add the argument to the block.
    SILValue phi(
        block->createPhiArgument(ssaUpdater->type, ssaUpdater->ownershipKind));

    // Mark all predecessor blocks with the sentinel undef value.
    SmallVector<SILBasicBlock *, 4> predBlockList(
        block->getPredecessorBlocks());

    for (auto *predBlock : predBlockList) {
      TermInst *ti = predBlock->getTerminator();
      addNewEdgeValueToBranch(ti, block, ssaUpdater->phiSentinel.get(),
                              ssaUpdater->deleter);
    }

    return phi;
  }

  /// Add \p value as an operand of the phi argument \p phi for the specified
  /// predecessor block \p predBlock.
  static void AddPHIOperand(SILPhiArgument *phi, SILValue value,
                            SILBasicBlock *predBlock) {
    auto *phiBlock = phi->getParent();
    size_t phiArgIndex = phi->getIndex();
    auto *ti = predBlock->getTerminator();

    changeEdgeValue(ti, phiBlock, phiArgIndex, value);
  }

  /// Check if an instruction is a PHI.
  static SILPhiArgument *InstrIsPHI(ValueBase *valueBase) {
    return dyn_cast<SILPhiArgument>(valueBase);
  }

  /// Check if the instruction that defines the specified SILValue is a PHI
  /// instruction.
  static SILPhiArgument *ValueIsPHI(SILValue value, SILSSAUpdater *) {
    return InstrIsPHI(value);
  }

  /// Like ValueIsPHI but also check if the PHI has no source
  /// operands, i.e., it was just added.
  static SILPhiArgument *ValueIsNewPHI(SILValue value,
                                       SILSSAUpdater *ssaUpdater) {
    SILPhiArgument *phiArg = ValueIsPHI(value, ssaUpdater);
    if (!phiArg) {
      return nullptr;
    }

    auto *phiBlock = phiArg->getParent();
    size_t phiArgEdgeIndex = phiArg->getIndex();

    // If all predecessor edges are 'not set' this is a new phi.
    for (auto *predBlock : phiBlock->getPredecessorBlocks()) {
      OperandValueArrayRef edgeValues =
          getEdgeValuesForTerminator(predBlock->getTerminator(), phiBlock);

      assert(phiArgEdgeIndex < edgeValues.size() && "Not enough edges!");

      SILValue edgeValue = edgeValues[phiArgEdgeIndex];
      // Check for the 'not set' sentinel.
      if (edgeValue != ssaUpdater->phiSentinel.get())
        return nullptr;
    }
    return phiArg;
  }

  static SILValue GetPHIValue(SILPhiArgument *phi) { return phi; }
};

} // namespace toolchain

/// Check to see if AvailableVals has an entry for the specified BB and if so,
/// return it.  If not, construct SSA form by first calculating the required
/// placement of PHIs and then inserting new PHIs where needed.
SILValue SILSSAUpdater::getValueAtEndOfBlockInternal(SILBasicBlock *block) {
  AvailableValsTy &availableValues = *blockToAvailableValueMap;
  auto iter = availableValues.find(block);
  if (iter != availableValues.end())
    return iter->second;

  toolchain::SSAUpdaterImpl<SILSSAUpdater> impl(this, &availableValues,
                                           insertedPhis);
  return impl.GetValue(block);
}

/// Construct a use wrapper. For branches we store information so that we
/// can reconstruct the use after the branch has been modified.
///
/// When a branch is modified existing pointers to the operand
/// (ValueUseIterator) become invalid as they point to freed operands.  Instead
/// we store the branch's parent and the idx so that we can reconstruct the use.
UseWrapper::UseWrapper(Operand *inputUse) {
  wrappedUse = nullptr;
  type = kRegularUse;

  SILInstruction *user = inputUse->getUser();

  // Direct branch user.
  if (auto *br = dyn_cast<BranchInst>(user)) {
    for (auto pair : toolchain::enumerate(user->getAllOperands())) {
      if (inputUse == &pair.value()) {
        index = pair.index();
        type = kBranchUse;
        parent = br->getParent();
        return;
      }
    }
  }

  // Conditional branch user.
  if (auto *cbi = dyn_cast<CondBranchInst>(user)) {
    auto operands = user->getAllOperands();
    auto numTrueArgs = cbi->getTrueArgs().size();
    for (auto pair : toolchain::enumerate(operands)) {
      if (inputUse == &pair.value()) {
        unsigned i = pair.index();
        // We treat the condition as part of the true args.
        if (i < numTrueArgs + 1) {
          index = i;
          type = kCondBranchUseTrue;
        } else {
          index = i - numTrueArgs - 1;
          type = kCondBranchUseFalse;
        }
        parent = cbi->getParent();
        return;
      }
    }
  }

  wrappedUse = inputUse;
}

/// Return the operand we wrap. Reconstructing branch operands.
Operand *UseWrapper::getOperand() {
  switch (type) {
  case kRegularUse:
    return wrappedUse;

  case kBranchUse: {
    auto *br = cast<BranchInst>(parent->getTerminator());
    assert(index < br->getNumArgs());
    return &br->getAllOperands()[index];
  }

  case kCondBranchUseTrue:
  case kCondBranchUseFalse: {
    auto *cbi = cast<CondBranchInst>(parent->getTerminator());
    auto indexToUse = [&]() -> unsigned {
      if (type == kCondBranchUseTrue)
        return index;
      return cbi->getTrueArgs().size() + 1 + index;
    }();
    assert(indexToUse < cbi->getAllOperands().size());
    return &cbi->getAllOperands()[indexToUse];
  }
  }

  toolchain_unreachable("uninitialize use type");
}

namespace language::test {
// Arguments:
// * the first arguments are values, which are added as "available values" to the SSA-updater
// * the next arguments are operands, which are set to the computed values at that place
static FunctionTest SSAUpdaterTest("update_ssa", [](auto &function,
                                                    auto &arguments,
                                                    auto &test) {
  SILSSAUpdater updater;
  SILValue firstVal = arguments.takeValue();
  updater.initialize(&function, firstVal->getType(), firstVal->getOwnershipKind());
  updater.addAvailableValue(firstVal->getParentBlock(), firstVal);

  while (arguments.hasUntaken()) {
    auto &arg = arguments.takeArgument();
    if (isa<ValueArgument>(arg)) {
      SILValue val = cast<ValueArgument>(arg).getValue();
      updater.addAvailableValue(val->getParentBlock(), val);
    } else if (isa<OperandArgument>(arg)) {
      Operand *op = cast<OperandArgument>(arg).getValue();
      SILValue newValue = updater.getValueInMiddleOfBlock(op->getUser()->getParent());
      op->set(newValue);
    }
  }
});
} // end namespace language::test

