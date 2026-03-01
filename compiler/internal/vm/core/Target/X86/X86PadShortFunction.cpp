//===-------- X86PadShortFunction.cpp - pad short functions -----------===//
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
// This file defines the pass which will pad short functions to prevent
// a stall if a function returns before the return address is ready. This
// is needed for some Intel Atom processors.
//
//===----------------------------------------------------------------------===//


#include "X86.h"
#include "X86InstrInfo.h"
#include "X86Subtarget.h"
#include "vm/core/ADT/Statistic.h"
#include "vm/core/Analysis/ProfileSummaryInfo.h"
#include "vm/core/CodeGen/LazyMachineBlockFrequencyInfo.h"
#include "vm/core/CodeGen/MachineFunctionPass.h"
#include "vm/core/CodeGen/MachineInstrBuilder.h"
#include "vm/core/CodeGen/MachineSizeOpts.h"
#include "vm/core/CodeGen/Passes.h"
#include "vm/core/CodeGen/TargetSchedule.h"
#include "vm/core/IR/Function.h"

using namespace vm::core;

#define DEBUG_TYPE "x86-pad-short-functions"

STATISTIC(NumBBsPadded, "Number of basic blocks padded");

namespace {
  struct VisitedBBInfo {
    // HasReturn - Whether the BB contains a return instruction
    bool HasReturn = false;

    // Cycles - Number of cycles until return if HasReturn is true, otherwise
    // number of cycles until end of the BB
    unsigned int Cycles = 0;

    VisitedBBInfo() = default;
    VisitedBBInfo(bool HasReturn, unsigned int Cycles)
      : HasReturn(HasReturn), Cycles(Cycles) {}
  };

  struct PadShortFunc : public MachineFunctionPass {
    static char ID;
    PadShortFunc() : MachineFunctionPass(ID) {}

    bool runOnMachineFunction(MachineFunction &MF) override;

    void getAnalysisUsage(AnalysisUsage &AU) const override {
      AU.addRequired<ProfileSummaryInfoWrapperPass>();
      AU.addRequired<LazyMachineBlockFrequencyInfoPass>();
      AU.addPreserved<LazyMachineBlockFrequencyInfoPass>();
      MachineFunctionPass::getAnalysisUsage(AU);
    }

    MachineFunctionProperties getRequiredProperties() const override {
      return MachineFunctionProperties().setNoVRegs();
    }

    StringRef getPassName() const override {
      return "X86 Atom pad short functions";
    }

  private:
    void findReturns(MachineBasicBlock *MBB,
                     unsigned int Cycles = 0);

    bool cyclesUntilReturn(MachineBasicBlock *MBB,
                           unsigned int &Cycles);

    void addPadding(MachineBasicBlock *MBB,
                    MachineBasicBlock::iterator &MBBI,
                    unsigned int NOOPsToAdd);

    const unsigned int Threshold = 4;

    // ReturnBBs - Maps basic blocks that return to the minimum number of
    // cycles until the return, starting from the entry block.
    DenseMap<MachineBasicBlock*, unsigned int> ReturnBBs;

    // VisitedBBs - Cache of previously visited BBs.
    DenseMap<MachineBasicBlock*, VisitedBBInfo> VisitedBBs;

    TargetSchedModel TSM;
  };

  char PadShortFunc::ID = 0;
}

FunctionPass *toolchain::createX86PadShortFunctions() {
  return new PadShortFunc();
}

