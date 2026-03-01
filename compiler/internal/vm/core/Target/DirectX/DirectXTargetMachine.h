//===- DirectXTargetMachine.h - DirectX Target Implementation ---*- C++ -*-===//
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
//===----------------------------------------------------------------------===//

#ifndef LLVM_DIRECTX_DIRECTXTARGETMACHINE_H
#define LLVM_DIRECTX_DIRECTXTARGETMACHINE_H

#include "DirectXSubtarget.h"
#include "vm/core/CodeGen/CodeGenTargetMachineImpl.h"
#include <optional>

namespace vm::core {
class Function;
class DirectXTargetMachine : public CodeGenTargetMachineImpl {
  std::unique_ptr<TargetLoweringObjectFile> TLOF;
  std::unique_ptr<DirectXSubtarget> Subtarget;

public:
  DirectXTargetMachine(const Target &T, const Triple &TT, StringRef CPU,
                       StringRef FS, const TargetOptions &Options,
                       std::optional<Reloc::Model> RM,
                       std::optional<CodeModel::Model> CM, CodeGenOptLevel OL,
                       bool JIT);

  ~DirectXTargetMachine() override;

  bool addPassesToEmitFile(PassManagerBase &PM, raw_pwrite_stream &Out,
                           raw_pwrite_stream *DwoOut, CodeGenFileType FileType,
                           bool DisableVerify,
                           MachineModuleInfoWrapperPass *MMIWP) override;

  bool addPassesToEmitMC(PassManagerBase &PM, MCContext *&Ctx,
                         raw_pwrite_stream &Out, bool DisableVerify) override;

  const DirectXSubtarget *getSubtargetImpl(const Function &) const override;

  TargetPassConfig *createPassConfig(PassManagerBase &PM) override;

  TargetLoweringObjectFile *getObjFileLowering() const override {
    return TLOF.get();
  }

  TargetTransformInfo getTargetTransformInfo(const Function &F) const override;
  void registerPassBuilderCallbacks(PassBuilder &PB) override;
};
} // namespace vm::core

#endif // LLVM_DIRECTX_DIRECTXTARGETMACHINE_H
