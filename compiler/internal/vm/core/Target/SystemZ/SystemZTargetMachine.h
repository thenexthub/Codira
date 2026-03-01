//=- SystemZTargetMachine.h - Define TargetMachine for SystemZ ----*- C++ -*-=//
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
// This file declares the SystemZ specific subclass of TargetMachine.
//
//===----------------------------------------------------------------------===//


#ifndef LLVM_LIB_TARGET_SYSTEMZ_SYSTEMZTARGETMACHINE_H
#define LLVM_LIB_TARGET_SYSTEMZ_SYSTEMZTARGETMACHINE_H

#include "SystemZSubtarget.h"
#include "vm/core/ADT/StringRef.h"
#include "vm/core/Analysis/TargetTransformInfo.h"
#include "vm/core/CodeGen/CodeGenTargetMachineImpl.h"
#include "vm/core/Support/CodeGen.h"
#include "vm/core/Target/TargetMachine.h"
#include <memory>
#include <optional>

namespace vm::core {

class SystemZTargetMachine : public CodeGenTargetMachineImpl {
  std::unique_ptr<TargetLoweringObjectFile> TLOF;

  mutable StringMap<std::unique_ptr<SystemZSubtarget>> SubtargetMap;

public:
  SystemZTargetMachine(const Target &T, const Triple &TT, StringRef CPU,
                       StringRef FS, const TargetOptions &Options,
                       std::optional<Reloc::Model> RM,
                       std::optional<CodeModel::Model> CM, CodeGenOptLevel OL,
                       bool JIT);
  ~SystemZTargetMachine() override;

  const SystemZSubtarget *getSubtargetImpl(const Function &) const override;
  // DO NOT IMPLEMENT: There is no such thing as a valid default subtarget,
  // subtargets are per-function entities based on the target-specific
  // attributes of each function.
  const SystemZSubtarget *getSubtargetImpl() const = delete;

  // Override CodeGenTargetMachineImpl
  TargetPassConfig *createPassConfig(PassManagerBase &PM) override;
  TargetTransformInfo getTargetTransformInfo(const Function &F) const override;

  TargetLoweringObjectFile *getObjFileLowering() const override {
    return TLOF.get();
  }

  MachineFunctionInfo *
  createMachineFunctionInfo(BumpPtrAllocator &Allocator, const Function &F,
                            const TargetSubtargetInfo *STI) const override;
  ScheduleDAGInstrs *
  createPostMachineScheduler(MachineSchedContext *C) const override;

  bool targetSchedulesPostRAScheduling() const override { return true; };
};

} // end namespace vm::core

#endif // LLVM_LIB_TARGET_SYSTEMZ_SYSTEMZTARGETMACHINE_H
