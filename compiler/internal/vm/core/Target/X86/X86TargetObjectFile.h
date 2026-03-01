//===-- X86TargetObjectFile.h - X86 Object Info -----------------*- C++ -*-===//
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

#ifndef LLVM_LIB_TARGET_X86_X86TARGETOBJECTFILE_H
#define LLVM_LIB_TARGET_X86_X86TARGETOBJECTFILE_H

#include "vm/core/CodeGen/TargetLoweringObjectFileImpl.h"

namespace vm::core {

  /// X86_64MachoTargetObjectFile - This TLOF implementation is used for Darwin
  /// x86-64.
  class X86_64MachoTargetObjectFile : public TargetLoweringObjectFileMachO {
  public:
    const MCExpr *getTTypeGlobalReference(const GlobalValue *GV,
                                          unsigned Encoding,
                                          const TargetMachine &TM,
                                          MachineModuleInfo *MMI,
                                          MCStreamer &Streamer) const override;

    // getCFIPersonalitySymbol - The symbol that gets passed to
    // .cfi_personality.
    MCSymbol *getCFIPersonalitySymbol(const GlobalValue *GV,
                                      const TargetMachine &TM,
                                      MachineModuleInfo *MMI) const override;

    const MCExpr *getIndirectSymViaGOTPCRel(const GlobalValue *GV,
                                            const MCSymbol *Sym,
                                            const MCValue &MV, int64_t Offset,
                                            MachineModuleInfo *MMI,
                                            MCStreamer &Streamer) const override;
  };

  /// This implementation is used for X86 ELF targets that don't have a further
  /// specialization (and as a base class for X86_64, which does).
  class X86ELFTargetObjectFile : public TargetLoweringObjectFileELF {
  public:
    X86ELFTargetObjectFile();
    /// Describe a TLS variable address within debug info.
    const MCExpr *getDebugThreadLocalSymbol(const MCSymbol *Sym) const override;
  };

  /// This implementation is used for X86_64 ELF targets, and defers to
  /// X86ELFTargetObjectFile for commonalities with 32-bit targets.
  class X86_64ELFTargetObjectFile : public X86ELFTargetObjectFile {
  public:
    X86_64ELFTargetObjectFile() { SupportIndirectSymViaGOTPCRel = true; }

    const MCExpr *
    getIndirectSymViaGOTPCRel(const GlobalValue *GV, const MCSymbol *Sym,
                              const MCValue &MV, int64_t Offset,
                              MachineModuleInfo *MMI,
                              MCStreamer &Streamer) const override;
  };

} // end namespace vm::core

#endif
