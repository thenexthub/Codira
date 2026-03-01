//===- lib/Target/AMDGPU/AMDGPUCallLowering.h - Call lowering -*- C++ -*---===//
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

#ifndef LLVM_LIB_TARGET_AMDGPU_AMDGPUCALLLOWERING_H
#define LLVM_LIB_TARGET_AMDGPU_AMDGPUCALLLOWERING_H

#include "vm/core/CodeGen/GlobalISel/CallLowering.h"

namespace vm::core {

class AMDGPUTargetLowering;
class GCNSubtarget;
class MachineInstrBuilder;
class SIMachineFunctionInfo;

class AMDGPUCallLowering final : public CallLowering {
  void lowerParameterPtr(Register DstReg, MachineIRBuilder &B,
                         uint64_t Offset) const;

  void lowerParameter(MachineIRBuilder &B, ArgInfo &AI, uint64_t Offset,
                      Align Alignment) const;

  bool canLowerReturn(MachineFunction &MF, CallingConv::ID CallConv,
                      SmallVectorImpl<BaseArgInfo> &Outs,
                      bool IsVarArg) const override;

  bool lowerReturnVal(MachineIRBuilder &B, const Value *Val,
                      ArrayRef<Register> VRegs, MachineInstrBuilder &Ret) const;

  void addOriginalExecToReturn(MachineFunction &MF,
                               MachineInstrBuilder &Ret) const;

public:
  AMDGPUCallLowering(const AMDGPUTargetLowering &TLI);

  bool lowerReturn(MachineIRBuilder &B, const Value *Val,
                   ArrayRef<Register> VRegs,
                   FunctionLoweringInfo &FLI) const override;

  bool lowerFormalArgumentsKernel(MachineIRBuilder &B, const Function &F,
                                  ArrayRef<ArrayRef<Register>> VRegs) const;

  bool lowerFormalArguments(MachineIRBuilder &B, const Function &F,
                            ArrayRef<ArrayRef<Register>> VRegs,
                            FunctionLoweringInfo &FLI) const override;

  bool passSpecialInputs(MachineIRBuilder &MIRBuilder,
                         CCState &CCInfo,
                         SmallVectorImpl<std::pair<MCRegister, Register>> &ArgRegs,
                         CallLoweringInfo &Info) const;

  bool
  doCallerAndCalleePassArgsTheSameWay(CallLoweringInfo &Info,
                                      MachineFunction &MF,
                                      SmallVectorImpl<ArgInfo> &InArgs) const;

  bool
  areCalleeOutgoingArgsTailCallable(CallLoweringInfo &Info, MachineFunction &MF,
                                    SmallVectorImpl<ArgInfo> &OutArgs) const;

  /// Returns true if the call can be lowered as a tail call.
  bool
  isEligibleForTailCallOptimization(MachineIRBuilder &MIRBuilder,
                                    CallLoweringInfo &Info,
                                    SmallVectorImpl<ArgInfo> &InArgs,
                                    SmallVectorImpl<ArgInfo> &OutArgs) const;

  void handleImplicitCallArguments(
      MachineIRBuilder &MIRBuilder, MachineInstrBuilder &CallInst,
      const GCNSubtarget &ST, const SIMachineFunctionInfo &MFI,
      CallingConv::ID CalleeCC,
      ArrayRef<std::pair<MCRegister, Register>> ImplicitArgRegs) const;

  bool lowerTailCall(MachineIRBuilder &MIRBuilder, CallLoweringInfo &Info,
                     SmallVectorImpl<ArgInfo> &OutArgs) const;
  bool lowerChainCall(MachineIRBuilder &MIRBuilder,
                      CallLoweringInfo &Info) const;
  bool lowerCall(MachineIRBuilder &MIRBuilder,
                 CallLoweringInfo &Info) const override;

  static CCAssignFn *CCAssignFnForCall(CallingConv::ID CC, bool IsVarArg);
  static CCAssignFn *CCAssignFnForReturn(CallingConv::ID CC, bool IsVarArg);
};
} // End of namespace vm::core;
#endif
