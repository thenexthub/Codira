//===- MipsMachineFunctionInfo.h - Private data used for Mips ---*- C++ -*-===//
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
// This file declares the Mips specific subclass of MachineFunctionInfo.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_MIPS_MIPSMACHINEFUNCTION_H
#define LLVM_LIB_TARGET_MIPS_MIPSMACHINEFUNCTION_H

#include "Mips16HardFloatInfo.h"
#include "vm/core/CodeGen/MachineFunction.h"
#include "vm/core/CodeGen/MachineMemOperand.h"
#include <map>

namespace vm::core {

/// MipsFunctionInfo - This class is derived from MachineFunction private
/// Mips target-specific information for each MachineFunction.
class MipsFunctionInfo : public MachineFunctionInfo {
public:
  MipsFunctionInfo(const Function &F, const TargetSubtargetInfo *STI) {}

  MachineFunctionInfo *
  clone(BumpPtrAllocator &Allocator, MachineFunction &DestMF,
        const DenseMap<MachineBasicBlock *, MachineBasicBlock *> &Src2DstMBB)
      const override;

  ~MipsFunctionInfo() override;

  unsigned getSRetReturnReg() const { return SRetReturnReg; }
  void setSRetReturnReg(unsigned Reg) { SRetReturnReg = Reg; }

  bool globalBaseRegSet() const;
  Register getGlobalBaseReg(MachineFunction &MF);
  Register getGlobalBaseRegForGlobalISel(MachineFunction &MF);

  // Insert instructions to initialize the global base register in the
  // first MBB of the function.
  void initGlobalBaseReg(MachineFunction &MF);

  int getVarArgsFrameIndex() const { return VarArgsFrameIndex; }
  void setVarArgsFrameIndex(int Index) { VarArgsFrameIndex = Index; }

  bool hasByvalArg() const { return HasByvalArg; }
  void setFormalArgInfo(unsigned Size, bool HasByval) {
    IncomingArgSize = Size;
    HasByvalArg = HasByval;
  }

  unsigned getIncomingArgSize() const { return IncomingArgSize; }

  bool callsEhReturn() const { return CallsEhReturn; }
  void setCallsEhReturn() { CallsEhReturn = true; }

  void createEhDataRegsFI(MachineFunction &MF);
  int getEhDataRegFI(unsigned Reg) const { return EhDataRegFI[Reg]; }
  bool isEhDataRegFI(int FI) const;

  /// Create a MachinePointerInfo that has an ExternalSymbolPseudoSourceValue
  /// object representing a GOT entry for an external function.
  MachinePointerInfo callPtrInfo(MachineFunction &MF, const char *ES);

  // Functions with the "interrupt" attribute require special prologues,
  // epilogues and additional spill slots.
  bool isISR() const { return IsISR; }
  void setISR() { IsISR = true; }
  void createISRRegFI(MachineFunction &MF);
  int getISRRegFI(Register Reg) const { return ISRDataRegFI[Reg]; }
  bool isISRRegFI(int FI) const;

  /// Create a MachinePointerInfo that has a GlobalValuePseudoSourceValue object
  /// representing a GOT entry for a global function.
  MachinePointerInfo callPtrInfo(MachineFunction &MF, const GlobalValue *GV);

  void setSaveS2() { SaveS2 = true; }
  bool hasSaveS2() const { return SaveS2; }

  int getMoveF64ViaSpillFI(MachineFunction &MF, const TargetRegisterClass *RC);

  std::map<const char *, const Mips16HardFloatInfo::FuncSignature *>
  StubsNeeded;

private:
  virtual void anchor();

  /// SRetReturnReg - Some subtargets require that sret lowering includes
  /// returning the value of the returned struct in a register. This field
  /// holds the virtual register into which the sret argument is passed.
  Register SRetReturnReg;

  /// GlobalBaseReg - keeps track of the virtual register initialized for
  /// use as the global base register. This is used for PIC in some PIC
  /// relocation models.
  Register GlobalBaseReg;

  /// VarArgsFrameIndex - FrameIndex for start of varargs area.
  int VarArgsFrameIndex = 0;

  /// True if function has a byval argument.
  bool HasByvalArg;

  /// Size of incoming argument area.
  unsigned IncomingArgSize;

  /// CallsEhReturn - Whether the function calls toolchain.eh.return.
  bool CallsEhReturn = false;

  /// Frame objects for spilling eh data registers.
  int EhDataRegFI[4];

  /// ISR - Whether the function is an Interrupt Service Routine.
  bool IsISR = false;

  /// Frame objects for spilling C0_STATUS, C0_EPC
  int ISRDataRegFI[2];

  // saveS2
  bool SaveS2 = false;

  /// FrameIndex for expanding BuildPairF64 nodes to spill and reload when the
  /// O32 FPXX ABI is enabled. -1 is used to denote invalid index.
  int MoveF64ViaSpillFI = -1;
};

} // end namespace vm::core

#endif // LLVM_LIB_TARGET_MIPS_MIPSMACHINEFUNCTION_H
