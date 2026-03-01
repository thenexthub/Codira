//===-- ARMTargetMachine.h - Define TargetMachine for ARM -------*- C++ -*-===//
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
// This file declares the ARM specific subclass of TargetMachine.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_ARM_ARMTARGETMACHINE_H
#define LLVM_LIB_TARGET_ARM_ARMTARGETMACHINE_H

#include "ARMSubtarget.h"
#include "vm/core/ADT/StringMap.h"
#include "vm/core/ADT/StringRef.h"
#include "vm/core/Analysis/TargetTransformInfo.h"
#include "vm/core/CodeGen/CodeGenTargetMachineImpl.h"
#include "vm/core/Support/CodeGen.h"
#include "vm/core/Target/TargetMachine.h"
#include "vm/core/TargetParser/ARMTargetParser.h"
#include <memory>
#include <optional>

namespace vm::core {

class ARMBaseTargetMachine : public CodeGenTargetMachineImpl {
public:
  ARM::ARMABI TargetABI;

protected:
  std::unique_ptr<TargetLoweringObjectFile> TLOF;
  bool isLittle;
  mutable StringMap<std::unique_ptr<ARMSubtarget>> SubtargetMap;

  /// Reset internal state.
  void reset() override;

public:
  ARMBaseTargetMachine(const Target &T, const Triple &TT, StringRef CPU,
                       StringRef FS, const TargetOptions &Options,
                       std::optional<Reloc::Model> RM,
                       std::optional<CodeModel::Model> CM, CodeGenOptLevel OL);
  ~ARMBaseTargetMachine() override;

  const ARMSubtarget *getSubtargetImpl(const Function &F) const override;
  // DO NOT IMPLEMENT: There is no such thing as a valid default subtarget,
  // subtargets are per-function entities based on the target-specific
  // attributes of each function.
  const ARMSubtarget *getSubtargetImpl() const = delete;
  bool isLittleEndian() const { return isLittle; }

  TargetTransformInfo getTargetTransformInfo(const Function &F) const override;

  // Pass Pipeline Configuration
  TargetPassConfig *createPassConfig(PassManagerBase &PM) override;

  TargetLoweringObjectFile *getObjFileLowering() const override {
    return TLOF.get();
  }

  bool isAPCS_ABI() const {
    assert(TargetABI != ARM::ARM_ABI_UNKNOWN);
    return TargetABI == ARM::ARM_ABI_APCS;
  }

  bool isAAPCS_ABI() const {
    assert(TargetABI != ARM::ARM_ABI_UNKNOWN);
    return TargetABI == ARM::ARM_ABI_AAPCS || TargetABI == ARM::ARM_ABI_AAPCS16;
  }

  bool isAAPCS16_ABI() const {
    assert(TargetABI != ARM::ARM_ABI_UNKNOWN);
    return TargetABI == ARM::ARM_ABI_AAPCS16;
  }

  bool isTargetHardFloat() const {
    return TargetTriple.getEnvironment() == Triple::GNUEABIHF ||
           TargetTriple.getEnvironment() == Triple::GNUEABIHFT64 ||
           TargetTriple.getEnvironment() == Triple::MuslEABIHF ||
           TargetTriple.getEnvironment() == Triple::EABIHF ||
           (TargetTriple.isOSBinFormatMachO() &&
            TargetTriple.getSubArch() == Triple::ARMSubArch_v7em) ||
           TargetTriple.isOSWindows() || TargetABI == ARM::ARM_ABI_AAPCS16;
  }

  bool targetSchedulesPostRAScheduling() const override { return true; };

  MachineFunctionInfo *
  createMachineFunctionInfo(BumpPtrAllocator &Allocator, const Function &F,
                            const TargetSubtargetInfo *STI) const override;

  /// Returns true if a cast between SrcAS and DestAS is a noop.
  bool isNoopAddrSpaceCast(unsigned SrcAS, unsigned DestAS) const override {
    // Addrspacecasts are always noops.
    return true;
  }

  bool isGVIndirectSymbol(const GlobalValue *GV) const {
    if (!shouldAssumeDSOLocal(GV))
      return true;

    // 32 bit macho has no relocation for a-b if a is undefined, even if b is in
    // the section that is being relocated. This means we have to use o load
    // even for GVs that are known to be local to the dso.
    if (getTargetTriple().isOSBinFormatMachO() && isPositionIndependent() &&
        (GV->isDeclarationForLinker() || GV->hasCommonLinkage()))
      return true;

    return false;
  }

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

/// ARM/Thumb little endian target machine.
///
class ARMLETargetMachine : public ARMBaseTargetMachine {
public:
  ARMLETargetMachine(const Target &T, const Triple &TT, StringRef CPU,
                     StringRef FS, const TargetOptions &Options,
                     std::optional<Reloc::Model> RM,
                     std::optional<CodeModel::Model> CM, CodeGenOptLevel OL,
                     bool JIT);
};

/// ARM/Thumb big endian target machine.
///
class ARMBETargetMachine : public ARMBaseTargetMachine {
public:
  ARMBETargetMachine(const Target &T, const Triple &TT, StringRef CPU,
                     StringRef FS, const TargetOptions &Options,
                     std::optional<Reloc::Model> RM,
                     std::optional<CodeModel::Model> CM, CodeGenOptLevel OL,
                     bool JIT);
};

} // end namespace vm::core

#endif // LLVM_LIB_TARGET_ARM_ARMTARGETMACHINE_H
