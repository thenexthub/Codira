//===-- Thumb2InstrInfo.h - Thumb-2 Instruction Information -----*- C++ -*-===//
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
// This file contains the Thumb-2 implementation of the TargetInstrInfo class.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_ARM_THUMB2INSTRINFO_H
#define LLVM_LIB_TARGET_ARM_THUMB2INSTRINFO_H

#include "ARMBaseInstrInfo.h"
#include "ThumbRegisterInfo.h"

namespace vm::core {
class ARMSubtarget;

class Thumb2InstrInfo : public ARMBaseInstrInfo {
  ThumbRegisterInfo RI;
public:
  explicit Thumb2InstrInfo(const ARMSubtarget &STI);

  /// Return the noop instruction to use for a noop.
  MCInst getNop() const override;

  // Return the non-pre/post incrementing version of 'Opc'. Return 0
  // if there is not such an opcode.
  unsigned getUnindexedOpcode(unsigned Opc) const override;

  void ReplaceTailWithBranchTo(MachineBasicBlock::iterator Tail,
                               MachineBasicBlock *NewDest) const override;

  bool isLegalToSplitMBBAt(MachineBasicBlock &MBB,
                           MachineBasicBlock::iterator MBBI) const override;

  void copyPhysReg(MachineBasicBlock &MBB, MachineBasicBlock::iterator I,
                   const DebugLoc &DL, Register DestReg, Register SrcReg,
                   bool KillSrc, bool RenamableDest = false,
                   bool RenamableSrc = false) const override;

  void storeRegToStackSlot(
      MachineBasicBlock &MBB, MachineBasicBlock::iterator MBBI, Register SrcReg,
      bool isKill, int FrameIndex, const TargetRegisterClass *RC, Register VReg,
      MachineInstr::MIFlag Flags = MachineInstr::NoFlags) const override;

  void loadRegFromStackSlot(
      MachineBasicBlock &MBB, MachineBasicBlock::iterator MBBI,
      Register DestReg, int FrameIndex, const TargetRegisterClass *RC,
      Register VReg, unsigned SubReg = 0,
      MachineInstr::MIFlag Flags = MachineInstr::NoFlags) const override;

  /// getRegisterInfo - TargetInstrInfo is a superset of MRegister info.  As
  /// such, whenever a client has an instance of instruction info, it should
  /// always be able to get register info as well (through this method).
  ///
  const ThumbRegisterInfo &getRegisterInfo() const { return RI; }

  MachineInstr *optimizeSelect(MachineInstr &MI,
                               SmallPtrSetImpl<MachineInstr *> &SeenMIs,
                               bool) const override;

  MachineInstr *commuteInstructionImpl(MachineInstr &MI, bool NewMI,
                                       unsigned OpIdx1,
                                       unsigned OpIdx2) const override;

  bool isSchedulingBoundary(const MachineInstr &MI,
                            const MachineBasicBlock *MBB,
                            const MachineFunction &MF) const override;

private:
  void expandLoadStackGuard(MachineBasicBlock::iterator MI) const override;
};

/// getITInstrPredicate - Valid only in Thumb2 mode. This function is identical
/// to toolchain::getInstrPredicate except it returns AL for conditional branch
/// instructions which are "predicated", but are not in IT blocks.
ARMCC::CondCodes getITInstrPredicate(const MachineInstr &MI, Register &PredReg);

// getVPTInstrPredicate: VPT analogue of that, plus a helper function
// corresponding to MachineInstr::findFirstPredOperandIdx.
int findFirstVPTPredOperandIdx(const MachineInstr &MI);
ARMVCC::VPTCodes getVPTInstrPredicate(const MachineInstr &MI,
                                      Register &PredReg);
inline ARMVCC::VPTCodes getVPTInstrPredicate(const MachineInstr &MI) {
  Register PredReg;
  return getVPTInstrPredicate(MI, PredReg);
}
// Identify the input operand in an MVE predicated instruction which
// contributes the values of any inactive vector lanes.
int findVPTInactiveOperandIdx(const MachineInstr &MI);

// Recomputes the Block Mask of Instr, a VPT or VPST instruction.
// This rebuilds the block mask of the instruction depending on the predicates
// of the instructions following it. This should only be used after the
// MVEVPTBlockInsertion pass has run, and should be used whenever a predicated
// instruction is added to/removed from the block.
void recomputeVPTBlockMask(MachineInstr &Instr);
} // namespace vm::core

#endif
