//===-- NVPTXTargetMachine.h - Define TargetMachine for NVPTX ---*- C++ -*-===//
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
// This file declares the NVPTX specific subclass of TargetMachine.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_NVPTX_NVPTXTARGETMACHINE_H
#define LLVM_LIB_TARGET_NVPTX_NVPTXTARGETMACHINE_H

#include "NVPTXSubtarget.h"
#include "vm/core/CodeGen/CodeGenTargetMachineImpl.h"
#include <optional>
#include <utility>

namespace vm::core {

/// NVPTXTargetMachine
///
class NVPTXTargetMachine : public CodeGenTargetMachineImpl {
  bool is64bit;
  std::unique_ptr<TargetLoweringObjectFile> TLOF;
  NVPTX::DrvInterface drvInterface;
  NVPTXSubtarget Subtarget;

  // Hold Strings that can be free'd all together with NVPTXTargetMachine
  BumpPtrAllocator StrAlloc;
  UniqueStringSaver StrPool;

public:
  NVPTXTargetMachine(const Target &T, const Triple &TT, StringRef CPU,
                     StringRef FS, const TargetOptions &Options,
                     std::optional<Reloc::Model> RM,
                     std::optional<CodeModel::Model> CM, CodeGenOptLevel OP,
                     bool is64bit);
  ~NVPTXTargetMachine() override;
  const NVPTXSubtarget *getSubtargetImpl(const Function &) const override {
    return &Subtarget;
  }
  const NVPTXSubtarget *getSubtargetImpl() const { return &Subtarget; }
  bool is64Bit() const { return is64bit; }
  NVPTX::DrvInterface getDrvInterface() const { return drvInterface; }
  UniqueStringSaver &getStrPool() const {
    return const_cast<UniqueStringSaver &>(StrPool);
  }

  TargetPassConfig *createPassConfig(PassManagerBase &PM) override;

  // Emission of machine code through MCODEIT is not supported.
  bool addPassesToEmitMC(PassManagerBase &, MCContext *&, raw_pwrite_stream &,
                         bool = true) override {
    return true;
  }
  TargetLoweringObjectFile *getObjFileLowering() const override {
    return TLOF.get();
  }

  MachineFunctionInfo *
  createMachineFunctionInfo(BumpPtrAllocator &Allocator, const Function &F,
                            const TargetSubtargetInfo *STI) const override;

  void registerEarlyDefaultAliasAnalyses(AAManager &AAM) override;

  void registerPassBuilderCallbacks(PassBuilder &PB) override;

  TargetTransformInfo getTargetTransformInfo(const Function &F) const override;

  bool isMachineVerifierClean() const override {
    return false;
  }

  std::pair<const Value *, unsigned>
  getPredicatedAddrSpace(const Value *V) const override;
}; // NVPTXTargetMachine.

class NVPTXTargetMachine32 : public NVPTXTargetMachine {
  virtual void anchor();

public:
  NVPTXTargetMachine32(const Target &T, const Triple &TT, StringRef CPU,
                       StringRef FS, const TargetOptions &Options,
                       std::optional<Reloc::Model> RM,
                       std::optional<CodeModel::Model> CM, CodeGenOptLevel OL,
                       bool JIT);
};

class NVPTXTargetMachine64 : public NVPTXTargetMachine {
  virtual void anchor();

public:
  NVPTXTargetMachine64(const Target &T, const Triple &TT, StringRef CPU,
                       StringRef FS, const TargetOptions &Options,
                       std::optional<Reloc::Model> RM,
                       std::optional<CodeModel::Model> CM, CodeGenOptLevel OL,
                       bool JIT);
};

} // end namespace vm::core

#endif
