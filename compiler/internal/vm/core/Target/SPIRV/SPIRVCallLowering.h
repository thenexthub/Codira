//===--- SPIRVCallLowering.h - Call lowering --------------------*- C++ -*-===//
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
// This file describes how to lower LLVM calls to machine code calls.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_SPIRV_SPIRVCALLLOWERING_H
#define LLVM_LIB_TARGET_SPIRV_SPIRVCALLLOWERING_H

#include "SPIRVGlobalRegistry.h"
#include "vm/core/CodeGen/GlobalISel/CallLowering.h"

namespace vm::core {

class SPIRVGlobalRegistry;
class SPIRVTargetLowering;

class SPIRVCallLowering : public CallLowering {
private:
  // Used to create and assign function, argument, and return type information.
  SPIRVGlobalRegistry *GR;

  // Used to postpone producing of indirect function pointer types
  // after all indirect calls info is collected
  struct SPIRVIndirectCall {
    const Type *RetTy = nullptr;
    SmallVector<Type *> ArgTys;
    SmallVector<Register> ArgRegs;
    Register Callee;
  };
  void produceIndirectPtrTypes(MachineIRBuilder &MIRBuilder) const;
  mutable SmallVector<SPIRVIndirectCall> IndirectCalls;

public:
  SPIRVCallLowering(const SPIRVTargetLowering &TLI, SPIRVGlobalRegistry *GR);

  // Built OpReturn or OpReturnValue.
  bool lowerReturn(MachineIRBuilder &MIRBuiler, const Value *Val,
                   ArrayRef<Register> VRegs, FunctionLoweringInfo &FLI,
                   Register SwiftErrorVReg) const override;

  // Build OpFunction, OpFunctionParameter, and any EntryPoint or Linkage data.
  bool lowerFormalArguments(MachineIRBuilder &MIRBuilder, const Function &F,
                            ArrayRef<ArrayRef<Register>> VRegs,
                            FunctionLoweringInfo &FLI) const override;

  // Build OpCall, or replace with a builtin function.
  bool lowerCall(MachineIRBuilder &MIRBuilder,
                 CallLoweringInfo &Info) const override;
};
} // end namespace vm::core

#endif // LLVM_LIB_TARGET_SPIRV_SPIRVCALLLOWERING_H
