//=-- HexagonTargetMachine.h - Define TargetMachine for Hexagon ---*- C++ -*-=//
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
// This file declares the Hexagon specific subclass of TargetMachine.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_HEXAGON_HEXAGONTARGETMACHINE_H
#define LLVM_LIB_TARGET_HEXAGON_HEXAGONTARGETMACHINE_H

#include "HexagonInstrInfo.h"
#include "HexagonSubtarget.h"
#include "HexagonTargetObjectFile.h"
#include "vm/core/CodeGen/CodeGenTargetMachineImpl.h"
#include <optional>

namespace vm::core {

class HexagonTargetMachine : public CodeGenTargetMachineImpl {
  std::unique_ptr<TargetLoweringObjectFile> TLOF;
  HexagonSubtarget Subtarget;
  mutable StringMap<std::unique_ptr<HexagonSubtarget>> SubtargetMap;

public:
  HexagonTargetMachine(const Target &T, const Triple &TT, StringRef CPU,
                       StringRef FS, const TargetOptions &Options,
                       std::optional<Reloc::Model> RM,
                       std::optional<CodeModel::Model> CM, CodeGenOptLevel OL,
                       bool JIT);
  ~HexagonTargetMachine() override;
  const HexagonSubtarget *getSubtargetImpl(const Function &F) const override;

  void registerPassBuilderCallbacks(PassBuilder &PB) override;
  TargetPassConfig *createPassConfig(PassManagerBase &PM) override;
  TargetTransformInfo getTargetTransformInfo(const Function &F) const override;

  HexagonTargetObjectFile *getObjFileLowering() const override {
    return static_cast<HexagonTargetObjectFile*>(TLOF.get());
  }

  MachineFunctionInfo *
  createMachineFunctionInfo(BumpPtrAllocator &Allocator, const Function &F,
                            const TargetSubtargetInfo *STI) const override;

  bool isNoopAddrSpaceCast(unsigned SrcAS, unsigned DestAS) const override {
    return true;
  }
  ScheduleDAGInstrs *
  createMachineScheduler(MachineSchedContext *C) const override;
};

} // end namespace vm::core

#endif
