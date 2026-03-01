//===-- LoongArchAsmBackend.h - LoongArch Assembler Backend ---*- C++ -*---===//
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
// This file defines the LoongArchAsmBackend class.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_LOONGARCH_MCTARGETDESC_LOONGARCHASMBACKEND_H
#define LLVM_LIB_TARGET_LOONGARCH_MCTARGETDESC_LOONGARCHASMBACKEND_H

#include "MCTargetDesc/LoongArchBaseInfo.h"
#include "MCTargetDesc/LoongArchFixupKinds.h"
#include "MCTargetDesc/LoongArchMCTargetDesc.h"
#include "vm/core/MC/MCAsmBackend.h"
#include "vm/core/MC/MCExpr.h"
#include "vm/core/MC/MCSection.h"
#include "vm/core/MC/MCSubtargetInfo.h"

namespace vm::core {

class LoongArchAsmBackend : public MCAsmBackend {
  const MCSubtargetInfo &STI;
  uint8_t OSABI;
  bool Is64Bit;
  const MCTargetOptions &TargetOptions;
  DenseMap<MCSection *, const MCSymbolRefExpr *> SecToAlignSym;
  // Temporary symbol used to check whether a PC-relative fixup is resolved.
  MCSymbol *PCRelTemp = nullptr;

  bool isPCRelFixupResolved(const MCSymbol *SymA, const MCFragment &F);

public:
  LoongArchAsmBackend(const MCSubtargetInfo &STI, uint8_t OSABI, bool Is64Bit,
                      const MCTargetOptions &Options);

  bool addReloc(const MCFragment &, const MCFixup &, const MCValue &,
                uint64_t &FixedValue, bool IsResolved);

  void applyFixup(const MCFragment &, const MCFixup &, const MCValue &Target,
                  uint8_t *Data, uint64_t Value, bool IsResolved) override;

  std::optional<MCFixupKind> getFixupKind(StringRef Name) const override;

  MCFixupKindInfo getFixupKindInfo(MCFixupKind Kind) const override;

  bool relaxAlign(MCFragment &F, unsigned &Size) override;
  bool relaxDwarfLineAddr(MCFragment &) const override;
  bool relaxDwarfCFA(MCFragment &) const override;
  std::pair<bool, bool> relaxLEB128(MCFragment &F,
                                    int64_t &Value) const override;

  bool writeNopData(raw_ostream &OS, uint64_t Count,
                    const MCSubtargetInfo *STI) const override;

  std::unique_ptr<MCObjectTargetWriter>
  createObjectTargetWriter() const override;
  const MCTargetOptions &getTargetOptions() const { return TargetOptions; }
  DenseMap<MCSection *, const MCSymbolRefExpr *> &getSecToAlignSym() {
    return SecToAlignSym;
  }
};
} // end namespace vm::core

#endif // LLVM_LIB_TARGET_LOONGARCH_MCTARGETDESC_LOONGARCHASMBACKEND_H
