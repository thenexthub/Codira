//===- StackLifetime.cpp - Alloca Lifetime Analysis -----------------------===//
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

#include "vm/core/Analysis/StackLifetime.h"
#include "vm/core/ADT/DepthFirstIterator.h"
#include "vm/core/ADT/STLExtras.h"
#include "vm/core/ADT/SmallVector.h"
#include "vm/core/ADT/StringExtras.h"
#include "vm/core/Analysis/ValueTracking.h"
#include "vm/core/Config/toolchain-config.h"
#include "vm/core/IR/AssemblyAnnotationWriter.h"
#include "vm/core/IR/BasicBlock.h"
#include "vm/core/IR/CFG.h"
#include "vm/core/IR/InstIterator.h"
#include "vm/core/IR/Instructions.h"
#include "vm/core/IR/IntrinsicInst.h"
#include "vm/core/IR/Value.h"
#include "vm/core/Support/Casting.h"
#include "vm/core/Support/Compiler.h"
#include "vm/core/Support/Debug.h"
#include "vm/core/Support/FormattedStream.h"
#include <algorithm>
#include <tuple>

using namespace vm::core;

#define DEBUG_TYPE "stack-lifetime"

const StackLifetime::LiveRange &
StackLifetime::getLiveRange(const AllocaInst *AI) const {
  const auto IT = AllocaNumbering.find(AI);
  assert(IT != AllocaNumbering.end());
  return LiveRanges[IT->second];
}

bool StackLifetime::isReachable(const Instruction *I) const {
  return BlockInstRange.contains(I->getParent());
}

bool StackLifetime::isAliveAfter(const AllocaInst *AI,
                                 const Instruction *I) const {
  const BasicBlock *BB = I->getParent();
  auto ItBB = BlockInstRange.find(BB);
  assert(ItBB != BlockInstRange.end() && "Unreachable is not expected");

  // Search the block for the first instruction following 'I'.
  auto It = std::upper_bound(Instructions.begin() + ItBB->getSecond().first + 1,
                             Instructions.begin() + ItBB->getSecond().second, I,
                             [](const Instruction *L, const Instruction *R) {
                               return L->comesBefore(R);
                             });
  --It;
  unsigned InstNum = It - Instructions.begin();
  return getLiveRange(AI).test(InstNum);
}

void StackLifetime::collectMarkers() {
  InterestingAllocas.resize(NumAllocas);
  DenseMap<const BasicBlock *, SmallDenseMap<const IntrinsicInst *, Marker>>
      BBMarkerSet;

  // Compute the set of start/end markers per basic block.
  for (const BasicBlock *BB : depth_first(&F)) {
    for (const Instruction &I : *BB) {
      const IntrinsicInst *II = dyn_cast<IntrinsicInst>(&I);
      if (!II || !II->isLifetimeStartOrEnd())
        continue;
      const AllocaInst *AI = dyn_cast<AllocaInst>(II->getArgOperand(0));
      if (!AI)
        continue;
      auto It = AllocaNumbering.find(AI);
      if (It == AllocaNumbering.end())
        continue;
      auto AllocaNo = It->second;
      bool IsStart = II->getIntrinsicID() == Intrinsic::lifetime_start;
      if (IsStart)
        InterestingAllocas.set(AllocaNo);
      BBMarkerSet[BB][II] = {AllocaNo, IsStart};
    }
  }

  // Compute instruction numbering. Only the following instructions are
  // considered:
  // * Basic block entries
  // * Lifetime markers
  // For each basic block, compute
  // * the list of markers in the instruction order
  // * the sets of allocas whose lifetime starts or ends in this BB
  LLVM_DEBUG(dbgs() << "Instructions:\n");
  for (const BasicBlock *BB : depth_first(&F)) {
    LLVM_DEBUG(dbgs() << "  " << Instructions.size() << ": BB " << BB->getName()
                      << "\n");
    auto BBStart = Instructions.size();
    Instructions.push_back(nullptr);

    BlockLifetimeInfo &BlockInfo =
        BlockLiveness.try_emplace(BB, NumAllocas).first->getSecond();

    auto &BlockMarkerSet = BBMarkerSet[BB];
    if (BlockMarkerSet.empty()) {
      BlockInstRange[BB] = std::make_pair(BBStart, Instructions.size());
      continue;
    }

    auto ProcessMarker = [&](const IntrinsicInst *I, const Marker &M) {
      LLVM_DEBUG(dbgs() << "  " << Instructions.size() << ":  "
                        << (M.IsStart ? "start " : "end   ") << M.AllocaNo
                        << ", " << *I << "\n");

      BBMarkers[BB].push_back({Instructions.size(), M});
      Instructions.push_back(I);

      if (M.IsStart) {
        BlockInfo.End.reset(M.AllocaNo);
        BlockInfo.Begin.set(M.AllocaNo);
      } else {
        BlockInfo.Begin.reset(M.AllocaNo);
        BlockInfo.End.set(M.AllocaNo);
      }
    };

    if (BlockMarkerSet.size() == 1) {
      ProcessMarker(BlockMarkerSet.begin()->getFirst(),
                    BlockMarkerSet.begin()->getSecond());
    } else {
      // Scan the BB to determine the marker order.
      for (const Instruction &I : *BB) {
        const IntrinsicInst *II = dyn_cast<IntrinsicInst>(&I);
        if (!II)
          continue;
        auto It = BlockMarkerSet.find(II);
        if (It == BlockMarkerSet.end())
          continue;
        ProcessMarker(II, It->getSecond());
      }
    }

    BlockInstRange[BB] = std::make_pair(BBStart, Instructions.size());
  }
}

