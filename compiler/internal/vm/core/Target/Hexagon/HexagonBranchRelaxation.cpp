//===--- HexagonBranchRelaxation.cpp - Identify and relax long jumps ------===//
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

#include "Hexagon.h"
#include "HexagonInstrInfo.h"
#include "HexagonSubtarget.h"
#include "vm/core/ADT/DenseMap.h"
#include "vm/core/ADT/SmallVector.h"
#include "vm/core/ADT/StringRef.h"
#include "vm/core/CodeGen/MachineBasicBlock.h"
#include "vm/core/CodeGen/MachineFunction.h"
#include "vm/core/CodeGen/MachineFunctionPass.h"
#include "vm/core/CodeGen/MachineInstr.h"
#include "vm/core/CodeGen/MachineOperand.h"
#include "vm/core/CodeGen/Passes.h"
#include "vm/core/Pass.h"
#include "vm/core/Support/CommandLine.h"
#include "vm/core/Support/Debug.h"
#include "vm/core/Support/raw_ostream.h"
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <iterator>

#define DEBUG_TYPE "hexagon-brelax"

using namespace vm::core;

// Since we have no exact knowledge of code layout, allow some safety buffer
// for jump target. This is measured in bytes.
static cl::opt<uint32_t>
    BranchRelaxSafetyBuffer("branch-relax-safety-buffer", cl::init(200),
                            cl::Hidden, cl::desc("safety buffer size"));

namespace {

  struct HexagonBranchRelaxation : public MachineFunctionPass {
  public:
    static char ID;

    HexagonBranchRelaxation() : MachineFunctionPass(ID) {}

    bool runOnMachineFunction(MachineFunction &MF) override;

    StringRef getPassName() const override {
      return "Hexagon Branch Relaxation";
    }

    void getAnalysisUsage(AnalysisUsage &AU) const override {
      AU.setPreservesCFG();
      MachineFunctionPass::getAnalysisUsage(AU);
    }

  private:
    const HexagonInstrInfo *HII;
    const HexagonRegisterInfo *HRI;

    bool relaxBranches(MachineFunction &MF);
    void computeOffset(MachineFunction &MF,
          DenseMap<MachineBasicBlock*, unsigned> &BlockToInstOffset);
    bool reGenerateBranch(MachineFunction &MF,
          DenseMap<MachineBasicBlock*, unsigned> &BlockToInstOffset);
    bool isJumpOutOfRange(MachineInstr &MI,
          DenseMap<MachineBasicBlock*, unsigned> &BlockToInstOffset);
  };

  char HexagonBranchRelaxation::ID = 0;

} // end anonymous namespace

INITIALIZE_PASS(HexagonBranchRelaxation, "hexagon-brelax",
                "Hexagon Branch Relaxation", false, false)

FunctionPass *toolchain::createHexagonBranchRelaxation() {
  return new HexagonBranchRelaxation();
}

bool HexagonBranchRelaxation::runOnMachineFunction(MachineFunction &MF) {
  LLVM_DEBUG(dbgs() << "****** Hexagon Branch Relaxation ******\n");

  auto &HST = MF.getSubtarget<HexagonSubtarget>();
  HII = HST.getInstrInfo();
  HRI = HST.getRegisterInfo();

  bool Changed = false;
  Changed = relaxBranches(MF);
  return Changed;
}

void HexagonBranchRelaxation::computeOffset(MachineFunction &MF,
      DenseMap<MachineBasicBlock*, unsigned> &OffsetMap) {
  // offset of the current instruction from the start.
  unsigned InstOffset = 0;
  for (auto &B : MF) {
    if (B.getAlignment() != Align(1)) {
      // Although we don't know the exact layout of the final code, we need
      // to account for alignment padding somehow. This heuristic pads each
      // aligned basic block according to the alignment value.
      InstOffset = alignTo(InstOffset, B.getAlignment());
    }
    OffsetMap[&B] = InstOffset;
    for (auto &MI : B.instrs()) {
      InstOffset += HII->getSize(MI);
      // Assume that all extendable branches will be extended.
      if (MI.isBranch() && HII->isExtendable(MI))
        InstOffset += HEXAGON_INSTR_SIZE;
    }
  }
}

/// relaxBranches - For Hexagon, if the jump target/loop label is too far from
/// the jump/loop instruction then, we need to make sure that we have constant
/// extenders set for jumps and loops.

