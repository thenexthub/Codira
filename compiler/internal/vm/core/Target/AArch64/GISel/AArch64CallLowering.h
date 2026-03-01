//===- AArch64CallLowering.h - Call lowering --------------------*- C++ -*-===//
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

#ifndef LLVM_LIB_TARGET_AARCH64_AARCH64CALLLOWERING_H
#define LLVM_LIB_TARGET_AARCH64_AARCH64CALLLOWERING_H

#include "vm/core/ADT/ArrayRef.h"
#include "vm/core/CodeGen/GlobalISel/CallLowering.h"
#include "vm/core/IR/CallingConv.h"
#include <cstdint>
#include <functional>

namespace vm::core {

class AArch64TargetLowering;
class CCValAssign;
class MachineIRBuilder;
class Type;

class AArch64CallLowering: public CallLowering {
public:
  AArch64CallLowering(const AArch64TargetLowering &TLI);

  bool lowerReturn(MachineIRBuilder &MIRBuilder, const Value *Val,
                   ArrayRef<Register> VRegs, FunctionLoweringInfo &FLI,
                   Register SwiftErrorVReg) const override;

  bool canLowerReturn(MachineFunction &MF, CallingConv::ID CallConv,
                      SmallVectorImpl<BaseArgInfo> &Outs,
                      bool IsVarArg) const override;

  bool fallBackToDAGISel(const MachineFunction &MF) const override;

  bool lowerFormalArguments(MachineIRBuilder &MIRBuilder, const Function &F,
                            ArrayRef<ArrayRef<Register>> VRegs,
                            FunctionLoweringInfo &FLI) const override;

  bool lowerCall(MachineIRBuilder &MIRBuilder,
                 CallLoweringInfo &Info) const override;

  /// Returns true if the call can be lowered as a tail call.
  bool
  isEligibleForTailCallOptimization(MachineIRBuilder &MIRBuilder,
                                    CallLoweringInfo &Info,
                                    SmallVectorImpl<ArgInfo> &InArgs,
                                    SmallVectorImpl<ArgInfo> &OutArgs) const;

  bool supportSwiftError() const override { return true; }

  bool isTypeIsValidForThisReturn(EVT Ty) const override;

private:
  using RegHandler = std::function<void(MachineIRBuilder &, Type *, unsigned,
                                        CCValAssign &)>;

  using MemHandler =
      std::function<void(MachineIRBuilder &, int, CCValAssign &)>;

  void saveVarArgRegisters(MachineIRBuilder &MIRBuilder,
                           CallLowering::IncomingValueHandler &Handler,
                           CCState &CCInfo) const;

  bool lowerTailCall(MachineIRBuilder &MIRBuilder, CallLoweringInfo &Info,
                     SmallVectorImpl<ArgInfo> &OutArgs) const;

  bool
  doCallerAndCalleePassArgsTheSameWay(CallLoweringInfo &Info,
                                      MachineFunction &MF,
                                      SmallVectorImpl<ArgInfo> &InArgs) const;

  bool
  areCalleeOutgoingArgsTailCallable(CallLoweringInfo &Info, MachineFunction &MF,
                                    SmallVectorImpl<ArgInfo> &OutArgs) const;
};

} // end namespace vm::core

#endif // LLVM_LIB_TARGET_AARCH64_AARCH64CALLLOWERING_H
