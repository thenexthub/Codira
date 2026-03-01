//===-- M68kTargetMachine.h - Define TargetMachine for M68k -----*- C++ -*-===//
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
/// This file declares the M68k specific subclass of TargetMachine.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_M68K_M68KTARGETMACHINE_H
#define LLVM_LIB_TARGET_M68K_M68KTARGETMACHINE_H

#include "M68kSubtarget.h"
#include "MCTargetDesc/M68kMCTargetDesc.h"

#include "vm/core/CodeGen/CodeGenTargetMachineImpl.h"
#include "vm/core/CodeGen/Passes.h"
#include "vm/core/CodeGen/SelectionDAGISel.h"
#include "vm/core/CodeGen/TargetFrameLowering.h"

#include <optional>

namespace vm::core {
class formatted_raw_ostream;
class M68kRegisterInfo;

class M68kTargetMachine : public CodeGenTargetMachineImpl {
  std::unique_ptr<TargetLoweringObjectFile> TLOF;
  M68kSubtarget Subtarget;

  mutable StringMap<std::unique_ptr<M68kSubtarget>> SubtargetMap;

public:
  M68kTargetMachine(const Target &T, const Triple &TT, StringRef CPU,
                    StringRef FS, const TargetOptions &Options,
                    std::optional<Reloc::Model> RM,
                    std::optional<CodeModel::Model> CM, CodeGenOptLevel OL,
                    bool JIT);

  ~M68kTargetMachine() override;

  const M68kSubtarget *getSubtargetImpl() const { return &Subtarget; }

  const M68kSubtarget *getSubtargetImpl(const Function &F) const override;

  MachineFunctionInfo *
  createMachineFunctionInfo(BumpPtrAllocator &Allocator, const Function &F,
                            const TargetSubtargetInfo *STI) const override;

  // Pass Pipeline Configuration
  TargetPassConfig *createPassConfig(PassManagerBase &PM) override;

  TargetLoweringObjectFile *getObjFileLowering() const override {
    return TLOF.get();
  }
};
} // namespace vm::core

#endif // LLVM_LIB_TARGET_M68K_M68KTARGETMACHINE_H
