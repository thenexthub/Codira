//===---------------------- InOrderIssueStage.cpp ---------------*- C++ -*-===//
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
/// \file
///
/// InOrderIssueStage implements an in-order execution pipeline.
///
//===----------------------------------------------------------------------===//

#include "vm/core/MCA/Stages/InOrderIssueStage.h"
#include "vm/core/MCA/HardwareUnits/LSUnit.h"
#include "vm/core/MCA/HardwareUnits/RegisterFile.h"
#include "vm/core/MCA/HardwareUnits/RetireControlUnit.h"
#include "vm/core/MCA/Instruction.h"

#define DEBUG_TYPE "toolchain-mca"
namespace vm::core {
namespace mca {

void StallInfo::clear() {
  IR.invalidate();
  CyclesLeft = 0;
  Kind = StallKind::DEFAULT;
}

void StallInfo::update(const InstRef &Inst, unsigned Cycles, StallKind SK) {
  IR = Inst;
  CyclesLeft = Cycles;
  Kind = SK;
}

void StallInfo::cycleEnd() {
  if (!isValid())
    return;

  if (!CyclesLeft)
    return;

  --CyclesLeft;
}

InOrderIssueStage::InOrderIssueStage(const MCSubtargetInfo &STI,
                                     RegisterFile &PRF, CustomBehaviour &CB,
                                     LSUnitBase &LSU)
    : STI(STI), PRF(PRF), RM(STI.getSchedModel()), CB(CB), LSU(LSU),
      NumIssued(), CarryOver(), Bandwidth(), LastWriteBackCycle() {}

unsigned InOrderIssueStage::getIssueWidth() const {
  return STI.getSchedModel().IssueWidth;
}

bool InOrderIssueStage::hasWorkToComplete() const {
  return !IssuedInst.empty() || SI.isValid() || CarriedOver;
}

bool InOrderIssueStage::isAvailable(const InstRef &IR) const {
  if (SI.isValid() || CarriedOver)
    return false;

  const Instruction &Inst = *IR.getInstruction();
  unsigned NumMicroOps = Inst.getNumMicroOps();

  bool ShouldCarryOver = NumMicroOps > getIssueWidth();
  if (Bandwidth < NumMicroOps && !ShouldCarryOver)
    return false;

  // Instruction with BeginGroup must be the first instruction to be issued in a
  // cycle.
  if (Inst.getBeginGroup() && NumIssued != 0)
    return false;

  return true;
}

static bool hasResourceHazard(const ResourceManager &RM, const InstRef &IR) {
  if (RM.checkAvailability(IR.getInstruction()->getDesc())) {
    LLVM_DEBUG(dbgs() << "[E] Stall #" << IR << '\n');
    return true;
  }

  return false;
}

static unsigned findFirstWriteBackCycle(const InstRef &IR) {
  unsigned FirstWBCycle = IR.getInstruction()->getLatency();
  for (const WriteState &WS : IR.getInstruction()->getDefs()) {
    int CyclesLeft = WS.getCyclesLeft();
    if (CyclesLeft == UNKNOWN_CYCLES)
      CyclesLeft = WS.getLatency();
    if (CyclesLeft < 0)
      CyclesLeft = 0;
    FirstWBCycle = std::min(FirstWBCycle, (unsigned)CyclesLeft);
  }
  return FirstWBCycle;
}

/// Return a number of cycles left until register requirements of the
/// instructions are met.
static unsigned checkRegisterHazard(const RegisterFile &PRF,
                                    const MCSubtargetInfo &STI,
                                    const InstRef &IR) {
  for (const ReadState &RS : IR.getInstruction()->getUses()) {
    RegisterFile::RAWHazard Hazard = PRF.checkRAWHazards(STI, RS);
    if (Hazard.isValid())
      return Hazard.hasUnknownCycles() ? 1U : Hazard.CyclesLeft;
  }

  return 0;
}

bool InOrderIssueStage::canExecute(const InstRef &IR) {
  assert(!SI.getCyclesLeft() && "Should not have reached this code!");
  assert(!SI.isValid() && "Should not have reached this code!");

  if (unsigned Cycles = checkRegisterHazard(PRF, STI, IR)) {
    SI.update(IR, Cycles, StallInfo::StallKind::REGISTER_DEPS);
    return false;
  }

  if (hasResourceHazard(RM, IR)) {
    SI.update(IR, /* delay */ 1, StallInfo::StallKind::DISPATCH);
    return false;
  }

  if (IR.getInstruction()->isMemOp() && !LSU.isReady(IR)) {
    // This load (store) aliases with a preceding store (load). Delay
    // it until the depenency is cleared.
    SI.update(IR, /* delay */ 1, StallInfo::StallKind::LOAD_STORE);
    return false;
  }

  if (unsigned CustomStallCycles = CB.checkCustomHazard(IssuedInst, IR)) {
    SI.update(IR, CustomStallCycles, StallInfo::StallKind::CUSTOM_STALL);
    return false;
  }

  if (LastWriteBackCycle) {
    if (!IR.getInstruction()->getRetireOOO()) {
      unsigned NextWriteBackCycle = findFirstWriteBackCycle(IR);
      // Delay the instruction to ensure that writes happen in program order.
      if (NextWriteBackCycle < LastWriteBackCycle) {
        SI.update(IR, LastWriteBackCycle - NextWriteBackCycle,
                  StallInfo::StallKind::DELAY);
        return false;
      }
    }
  }

  return true;
}

static void addRegisterReadWrite(RegisterFile &PRF, Instruction &IS,
                                 unsigned SourceIndex,
                                 const MCSubtargetInfo &STI,
                                 SmallVectorImpl<unsigned> &UsedRegs) {
  assert(!IS.isEliminated());

  for (ReadState &RS : IS.getUses())
    PRF.addRegisterRead(RS, STI);

  for (WriteState &WS : IS.getDefs())
    PRF.addRegisterWrite(WriteRef(SourceIndex, &WS), UsedRegs);
}

void InOrderIssueStage::notifyInstructionIssued(const InstRef &IR,
                                                ArrayRef<ResourceUse> UsedRes) {
  notifyEvent<HWInstructionEvent>(
      HWInstructionEvent(HWInstructionEvent::Ready, IR));
  notifyEvent<HWInstructionEvent>(HWInstructionIssuedEvent(IR, UsedRes));

  LLVM_DEBUG(dbgs() << "[E] Issued #" << IR << "\n");
}

void InOrderIssueStage::notifyInstructionDispatched(
    const InstRef &IR, unsigned Ops, ArrayRef<unsigned> UsedRegs) {
  notifyEvent<HWInstructionEvent>(
      HWInstructionDispatchedEvent(IR, UsedRegs, Ops));

  LLVM_DEBUG(dbgs() << "[E] Dispatched #" << IR << "\n");
}

void InOrderIssueStage::notifyInstructionExecuted(const InstRef &IR) {
  notifyEvent<HWInstructionEvent>(
      HWInstructionEvent(HWInstructionEvent::Executed, IR));
  LLVM_DEBUG(dbgs() << "[E] Instruction #" << IR << " is executed\n");
}

void InOrderIssueStage::notifyInstructionRetired(const InstRef &IR,
                                                 ArrayRef<unsigned> FreedRegs) {
  notifyEvent<HWInstructionEvent>(HWInstructionRetiredEvent(IR, FreedRegs));
  LLVM_DEBUG(dbgs() << "[E] Retired #" << IR << " \n");
}

toolchain::Error InOrderIssueStage::execute(InstRef &IR) {
  Instruction &IS = *IR.getInstruction();
  if (IS.isMemOp())
    IS.setLSUTokenID(LSU.dispatch(IR));

  if (toolchain::Error E = tryIssue(IR))
    return E;

  if (SI.isValid())
    notifyStallEvent();

  return toolchain::ErrorSuccess();
}

toolchain::Error InOrderIssueStage::tryIssue(InstRef &IR) {
  Instruction &IS = *IR.getInstruction();
  unsigned SourceIndex = IR.getSourceIndex();
  const InstrDesc &Desc = IS.getDesc();

  if (!canExecute(IR)) {
    LLVM_DEBUG(dbgs() << "[N] Stalled #" << SI.getInstruction() << " for "
                      << SI.getCyclesLeft() << " cycles\n");
    Bandwidth = 0;
    return toolchain::ErrorSuccess();
  }

  unsigned RCUTokenID = RetireControlUnit::UnhandledTokenID;
  IS.dispatch(RCUTokenID);

  SmallVector<unsigned, 4> UsedRegs(PRF.getNumRegisterFiles());
  addRegisterReadWrite(PRF, IS, SourceIndex, STI, UsedRegs);

  unsigned NumMicroOps = IS.getNumMicroOps();
  notifyInstructionDispatched(IR, NumMicroOps, UsedRegs);

  SmallVector<ResourceUse, 4> UsedResources;
  RM.issueInstruction(Desc, UsedResources);
  IS.execute(SourceIndex);

  if (IS.isMemOp())
    LSU.onInstructionIssued(IR);

  // Replace resource masks with valid resource processor IDs.
  for (ResourceUse &Use : UsedResources) {
    uint64_t Mask = Use.first.first;
    Use.first.first = RM.resolveResourceMask(Mask);
  }
  notifyInstructionIssued(IR, UsedResources);

  bool ShouldCarryOver = NumMicroOps > Bandwidth;
  if (ShouldCarryOver) {
    CarryOver = NumMicroOps - Bandwidth;
    CarriedOver = IR;
    Bandwidth = 0;
    NumIssued += Bandwidth;
    LLVM_DEBUG(dbgs() << "[N] Carry over #" << IR << " \n");
  } else {
    NumIssued += NumMicroOps;
    Bandwidth = IS.getEndGroup() ? 0 : Bandwidth - NumMicroOps;
  }

  // If the instruction has a latency of 0, we need to handle
  // the execution and retirement now. If the instruction is issued in multiple
  // cycles, we cannot handle the instruction being executed here so we make
  // updateCarriedOver responsible.
  if (IS.isExecuted() && !ShouldCarryOver) {
    PRF.onInstructionExecuted(&IS);
    LSU.onInstructionExecuted(IR);
    notifyInstructionExecuted(IR);

    retireInstruction(IR);
    return toolchain::ErrorSuccess();
  }

  IssuedInst.push_back(IR);

  if (!IR.getInstruction()->getRetireOOO())
    LastWriteBackCycle = IS.getCyclesLeft();

  return toolchain::ErrorSuccess();
}

void InOrderIssueStage::updateIssuedInst() {
  // Update other instructions. Executed instructions will be retired during the
  // next cycle.
  unsigned NumExecuted = 0;
  for (auto I = IssuedInst.begin(), E = IssuedInst.end();
       I != (E - NumExecuted);) {
    InstRef &IR = *I;
    Instruction &IS = *IR.getInstruction();

    IS.cycleEvent();
    if (!IS.isExecuted()) {
      LLVM_DEBUG(dbgs() << "[N] Instruction #" << IR
                        << " is still executing\n");
      ++I;
      continue;
    }

    // If the instruction takes multiple cycles to issue, defer these calls
    // to updateCarriedOver. We still remove from IssuedInst even if there is
    // carry over to avoid an extra call to cycleEvent in the next cycle.
    if (!CarriedOver) {
      PRF.onInstructionExecuted(&IS);
      LSU.onInstructionExecuted(IR);
      notifyInstructionExecuted(IR);

      retireInstruction(*I);
    }

    ++NumExecuted;

    std::iter_swap(I, E - NumExecuted);
  }

  if (NumExecuted)
    IssuedInst.resize(IssuedInst.size() - NumExecuted);
}

void InOrderIssueStage::updateCarriedOver() {
  if (!CarriedOver)
    return;

  assert(!SI.isValid() && "A stalled instruction cannot be carried over.");

  if (CarryOver > Bandwidth) {
    CarryOver -= Bandwidth;
    Bandwidth = 0;
    LLVM_DEBUG(dbgs() << "[N] Carry over (" << CarryOver << "uops left) #"
                      << CarriedOver << " \n");
    return;
  }

  LLVM_DEBUG(dbgs() << "[N] Carry over (complete) #" << CarriedOver << " \n");

  if (CarriedOver.getInstruction()->getEndGroup())
    Bandwidth = 0;
  else
    Bandwidth -= CarryOver;

  // updateIssuedInst defered these calls to updateCarriedOver when there was
  // a carry over.
  if (CarriedOver.getInstruction()->isExecuted()) {
    PRF.onInstructionExecuted(CarriedOver.getInstruction());
    LSU.onInstructionExecuted(CarriedOver);
    notifyInstructionExecuted(CarriedOver);

    retireInstruction(CarriedOver);
  }

  CarriedOver = InstRef();
  CarryOver = 0;
}

void InOrderIssueStage::retireInstruction(InstRef &IR) {
  Instruction &IS = *IR.getInstruction();
  IS.retire();

  toolchain::SmallVector<unsigned, 4> FreedRegs(PRF.getNumRegisterFiles());
  for (const WriteState &WS : IS.getDefs())
    PRF.removeRegisterWrite(WS, FreedRegs);

  if (IS.isMemOp())
    LSU.onInstructionRetired(IR);

  notifyInstructionRetired(IR, FreedRegs);
}

void InOrderIssueStage::notifyStallEvent() {
  assert(SI.getCyclesLeft() && "A zero cycles stall?");
  assert(SI.isValid() && "Invalid stall information found!");

  const InstRef &IR = SI.getInstruction();

  switch (SI.getStallKind()) {
  default:
    break;
  case StallInfo::StallKind::REGISTER_DEPS: {
    notifyEvent<HWStallEvent>(
        HWStallEvent(HWStallEvent::RegisterFileStall, IR));
    notifyEvent<HWPressureEvent>(
        HWPressureEvent(HWPressureEvent::REGISTER_DEPS, IR));
    break;
  }
  case StallInfo::StallKind::DISPATCH: {
    notifyEvent<HWStallEvent>(
        HWStallEvent(HWStallEvent::DispatchGroupStall, IR));
    notifyEvent<HWPressureEvent>(
        HWPressureEvent(HWPressureEvent::RESOURCES, IR));
    break;
  }
  case StallInfo::StallKind::CUSTOM_STALL: {
    notifyEvent<HWStallEvent>(
        HWStallEvent(HWStallEvent::CustomBehaviourStall, IR));
    break;
  }
  }
}

toolchain::Error InOrderIssueStage::cycleStart() {
  NumIssued = 0;
  Bandwidth = getIssueWidth();

  PRF.cycleStart();
  LSU.cycleEvent();

  // Release consumed resources.
  SmallVector<ResourceRef, 4> Freed;
  RM.cycleEvent(Freed);

  updateIssuedInst();

  // Continue to issue the instruction carried over from the previous cycle
  updateCarriedOver();

  // Issue instructions scheduled for this cycle
  if (SI.isValid()) {
    if (!SI.getCyclesLeft()) {
      // Make a copy of the reference, and try issue it again.
      // Do not take the instruction reference because SI.clear() will
      // invalidate it.
      InstRef IR = SI.getInstruction();
      SI.clear();

      if (toolchain::Error E = tryIssue(IR))
        return E;
    }

    if (SI.getCyclesLeft()) {
      // The instruction is still stalled, cannot issue any new instructions in
      // this cycle.
      notifyStallEvent();
      Bandwidth = 0;
      return toolchain::ErrorSuccess();
    }
  }

  assert((NumIssued <= getIssueWidth()) && "Overflow.");
  return toolchain::ErrorSuccess();
}

toolchain::Error InOrderIssueStage::cycleEnd() {
  PRF.cycleEnd();
  SI.cycleEnd();

  if (LastWriteBackCycle > 0)
    --LastWriteBackCycle;

  return toolchain::ErrorSuccess();
}

} // namespace mca
} // namespace vm::core
