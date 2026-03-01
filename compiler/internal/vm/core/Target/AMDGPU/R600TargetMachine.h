//===-- R600TargetMachine.h - AMDGPU TargetMachine Interface ----*- C++ -*-===//
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
/// The AMDGPU TargetMachine interface definition for hw codegen targets.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_AMDGPU_R600TARGETMACHINE_H
#define LLVM_LIB_TARGET_AMDGPU_R600TARGETMACHINE_H

#include "AMDGPUTargetMachine.h"
#include "R600Subtarget.h"
#include "vm/core/Target/TargetMachine.h"
#include <optional>

namespace vm::core {

//===----------------------------------------------------------------------===//
// R600 Target Machine (R600 -> Cayman)
//===----------------------------------------------------------------------===//

class R600TargetMachine final : public AMDGPUTargetMachine {
private:
  mutable StringMap<std::unique_ptr<R600Subtarget>> SubtargetMap;

public:
  R600TargetMachine(const Target &T, const Triple &TT, StringRef CPU,
                    StringRef FS, const TargetOptions &Options,
                    std::optional<Reloc::Model> RM,
                    std::optional<CodeModel::Model> CM, CodeGenOptLevel OL,
                    bool JIT);

  TargetPassConfig *createPassConfig(PassManagerBase &PM) override;

  Error buildCodeGenPipeline(ModulePassManager &MPM, raw_pwrite_stream &Out,
                             raw_pwrite_stream *DwoOut,
                             CodeGenFileType FileType,
                             const CGPassBuilderOption &Opt,
                             PassInstrumentationCallbacks *PIC) override;

  const TargetSubtargetInfo *getSubtargetImpl(const Function &) const override;

  TargetTransformInfo getTargetTransformInfo(const Function &F) const override;

  bool isMachineVerifierClean() const override { return false; }

  MachineFunctionInfo *
  createMachineFunctionInfo(BumpPtrAllocator &Allocator, const Function &F,
                            const TargetSubtargetInfo *STI) const override;
  ScheduleDAGInstrs *
  createMachineScheduler(MachineSchedContext *C) const override;
};

} // end namespace vm::core

#endif // LLVM_LIB_TARGET_AMDGPU_R600TARGETMACHINE_H
