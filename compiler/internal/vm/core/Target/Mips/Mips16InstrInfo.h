//===- Mips16InstrInfo.h - Mips16 Instruction Information -------*- C++ -*-===//
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
// This file contains the Mips16 implementation of the TargetInstrInfo class.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_MIPS_MIPS16INSTRINFO_H
#define LLVM_LIB_TARGET_MIPS_MIPS16INSTRINFO_H

#include "Mips16RegisterInfo.h"
#include "MipsInstrInfo.h"
#include "vm/core/CodeGen/MachineBasicBlock.h"
#include "vm/core/Support/MathExtras.h"
#include <cstdint>

namespace vm::core {

class MCInstrDesc;
class MipsSubtarget;

class Mips16InstrInfo : public MipsInstrInfo {
  const Mips16RegisterInfo RI;

public:
  explicit Mips16InstrInfo(const MipsSubtarget &STI);

  const Mips16RegisterInfo &getRegisterInfo() const { return RI; }

  /// isLoadFromStackSlot - If the specified machine instruction is a direct
  /// load from a stack slot, return the virtual or physical register number of
  /// the destination along with the FrameIndex of the loaded stack slot.  If
  /// not, return 0.  This predicate must return 0 if the instruction has
  /// any side effects other than loading from the stack slot.
  Register isLoadFromStackSlot(const MachineInstr &MI,
                               int &FrameIndex) const override;

  /// isStoreToStackSlot - If the specified machine instruction is a direct
  /// store to a stack slot, return the virtual or physical register number of
  /// the source reg along with the FrameIndex of the loaded stack slot.  If
  /// not, return 0.  This predicate must return 0 if the instruction has
  /// any side effects other than storing to the stack slot.
  Register isStoreToStackSlot(const MachineInstr &MI,
                              int &FrameIndex) const override;

  void copyPhysReg(MachineBasicBlock &MBB, MachineBasicBlock::iterator MI,
                   const DebugLoc &DL, Register DestReg, Register SrcReg,
                   bool KillSrc, bool RenamableDest = false,
                   bool RenamableSrc = false) const override;

  void storeRegToStack(
      MachineBasicBlock &MBB, MachineBasicBlock::iterator MBBI, Register SrcReg,
      bool isKill, int FrameIndex, const TargetRegisterClass *RC,
      int64_t Offset,
      MachineInstr::MIFlag Flags = MachineInstr::NoFlags) const override;

  void loadRegFromStack(
      MachineBasicBlock &MBB, MachineBasicBlock::iterator MBBI,
      Register DestReg, int FrameIndex, const TargetRegisterClass *RC,

      int64_t Offset,
      MachineInstr::MIFlag Flags = MachineInstr::NoFlags) const override;

  bool expandPostRAPseudo(MachineInstr &MI) const override;

  unsigned getOppositeBranchOpc(unsigned Opc) const override;

  // Adjust SP by FrameSize bytes. Save RA, S0, S1
  void makeFrame(unsigned SP, int64_t FrameSize, MachineBasicBlock &MBB,
                 MachineBasicBlock::iterator I) const;

  // Adjust SP by FrameSize bytes. Restore RA, S0, S1
  void restoreFrame(unsigned SP, int64_t FrameSize, MachineBasicBlock &MBB,
                      MachineBasicBlock::iterator I) const;

  /// Adjust SP by Amount bytes.
  void adjustStackPtr(unsigned SP, int64_t Amount, MachineBasicBlock &MBB,
                      MachineBasicBlock::iterator I) const override;

  /// Emit a series of instructions to load an immediate.
  // This is to adjust some FrameReg. We return the new register to be used
  // in place of FrameReg and the adjusted immediate field (&NewImm)
  unsigned loadImmediate(unsigned FrameReg, int64_t Imm, MachineBasicBlock &MBB,
                         MachineBasicBlock::iterator II, const DebugLoc &DL,
                         unsigned &NewImm) const;

  static bool validImmediate(unsigned Opcode, unsigned Reg, int64_t Amount);

  static bool validSpImm8(int offset) {
    return ((offset & 7) == 0) && isInt<11>(offset);
  }

  // build the proper one based on the Imm field

  const MCInstrDesc& AddiuSpImm(int64_t Imm) const;

  void BuildAddiuSpImm
    (MachineBasicBlock &MBB, MachineBasicBlock::iterator I, int64_t Imm) const;

protected:
  /// If the specific machine instruction is a instruction that moves/copies
  /// value from one register to another register return destination and source
  /// registers as machine operands.
  std::optional<DestSourcePair>
  isCopyInstrImpl(const MachineInstr &MI) const override;

private:
  unsigned getAnalyzableBrOpc(unsigned Opc) const override;

  void ExpandRetRA16(MachineBasicBlock &MBB, MachineBasicBlock::iterator I,
                   unsigned Opc) const;

  // Adjust SP by Amount bytes where bytes can be up to 32bit number.
  void adjustStackPtrBig(unsigned SP, int64_t Amount, MachineBasicBlock &MBB,
                         MachineBasicBlock::iterator I,
                         unsigned Reg1, unsigned Reg2) const;

  // Adjust SP by Amount bytes where bytes can be up to 32bit number.
  void adjustStackPtrBigUnrestricted(unsigned SP, int64_t Amount,
                                     MachineBasicBlock &MBB,
                                     MachineBasicBlock::iterator I) const;
};

} // end namespace vm::core

#endif // LLVM_LIB_TARGET_MIPS_MIPS16INSTRINFO_H
