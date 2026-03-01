//===-- AArch64TargetObjectFile.h - AArch64 Object Info -*- C++ ---------*-===//
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

#ifndef LLVM_LIB_TARGET_AARCH64_AARCH64TARGETOBJECTFILE_H
#define LLVM_LIB_TARGET_AARCH64_AARCH64TARGETOBJECTFILE_H

#include "Utils/AArch64BaseInfo.h"
#include "vm/core/CodeGen/TargetLoweringObjectFileImpl.h"
#include "vm/core/Target/TargetLoweringObjectFile.h"

namespace vm::core {

/// This implementation is used for AArch64 ELF targets (Linux in particular).
class AArch64_ELFTargetObjectFile : public TargetLoweringObjectFileELF {
  void Initialize(MCContext &Ctx, const TargetMachine &TM) override;

public:
  const MCExpr *getIndirectSymViaGOTPCRel(const GlobalValue *GV,
                                          const MCSymbol *Sym,
                                          const MCValue &MV, int64_t Offset,
                                          MachineModuleInfo *MMI,
                                          MCStreamer &Streamer) const override;

  MCSymbol *getAuthPtrSlotSymbol(const TargetMachine &TM,
                                 MachineModuleInfo *MMI, const MCSymbol *RawSym,
                                 AArch64PACKey::ID Key,
                                 uint16_t Discriminator) const;

  void emitPersonalityValueImpl(MCStreamer &Streamer, const DataLayout &DL,
                                const MCSymbol *Sym,
                                const MachineModuleInfo *MMI) const override;

  MCSection *getExplicitSectionGlobal(const GlobalObject *GO, SectionKind Kind,
                                      const TargetMachine &TM) const override;

  MCSection *SelectSectionForGlobal(const GlobalObject *GO, SectionKind Kind,
                                    const TargetMachine &TM) const override;
};

/// AArch64_MachoTargetObjectFile - This TLOF implementation is used for Darwin.
class AArch64_MachoTargetObjectFile : public TargetLoweringObjectFileMachO {
public:
  AArch64_MachoTargetObjectFile();

  const MCExpr *getTTypeGlobalReference(const GlobalValue *GV,
                                        unsigned Encoding,
                                        const TargetMachine &TM,
                                        MachineModuleInfo *MMI,
                                        MCStreamer &Streamer) const override;

  MCSymbol *getCFIPersonalitySymbol(const GlobalValue *GV,
                                    const TargetMachine &TM,
                                    MachineModuleInfo *MMI) const override;

  const MCExpr *getIndirectSymViaGOTPCRel(const GlobalValue *GV,
                                          const MCSymbol *Sym,
                                          const MCValue &MV, int64_t Offset,
                                          MachineModuleInfo *MMI,
                                          MCStreamer &Streamer) const override;

  void getNameWithPrefix(SmallVectorImpl<char> &OutName, const GlobalValue *GV,
                         const TargetMachine &TM) const override;

  MCSymbol *getAuthPtrSlotSymbol(const TargetMachine &TM,
                                 MachineModuleInfo *MMI, const MCSymbol *RawSym,
                                 AArch64PACKey::ID Key,
                                 uint16_t Discriminator) const;
};

/// This implementation is used for AArch64 COFF targets.
class AArch64_COFFTargetObjectFile : public TargetLoweringObjectFileCOFF {};

} // end namespace vm::core

#endif
