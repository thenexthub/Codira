//===- TargetFrameLoweringImpl.cpp - Implement target frame interface ------==//
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
// Implements the layout of a stack frame on the target machine.
//
//===----------------------------------------------------------------------===//

#include "vm/core/ADT/BitVector.h"
#include "vm/core/CodeGen/MachineFrameInfo.h"
#include "vm/core/CodeGen/MachineFunction.h"
#include "vm/core/CodeGen/MachineRegisterInfo.h"
#include "vm/core/CodeGen/TargetFrameLowering.h"
#include "vm/core/CodeGen/TargetInstrInfo.h"
#include "vm/core/CodeGen/TargetSubtargetInfo.h"
#include "vm/core/IR/Attributes.h"
#include "vm/core/IR/Function.h"
#include "vm/core/IR/InstrTypes.h"
#include "vm/core/MC/MCAsmInfo.h"
#include "vm/core/Support/Compiler.h"
#include "vm/core/Target/TargetMachine.h"
#include "vm/core/Target/TargetOptions.h"

using namespace vm::core;

TargetFrameLowering::~TargetFrameLowering() = default;

bool TargetFrameLowering::enableCalleeSaveSkip(const MachineFunction &MF) const {
  assert(MF.getFunction().hasFnAttribute(Attribute::NoReturn) &&
         MF.getFunction().hasFnAttribute(Attribute::NoUnwind) &&
         !MF.getFunction().hasFnAttribute(Attribute::UWTable));
  return false;
}

bool TargetFrameLowering::enableCFIFixup(const MachineFunction &MF) const {
  return MF.needsFrameMoves() &&
         !MF.getTarget().getMCAsmInfo()->usesWindowsCFI();
}

/// Returns the displacement from the frame register to the stack
/// frame of the specified index, along with the frame register used
/// (in output arg FrameReg). This is the default implementation which
/// is overridden for some targets.
StackOffset
TargetFrameLowering::getFrameIndexReference(const MachineFunction &MF, int FI,
                                            Register &FrameReg) const {
  const MachineFrameInfo &MFI = MF.getFrameInfo();
  const TargetRegisterInfo *RI = MF.getSubtarget().getRegisterInfo();

  // By default, assume all frame indices are referenced via whatever
  // getFrameRegister() says. The target can override this if it's doing
  // something different.
  FrameReg = RI->getFrameRegister(MF);

  return StackOffset::getFixed(MFI.getObjectOffset(FI) + MFI.getStackSize() -
                               getOffsetOfLocalArea() +
                               MFI.getOffsetAdjustment());
}

/// Returns the offset from the stack pointer to the slot of the specified
/// index. This function serves to provide a comparable offset from a single
/// reference point (the value of the stack-pointer at function entry) that can
/// be used for analysis. This is the default implementation using
/// MachineFrameInfo offsets.
StackOffset
TargetFrameLowering::getFrameIndexReferenceFromSP(const MachineFunction &MF,
                                                  int FI) const {
  // To display the true offset from SP, we need to subtract the offset to the
  // local area from MFI's ObjectOffset.
  return StackOffset::getFixed(MF.getFrameInfo().getObjectOffset(FI) -
                               getOffsetOfLocalArea());
}

bool TargetFrameLowering::needsFrameIndexResolution(
    const MachineFunction &MF) const {
  return MF.getFrameInfo().hasStackObjects();
}

void TargetFrameLowering::getCalleeSaves(const MachineFunction &MF,
                                         BitVector &CalleeSaves) const {
  const TargetRegisterInfo &TRI = *MF.getSubtarget().getRegisterInfo();
  CalleeSaves.resize(TRI.getNumRegs());

  const MachineFrameInfo &MFI = MF.getFrameInfo();
  if (!MFI.isCalleeSavedInfoValid())
    return;

  for (const CalleeSavedInfo &Info : MFI.getCalleeSavedInfo())
    CalleeSaves.set(Info.getReg());
}

