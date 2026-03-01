//===-- SPIRVISelLowering.h - SPIR-V DAG Lowering Interface -----*- C++ -*-===//
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
// This file defines the interfaces that SPIR-V uses to lower LLVM code into a
// selection DAG.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_SPIRV_SPIRVISELLOWERING_H
#define LLVM_LIB_TARGET_SPIRV_SPIRVISELLOWERING_H

#include "SPIRVGlobalRegistry.h"
#include "vm/core/CodeGen/TargetLowering.h"
#include <set>

namespace vm::core {
class SPIRVSubtarget;

class SPIRVTargetLowering : public TargetLowering {
  const SPIRVSubtarget &STI;

  // Record of already processed machine functions
  mutable std::set<const MachineFunction *> ProcessedMF;

public:
  explicit SPIRVTargetLowering(const TargetMachine &TM,
                               const SPIRVSubtarget &ST);

  // Stop IRTranslator breaking up FMA instrs to preserve types information.
  bool isFMAFasterThanFMulAndFAdd(const MachineFunction &MF,
                                  EVT) const override {
    return true;
  }

  // prevent creation of jump tables
  bool areJTsAllowed(const Function *) const override { return false; }

  // This is to prevent sexts of non-i64 vector indices which are generated
  // within general IRTranslator hence type generation for it is omitted.
  unsigned getVectorIdxWidth(const DataLayout &DL) const override { return 32; }
  unsigned getNumRegistersForCallingConv(LLVMContext &Context,
                                         CallingConv::ID CC,
                                         EVT VT) const override;
  MVT getRegisterTypeForCallingConv(LLVMContext &Context, CallingConv::ID CC,
                                    EVT VT) const override;
  bool getTgtMemIntrinsic(IntrinsicInfo &Info, const CallBase &I,
                          MachineFunction &MF,
                          unsigned Intrinsic) const override;

  std::pair<unsigned, const TargetRegisterClass *>
  getRegForInlineAsmConstraint(const TargetRegisterInfo *TRI,
                               StringRef Constraint, MVT VT) const override;
  unsigned
  getNumRegisters(LLVMContext &Context, EVT VT,
                  std::optional<MVT> RegisterVT = std::nullopt) const override {
    return 1;
  }

  // Call the default implementation and finalize target lowering by inserting
  // extra instructions required to preserve validity of SPIR-V code imposed by
  // the standard.
  void finalizeLowering(MachineFunction &MF) const override;

  MVT getPreferredSwitchConditionType(LLVMContext &Context,
                                      EVT ConditionVT) const override {
    return ConditionVT.getSimpleVT();
  }

  bool enforcePtrTypeCompatibility(MachineInstr &I, unsigned PtrOpIdx,
                                   unsigned OpIdx) const;
  bool insertLogicalCopyOnResult(MachineInstr &I,
                                 SPIRVType *NewResultType) const;
};
} // namespace vm::core

#endif // LLVM_LIB_TARGET_SPIRV_SPIRVISELLOWERING_H
