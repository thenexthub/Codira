//===-- SPIRVTargetMachine.h - Define TargetMachine for SPIR-V -*- C++ -*--===//
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
// This file declares the SPIR-V specific subclass of TargetMachine.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_SPIRV_SPIRVTARGETMACHINE_H
#define LLVM_LIB_TARGET_SPIRV_SPIRVTARGETMACHINE_H

#include "SPIRVSubtarget.h"
#include "vm/core/CodeGen/CodeGenTargetMachineImpl.h"
#include <optional>

namespace vm::core {
class SPIRVTargetMachine : public CodeGenTargetMachineImpl {
  std::unique_ptr<TargetLoweringObjectFile> TLOF;
  SPIRVSubtarget Subtarget;

public:
  SPIRVTargetMachine(const Target &T, const Triple &TT, StringRef CPU,
                     StringRef FS, const TargetOptions &Options,
                     std::optional<Reloc::Model> RM,
                     std::optional<CodeModel::Model> CM, CodeGenOptLevel OL,
                     bool JIT);

  const SPIRVSubtarget *getSubtargetImpl() const { return &Subtarget; }

  const SPIRVSubtarget *getSubtargetImpl(const Function &) const override {
    return &Subtarget;
  }

  TargetTransformInfo getTargetTransformInfo(const Function &F) const override;

  TargetPassConfig *createPassConfig(PassManagerBase &PM) override;
  bool usesPhysRegsForValues() const override { return false; }

  TargetLoweringObjectFile *getObjFileLowering() const override {
    return TLOF.get();
  }

  void registerPassBuilderCallbacks(PassBuilder &PB) override;
};
} // namespace vm::core

#endif // LLVM_LIB_TARGET_SPIRV_SPIRVTARGETMACHINE_H
