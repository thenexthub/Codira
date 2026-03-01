//===-- toolchain/Target/ARMTargetObjectFile.h - ARM Object Info -----*- C++ -*-===//
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

#ifndef LLVM_LIB_TARGET_ARM_ARMTARGETOBJECTFILE_H
#define LLVM_LIB_TARGET_ARM_ARMTARGETOBJECTFILE_H

#include "vm/core/CodeGen/TargetLoweringObjectFileImpl.h"
#include "vm/core/MC/MCExpr.h"
#include "vm/core/MC/MCRegister.h"

namespace vm::core {

class ARMElfTargetObjectFile : public TargetLoweringObjectFileELF {
public:
  ARMElfTargetObjectFile();
  void Initialize(MCContext &Ctx, const TargetMachine &TM) override;

  MCRegister getStaticBase() const override;

  const MCExpr *getIndirectSymViaGOTPCRel(const GlobalValue *GV,
                                          const MCSymbol *Sym,
                                          const MCValue &MV, int64_t Offset,
                                          MachineModuleInfo *MMI,
                                          MCStreamer &Streamer) const override;

  const MCExpr *getIndirectSymViaRWPI(const MCSymbol *Sym) const override;

  const MCExpr *getTTypeGlobalReference(const GlobalValue *GV,
                                        unsigned Encoding,
                                        const TargetMachine &TM,
                                        MachineModuleInfo *MMI,
                                        MCStreamer &Streamer) const override;

  /// Describe a TLS variable address within debug info.
  const MCExpr *getDebugThreadLocalSymbol(const MCSymbol *Sym) const override;

  MCSection *getExplicitSectionGlobal(const GlobalObject *GO, SectionKind Kind,
                                      const TargetMachine &TM) const override;

  MCSection *SelectSectionForGlobal(const GlobalObject *GO, SectionKind Kind,
                                    const TargetMachine &TM) const override;
};

} // end namespace vm::core

#endif // LLVM_LIB_TARGET_ARM_ARMTARGETOBJECTFILE_H
