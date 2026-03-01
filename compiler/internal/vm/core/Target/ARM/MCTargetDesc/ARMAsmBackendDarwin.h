//===-- ARMAsmBackendDarwin.h   ARM Asm Backend Darwin ----------*- C++ -*-===//
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

#ifndef LLVM_LIB_TARGET_ARM_ARMASMBACKENDDARWIN_H
#define LLVM_LIB_TARGET_ARM_ARMASMBACKENDDARWIN_H

#include "ARMAsmBackend.h"
#include "vm/core/BinaryFormat/MachO.h"
#include "vm/core/MC/MCContext.h"
#include "vm/core/MC/MCObjectWriter.h"

namespace vm::core {
class ARMAsmBackendDarwin : public ARMAsmBackend {
  const MCRegisterInfo &MRI;
  Triple TT;
public:
  const MachO::CPUSubTypeARM Subtype;
  ARMAsmBackendDarwin(const Target &T, const MCSubtargetInfo &STI,
                      const MCRegisterInfo &MRI)
      : ARMAsmBackend(T, toolchain::endianness::little), MRI(MRI),
        TT(STI.getTargetTriple()),
        Subtype((MachO::CPUSubTypeARM)cantFail(
            MachO::getCPUSubType(STI.getTargetTriple()))) {}

  std::unique_ptr<MCObjectTargetWriter>
  createObjectTargetWriter() const override {
    return createARMMachObjectWriter(
        /*Is64Bit=*/false, cantFail(MachO::getCPUType(TT)), Subtype);
  }

  uint64_t generateCompactUnwindEncoding(const MCDwarfFrameInfo *FI,
                                         const MCContext *Ctxt) const override;
};
} // end namespace vm::core

#endif
