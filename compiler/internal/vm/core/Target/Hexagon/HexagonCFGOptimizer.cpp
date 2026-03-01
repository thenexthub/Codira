//===- HexagonCFGOptimizer.cpp - CFG optimizations ------------------------===//
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
#include "MCTargetDesc/HexagonMCTargetDesc.h"
#include "vm/core/CodeGen/MachineBasicBlock.h"
#include "vm/core/CodeGen/MachineFunction.h"
#include "vm/core/CodeGen/MachineFunctionPass.h"
#include "vm/core/CodeGen/MachineInstr.h"
#include "vm/core/CodeGen/MachineOperand.h"
#include "vm/core/CodeGen/TargetInstrInfo.h"
#include "vm/core/CodeGen/TargetSubtargetInfo.h"
#include "vm/core/Pass.h"
#include "vm/core/Support/ErrorHandling.h"
#include <cassert>
#include <vector>

using namespace vm::core;

#define DEBUG_TYPE "hexagon_cfg"

namespace {

class HexagonCFGOptimizer : public MachineFunctionPass {
private:
  void InvertAndChangeJumpTarget(MachineInstr &, MachineBasicBlock *);
  bool isOnFallThroughPath(MachineBasicBlock *MBB);

public:
  static char ID;

  HexagonCFGOptimizer() : MachineFunctionPass(ID) {}

  StringRef getPassName() const override { return "Hexagon CFG Optimizer"; }
  bool runOnMachineFunction(MachineFunction &Fn) override;

  MachineFunctionProperties getRequiredProperties() const override {
    return MachineFunctionProperties().setNoVRegs();
  }
};

} // end anonymous namespace

char HexagonCFGOptimizer::ID = 0;

static bool IsConditionalBranch(int Opc) {
  switch (Opc) {
    case Hexagon::J2_jumpt:
    case Hexagon::J2_jumptpt:
    case Hexagon::J2_jumpf:
    case Hexagon::J2_jumpfpt:
    case Hexagon::J2_jumptnew:
    case Hexagon::J2_jumpfnew:
    case Hexagon::J2_jumptnewpt:
    case Hexagon::J2_jumpfnewpt:
      return true;
  }
  return false;
}

static bool IsUnconditionalJump(int Opc) {
  return (Opc == Hexagon::J2_jump);
}

void HexagonCFGOptimizer::InvertAndChangeJumpTarget(
    MachineInstr &MI, MachineBasicBlock *NewTarget) {
  const TargetInstrInfo *TII =
      MI.getParent()->getParent()->getSubtarget().getInstrInfo();
  int NewOpcode = 0;
  switch (MI.getOpcode()) {
  case Hexagon::J2_jumpt:
    NewOpcode = Hexagon::J2_jumpf;
    break;
  case Hexagon::J2_jumpf:
    NewOpcode = Hexagon::J2_jumpt;
    break;
  case Hexagon::J2_jumptnewpt:
    NewOpcode = Hexagon::J2_jumpfnewpt;
    break;
  case Hexagon::J2_jumpfnewpt:
    NewOpcode = Hexagon::J2_jumptnewpt;
    break;
  default:
    llvm_unreachable("Cannot handle this case");
  }

  MI.setDesc(TII->get(NewOpcode));
  MI.getOperand(1).setMBB(NewTarget);
}

bool HexagonCFGOptimizer::isOnFallThroughPath(MachineBasicBlock *MBB) {
  if (MBB->canFallThrough())
    return true;
  for (MachineBasicBlock *PB : MBB->predecessors())
    if (PB->isLayoutSuccessor(MBB) && PB->canFallThrough())
      return true;
  return false;
}