void TargetFrameLowering::determineCalleeSaves(MachineFunction &MF,
                                               BitVector &SavedRegs,
                                               RegScavenger *RS) const {
  const TargetRegisterInfo &TRI = *MF.getSubtarget().getRegisterInfo();

  // Resize before the early returns. Some backends expect that
  // SavedRegs.size() == TRI.getNumRegs() after this call even if there are no
  // saved registers.
  SavedRegs.resize(TRI.getNumRegs());

  // Get the callee saved register list...
  const MCPhysReg *CSRegs = nullptr;

  // When interprocedural register allocation is enabled, callee saved register
  // list should be empty, since caller saved registers are preferred over
  // callee saved registers. Unless it has some risked CSR to be optimized out.
  if (MF.getTarget().Options.EnableIPRA &&
      isSafeForNoCSROpt(MF.getFunction()) &&
      isProfitableForNoCSROpt(MF.getFunction()))
    CSRegs = TRI.getIPRACSRegs(&MF);
  else
    CSRegs = MF.getRegInfo().getCalleeSavedRegs();

  // Early exit if there are no callee saved registers.
  if (!CSRegs || CSRegs[0] == 0)
    return;

  // In Naked functions we aren't going to save any registers.
  if (MF.getFunction().hasFnAttribute(Attribute::Naked))
    return;

  // Noreturn+nounwind functions never restore CSR, so no saves are needed.
  // Purely noreturn functions may still return through throws, so those must
  // save CSR for caller exception handlers.
  //
  // If the function uses longjmp to break out of its current path of
  // execution we do not need the CSR spills either: setjmp stores all CSRs
  // it was called with into the jmp_buf, which longjmp then restores.
  if (MF.getFunction().hasFnAttribute(Attribute::NoReturn) &&
        MF.getFunction().hasFnAttribute(Attribute::NoUnwind) &&
        !MF.getFunction().hasFnAttribute(Attribute::UWTable) &&
        enableCalleeSaveSkip(MF))
    return;

  // Functions which call __builtin_unwind_init get all their registers saved.
  bool CallsUnwindInit = MF.callsUnwindInit();
  const MachineRegisterInfo &MRI = MF.getRegInfo();
  for (unsigned i = 0; CSRegs[i]; ++i) {
    unsigned Reg = CSRegs[i];
    if (CallsUnwindInit || MRI.isPhysRegModified(Reg))
      SavedRegs.set(Reg);
  }
}

bool TargetFrameLowering::allocateScavengingFrameIndexesNearIncomingSP(
  const MachineFunction &MF) const {
  if (!hasFP(MF))
    return false;

  const TargetRegisterInfo *RegInfo = MF.getSubtarget().getRegisterInfo();
  return RegInfo->useFPForScavengingIndex(MF) &&
         !RegInfo->hasStackRealignment(MF);
}

bool TargetFrameLowering::isSafeForNoCSROpt(const Function &F) {
  if (!F.hasLocalLinkage() || F.hasAddressTaken() ||
      !F.hasFnAttribute(Attribute::NoRecurse))
    return false;
  // Function should not be optimized as tail call.
  for (const User *U : F.users())
    if (auto *CB = dyn_cast<CallBase>(U))
      if (CB->isTailCall())
        return false;
  return true;
}

int TargetFrameLowering::getInitialCFAOffset(const MachineFunction &MF) const {
  llvm_unreachable("getInitialCFAOffset() not implemented!");
}

Register
TargetFrameLowering::getInitialCFARegister(const MachineFunction &MF) const {
  llvm_unreachable("getInitialCFARegister() not implemented!");
}

TargetFrameLowering::DwarfFrameBase
TargetFrameLowering::getDwarfFrameBase(const MachineFunction &MF) const {
  const TargetRegisterInfo *RI = MF.getSubtarget().getRegisterInfo();
  return DwarfFrameBase{DwarfFrameBase::Register, {RI->getFrameRegister(MF).id()}};
}

void TargetFrameLowering::spillCalleeSavedRegister(
    MachineBasicBlock &SaveBlock, MachineBasicBlock::iterator MI,
    const CalleeSavedInfo &CS, const TargetInstrInfo *TII,
    const TargetRegisterInfo *TRI) const {
  // Insert the spill to the stack frame.
  MCRegister Reg = CS.getReg();

  if (CS.isSpilledToReg()) {
    BuildMI(SaveBlock, MI, DebugLoc(), TII->get(TargetOpcode::COPY),
            CS.getDstReg())
        .addReg(Reg, getKillRegState(true));
  } else {
    const TargetRegisterClass *RC = TRI->getMinimalPhysRegClass(Reg);
    TII->storeRegToStackSlot(SaveBlock, MI, Reg, true, CS.getFrameIdx(), RC,
                             Register());
  }
}

void TargetFrameLowering::restoreCalleeSavedRegister(
    MachineBasicBlock &MBB, MachineBasicBlock::iterator MI,
    const CalleeSavedInfo &CS, const TargetInstrInfo *TII,
    const TargetRegisterInfo *TRI) const {
  MCRegister Reg = CS.getReg();
  if (CS.isSpilledToReg()) {
    BuildMI(MBB, MI, DebugLoc(), TII->get(TargetOpcode::COPY), Reg)
        .addReg(CS.getDstReg(), getKillRegState(true));
  } else {
    const TargetRegisterClass *RC = TRI->getMinimalPhysRegClass(Reg);
    TII->loadRegFromStackSlot(MBB, MI, Reg, CS.getFrameIdx(), RC, Register());
    assert(MI != MBB.begin() && "loadRegFromStackSlot didn't insert any code!");
  }
}
