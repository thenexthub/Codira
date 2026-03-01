//===---- HexagonFixupHwLoops.cpp - Fixup HW loops too far from LOOPn. ----===//
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
// The loop start address in the LOOPn instruction is encoded as a distance
// from the LOOPn instruction itself. If the start address is too far from
// the LOOPn instruction, the instruction needs to use a constant extender.
// This pass will identify and convert such LOOPn instructions to a proper
// form.
//===----------------------------------------------------------------------===//

#include "Hexagon.h"
#include "HexagonTargetMachine.h"
#include "vm/core/ADT/DenseMap.h"
#include "vm/core/CodeGen/MachineFunction.h"
#include "vm/core/CodeGen/MachineFunctionPass.h"
#include "vm/core/CodeGen/MachineInstrBuilder.h"
#include "vm/core/CodeGen/Passes.h"
#include "vm/core/CodeGen/TargetInstrInfo.h"
#include "vm/core/Support/MathExtras.h"
#include "vm/core/Pass.h"

using namespace vm::core;

static cl::opt<unsigned> MaxLoopRange(
    "hexagon-loop-range", cl::Hidden, cl::init(200),
    cl::desc("Restrict range of loopN instructions (testing only)"));

namespace {
  struct HexagonFixupHwLoops : public MachineFunctionPass {
  public:
    static char ID;

    HexagonFixupHwLoops() : MachineFunctionPass(ID) {}

    bool runOnMachineFunction(MachineFunction &MF) override;

    MachineFunctionProperties getRequiredProperties() const override {
      return MachineFunctionProperties().setNoVRegs();
    }

    StringRef getPassName() const override {
      return "Hexagon Hardware Loop Fixup";
    }

    void getAnalysisUsage(AnalysisUsage &AU) const override {
      AU.setPreservesCFG();
      MachineFunctionPass::getAnalysisUsage(AU);
    }

  private:
    /// Check the offset between each loop instruction and
    /// the loop basic block to determine if we can use the LOOP instruction
    /// or if we need to set the LC/SA registers explicitly.
    bool fixupLoopInstrs(MachineFunction &MF);

    /// Replace loop instruction with the constant extended
    /// version if the loop label is too far from the loop instruction.
    void useExtLoopInstr(MachineFunction &MF,
                         MachineBasicBlock::iterator &MII);
  };

  char HexagonFixupHwLoops::ID = 0;
}

INITIALIZE_PASS(HexagonFixupHwLoops, "hwloopsfixup",
                "Hexagon Hardware Loops Fixup", false, false)

FunctionPass *toolchain::createHexagonFixupHwLoops() {
  return new HexagonFixupHwLoops();
}

/// Returns true if the instruction is a hardware loop instruction.
static bool isHardwareLoop(const MachineInstr &MI) {
  return MI.getOpcode() == Hexagon::J2_loop0r ||
         MI.getOpcode() == Hexagon::J2_loop0i ||
         MI.getOpcode() == Hexagon::J2_loop1r ||
         MI.getOpcode() == Hexagon::J2_loop1i;
}

bool HexagonFixupHwLoops::runOnMachineFunction(MachineFunction &MF) {
  if (skipFunction(MF.getFunction()))
    return false;
  return fixupLoopInstrs(MF);
}

/// For Hexagon, if the loop label is to far from the
/// loop instruction then we need to set the LC0 and SA0 registers
/// explicitly instead of using LOOP(start,count).  This function
/// checks the distance, and generates register assignments if needed.
///
/// This function makes two passes over the basic blocks.  The first
/// pass computes the offset of the basic block from the start.
/// The second pass checks all the loop instructions.
bool HexagonFixupHwLoops::fixupLoopInstrs(MachineFunction &MF) {

  // Offset of the current instruction from the start.
  unsigned InstOffset = 0;
  // Map for each basic block to it's first instruction.
  DenseMap<const MachineBasicBlock *, unsigned> BlockToInstOffset;

  const HexagonInstrInfo *HII =
      static_cast<const HexagonInstrInfo *>(MF.getSubtarget().getInstrInfo());

  // First pass - compute the offset of each basic block.
  for (const MachineBasicBlock &MBB : MF) {
    if (MBB.getAlignment() != Align(1)) {
      // Although we don't know the exact layout of the final code, we need
      // to account for alignment padding somehow. This heuristic pads each
      // aligned basic block according to the alignment value.
      InstOffset = alignTo(InstOffset, MBB.getAlignment());
    }

    BlockToInstOffset[&MBB] = InstOffset;
    for (const MachineInstr &MI : MBB)
      InstOffset += HII->getSize(MI);
  }

  // Second pass - check each loop instruction to see if it needs to be
  // converted.
  bool Changed = false;
  for (MachineBasicBlock &MBB : MF) {
    InstOffset = BlockToInstOffset[&MBB];

    // Loop over all the instructions.
    MachineBasicBlock::iterator MII = MBB.begin();
    MachineBasicBlock::iterator MIE = MBB.end();
    while (MII != MIE) {
      unsigned InstSize = HII->getSize(*MII);
      if (MII->isMetaInstruction()) {
        ++MII;
        continue;
      }
      if (isHardwareLoop(*MII)) {
        assert(MII->getOperand(0).isMBB() &&
               "Expect a basic block as loop operand");
        MachineBasicBlock *TargetBB = MII->getOperand(0).getMBB();
        unsigned Diff = AbsoluteDifference(InstOffset,
                                           BlockToInstOffset[TargetBB]);
        if (Diff > MaxLoopRange) {
          useExtLoopInstr(MF, MII);
          MII = MBB.erase(MII);
          Changed = true;
        } else {
          ++MII;
        }
      } else {
        ++MII;
      }
      InstOffset += InstSize;
    }
  }

  return Changed;
}

/// Replace loop instructions with the constant extended version.
void HexagonFixupHwLoops::useExtLoopInstr(MachineFunction &MF,
                                          MachineBasicBlock::iterator &MII) {
  const TargetInstrInfo *TII = MF.getSubtarget().getInstrInfo();
  MachineBasicBlock *MBB = MII->getParent();
  DebugLoc DL = MII->getDebugLoc();
  MachineInstrBuilder MIB;
  unsigned newOp;
  switch (MII->getOpcode()) {
  case Hexagon::J2_loop0r:
    newOp = Hexagon::J2_loop0rext;
    break;
  case Hexagon::J2_loop0i:
    newOp = Hexagon::J2_loop0iext;
    break;
  case Hexagon::J2_loop1r:
    newOp = Hexagon::J2_loop1rext;
    break;
  case Hexagon::J2_loop1i:
    newOp = Hexagon::J2_loop1iext;
    break;
  default:
    llvm_unreachable("Invalid Hardware Loop Instruction.");
  }
  MIB = BuildMI(*MBB, MII, DL, TII->get(newOp));

  for (unsigned i = 0; i < MII->getNumOperands(); ++i)
    MIB.add(MII->getOperand(i));
}
