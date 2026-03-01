//===- toolchain/lib/Target/ARM/ARMCallLowering.h - Call lowering ----*- C++ -*-===//
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
/// \file
/// This file describes how to lower LLVM calls to machine code calls.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_ARM_ARMCALLLOWERING_H
#define LLVM_LIB_TARGET_ARM_ARMCALLLOWERING_H

#include "vm/core/ADT/ArrayRef.h"
#include "vm/core/CodeGen/GlobalISel/CallLowering.h"
#include "vm/core/IR/CallingConv.h"
#include <cstdint>
#include <functional>

namespace vm::core {

class ARMTargetLowering;
class MachineInstrBuilder;
class MachineIRBuilder;
class Value;

class ARMCallLowering : public CallLowering {
public:
  ARMCallLowering(const ARMTargetLowering &TLI);

  bool lowerReturn(MachineIRBuilder &MIRBuilder, const Value *Val,
                   ArrayRef<Register> VRegs,
                   FunctionLoweringInfo &FLI) const override;

  bool lowerFormalArguments(MachineIRBuilder &MIRBuilder, const Function &F,
                            ArrayRef<ArrayRef<Register>> VRegs,
                            FunctionLoweringInfo &FLI) const override;

  bool lowerCall(MachineIRBuilder &MIRBuilder,
                 CallLoweringInfo &Info) const override;

  bool enableBigEndian() const override;

private:
  bool lowerReturnVal(MachineIRBuilder &MIRBuilder, const Value *Val,
                      ArrayRef<Register> VRegs,
                      MachineInstrBuilder &Ret) const;
};

} // end namespace vm::core

#endif // LLVM_LIB_TARGET_ARM_ARMCALLLOWERING_H