/// There are six iterations in this phase. It's self explanatory below.
bool HexagonBranchRelaxation::relaxBranches(MachineFunction &MF) {
  // Compute the offset of each basic block
  // offset of the current instruction from the start.
  // map for each instruction to the beginning of the function
  DenseMap<MachineBasicBlock*, unsigned> BlockToInstOffset;
  computeOffset(MF, BlockToInstOffset);

  return reGenerateBranch(MF, BlockToInstOffset);
}

/// Check if a given instruction is:
/// - a jump to a distant target
/// - that exceeds its immediate range
/// If both conditions are true, it requires constant extension.
bool HexagonBranchRelaxation::isJumpOutOfRange(MachineInstr &MI,
      DenseMap<MachineBasicBlock*, unsigned> &BlockToInstOffset) {
  MachineBasicBlock &B = *MI.getParent();
  auto FirstTerm = B.getFirstInstrTerminator();
  if (FirstTerm == B.instr_end())
    return false;

  if (HII->isExtended(MI))
    return false;

  unsigned InstOffset = BlockToInstOffset[&B];
  unsigned Distance = 0;

  // To save time, estimate exact position of a branch instruction
  // as one at the end of the MBB.
  // Number of instructions times typical instruction size.
  InstOffset += HII->nonDbgBBSize(&B) * HEXAGON_INSTR_SIZE;

  MachineBasicBlock *TBB = nullptr, *FBB = nullptr;
  SmallVector<MachineOperand, 4> Cond;

  // Try to analyze this branch.
  if (HII->analyzeBranch(B, TBB, FBB, Cond, false)) {
    // Could not analyze it. See if this is something we can recognize.
    // If it is a NVJ, it should always have its target in
    // a fixed location.
    if (HII->isNewValueJump(*FirstTerm))
      TBB = FirstTerm->getOperand(HII->getCExtOpNum(*FirstTerm)).getMBB();
  }
  if (TBB && &MI == &*FirstTerm) {
    Distance = std::abs((long long)InstOffset - BlockToInstOffset[TBB])
                + BranchRelaxSafetyBuffer;
    return !HII->isJumpWithinBranchRange(*FirstTerm, Distance);
  }
  if (FBB) {
    // Look for second terminator.
    auto SecondTerm = std::next(FirstTerm);
    assert(SecondTerm != B.instr_end() &&
          (SecondTerm->isBranch() || SecondTerm->isCall()) &&
          "Bad second terminator");
    if (&MI != &*SecondTerm)
      return false;
    // Analyze the second branch in the BB.
    Distance = std::abs((long long)InstOffset - BlockToInstOffset[FBB])
                + BranchRelaxSafetyBuffer;
    return !HII->isJumpWithinBranchRange(*SecondTerm, Distance);
  }
  return false;
}

bool HexagonBranchRelaxation::reGenerateBranch(MachineFunction &MF,
      DenseMap<MachineBasicBlock*, unsigned> &BlockToInstOffset) {
  bool Changed = false;

  for (auto &B : MF) {
    for (auto &MI : B) {
      if (!MI.isBranch() || !isJumpOutOfRange(MI, BlockToInstOffset))
        continue;
      LLVM_DEBUG(dbgs() << "Long distance jump. isExtendable("
                        << HII->isExtendable(MI) << ") isConstExtended("
                        << HII->isConstExtended(MI) << ") " << MI);

      // Since we have not merged HW loops relaxation into
      // this code (yet), soften our approach for the moment.
      if (!HII->isExtendable(MI) && !HII->isExtended(MI)) {
        LLVM_DEBUG(dbgs() << "\tUnderimplemented relax branch instruction.\n");
      } else {
        // Find which operand is expandable.
        int ExtOpNum = HII->getCExtOpNum(MI);
        MachineOperand &MO = MI.getOperand(ExtOpNum);
        // This need to be something we understand. So far we assume all
        // branches have only MBB address as expandable field.
        // If it changes, this will need to be expanded.
        assert(MO.isMBB() && "Branch with unknown expandable field type");
        // Mark given operand as extended.
        MO.addTargetFlag(HexagonII::HMOTF_ConstExtended);
        Changed = true;
      }
    }
  }
  return Changed;
}
