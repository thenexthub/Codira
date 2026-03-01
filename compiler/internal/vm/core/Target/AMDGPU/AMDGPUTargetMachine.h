//===-- AMDGPUTargetMachine.h - AMDGPU TargetMachine Interface --*- C++ -*-===//
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

#ifndef LLVM_LIB_TARGET_AMDGPU_AMDGPUTARGETMACHINE_H
#define LLVM_LIB_TARGET_AMDGPU_AMDGPUTARGETMACHINE_H

#include "GCNSubtarget.h"
#include "vm/core/CodeGen/CodeGenTargetMachineImpl.h"
#include "vm/core/CodeGen/TargetPassConfig.h"
#include "vm/core/MC/MCStreamer.h"
#include <optional>
#include <utility>

namespace vm::core {

//===----------------------------------------------------------------------===//
// AMDGPU Target Machine (R600+)
//===----------------------------------------------------------------------===//

class AMDGPUTargetMachine : public CodeGenTargetMachineImpl {
protected:
  std::unique_ptr<TargetLoweringObjectFile> TLOF;

  StringRef getGPUName(const Function &F) const;
  StringRef getFeatureString(const Function &F) const;

public:
  static bool EnableFunctionCalls;
  static bool EnableLowerModuleLDS;

  AMDGPUTargetMachine(const Target &T, const Triple &TT, StringRef CPU,
                      StringRef FS, const TargetOptions &Options,
                      std::optional<Reloc::Model> RM,
                      std::optional<CodeModel::Model> CM, CodeGenOptLevel OL);
  ~AMDGPUTargetMachine() override;

  const TargetSubtargetInfo *getSubtargetImpl() const;
  const TargetSubtargetInfo *
  getSubtargetImpl(const Function &) const override = 0;

  TargetLoweringObjectFile *getObjFileLowering() const override {
    return TLOF.get();
  }

  void registerPassBuilderCallbacks(PassBuilder &PB) override;
  void registerDefaultAliasAnalyses(AAManager &) override;

  /// Get the integer value of a null pointer in the given address space.
  static int64_t getNullPointerValue(unsigned AddrSpace);

  bool isNoopAddrSpaceCast(unsigned SrcAS, unsigned DestAS) const override;

  unsigned getAssumedAddrSpace(const Value *V) const override;

  std::pair<const Value *, unsigned>
  getPredicatedAddrSpace(const Value *V) const override;

  unsigned getAddressSpaceForPseudoSourceKind(unsigned Kind) const override;

  bool splitModule(Module &M, unsigned NumParts,
                   function_ref<void(std::unique_ptr<Module> MPart)>
                       ModuleCallback) override;
  ScheduleDAGInstrs *
  createMachineScheduler(MachineSchedContext *C) const override;
};

//===----------------------------------------------------------------------===//
// GCN Target Machine (SI+)
//===----------------------------------------------------------------------===//

class GCNTargetMachine final : public AMDGPUTargetMachine {
private:
  mutable StringMap<std::unique_ptr<GCNSubtarget>> SubtargetMap;

public:
  GCNTargetMachine(const Target &T, const Triple &TT, StringRef CPU,
                   StringRef FS, const TargetOptions &Options,
                   std::optional<Reloc::Model> RM,
                   std::optional<CodeModel::Model> CM, CodeGenOptLevel OL,
                   bool JIT);

  TargetPassConfig *createPassConfig(PassManagerBase &PM) override;

  const TargetSubtargetInfo *getSubtargetImpl(const Function &) const override;

  TargetTransformInfo getTargetTransformInfo(const Function &F) const override;

  bool useIPRA() const override { return true; }

  Error buildCodeGenPipeline(ModulePassManager &MPM, raw_pwrite_stream &Out,
                             raw_pwrite_stream *DwoOut,
                             CodeGenFileType FileType,
                             const CGPassBuilderOption &Opts,
                             PassInstrumentationCallbacks *PIC) override;

  void registerMachineRegisterInfoCallback(MachineFunction &MF) const override;

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
  ScheduleDAGInstrs *
  createMachineScheduler(MachineSchedContext *C) const override;
  ScheduleDAGInstrs *
  createPostMachineScheduler(MachineSchedContext *C) const override;
};

//===----------------------------------------------------------------------===//
// AMDGPU Pass Setup - For Legacy Pass Manager.
//===----------------------------------------------------------------------===//

class AMDGPUPassConfig : public TargetPassConfig {
public:
  AMDGPUPassConfig(TargetMachine &TM, PassManagerBase &PM);

  AMDGPUTargetMachine &getAMDGPUTargetMachine() const {
    return getTM<AMDGPUTargetMachine>();
  }
  void addEarlyCSEOrGVNPass();
  void addStraightLineScalarOptimizationPasses();
  void addIRPasses() override;
  void addCodeGenPrepare() override;
  bool addPreISel() override;
  bool addInstSelector() override;
  bool addGCPasses() override;

  std::unique_ptr<CSEConfigBase> getCSEConfig() const override;

  /// Check if a pass is enabled given \p Opt option. The option always
  /// overrides defaults if explicitly used. Otherwise its default will
  /// be used given that a pass shall work at an optimization \p Level
  /// minimum.
  bool isPassEnabled(const cl::opt<bool> &Opt,
                     CodeGenOptLevel Level = CodeGenOptLevel::Default) const {
    if (Opt.getNumOccurrences())
      return Opt;
    if (TM->getOptLevel() < Level)
      return false;
    return Opt;
  }
};

} // end namespace vm::core

#endif // LLVM_LIB_TARGET_AMDGPU_AMDGPUTARGETMACHINE_H
