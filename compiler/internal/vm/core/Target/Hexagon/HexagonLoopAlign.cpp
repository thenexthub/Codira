//===----- HexagonLoopAlign.cpp - Generate loop alignment directives  -----===//
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
// Inspect a basic block and if its single basic block loop with a small
// number of instructions, set the prefLoopAlignment to 32 bytes (5).
//===----------------------------------------------------------------------===//

#include "Hexagon.h"
#include "HexagonTargetMachine.h"
#include "vm/core/CodeGen/MachineBlockFrequencyInfo.h"
#include "vm/core/CodeGen/MachineBranchProbabilityInfo.h"
#include "vm/core/Support/Debug.h"

#define DEBUG_TYPE "hexagon-loop-align"

using namespace vm::core;

static cl::opt<bool>
    DisableLoopAlign("disable-hexagon-loop-align", cl::Hidden,
                     cl::desc("Disable Hexagon loop alignment pass"));

static cl::opt<uint32_t> HVXLoopAlignLimitUB(
    "hexagon-hvx-loop-align-limit-ub", cl::Hidden, cl::init(16),
    cl::desc("Set hexagon hvx loop upper bound align limit"));

static cl::opt<uint32_t> TinyLoopAlignLimitUB(
    "hexagon-tiny-loop-align-limit-ub", cl::Hidden, cl::init(16),
    cl::desc("Set hexagon tiny-core loop upper bound align limit"));

static cl::opt<uint32_t>
    LoopAlignLimitUB("hexagon-loop-align-limit-ub", cl::Hidden, cl::init(8),
                     cl::desc("Set hexagon loop upper bound align limit"));

static cl::opt<uint32_t>
    LoopAlignLimitLB("hexagon-loop-align-limit-lb", cl::Hidden, cl::init(4),
                     cl::desc("Set hexagon loop lower bound align limit"));

static cl::opt<uint32_t>
    LoopBndlAlignLimit("hexagon-loop-bundle-align-limit", cl::Hidden,
                       cl::init(4),
                       cl::desc("Set hexagon loop align bundle limit"));

static cl::opt<uint32_t> TinyLoopBndlAlignLimit(
    "hexagon-tiny-loop-bundle-align-limit", cl::Hidden, cl::init(8),
    cl::desc("Set hexagon tiny-core loop align bundle limit"));

static cl::opt<uint32_t>
    LoopEdgeThreshold("hexagon-loop-edge-threshold", cl::Hidden, cl::init(7500),
                      cl::desc("Set hexagon loop align edge threshold"));

namespace {

class HexagonLoopAlign : public MachineFunctionPass {
  const HexagonSubtarget *HST = nullptr;
  const TargetMachine *HTM = nullptr;
  const HexagonInstrInfo *HII = nullptr;

public:
  static char ID;
  HexagonLoopAlign() : MachineFunctionPass(ID) {}
  bool shouldBalignLoop(MachineBasicBlock &BB, bool AboveThres);
  bool isSingleLoop(MachineBasicBlock &MBB);
  bool attemptToBalignSmallLoop(MachineFunction &MF, MachineBasicBlock &MBB);

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<MachineBranchProbabilityInfoWrapperPass>();
    AU.addRequired<MachineBlockFrequencyInfoWrapperPass>();
    MachineFunctionPass::getAnalysisUsage(AU);
  }

  StringRef getPassName() const override { return "Hexagon LoopAlign pass"; }
  bool runOnMachineFunction(MachineFunction &MF) override;
};

char HexagonLoopAlign::ID = 0;

