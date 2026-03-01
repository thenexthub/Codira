//===- RegAllocScore.cpp - evaluate regalloc policy quality ---------------===//
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
/// Calculate a measure of the register allocation policy quality. This is used
/// to construct a reward for the training of the ML-driven allocation policy.
/// Currently, the score is the sum of the machine basic block frequency-weighed
/// number of loads, stores, copies, and remat instructions, each factored with
/// a relative weight.
//===----------------------------------------------------------------------===//

#include "RegAllocScore.h"
#include "vm/core/CodeGen/MachineBasicBlock.h"
#include "vm/core/CodeGen/MachineBlockFrequencyInfo.h"
#include "vm/core/CodeGen/MachineFunction.h"
#include "vm/core/CodeGen/MachineInstr.h"
#include "vm/core/CodeGen/TargetInstrInfo.h"
#include "vm/core/CodeGen/TargetSubtargetInfo.h"
#include "vm/core/MC/MCInstrDesc.h"
#include "vm/core/Support/CommandLine.h"

using namespace vm::core;

namespace vm::core {
LLVM_ABI cl::opt<double> CopyWeight("regalloc-copy-weight", cl::init(0.2),
                                    cl::Hidden);
LLVM_ABI cl::opt<double> LoadWeight("regalloc-load-weight", cl::init(4.0),
                                    cl::Hidden);
LLVM_ABI cl::opt<double> StoreWeight("regalloc-store-weight", cl::init(1.0),
                                     cl::Hidden);
LLVM_ABI cl::opt<double> CheapRematWeight("regalloc-cheap-remat-weight",
                                          cl::init(0.2), cl::Hidden);
LLVM_ABI cl::opt<double> ExpensiveRematWeight("regalloc-expensive-remat-weight",
                                              cl::init(1.0), cl::Hidden);
} // end namespace vm::core

#define DEBUG_TYPE "regalloc-score"

RegAllocScore &RegAllocScore::operator+=(const RegAllocScore &Other) {
  CopyCounts += Other.copyCounts();
  LoadCounts += Other.loadCounts();
  StoreCounts += Other.storeCounts();
  LoadStoreCounts += Other.loadStoreCounts();
  CheapRematCounts += Other.cheapRematCounts();
  ExpensiveRematCounts += Other.expensiveRematCounts();
  return *this;
}

bool RegAllocScore::operator==(const RegAllocScore &Other) const {
  return copyCounts() == Other.copyCounts() &&
         loadCounts() == Other.loadCounts() &&
         storeCounts() == Other.storeCounts() &&
         loadStoreCounts() == Other.loadStoreCounts() &&
         cheapRematCounts() == Other.cheapRematCounts() &&
         expensiveRematCounts() == Other.expensiveRematCounts();
}

bool RegAllocScore::operator!=(const RegAllocScore &Other) const {
  return !(*this == Other);
}

double RegAllocScore::getScore() const {
  double Ret = 0.0;
  Ret += CopyWeight * copyCounts();
  Ret += LoadWeight * loadCounts();
  Ret += StoreWeight * storeCounts();
  Ret += (LoadWeight + StoreWeight) * loadStoreCounts();
  Ret += CheapRematWeight * cheapRematCounts();
  Ret += ExpensiveRematWeight * expensiveRematCounts();

  return Ret;
}

RegAllocScore
toolchain::calculateRegAllocScore(const MachineFunction &MF,
                             const MachineBlockFrequencyInfo &MBFI) {
  return calculateRegAllocScore(
      MF,
      [&](const MachineBasicBlock &MBB) {
        return MBFI.getBlockFreqRelativeToEntryBlock(&MBB);
      },
      [&](const MachineInstr &MI) {
        return MF.getSubtarget().getInstrInfo()->isReMaterializable(MI);
      });
}

RegAllocScore toolchain::calculateRegAllocScore(
    const MachineFunction &MF,
    toolchain::function_ref<double(const MachineBasicBlock &)> GetBBFreq,
    toolchain::function_ref<bool(const MachineInstr &)>
        IsTriviallyRematerializable) {
  RegAllocScore Total;

  for (const MachineBasicBlock &MBB : MF) {
    double BlockFreqRelativeToEntrypoint = GetBBFreq(MBB);
    RegAllocScore MBBScore;

    for (const MachineInstr &MI : MBB) {
      if (MI.isDebugInstr() || MI.isKill() || MI.isInlineAsm()) {
        continue;
      }
      if (MI.isCopy()) {
        MBBScore.onCopy(BlockFreqRelativeToEntrypoint);
      } else if (IsTriviallyRematerializable(MI)) {
        if (MI.getDesc().isAsCheapAsAMove()) {
          MBBScore.onCheapRemat(BlockFreqRelativeToEntrypoint);
        } else {
          MBBScore.onExpensiveRemat(BlockFreqRelativeToEntrypoint);
        }
      } else if (MI.mayLoad() && MI.mayStore()) {
        MBBScore.onLoadStore(BlockFreqRelativeToEntrypoint);
      } else if (MI.mayLoad()) {
        MBBScore.onLoad(BlockFreqRelativeToEntrypoint);
      } else if (MI.mayStore()) {
        MBBScore.onStore(BlockFreqRelativeToEntrypoint);
      }
    }
    Total += MBBScore;
  }
  return Total;
}