/// runOnMachineFunction - Loop over all of the basic blocks, inserting
/// NOOP instructions before early exits.
bool PadShortFunc::runOnMachineFunction(MachineFunction &MF) {
  LLVM_DEBUG(dbgs() << "Start X86PadShortFunctionPass\n";);
  if (skipFunction(MF.getFunction()))
    return false;

  if (MF.getFunction().hasOptSize())
    return false;

  if (!MF.getSubtarget<X86Subtarget>().padShortFunctions())
    return false;

  TSM.init(&MF.getSubtarget());

  auto *PSI =
      &getAnalysis<ProfileSummaryInfoWrapperPass>().getPSI();
  auto *MBFI = (PSI && PSI->hasProfileSummary()) ?
               &getAnalysis<LazyMachineBlockFrequencyInfoPass>().getBFI() :
               nullptr;

  // Search through basic blocks and mark the ones that have early returns
  ReturnBBs.clear();
  VisitedBBs.clear();
  findReturns(&MF.front());

  bool MadeChange = false;

  // Pad the identified basic blocks with NOOPs
  for (const auto &ReturnBB : ReturnBBs) {
    MachineBasicBlock *MBB = ReturnBB.first;
    unsigned Cycles = ReturnBB.second;

    if (toolchain::shouldOptimizeForSize(MBB, PSI, MBFI))
      continue;

    if (Cycles < Threshold) {
      // BB ends in a return. Skip over any DBG_VALUE instructions
      // trailing the terminator.
      assert(MBB->size() > 0 &&
             "Basic block should contain at least a RET but is empty");
      MachineBasicBlock::iterator ReturnLoc = --MBB->end();

      while (ReturnLoc->isDebugInstr())
        --ReturnLoc;
      assert(ReturnLoc->isReturn() && !ReturnLoc->isCall() &&
             "Basic block does not end with RET");

      addPadding(MBB, ReturnLoc, Threshold - Cycles);
      NumBBsPadded++;
      MadeChange = true;
    }
  }
  LLVM_DEBUG(dbgs() << "End X86PadShortFunctionPass\n";);
  return MadeChange;
}

/// findReturn - Starting at MBB, follow control flow and add all
/// basic blocks that contain a return to ReturnBBs.
void PadShortFunc::findReturns(MachineBasicBlock *MBB, unsigned int Cycles) {
  // If this BB has a return, note how many cycles it takes to get there.
  bool hasReturn = cyclesUntilReturn(MBB, Cycles);
  if (Cycles >= Threshold)
    return;

  if (hasReturn) {
    unsigned int &NumCycles = ReturnBBs[MBB];
    NumCycles = std::max(NumCycles, Cycles);
    return;
  }

  // Follow branches in BB and look for returns
  for (MachineBasicBlock *Succ : MBB->successors())
    if (Succ != MBB)
      findReturns(Succ, Cycles);
}

/// cyclesUntilReturn - return true if the MBB has a return instruction,
/// and return false otherwise.
/// Cycles will be incremented by the number of cycles taken to reach the
/// return or the end of the BB, whichever occurs first.
bool PadShortFunc::cyclesUntilReturn(MachineBasicBlock *MBB,
                                     unsigned int &Cycles) {
  // Return cached result if BB was previously visited
  auto [It, Inserted] = VisitedBBs.try_emplace(MBB);
  if (!Inserted) {
    VisitedBBInfo BBInfo = It->second;
    Cycles += BBInfo.Cycles;
    return BBInfo.HasReturn;
  }

  unsigned int CyclesToEnd = 0;

  for (MachineInstr &MI : *MBB) {
    // Mark basic blocks with a return instruction. Calls to other
    // functions do not count because the called function will be padded,
    // if necessary.
    if (MI.isReturn() && !MI.isCall()) {
      It->second = VisitedBBInfo(true, CyclesToEnd);
      Cycles += CyclesToEnd;
      return true;
    }

    CyclesToEnd += TSM.computeInstrLatency(&MI);
  }

  It->second = VisitedBBInfo(false, CyclesToEnd);
  Cycles += CyclesToEnd;
  return false;
}

/// addPadding - Add the given number of NOOP instructions to the function
/// just prior to the return at MBBI
void PadShortFunc::addPadding(MachineBasicBlock *MBB,
                              MachineBasicBlock::iterator &MBBI,
                              unsigned int NOOPsToAdd) {
  const DebugLoc &DL = MBBI->getDebugLoc();
  unsigned IssueWidth = TSM.getIssueWidth();

  for (unsigned i = 0, e = IssueWidth * NOOPsToAdd; i != e; ++i)
    BuildMI(*MBB, MBBI, DL, TSM.getInstrInfo()->get(X86::NOOP));
}
