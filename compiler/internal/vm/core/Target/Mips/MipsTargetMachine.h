//===- MipsTargetMachine.h - Define TargetMachine for Mips ------*- C++ -*-===//
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
// This file declares the Mips specific subclass of TargetMachine.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_MIPS_MIPSTARGETMACHINE_H
#define LLVM_LIB_TARGET_MIPS_MIPSTARGETMACHINE_H

#include "MCTargetDesc/MipsABIInfo.h"
#include "MipsSubtarget.h"
#include "vm/core/ADT/StringMap.h"
#include "vm/core/ADT/StringRef.h"
#include "vm/core/CodeGen/CodeGenTargetMachineImpl.h"
#include "vm/core/Support/CodeGen.h"
#include <memory>
#include <optional>

namespace vm::core {

class MipsTargetMachine : public CodeGenTargetMachineImpl {
  bool isLittle;
  std::unique_ptr<TargetLoweringObjectFile> TLOF;
  // Selected ABI
  MipsABIInfo ABI;
  const MipsSubtarget *Subtarget;
  MipsSubtarget DefaultSubtarget;
  MipsSubtarget NoMips16Subtarget;
  MipsSubtarget Mips16Subtarget;

  mutable StringMap<std::unique_ptr<MipsSubtarget>> SubtargetMap;

public:
  MipsTargetMachine(const Target &T, const Triple &TT, StringRef CPU,
                    StringRef FS, const TargetOptions &Options,
                    std::optional<Reloc::Model> RM,
                    std::optional<CodeModel::Model> CM, CodeGenOptLevel OL,
                    bool JIT, bool isLittle);
  ~MipsTargetMachine() override;

  TargetTransformInfo getTargetTransformInfo(const Function &F) const override;

  const MipsSubtarget *getSubtargetImpl() const {
    if (Subtarget)
      return Subtarget;
    return &DefaultSubtarget;
  }

  const MipsSubtarget *getSubtargetImpl(const Function &F) const override;

  /// Reset the subtarget for the Mips target.
  void resetSubtarget(MachineFunction *MF);

  // Pass Pipeline Configuration
  TargetPassConfig *createPassConfig(PassManagerBase &PM) override;

  TargetLoweringObjectFile *getObjFileLowering() const override {
    return TLOF.get();
  }

  MachineFunctionInfo *
  createMachineFunctionInfo(BumpPtrAllocator &Allocator, const Function &F,
                            const TargetSubtargetInfo *STI) const override;

  /// Returns true if a cast between SrcAS and DestAS is a noop.
  bool isNoopAddrSpaceCast(unsigned SrcAS, unsigned DestAS) const override {
    // Mips doesn't have any special address spaces so we just reserve
    // the first 256 for software use (e.g. OpenCL) and treat casts
    // between them as noops.
    return SrcAS < 256 && DestAS < 256;
  }

  bool isLittleEndian() const { return isLittle; }
  const MipsABIInfo &getABI() const { return ABI; }
};

/// Mips32/64 big endian target machine.
///
class MipsebTargetMachine : public MipsTargetMachine {
  virtual void anchor();

public:
  MipsebTargetMachine(const Target &T, const Triple &TT, StringRef CPU,
                      StringRef FS, const TargetOptions &Options,
                      std::optional<Reloc::Model> RM,
                      std::optional<CodeModel::Model> CM, CodeGenOptLevel OL,
                      bool JIT);
};

/// Mips32/64 little endian target machine.
///
class MipselTargetMachine : public MipsTargetMachine {
  virtual void anchor();

public:
  MipselTargetMachine(const Target &T, const Triple &TT, StringRef CPU,
                      StringRef FS, const TargetOptions &Options,
                      std::optional<Reloc::Model> RM,
                      std::optional<CodeModel::Model> CM, CodeGenOptLevel OL,
                      bool JIT);
};

} // end namespace vm::core

#endif // LLVM_LIB_TARGET_MIPS_MIPSTARGETMACHINE_H