void StackLifetime::calculateLocalLiveness() {
  bool Changed = true;

  // LiveIn, LiveOut and BitsIn have a different meaning deppends on type.
  // ::Maybe true bits represent "may be alive" allocas, ::Must true bits
  // represent "may be dead". After the loop we will convert ::Must bits from
  // "may be dead" to "must be alive".
  while (Changed) {
    // TODO: Consider switching to worklist instead of traversing entire graph.
    Changed = false;

    for (const BasicBlock *BB : depth_first(&F)) {
      BlockLifetimeInfo &BlockInfo = BlockLiveness.find(BB)->getSecond();

      // Compute BitsIn by unioning together the LiveOut sets of all preds.
      BitVector BitsIn;
      for (const auto *PredBB : predecessors(BB)) {
        LivenessMap::const_iterator I = BlockLiveness.find(PredBB);
        // If a predecessor is unreachable, ignore it.
        if (I == BlockLiveness.end())
          continue;
        BitsIn |= I->second.LiveOut;
      }

      // Everything is "may be dead" for entry without predecessors.
      if (Type == LivenessType::Must && BitsIn.empty())
        BitsIn.resize(NumAllocas, true);

      // Update block LiveIn set, noting whether it has changed.
      if (!BitsIn.subsetOf(BlockInfo.LiveIn)) {
        BlockInfo.LiveIn |= BitsIn;
      }

      // Compute LiveOut by subtracting out lifetimes that end in this
      // block, then adding in lifetimes that begin in this block.  If
      // we have both BEGIN and END markers in the same basic block
      // then we know that the BEGIN marker comes after the END,
      // because we already handle the case where the BEGIN comes
      // before the END when collecting the markers (and building the
      // BEGIN/END vectors).
      switch (Type) {
      case LivenessType::May:
        BitsIn.reset(BlockInfo.End);
        // "may be alive" is set by lifetime start.
        BitsIn |= BlockInfo.Begin;
        break;
      case LivenessType::Must:
        BitsIn.reset(BlockInfo.Begin);
        // "may be dead" is set by lifetime end.
        BitsIn |= BlockInfo.End;
        break;
      }

      // Update block LiveOut set, noting whether it has changed.
      if (!BitsIn.subsetOf(BlockInfo.LiveOut)) {
        Changed = true;
        BlockInfo.LiveOut |= BitsIn;
      }
    }
  } // while changed.

  if (Type == LivenessType::Must) {
    // Convert from "may be dead" to "must be alive".
    for (auto &[BB, BlockInfo] : BlockLiveness) {
      BlockInfo.LiveIn.flip();
      BlockInfo.LiveOut.flip();
    }
  }
}

