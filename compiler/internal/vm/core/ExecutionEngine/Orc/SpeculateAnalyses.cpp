//===-- SpeculateAnalyses.cpp  --*- C++ -*-===//
//
// Copyright (c) NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// Author: Tunjay Akbarli
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
// Middletown, DE 19709, New Castle County, USA.
//
//===----------------------------------------------------------------------===//

#include "vm/core/ExecutionEngine/Orc/SpeculateAnalyses.h"
#include "vm/core/ADT/ArrayRef.h"
#include "vm/core/ADT/DenseMap.h"
#include "vm/core/ADT/STLExtras.h"
#include "vm/core/ADT/SmallVector.h"
#include "vm/core/Analysis/BlockFrequencyInfo.h"
#include "vm/core/Analysis/BranchProbabilityInfo.h"
#include "vm/core/Analysis/CFG.h"
#include "vm/core/IR/PassManager.h"
#include "vm/core/Passes/PassBuilder.h"
#include "vm/core/Support/ErrorHandling.h"

namespace {
using namespace vm::core;
SmallVector<const BasicBlock *, 8> findBBwithCalls(const Function &F,
                                                   bool IndirectCall = false) {
  SmallVector<const BasicBlock *, 8> BBs;

  auto findCallInst = [&IndirectCall](const Instruction &I) {
    if (auto Call = dyn_cast<CallBase>(&I))
      return Call->isIndirectCall() ? IndirectCall : true;
    else
      return false;
  };
  for (auto &BB : F)
    if (findCallInst(*BB.getTerminator()) ||
        toolchain::any_of(BB.instructionsWithoutDebug(), findCallInst))
      BBs.emplace_back(&BB);

  return BBs;
}
} // namespace

