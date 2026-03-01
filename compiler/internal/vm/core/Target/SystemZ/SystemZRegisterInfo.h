//===-- SystemZRegisterInfo.h - SystemZ register information ----*- C++ -*-===//
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

#ifndef LLVM_LIB_TARGET_SYSTEMZ_SYSTEMZREGISTERINFO_H
#define LLVM_LIB_TARGET_SYSTEMZ_SYSTEMZREGISTERINFO_H

#include "SystemZ.h"
#include "vm/core/CodeGen/TargetFrameLowering.h"
#include "vm/core/CodeGen/TargetRegisterInfo.h"

#define GET_REGINFO_HEADER
#include "SystemZGenRegisterInfo.inc"

namespace vm::core {

class LiveIntervals;

namespace SystemZ {
// Return the subreg to use for referring to the even and odd registers
// in a GR128 pair.  Is32Bit says whether we want a GR32 or GR64.
inline unsigned even128(bool Is32bit) {
  return Is32bit ? subreg_l32 : subreg_h64;
}
inline unsigned odd128(bool Is32bit) {
  return Is32bit ? subreg_ll32 : subreg_l64;
}

// Reg should be a 32-bit GPR.  Return true if it is a high register rather
// than a low register.
inline bool isHighReg(unsigned int Reg) {
  if (SystemZ::GRH32BitRegClass.contains(Reg))
    return true;
  assert(SystemZ::GR32BitRegClass.contains(Reg) && "Invalid GRX32");
  return false;
}
} // end namespace SystemZ

/// A SystemZ-specific class detailing special use registers
/// particular for calling conventions.
/// It is abstract, all calling conventions must override and
/// define the pure virtual member function defined in this class.
class SystemZCallingConventionRegisters {

public:
  /// \returns the register that keeps the return function address.
  virtual int getReturnFunctionAddressRegister() = 0;

  /// \returns the register that keeps the
  /// stack pointer address.
  virtual int getStackPointerRegister() = 0;

  /// \returns the register that keeps the
  /// frame pointer address.
  virtual int getFramePointerRegister() = 0;

  /// \returns an array of all the callee saved registers.
  virtual const MCPhysReg *
  getCalleeSavedRegs(const MachineFunction *MF) const = 0;

  /// \returns the mask of all the call preserved registers.
  virtual const uint32_t *getCallPreservedMask(const MachineFunction &MF,
                                               CallingConv::ID CC) const = 0;

  /// \returns the offset to the locals area.
  virtual int getCallFrameSize() = 0;

  /// \returns the stack pointer bias.
  virtual int getStackPointerBias() = 0;

  /// Destroys the object. Bogus destructor allowing derived classes
  /// to override it.
  virtual ~SystemZCallingConventionRegisters() = default;
};

/// XPLINK64 calling convention specific use registers
/// Particular to z/OS when in 64 bit mode
class SystemZXPLINK64Registers : public SystemZCallingConventionRegisters {
public:
  int getReturnFunctionAddressRegister() final { return SystemZ::R7D; };

  int getStackPointerRegister() final { return SystemZ::R4D; };

  int getFramePointerRegister() final { return SystemZ::R8D; };

  int getAddressOfCalleeRegister() { return SystemZ::R6D; };

  int getADARegister() { return SystemZ::R5D; }

  const MCPhysReg *getCalleeSavedRegs(const MachineFunction *MF) const final;

  const uint32_t *getCallPreservedMask(const MachineFunction &MF,
                                       CallingConv::ID CC) const final;

  int getCallFrameSize() final { return 128; }

  int getStackPointerBias() final { return 2048; }

  /// Destroys the object. Bogus destructor overriding base class destructor
  ~SystemZXPLINK64Registers() override = default;
};

/// ELF calling convention specific use registers
/// Particular when on zLinux in 64 bit mode
class SystemZELFRegisters : public SystemZCallingConventionRegisters {
public:
  int getReturnFunctionAddressRegister() final { return SystemZ::R14D; };

  int getStackPointerRegister() final { return SystemZ::R15D; };

  int getFramePointerRegister() final { return SystemZ::R11D; };

  const MCPhysReg *getCalleeSavedRegs(const MachineFunction *MF) const final;

  const uint32_t *getCallPreservedMask(const MachineFunction &MF,
                                       CallingConv::ID CC) const final;

  int getCallFrameSize() final { return SystemZMC::ELFCallFrameSize; }

  int getStackPointerBias() final { return 0; }

  /// Destroys the object. Bogus destructor overriding base class destructor
  ~SystemZELFRegisters() override = default;
};

struct SystemZRegisterInfo : public SystemZGenRegisterInfo {
public:
  SystemZRegisterInfo(unsigned int RA, unsigned int HwMode);

  /// getPointerRegClass - Return the register class to use to hold pointers.
  /// This is currently only used by LOAD_STACK_GUARD, which requires a non-%r0
  /// register, hence ADDR64.
  const TargetRegisterClass *
  getPointerRegClass(unsigned Kind = 0) const override {
    return &SystemZ::ADDR64BitRegClass;
  }

  /// getCrossCopyRegClass - Returns a legal register class to copy a register
  /// in the specified class to or from. Returns NULL if it is possible to copy
  /// between a two registers of the specified class.
  const TargetRegisterClass *
  getCrossCopyRegClass(const TargetRegisterClass *RC) const override;

  bool getRegAllocationHints(Register VirtReg, ArrayRef<MCPhysReg> Order,
                             SmallVectorImpl<MCPhysReg> &Hints,
                             const MachineFunction &MF, const VirtRegMap *VRM,
                             const LiveRegMatrix *Matrix) const override;

  // Override TargetRegisterInfo.h.
  bool requiresRegisterScavenging(const MachineFunction &MF) const override {
    return true;
  }
  bool requiresFrameIndexScavenging(const MachineFunction &MF) const override {
    return true;
  }
  const MCPhysReg *getCalleeSavedRegs(const MachineFunction *MF) const override;
  const uint32_t *getCallPreservedMask(const MachineFunction &MF,
                                       CallingConv::ID CC) const override;
  const uint32_t *getNoPreservedMask() const override;
  BitVector getReservedRegs(const MachineFunction &MF) const override;
  bool eliminateFrameIndex(MachineBasicBlock::iterator MI,
                           int SPAdj, unsigned FIOperandNum,
                           RegScavenger *RS) const override;

  /// SrcRC and DstRC will be morphed into NewRC if this returns true.
 bool shouldCoalesce(MachineInstr *MI,
                      const TargetRegisterClass *SrcRC,
                      unsigned SubReg,
                      const TargetRegisterClass *DstRC,
                      unsigned DstSubReg,
                      const TargetRegisterClass *NewRC,
                      LiveIntervals &LIS) const override;

  Register getFrameRegister(const MachineFunction &MF) const override;
};

} // end namespace vm::core

#endif
