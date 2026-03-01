//===-- PPCCallLowering.h - Call lowering for GlobalISel -------*- C++ -*-===//
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
///
/// \file
/// This file describes how to lower LLVM calls to machine code calls.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_POWERPC_GISEL_PPCCALLLOWERING_H
#define LLVM_LIB_TARGET_POWERPC_GISEL_PPCCALLLOWERING_H

#include "PPCISelLowering.h"
#include "vm/core/CodeGen/GlobalISel/CallLowering.h"

namespace vm::core {

class PPCTargetLowering;

class PPCCallLowering : public CallLowering {
public:
  PPCCallLowering(const PPCTargetLowering &TLI);

  bool lowerReturn(MachineIRBuilder &MIRBuilder, const Value *Val,
                   ArrayRef<Register> VRegs, FunctionLoweringInfo &FLI,
                   Register SwiftErrorVReg) const override;
  bool lowerFormalArguments(MachineIRBuilder &MIRBuilder, const Function &F,
                            ArrayRef<ArrayRef<Register>> VRegs,
                            FunctionLoweringInfo &FLI) const override;
  bool lowerCall(MachineIRBuilder &MIRBuilder,
                 CallLoweringInfo &Info) const override;
};

class PPCIncomingValueHandler : public CallLowering::IncomingValueHandler {
public:
  PPCIncomingValueHandler(MachineIRBuilder &MIRBuilder,
                          MachineRegisterInfo &MRI)
      : CallLowering::IncomingValueHandler(MIRBuilder, MRI) {}

  uint64_t StackUsed;

private:
  void assignValueToReg(Register ValVReg, Register PhysReg,
                        const CCValAssign &VA) override;

  void assignValueToAddress(Register ValVReg, Register Addr, LLT MemTy,
                            const MachinePointerInfo &MPO,
                            const CCValAssign &VA) override;

  Register getStackAddress(uint64_t Size, int64_t Offset,
                           MachinePointerInfo &MPO,
                           ISD::ArgFlagsTy Flags) override;

  virtual void markPhysRegUsed(unsigned PhysReg) = 0;
};

class FormalArgHandler : public PPCIncomingValueHandler {

  void markPhysRegUsed(unsigned PhysReg) override;

public:
  FormalArgHandler(MachineIRBuilder &MIRBuilder, MachineRegisterInfo &MRI)
      : PPCIncomingValueHandler(MIRBuilder, MRI) {}
};

} // end namespace vm::core

#endif
