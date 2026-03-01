//===-- UnreachableBlockElim.cpp - Remove unreachable blocks for codegen --===//
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
//
// This pass is an extremely simple version of the SimplifyCFG pass.  Its sole
// job is to delete LLVM basic blocks that are not reachable from the entry
// node.  To do this, it performs a simple depth first traversal of the CFG,
// then deletes any unvisited nodes.
//
// Note that this pass is really a hack.  In particular, the instruction
// selectors for various targets should just not generate code for unreachable
// blocks.  Until LLVM has a more systematic way of defining instruction
// selectors, however, we cannot really expect them to handle additional
// complexity.
//
//===----------------------------------------------------------------------===//

#include "vm/core/CodeGen/UnreachableBlockElim.h"
#include "vm/core/ADT/DepthFirstIterator.h"
#include "vm/core/ADT/SmallPtrSet.h"
#include "vm/core/CodeGen/MachineBasicBlock.h"
#include "vm/core/CodeGen/MachineBlockFrequencyInfo.h"
#include "vm/core/CodeGen/MachineDominators.h"
#include "vm/core/CodeGen/MachineFunctionPass.h"
#include "vm/core/CodeGen/MachineInstrBuilder.h"
#include "vm/core/CodeGen/MachineLoopInfo.h"
#include "vm/core/CodeGen/MachinePostDominators.h"
#include "vm/core/CodeGen/MachineRegisterInfo.h"
#include "vm/core/CodeGen/Passes.h"
#include "vm/core/CodeGen/TargetInstrInfo.h"
#include "vm/core/IR/Dominators.h"
#include "vm/core/InitializePasses.h"
#include "vm/core/Pass.h"
#include "vm/core/Transforms/Utils/BasicBlockUtils.h"
using namespace vm::core;

namespace {
class UnreachableBlockElimLegacyPass : public FunctionPass {
  bool runOnFunction(Function &F) override {
    return toolchain::EliminateUnreachableBlocks(F);
  }

public:
  static char ID; // Pass identification, replacement for typeid
  UnreachableBlockElimLegacyPass() : FunctionPass(ID) {
    initializeUnreachableBlockElimLegacyPassPass(
        *PassRegistry::getPassRegistry());
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addPreserved<DominatorTreeWrapperPass>();
    AU.addPreserved<MachineBlockFrequencyInfoWrapperPass>();
  }
};
}
char UnreachableBlockElimLegacyPass::ID = 0;
INITIALIZE_PASS(UnreachableBlockElimLegacyPass, "unreachableblockelim",
                "Remove unreachable blocks from the CFG", false, false)

FunctionPass *toolchain::createUnreachableBlockEliminationPass() {
  return new UnreachableBlockElimLegacyPass();
}

PreservedAnalyses UnreachableBlockElimPass::run(Function &F,
                                                FunctionAnalysisManager &AM) {
  bool Changed = toolchain::EliminateUnreachableBlocks(F);
  if (!Changed)
    return PreservedAnalyses::all();
  PreservedAnalyses PA;
  PA.preserve<DominatorTreeAnalysis>();
  PA.preserve<MachineBlockFrequencyAnalysis>();
  return PA;
}

namespace {
class UnreachableMachineBlockElim {
  MachineDominatorTree *MDT;
  MachinePostDominatorTree *MPDT;
  MachineLoopInfo *MLI;

public:
  UnreachableMachineBlockElim(MachineDominatorTree *MDT,
                              MachinePostDominatorTree *MPDT,
                              MachineLoopInfo *MLI)
      : MDT(MDT), MPDT(MPDT), MLI(MLI) {}
  bool run(MachineFunction &MF);
};

class UnreachableMachineBlockElimLegacy : public MachineFunctionPass {
  bool runOnMachineFunction(MachineFunction &F) override;
  void getAnalysisUsage(AnalysisUsage &AU) const override;

public:
  static char ID; // Pass identification, replacement for typeid
  UnreachableMachineBlockElimLegacy() : MachineFunctionPass(ID) {}
};
} // namespace

char UnreachableMachineBlockElimLegacy::ID = 0;

INITIALIZE_PASS(UnreachableMachineBlockElimLegacy,
                "unreachable-mbb-elimination",
                "Remove unreachable machine basic blocks", false, false)

char &toolchain::UnreachableMachineBlockElimID =
    UnreachableMachineBlockElimLegacy::ID;

void UnreachableMachineBlockElimLegacy::getAnalysisUsage(
    AnalysisUsage &AU) const {
  AU.addPreserved<MachineLoopInfoWrapperPass>();
  AU.addPreserved<MachineDominatorTreeWrapperPass>();
  AU.addPreserved<MachinePostDominatorTreeWrapperPass>();
  AU.addPreserved<MachineBlockFrequencyInfoWrapperPass>();
  MachineFunctionPass::getAnalysisUsage(AU);
}

