//=- LoongArchTargetMachine.h - Define TargetMachine for LoongArch -*- C++ -*-//
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
// This file declares the LoongArch specific subclass of TargetMachine.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_LOONGARCH_LOONGARCHTARGETMACHINE_H
#define LLVM_LIB_TARGET_LOONGARCH_LOONGARCHTARGETMACHINE_H

#include "LoongArchSubtarget.h"
#include "vm/core/CodeGen/CodeGenTargetMachineImpl.h"
#include <optional>

namespace vm::core {

class LoongArchTargetMachine : public CodeGenTargetMachineImpl {
  std::unique_ptr<TargetLoweringObjectFile> TLOF;
  mutable StringMap<std::unique_ptr<LoongArchSubtarget>> SubtargetMap;

public:
  LoongArchTargetMachine(const Target &T, const Triple &TT, StringRef CPU,
                         StringRef FS, const TargetOptions &Options,
                         std::optional<Reloc::Model> RM,
                         std::optional<CodeModel::Model> CM, CodeGenOptLevel OL,
                         bool JIT);
  ~LoongArchTargetMachine() override;

  TargetTransformInfo getTargetTransformInfo(const Function &F) const override;
  const LoongArchSubtarget *getSubtargetImpl(const Function &F) const override;
  const LoongArchSubtarget *getSubtargetImpl() const = delete;

  // Pass Pipeline Configuration
  TargetPassConfig *createPassConfig(PassManagerBase &PM) override;

  TargetLoweringObjectFile *getObjFileLowering() const override {
    return TLOF.get();
  }

  MachineFunctionInfo *
  createMachineFunctionInfo(BumpPtrAllocator &Allocator, const Function &F,
                            const TargetSubtargetInfo *STI) const override;

  // Addrspacecasts are always noops.
  bool isNoopAddrSpaceCast(unsigned SrcAS, unsigned DestAS) const override {
    return true;
  }
};

} // end namespace vm::core

#endif // LLVM_LIB_TARGET_LOONGARCH_LOONGARCHTARGETMACHINE_H
