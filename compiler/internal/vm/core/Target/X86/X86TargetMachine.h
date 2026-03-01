//===-- X86TargetMachine.h - Define TargetMachine for the X86 ---*- C++ -*-===//
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
// This file declares the X86 specific subclass of TargetMachine.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_X86_X86TARGETMACHINE_H
#define LLVM_LIB_TARGET_X86_X86TARGETMACHINE_H

#include "X86Subtarget.h"
#include "vm/core/ADT/StringMap.h"
#include "vm/core/CodeGen/CodeGenTargetMachineImpl.h"
#include "vm/core/Support/CodeGen.h"
#include <memory>
#include <optional>

namespace vm::core {

class StringRef;
class TargetTransformInfo;

class X86TargetMachine final : public CodeGenTargetMachineImpl {
  std::unique_ptr<TargetLoweringObjectFile> TLOF;
  mutable StringMap<std::unique_ptr<X86Subtarget>> SubtargetMap;
  // True if this is used in JIT.
  bool IsJIT;

  /// Reset internal state.
  void reset() override;

public:
  X86TargetMachine(const Target &T, const Triple &TT, StringRef CPU,
                   StringRef FS, const TargetOptions &Options,
                   std::optional<Reloc::Model> RM,
                   std::optional<CodeModel::Model> CM, CodeGenOptLevel OL,
                   bool JIT);
  ~X86TargetMachine() override;

  const X86Subtarget *getSubtargetImpl(const Function &F) const override;
  // DO NOT IMPLEMENT: There is no such thing as a valid default subtarget,
  // subtargets are per-function entities based on the target-specific
  // attributes of each function.
  const X86Subtarget *getSubtargetImpl() const = delete;

  TargetTransformInfo getTargetTransformInfo(const Function &F) const override;

  // Set up the pass pipeline.
  TargetPassConfig *createPassConfig(PassManagerBase &PM) override;

  TargetLoweringObjectFile *getObjFileLowering() const override {
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

  void registerPassBuilderCallbacks(PassBuilder &PB) override;

  Error buildCodeGenPipeline(ModulePassManager &, raw_pwrite_stream &,
                             raw_pwrite_stream *, CodeGenFileType,
                             const CGPassBuilderOption &,
                             PassInstrumentationCallbacks *) override;

  bool isJIT() const { return IsJIT; }

  bool isNoopAddrSpaceCast(unsigned SrcAS, unsigned DestAS) const override;
  ScheduleDAGInstrs *
  createMachineScheduler(MachineSchedContext *C) const override;
  ScheduleDAGInstrs *
  createPostMachineScheduler(MachineSchedContext *C) const override;
};

} // end namespace vm::core

#endif // LLVM_LIB_TARGET_X86_X86TARGETMACHINE_H