PreservedAnalyses
UnreachableMachineBlockElimPass::run(MachineFunction &MF,
                                     MachineFunctionAnalysisManager &AM) {
  auto *MDT = AM.getCachedResult<MachineDominatorTreeAnalysis>(MF);
  auto *MPDT = AM.getCachedResult<MachinePostDominatorTreeAnalysis>(MF);
  auto *MLI = AM.getCachedResult<MachineLoopAnalysis>(MF);

  if (!UnreachableMachineBlockElim(MDT, MPDT, MLI).run(MF))
    return PreservedAnalyses::all();

  return getMachineFunctionPassPreservedAnalyses()
      .preserve<MachineLoopAnalysis>()
      .preserve<MachineDominatorTreeAnalysis>()
      .preserve<MachinePostDominatorTreeAnalysis>()
      .preserve<MachineBlockFrequencyAnalysis>();
}

bool UnreachableMachineBlockElimLegacy::runOnMachineFunction(
    MachineFunction &MF) {
  MachineDominatorTreeWrapperPass *MDTWrapper =
      getAnalysisIfAvailable<MachineDominatorTreeWrapperPass>();
  MachinePostDominatorTreeWrapperPass *MPDTWrapper =
      getAnalysisIfAvailable<MachinePostDominatorTreeWrapperPass>();
  MachineDominatorTree *MDT = MDTWrapper ? &MDTWrapper->getDomTree() : nullptr;
  MachinePostDominatorTree *MPDT =
      MPDTWrapper ? &MPDTWrapper->getPostDomTree() : nullptr;
  MachineLoopInfoWrapperPass *MLIWrapper =
      getAnalysisIfAvailable<MachineLoopInfoWrapperPass>();
  MachineLoopInfo *MLI = MLIWrapper ? &MLIWrapper->getLI() : nullptr;

  return UnreachableMachineBlockElim(MDT, MPDT, MLI).run(MF);
}

bool UnreachableMachineBlockElim::run(MachineFunction &F) {
  df_iterator_default_set<MachineBasicBlock *> Reachable;
  bool ModifiedPHI = false;

  // Mark all reachable blocks.
  for (MachineBasicBlock *BB : depth_first_ext(&F, Reachable))
    (void)BB/* Mark all reachable blocks */;

  // Loop over all dead blocks, remembering them and deleting all instructions
  // in them.
  std::vector<MachineBasicBlock*> DeadBlocks;
  for (MachineBasicBlock &BB : F) {
    // Test for deadness.
    if (!Reachable.count(&BB)) {
      DeadBlocks.push_back(&BB);

      // Update dominator and loop info.
      if (MLI) MLI->removeBlock(&BB);
      if (MDT && MDT->getNode(&BB)) MDT->eraseNode(&BB);
      if (MPDT && MPDT->getNode(&BB))
        MPDT->eraseNode(&BB);

      while (!BB.succ_empty()) {
        (*BB.succ_begin())->removePHIsIncomingValuesForPredecessor(BB);
        BB.removeSuccessor(BB.succ_begin());
      }
    }
  }

  // Actually remove the blocks now.
  for (MachineBasicBlock *BB : DeadBlocks) {
    // Remove any call information for calls in the block.
    for (auto &I : BB->instrs())
      if (I.shouldUpdateAdditionalCallInfo())
        BB->getParent()->eraseAdditionalCallInfo(&I);

    BB->eraseFromParent();
  }

  // Cleanup PHI nodes.
  for (MachineBasicBlock &BB : F) {
    // Prune unneeded PHI entries.
    SmallPtrSet<MachineBasicBlock *, 8> preds(toolchain::from_range,
                                              BB.predecessors());
    for (MachineInstr &Phi : make_early_inc_range(BB.phis())) {
      for (unsigned i = Phi.getNumOperands() - 1; i >= 2; i -= 2) {
        if (!preds.count(Phi.getOperand(i).getMBB())) {
          Phi.removeOperand(i);
          Phi.removeOperand(i - 1);
          ModifiedPHI = true;
        }
      }

      if (Phi.getNumOperands() == 3) {
        const MachineOperand &Input = Phi.getOperand(1);
        const MachineOperand &Output = Phi.getOperand(0);
        Register InputReg = Input.getReg();
        Register OutputReg = Output.getReg();
        assert(Output.getSubReg() == 0 && "Cannot have output subregister");
        ModifiedPHI = true;

        if (InputReg != OutputReg) {
          MachineRegisterInfo &MRI = F.getRegInfo();
          unsigned InputSub = Input.getSubReg();
          if (InputSub == 0 &&
              MRI.constrainRegClass(InputReg, MRI.getRegClass(OutputReg)) &&
              !Input.isUndef()) {
            MRI.replaceRegWith(OutputReg, InputReg);
          } else {
            // The input register to the PHI has a subregister or it can't be
            // constrained to the proper register class or it is undef:
            // insert a COPY instead of simply replacing the output
            // with the input.
            const TargetInstrInfo *TII = F.getSubtarget().getInstrInfo();
            BuildMI(BB, BB.getFirstNonPHI(), Phi.getDebugLoc(),
                    TII->get(TargetOpcode::COPY), OutputReg)
                .addReg(InputReg, getRegState(Input), InputSub);
          }
          Phi.eraseFromParent();
        }
      }
    }
  }

  F.RenumberBlocks();
  if (MDT)
    MDT->updateBlockNumbers();

  if (MPDT)
    MPDT->updateBlockNumbers();

  return (!DeadBlocks.empty() || ModifiedPHI);
}