bool HexagonLoopAlign::shouldBalignLoop(MachineBasicBlock &BB,
                                        bool AboveThres) {
  bool isVec = false;
  unsigned InstCnt = 0;
  unsigned BndlCnt = 0;

  for (MachineBasicBlock::instr_iterator II = BB.instr_begin(),
                                         IE = BB.instr_end();
       II != IE; ++II) {

    // End if the instruction is endloop.
    if (HII->isEndLoopN(II->getOpcode()))
      break;
    // Count the number of bundles.
    if (II->isBundle()) {
      BndlCnt++;
      continue;
    }
    // Skip over debug instructions.
    if (II->isDebugInstr())
      continue;
    // Check if there are any HVX instructions in loop.
    isVec |= HII->isHVXVec(*II);
    // Count the number of instructions.
    InstCnt++;
  }

  LLVM_DEBUG({
    dbgs() << "Bundle Count : " << BndlCnt << "\n";
    dbgs() << "Instruction Count : " << InstCnt << "\n";
  });

  unsigned LimitUB = 0;
  unsigned LimitBndl = LoopBndlAlignLimit;
  // The conditions in the order of priority.
  if (HST->isTinyCore()) {
    LimitUB = TinyLoopAlignLimitUB;
    LimitBndl = TinyLoopBndlAlignLimit;
  } else if (isVec)
    LimitUB = HVXLoopAlignLimitUB;
  else if (AboveThres)
    LimitUB = LoopAlignLimitUB;

  // if the upper bound is not set to a value, implies we didn't meet
  // the criteria.
  if (LimitUB == 0)
    return false;

  return InstCnt >= LoopAlignLimitLB && InstCnt <= LimitUB &&
         BndlCnt <= LimitBndl;
}

bool HexagonLoopAlign::isSingleLoop(MachineBasicBlock &MBB) {
  int Succs = MBB.succ_size();
  return (MBB.isSuccessor(&MBB) && (Succs == 2));
}

bool HexagonLoopAlign::attemptToBalignSmallLoop(MachineFunction &MF,
                                                MachineBasicBlock &MBB) {
  if (!isSingleLoop(MBB))
    return false;

  const MachineBranchProbabilityInfo *MBPI =
      &getAnalysis<MachineBranchProbabilityInfoWrapperPass>().getMBPI();
  const MachineBlockFrequencyInfo *MBFI =
      &getAnalysis<MachineBlockFrequencyInfoWrapperPass>().getMBFI();

  // Compute frequency of back edge,
  BlockFrequency BlockFreq = MBFI->getBlockFreq(&MBB);
  BranchProbability BrProb = MBPI->getEdgeProbability(&MBB, &MBB);
  BlockFrequency EdgeFreq = BlockFreq * BrProb;
  LLVM_DEBUG({
    dbgs() << "Loop Align Pass:\n";
    dbgs() << "\tedge with freq(" << EdgeFreq.getFrequency() << ")\n";
  });

  bool AboveThres = EdgeFreq.getFrequency() > LoopEdgeThreshold;
  if (shouldBalignLoop(MBB, AboveThres)) {
    // We found a loop, change its alignment to be 32 (5).
    MBB.setAlignment(toolchain::Align(1 << 5));
    return true;
  }
  return false;
}

// Inspect each basic block, and if its a single BB loop, see if it
// meets the criteria for increasing alignment to 32.

bool HexagonLoopAlign::runOnMachineFunction(MachineFunction &MF) {

  HST = &MF.getSubtarget<HexagonSubtarget>();
  HII = HST->getInstrInfo();
  HTM = &MF.getTarget();

  if (skipFunction(MF.getFunction()))
    return false;
  if (DisableLoopAlign)
    return false;

  // This optimization is performed at
  // i) -O2 and above, and  when the loop has a HVX instruction.
  // ii) -O3
  if (HST->useHVXOps()) {
    if (HTM->getOptLevel() < CodeGenOptLevel::Default)
      return false;
  } else {
    if (HTM->getOptLevel() < CodeGenOptLevel::Aggressive)
      return false;
  }

  bool Changed = false;
  for (MachineFunction::iterator MBBi = MF.begin(), MBBe = MF.end();
       MBBi != MBBe; ++MBBi) {
    MachineBasicBlock &MBB = *MBBi;
    Changed |= attemptToBalignSmallLoop(MF, MBB);
  }
  return Changed;
}

} // namespace

INITIALIZE_PASS(HexagonLoopAlign, "hexagon-loop-align",
                "Hexagon LoopAlign pass", false, false)

//===----------------------------------------------------------------------===//
//                         Public Constructor Functions
//===----------------------------------------------------------------------===//

FunctionPass *toolchain::createHexagonLoopAlign() { return new HexagonLoopAlign(); }