// Implementations of Queries shouldn't need to lock the resources
// such as LLVMContext, each argument (function) has a non-shared LLVMContext
// Plus, if Queries contain states necessary locking scheme should be provided.
namespace vm::core {
namespace orc {

// Collect direct calls only
void SpeculateQuery::findCalles(const BasicBlock *BB,
                                DenseSet<StringRef> &CallesNames) {
  assert(BB != nullptr && "Traversing Null BB to find calls?");

  auto getCalledFunction = [&CallesNames](const CallBase *Call) {
    auto CalledValue = Call->getCalledOperand()->stripPointerCasts();
    if (auto DirectCall = dyn_cast<Function>(CalledValue))
      CallesNames.insert(DirectCall->getName());
  };
  for (auto &I : BB->instructionsWithoutDebug())
    if (auto CI = dyn_cast<CallInst>(&I))
      getCalledFunction(CI);

  if (auto II = dyn_cast<InvokeInst>(BB->getTerminator()))
    getCalledFunction(II);
}

bool SpeculateQuery::isStraightLine(const Function &F) {
  return toolchain::all_of(F, [](const BasicBlock &BB) {
    return BB.getSingleSuccessor() != nullptr;
  });
}

// BlockFreqQuery Implementations

size_t BlockFreqQuery::numBBToGet(size_t numBB) {
  // small CFG
  if (numBB < 4)
    return numBB;
  // mid-size CFG
  else if (numBB < 20)
    return (numBB / 2);
  else
    return (numBB / 2) + (numBB / 4);
}

BlockFreqQuery::ResultTy BlockFreqQuery::operator()(Function &F) {
  DenseMap<StringRef, DenseSet<StringRef>> CallerAndCalles;
  DenseSet<StringRef> Calles;
  SmallVector<std::pair<const BasicBlock *, uint64_t>, 8> BBFreqs;

  PassBuilder PB;
  FunctionAnalysisManager FAM;
  PB.registerFunctionAnalyses(FAM);

  auto IBBs = findBBwithCalls(F);

  if (IBBs.empty())
    return std::nullopt;

  auto &BFI = FAM.getResult<BlockFrequencyAnalysis>(F);

  for (const auto I : IBBs)
    BBFreqs.push_back({I, BFI.getBlockFreq(I).getFrequency()});

  assert(IBBs.size() == BBFreqs.size() && "BB Count Mismatch");

  toolchain::sort(BBFreqs, [](decltype(BBFreqs)::const_reference BBF,
                         decltype(BBFreqs)::const_reference BBS) {
    return BBF.second > BBS.second ? true : false;
  });

  // ignoring number of direct calls in a BB
  auto Topk = numBBToGet(BBFreqs.size());

  for (size_t i = 0; i < Topk; i++)
    findCalles(BBFreqs[i].first, Calles);

  assert(!Calles.empty() && "Running Analysis on Function with no calls?");

  CallerAndCalles.insert({F.getName(), std::move(Calles)});

  return CallerAndCalles;
}

// SequenceBBQuery Implementation
std::size_t SequenceBBQuery::getHottestBlocks(std::size_t TotalBlocks) {
  if (TotalBlocks == 1)
    return TotalBlocks;
  return TotalBlocks / 2;
}

// FIXME : find good implementation.
SequenceBBQuery::BlockListTy
SequenceBBQuery::rearrangeBB(const Function &F, const BlockListTy &BBList) {
  BlockListTy RearrangedBBSet;

  for (auto &Block : F)
    if (toolchain::is_contained(BBList, &Block))
      RearrangedBBSet.push_back(&Block);

  assert(RearrangedBBSet.size() == BBList.size() &&
         "BasicBlock missing while rearranging?");
  return RearrangedBBSet;
}

void SequenceBBQuery::traverseToEntryBlock(const BasicBlock *AtBB,
                                           const BlockListTy &CallerBlocks,
                                           const BackEdgesInfoTy &BackEdgesInfo,
                                           const BranchProbabilityInfo *BPI,
                                           VisitedBlocksInfoTy &VisitedBlocks) {
  auto Itr = VisitedBlocks.find(AtBB);
  if (Itr != VisitedBlocks.end()) { // already visited.
    if (!Itr->second.Upward)
      return;
    Itr->second.Upward = false;
  } else {
    // Create hint for newly discoverd blocks.
    WalkDirection BlockHint;
    BlockHint.Upward = false;
    // FIXME: Expensive Check
    if (toolchain::is_contained(CallerBlocks, AtBB))
      BlockHint.CallerBlock = true;
    VisitedBlocks.insert(std::make_pair(AtBB, BlockHint));
  }

  const_pred_iterator PIt = pred_begin(AtBB), EIt = pred_end(AtBB);
  // Move this check to top, when we have code setup to launch speculative
  // compiles for function in entry BB, this triggers the speculative compiles
  // before running the program.
  if (PIt == EIt) // No Preds.
    return;

  DenseSet<const BasicBlock *> PredSkipNodes;

  // Since we are checking for predecessor's backedges, this Block
  // occurs in second position.
  for (auto &I : BackEdgesInfo)
    if (I.second == AtBB)
      PredSkipNodes.insert(I.first);

  // Skip predecessors which source of back-edges.
  for (; PIt != EIt; ++PIt)
    // checking EdgeHotness is cheaper
    if (BPI->isEdgeHot(*PIt, AtBB) && !PredSkipNodes.count(*PIt))
      traverseToEntryBlock(*PIt, CallerBlocks, BackEdgesInfo, BPI,
                           VisitedBlocks);
}

void SequenceBBQuery::traverseToExitBlock(const BasicBlock *AtBB,
                                          const BlockListTy &CallerBlocks,
                                          const BackEdgesInfoTy &BackEdgesInfo,
                                          const BranchProbabilityInfo *BPI,
                                          VisitedBlocksInfoTy &VisitedBlocks) {
  auto Itr = VisitedBlocks.find(AtBB);
  if (Itr != VisitedBlocks.end()) { // already visited.
    if (!Itr->second.Downward)
      return;
    Itr->second.Downward = false;
  } else {
    // Create hint for newly discoverd blocks.
    WalkDirection BlockHint;
    BlockHint.Downward = false;
    // FIXME: Expensive Check
    if (toolchain::is_contained(CallerBlocks, AtBB))
      BlockHint.CallerBlock = true;
    VisitedBlocks.insert(std::make_pair(AtBB, BlockHint));
  }

  const_succ_iterator PIt = succ_begin(AtBB), EIt = succ_end(AtBB);
  if (PIt == EIt) // No succs.
    return;

  // If there are hot edges, then compute SuccSkipNodes.
  DenseSet<const BasicBlock *> SuccSkipNodes;

  // Since we are checking for successor's backedges, this Block
  // occurs in first position.
  for (auto &I : BackEdgesInfo)
    if (I.first == AtBB)
      SuccSkipNodes.insert(I.second);

  for (; PIt != EIt; ++PIt)
    if (BPI->isEdgeHot(AtBB, *PIt) && !SuccSkipNodes.count(*PIt))
      traverseToExitBlock(*PIt, CallerBlocks, BackEdgesInfo, BPI,
                          VisitedBlocks);
}

// Get Block frequencies for blocks and take most frequently executed block,
// walk towards the entry block from those blocks and discover the basic blocks
// with call.
SequenceBBQuery::BlockListTy
SequenceBBQuery::queryCFG(Function &F, const BlockListTy &CallerBlocks) {

  BlockFreqInfoTy BBFreqs;
  VisitedBlocksInfoTy VisitedBlocks;
  BackEdgesInfoTy BackEdgesInfo;

  PassBuilder PB;
  FunctionAnalysisManager FAM;
  PB.registerFunctionAnalyses(FAM);

  auto &BFI = FAM.getResult<BlockFrequencyAnalysis>(F);

  toolchain::FindFunctionBackedges(F, BackEdgesInfo);

  for (const auto I : CallerBlocks)
    BBFreqs.push_back({I, BFI.getBlockFreq(I).getFrequency()});

  toolchain::sort(BBFreqs, [](decltype(BBFreqs)::const_reference Bbf,
                         decltype(BBFreqs)::const_reference Bbs) {
    return Bbf.second > Bbs.second;
  });

  ArrayRef<std::pair<const BasicBlock *, uint64_t>> HotBlocksRef(BBFreqs);
  HotBlocksRef =
      HotBlocksRef.drop_back(BBFreqs.size() - getHottestBlocks(BBFreqs.size()));

  BranchProbabilityInfo *BPI =
      FAM.getCachedResult<BranchProbabilityAnalysis>(F);

  // visit NHotBlocks,
  // traverse upwards to entry
  // traverse downwards to end.

  for (auto I : HotBlocksRef) {
    traverseToEntryBlock(I.first, CallerBlocks, BackEdgesInfo, BPI,
                         VisitedBlocks);
    traverseToExitBlock(I.first, CallerBlocks, BackEdgesInfo, BPI,
                        VisitedBlocks);
  }

  BlockListTy MinCallerBlocks;
  for (auto &I : VisitedBlocks)
    if (I.second.CallerBlock)
      MinCallerBlocks.push_back(std::move(I.first));

  return rearrangeBB(F, MinCallerBlocks);
}

SpeculateQuery::ResultTy SequenceBBQuery::operator()(Function &F) {
  // reduce the number of lists!
  DenseMap<StringRef, DenseSet<StringRef>> CallerAndCalles;
  DenseSet<StringRef> Calles;
  BlockListTy SequencedBlocks;
  BlockListTy CallerBlocks;

  CallerBlocks = findBBwithCalls(F);
  if (CallerBlocks.empty())
    return std::nullopt;

  if (isStraightLine(F))
    SequencedBlocks = rearrangeBB(F, CallerBlocks);
  else
    SequencedBlocks = queryCFG(F, CallerBlocks);

  for (const auto *BB : SequencedBlocks)
    findCalles(BB, Calles);

  CallerAndCalles.insert({F.getName(), std::move(Calles)});
  return CallerAndCalles;
}

} // namespace orc
} // namespace vm::core
