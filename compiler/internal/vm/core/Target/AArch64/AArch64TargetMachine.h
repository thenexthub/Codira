//==-- AArch64TargetMachine.h - Define TargetMachine for AArch64 -*- C++ -*-==//
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
// This file declares the AArch64 specific subclass of TargetMachine.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_AARCH64_AARCH64TARGETMACHINE_H
#define LLVM_LIB_TARGET_AARCH64_AARCH64TARGETMACHINE_H

#include "AArch64InstrInfo.h"
#include "AArch64Subtarget.h"
#include "vm/core/CodeGen/CodeGenTargetMachineImpl.h"
#include "vm/core/IR/DataLayout.h"
#include <optional>

namespace vm::core {

class AArch64TargetMachine : public CodeGenTargetMachineImpl {
protected:
  std::unique_ptr<TargetLoweringObjectFile> TLOF;
  mutable StringMap<std::unique_ptr<AArch64Subtarget>> SubtargetMap;

  /// Reset internal state.
  void reset() override;

public:
  AArch64TargetMachine(const Target &T, const Triple &TT, StringRef CPU,
                       StringRef FS, const TargetOptions &Options,
                       std::optional<Reloc::Model> RM,
                       std::optional<CodeModel::Model> CM, CodeGenOptLevel OL,
                       bool JIT, bool IsLittleEndian);

  ~AArch64TargetMachine() override;
  const AArch64Subtarget *getSubtargetImpl(const Function &F) const override;
  // DO NOT IMPLEMENT: There is no such thing as a valid default subtarget,
  // subtargets are per-function entities based on the target-specific
  // attributes of each function.
  const AArch64Subtarget *getSubtargetImpl() const = delete;

  // Pass Pipeline Configuration
  TargetPassConfig *createPassConfig(PassManagerBase &PM) override;

  void registerPassBuilderCallbacks(PassBuilder &PB) override;

  TargetTransformInfo getTargetTransformInfo(const Function &F) const override;

  TargetLoweringObjectFile* getObjFileLowering() const override {
    return TLOF.get();
  }

  MachineFunctionInfo *
  createMachineFunctionInfo(BumpPtrAllocator &Allocator, const Function &F,
                            const TargetSubtargetInfo *STI) const override;

  yaml::MachineFunctionInfo *createDefaultFuncInfoYAML() const override;
  yaml::MachineFunctionInfo *
  convertFuncInfoToYAML(const MachineFunction &MF) const override;
  bool parseMachineFunctionInfo(const yaml::MachineFunctionInfo &,
                                PerFunctionMIParsingState &PFS,
                                SMDiagnostic &Error,
                                SMRange &SourceRange) const override;

  /// Returns true if a cast between SrcAS and DestAS is a noop.
  bool isNoopAddrSpaceCast(unsigned SrcAS, unsigned DestAS) const override {
    return getPointerSize(SrcAS) == getPointerSize(DestAS);
  }
  ScheduleDAGInstrs *
  createMachineScheduler(MachineSchedContext *C) const override;

  ScheduleDAGInstrs *
  createPostMachineScheduler(MachineSchedContext *C) const override;

  size_t clearLinkerOptimizationHints(
      const SmallPtrSetImpl<MachineInstr *> &MIs) const override;

  /// Returns true if the new SME ABI lowering should be used.
  bool useNewSMEABILowering() const { return UseNewSMEABILowering; }

private:
  bool isLittle;
  bool UseNewSMEABILowering;
};

// AArch64 little endian target machine.
//
class AArch64leTargetMachine : public AArch64TargetMachine {
  virtual void anchor();

public:
  AArch64leTargetMachine(const Target &T, const Triple &TT, StringRef CPU,
                         StringRef FS, const TargetOptions &Options,
                         std::optional<Reloc::Model> RM,
                         std::optional<CodeModel::Model> CM, CodeGenOptLevel OL,
                         bool JIT);
};

// AArch64 big endian target machine.
//
class AArch64beTargetMachine : public AArch64TargetMachine {
  virtual void anchor();

public:
  AArch64beTargetMachine(const Target &T, const Triple &TT, StringRef CPU,
                         StringRef FS, const TargetOptions &Options,
                         std::optional<Reloc::Model> RM,
                         std::optional<CodeModel::Model> CM, CodeGenOptLevel OL,
                         bool JIT);
};

} // end namespace vm::core

#endif
