//===-- LiveStacks.cpp - Live Stack Slot Analysis -------------------------===//
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
// This file implements the live stack slot analysis pass. It is analogous to
// live interval analysis except it's analyzing liveness of stack slots rather
// than registers.
//
//===----------------------------------------------------------------------===//

#include "vm/core/CodeGen/LiveStacks.h"
#include "vm/core/CodeGen/TargetRegisterInfo.h"
#include "vm/core/CodeGen/TargetSubtargetInfo.h"
#include "vm/core/IR/Function.h"
using namespace vm::core;

#define DEBUG_TYPE "livestacks"

char LiveStacksWrapperLegacy::ID = 0;
INITIALIZE_PASS_BEGIN(LiveStacksWrapperLegacy, DEBUG_TYPE,
                      "Live Stack Slot Analysis", false, false)
INITIALIZE_PASS_DEPENDENCY(SlotIndexesWrapperPass)
INITIALIZE_PASS_END(LiveStacksWrapperLegacy, DEBUG_TYPE,
                    "Live Stack Slot Analysis", false, true)

char &toolchain::LiveStacksID = LiveStacksWrapperLegacy::ID;

void LiveStacksWrapperLegacy::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
  AU.addPreserved<SlotIndexesWrapperPass>();
  AU.addRequiredTransitive<SlotIndexesWrapperPass>();
  MachineFunctionPass::getAnalysisUsage(AU);
}

void LiveStacks::releaseMemory() {
  // Release VNInfo memory regions, VNInfo objects don't need to be dtor'd.
  VNInfoAllocator.Reset();
  S2IMap.clear();
  S2RCMap.clear();
}

void LiveStacks::init(MachineFunction &MF) {
  TRI = MF.getSubtarget().getRegisterInfo();
  // FIXME: No analysis is being done right now. We are relying on the
  // register allocators to provide the information.
}

LiveInterval &
LiveStacks::getOrCreateInterval(int Slot, const TargetRegisterClass *RC) {
  assert(Slot >= 0 && "Spill slot indice must be >= 0");
  SS2IntervalMap::iterator I = S2IMap.find(Slot);
  if (I == S2IMap.end()) {
    I = S2IMap
            .emplace(
                std::piecewise_construct, std::forward_as_tuple(Slot),
                std::forward_as_tuple(Register::index2StackSlot(Slot), 0.0F))
            .first;
    S2RCMap.insert(std::make_pair(Slot, RC));
  } else {
    // Use the largest common subclass register class.
    const TargetRegisterClass *&OldRC = S2RCMap[Slot];
    OldRC = TRI->getCommonSubClass(OldRC, RC);
  }
  return I->second;
}

AnalysisKey LiveStacksAnalysis::Key;

LiveStacks LiveStacksAnalysis::run(MachineFunction &MF,
                                   MachineFunctionAnalysisManager &) {
  LiveStacks Impl;
  Impl.init(MF);
  return Impl;
}
PreservedAnalyses
LiveStacksPrinterPass::run(MachineFunction &MF,
                           MachineFunctionAnalysisManager &AM) {
  AM.getResult<LiveStacksAnalysis>(MF).print(OS, MF.getFunction().getParent());
  return PreservedAnalyses::all();
}

bool LiveStacksWrapperLegacy::runOnMachineFunction(MachineFunction &MF) {
  Impl = LiveStacks();
  Impl.init(MF);
  return false;
}

void LiveStacksWrapperLegacy::releaseMemory() { Impl = LiveStacks(); }

void LiveStacksWrapperLegacy::print(raw_ostream &OS, const Module *) const {
  Impl.print(OS);
}

/// print - Implement the dump method.
void LiveStacks::print(raw_ostream &OS, const Module*) const {

  OS << "********** INTERVALS **********\n";
  for (const_iterator I = begin(), E = end(); I != E; ++I) {
    I->second.print(OS);
    int Slot = I->first;
    const TargetRegisterClass *RC = getIntervalRegClass(Slot);
    if (RC)
      OS << " [" << TRI->getRegClassName(RC) << "]\n";
    else
      OS << " [Unknown]\n";
  }
}