void StackLifetime::calculateLiveIntervals() {
  for (auto IT : BlockLiveness) {
    const BasicBlock *BB = IT.getFirst();
    BlockLifetimeInfo &BlockInfo = IT.getSecond();
    unsigned BBStart, BBEnd;
    std::tie(BBStart, BBEnd) = BlockInstRange[BB];

    BitVector Started, Ended;
    Started.resize(NumAllocas);
    Ended.resize(NumAllocas);
    SmallVector<unsigned, 8> Start;
    Start.resize(NumAllocas);

    // LiveIn ranges start at the first instruction.
    for (unsigned AllocaNo = 0; AllocaNo < NumAllocas; ++AllocaNo) {
      if (BlockInfo.LiveIn.test(AllocaNo)) {
        Started.set(AllocaNo);
        Start[AllocaNo] = BBStart;
      }
    }

    for (auto &It : BBMarkers[BB]) {
      unsigned InstNo = It.first;
      bool IsStart = It.second.IsStart;
      unsigned AllocaNo = It.second.AllocaNo;

      if (IsStart) {
        if (!Started.test(AllocaNo)) {
          Started.set(AllocaNo);
          Ended.reset(AllocaNo);
          Start[AllocaNo] = InstNo;
        }
      } else {
        if (Started.test(AllocaNo)) {
          LiveRanges[AllocaNo].addRange(Start[AllocaNo], InstNo);
          Started.reset(AllocaNo);
        }
        Ended.set(AllocaNo);
      }
    }

    for (unsigned AllocaNo = 0; AllocaNo < NumAllocas; ++AllocaNo)
      if (Started.test(AllocaNo))
        LiveRanges[AllocaNo].addRange(Start[AllocaNo], BBEnd);
  }
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD void StackLifetime::dumpAllocas() const {
  dbgs() << "Allocas:\n";
  for (unsigned AllocaNo = 0; AllocaNo < NumAllocas; ++AllocaNo)
    dbgs() << "  " << AllocaNo << ": " << *Allocas[AllocaNo] << "\n";
}

LLVM_DUMP_METHOD void StackLifetime::dumpBlockLiveness() const {
  dbgs() << "Block liveness:\n";
  for (auto IT : BlockLiveness) {
    const BasicBlock *BB = IT.getFirst();
    const BlockLifetimeInfo &BlockInfo = BlockLiveness.find(BB)->getSecond();
    auto BlockRange = BlockInstRange.find(BB)->getSecond();
    dbgs() << "  BB (" << BB->getName() << ") [" << BlockRange.first << ", " << BlockRange.second
           << "): begin " << BlockInfo.Begin << ", end " << BlockInfo.End
           << ", livein " << BlockInfo.LiveIn << ", liveout "
           << BlockInfo.LiveOut << "\n";
  }
}

LLVM_DUMP_METHOD void StackLifetime::dumpLiveRanges() const {
  dbgs() << "Alloca liveness:\n";
  for (unsigned AllocaNo = 0; AllocaNo < NumAllocas; ++AllocaNo)
    dbgs() << "  " << AllocaNo << ": " << LiveRanges[AllocaNo] << "\n";
}
#endif

StackLifetime::StackLifetime(const Function &F,
                             ArrayRef<const AllocaInst *> Allocas,
                             LivenessType Type)
    : F(F), Type(Type), Allocas(Allocas), NumAllocas(Allocas.size()) {
  LLVM_DEBUG(dumpAllocas());

  for (unsigned I = 0; I < NumAllocas; ++I)
    AllocaNumbering[Allocas[I]] = I;

  collectMarkers();
}

void StackLifetime::run() {
  LiveRanges.resize(NumAllocas, LiveRange(Instructions.size()));
  for (unsigned I = 0; I < NumAllocas; ++I)
    if (!InterestingAllocas.test(I))
      LiveRanges[I] = getFullLiveRange();

  calculateLocalLiveness();
  LLVM_DEBUG(dumpBlockLiveness());
  calculateLiveIntervals();
  LLVM_DEBUG(dumpLiveRanges());
}

class StackLifetime::LifetimeAnnotationWriter
    : public AssemblyAnnotationWriter {
  const StackLifetime &SL;

  void printInstrAlive(unsigned InstrNo, formatted_raw_ostream &OS) {
    SmallVector<StringRef, 16> Names;
    for (const auto &KV : SL.AllocaNumbering) {
      if (SL.LiveRanges[KV.getSecond()].test(InstrNo))
        Names.push_back(KV.getFirst()->getName());
    }
    toolchain::sort(Names);
    OS << "  ; Alive: <" << toolchain::join(Names, " ") << ">\n";
  }

  void emitBasicBlockStartAnnot(const BasicBlock *BB,
                                formatted_raw_ostream &OS) override {
    auto ItBB = SL.BlockInstRange.find(BB);
    if (ItBB == SL.BlockInstRange.end())
      return; // Unreachable.
    printInstrAlive(ItBB->getSecond().first, OS);
  }

  void printInfoComment(const Value &V, formatted_raw_ostream &OS) override {
    const Instruction *Instr = dyn_cast<Instruction>(&V);
    if (!Instr || !SL.isReachable(Instr))
      return;

    SmallVector<StringRef, 16> Names;
    for (const auto &KV : SL.AllocaNumbering) {
      if (SL.isAliveAfter(KV.getFirst(), Instr))
        Names.push_back(KV.getFirst()->getName());
    }
    toolchain::sort(Names);
    OS << "\n  ; Alive: <" << toolchain::join(Names, " ") << ">\n";
  }

public:
  LifetimeAnnotationWriter(const StackLifetime &SL) : SL(SL) {}
};

void StackLifetime::print(raw_ostream &OS) {
  LifetimeAnnotationWriter AAW(*this);
  F.print(OS, &AAW);
}

PreservedAnalyses StackLifetimePrinterPass::run(Function &F,
                                                FunctionAnalysisManager &AM) {
  SmallVector<const AllocaInst *, 8> Allocas;
  for (auto &I : instructions(F))
    if (const AllocaInst *AI = dyn_cast<AllocaInst>(&I))
      Allocas.push_back(AI);
  StackLifetime SL(F, Allocas, Type);
  SL.run();
  SL.print(OS);
  return PreservedAnalyses::all();
}

void StackLifetimePrinterPass::printPipeline(
    raw_ostream &OS, function_ref<StringRef(StringRef)> MapClassName2PassName) {
  static_cast<PassInfoMixin<StackLifetimePrinterPass> *>(this)->printPipeline(
      OS, MapClassName2PassName);
  OS << '<';
  switch (Type) {
  case StackLifetime::LivenessType::May:
    OS << "may";
    break;
  case StackLifetime::LivenessType::Must:
    OS << "must";
    break;
  }
  OS << '>';
}