bool HexagonCFGOptimizer::runOnMachineFunction(MachineFunction &Fn) {
  if (skipFunction(Fn.getFunction()))
    return false;

  // Loop over all of the basic blocks.
  for (MachineBasicBlock &MBB : Fn) {
    // Traverse the basic block.
    MachineBasicBlock::iterator MII = MBB.getFirstTerminator();
    if (MII != MBB.end()) {
      MachineInstr &MI = *MII;
      int Opc = MI.getOpcode();
      if (IsConditionalBranch(Opc)) {
        // (Case 1) Transform the code if the following condition occurs:
        //   BB1: if (p0) jump BB3
        //   ...falls-through to BB2 ...
        //   BB2: jump BB4
        //   ...next block in layout is BB3...
        //   BB3: ...
        //
        //  Transform this to:
        //  BB1: if (!p0) jump BB4
        //  Remove BB2
        //  BB3: ...
        //
        // (Case 2) A variation occurs when BB3 contains a JMP to BB4:
        //   BB1: if (p0) jump BB3
        //   ...falls-through to BB2 ...
        //   BB2: jump BB4
        //   ...other basic blocks ...
        //   BB4:
        //   ...not a fall-thru
        //   BB3: ...
        //     jump BB4
        //
        // Transform this to:
        //   BB1: if (!p0) jump BB4
        //   Remove BB2
        //   BB3: ...
        //   BB4: ...
        unsigned NumSuccs = MBB.succ_size();
        MachineBasicBlock::succ_iterator SI = MBB.succ_begin();
        MachineBasicBlock* FirstSucc = *SI;
        MachineBasicBlock* SecondSucc = *(++SI);
        MachineBasicBlock* LayoutSucc = nullptr;
        MachineBasicBlock* JumpAroundTarget = nullptr;

        if (MBB.isLayoutSuccessor(FirstSucc)) {
          LayoutSucc = FirstSucc;
          JumpAroundTarget = SecondSucc;
        } else if (MBB.isLayoutSuccessor(SecondSucc)) {
          LayoutSucc = SecondSucc;
          JumpAroundTarget = FirstSucc;
        } else {
          // Odd case...cannot handle.
        }

        // The target of the unconditional branch must be JumpAroundTarget.
        // TODO: If not, we should not invert the unconditional branch.
        MachineBasicBlock* CondBranchTarget = nullptr;
        if (MI.getOpcode() == Hexagon::J2_jumpt ||
            MI.getOpcode() == Hexagon::J2_jumpf) {
          CondBranchTarget = MI.getOperand(1).getMBB();
        }

        if (!LayoutSucc || (CondBranchTarget != JumpAroundTarget)) {
          continue;
        }

        if ((NumSuccs == 2) && LayoutSucc && (LayoutSucc->pred_size() == 1)) {
          // Ensure that BB2 has one instruction -- an unconditional jump.
          if ((LayoutSucc->size() == 1) &&
              IsUnconditionalJump(LayoutSucc->front().getOpcode())) {
            assert(JumpAroundTarget && "jump target is needed to process second basic block");
            MachineBasicBlock* UncondTarget =
              LayoutSucc->front().getOperand(0).getMBB();
            // Check if the layout successor of BB2 is BB3.
            bool case1 = LayoutSucc->isLayoutSuccessor(JumpAroundTarget);
            bool case2 = JumpAroundTarget->isSuccessor(UncondTarget) &&
              !JumpAroundTarget->empty() &&
              IsUnconditionalJump(JumpAroundTarget->back().getOpcode()) &&
              JumpAroundTarget->pred_size() == 1 &&
              JumpAroundTarget->succ_size() == 1;

            if (case1 || case2) {
              InvertAndChangeJumpTarget(MI, UncondTarget);
              MBB.replaceSuccessor(JumpAroundTarget, UncondTarget);

              // Remove the unconditional branch in LayoutSucc.
              LayoutSucc->erase(LayoutSucc->begin());
              LayoutSucc->replaceSuccessor(UncondTarget, JumpAroundTarget);

              // This code performs the conversion for case 2, which moves
              // the block to the fall-thru case (BB3 in the code above).
              if (case2 && !case1) {
                JumpAroundTarget->moveAfter(LayoutSucc);
                // only move a block if it doesn't have a fall-thru. otherwise
                // the CFG will be incorrect.
                if (!isOnFallThroughPath(UncondTarget))
                  UncondTarget->moveAfter(JumpAroundTarget);
              }

              // Correct live-in information. Is used by post-RA scheduler
              // The live-in to LayoutSucc is now all values live-in to
              // JumpAroundTarget.
              std::vector<MachineBasicBlock::RegisterMaskPair> OrigLiveIn(
                  LayoutSucc->livein_begin(), LayoutSucc->livein_end());
              std::vector<MachineBasicBlock::RegisterMaskPair> NewLiveIn(
                  JumpAroundTarget->livein_begin(),
                  JumpAroundTarget->livein_end());
              for (const auto &OrigLI : OrigLiveIn)
                LayoutSucc->removeLiveIn(OrigLI.PhysReg);
              for (const auto &NewLI : NewLiveIn)
                LayoutSucc->addLiveIn(NewLI);
            }
          }
        }
      }
    }
  }
  return true;
}

//===----------------------------------------------------------------------===//
//                         Public Constructor Functions
//===----------------------------------------------------------------------===//

INITIALIZE_PASS(HexagonCFGOptimizer, "hexagon-cfg", "Hexagon CFG Optimizer",
                false, false)

FunctionPass *toolchain::createHexagonCFGOptimizer() {
  return new HexagonCFGOptimizer();
}
