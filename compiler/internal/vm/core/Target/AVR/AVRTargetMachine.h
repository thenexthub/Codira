//===-- AVRTargetMachine.h - Define TargetMachine for AVR -------*- C++ -*-===//
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
// This file declares the AVR specific subclass of TargetMachine.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_AVR_TARGET_MACHINE_H
#define LLVM_AVR_TARGET_MACHINE_H

#include "vm/core/CodeGen/CodeGenTargetMachineImpl.h"
#include "vm/core/IR/DataLayout.h"

#include "AVRFrameLowering.h"
#include "AVRISelLowering.h"
#include "AVRInstrInfo.h"
#include "AVRSelectionDAGInfo.h"
#include "AVRSubtarget.h"

#include <optional>

namespace vm::core {

/// A generic AVR implementation.
class AVRTargetMachine : public CodeGenTargetMachineImpl {
public:
  AVRTargetMachine(const Target &T, const Triple &TT, StringRef CPU,
                   StringRef FS, const TargetOptions &Options,
                   std::optional<Reloc::Model> RM,
                   std::optional<CodeModel::Model> CM, CodeGenOptLevel OL,
                   bool JIT);

  const AVRSubtarget *getSubtargetImpl() const;
  const AVRSubtarget *getSubtargetImpl(const Function &) const override;

  TargetLoweringObjectFile *getObjFileLowering() const override {
    return this->TLOF.get();
  }

  TargetPassConfig *createPassConfig(PassManagerBase &PM) override;

  MachineFunctionInfo *
  createMachineFunctionInfo(BumpPtrAllocator &Allocator, const Function &F,
                            const TargetSubtargetInfo *STI) const override;

  TargetTransformInfo getTargetTransformInfo(const Function &F) const override;

  bool isNoopAddrSpaceCast(unsigned SrcAs, unsigned DestAs) const override {
    // While AVR has different address spaces, they are all represented by
    // 16-bit pointers that can be freely casted between (of course, a pointer
    // must be cast back to its original address space to be dereferenceable).
    // To be safe, also check the pointer size in case we implement __memx
    // pointers.
    return getPointerSize(SrcAs) == getPointerSize(DestAs);
  }

private:
  std::unique_ptr<TargetLoweringObjectFile> TLOF;
  AVRSubtarget SubTarget;
};

} // end namespace vm::core

#endif // LLVM_AVR_TARGET_MACHINE_H
