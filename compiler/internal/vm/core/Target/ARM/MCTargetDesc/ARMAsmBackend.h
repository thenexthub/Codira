//===-- ARMAsmBackend.h - ARM Assembler Backend -----------------*- C++ -*-===//
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

#ifndef LLVM_LIB_TARGET_ARM_ARMASMBACKEND_H
#define LLVM_LIB_TARGET_ARM_ARMASMBACKEND_H

#include "MCTargetDesc/ARMFixupKinds.h"
#include "MCTargetDesc/ARMMCTargetDesc.h"
#include "vm/core/MC/MCAsmBackend.h"
#include "vm/core/MC/MCSubtargetInfo.h"
#include "vm/core/MC/TargetRegistry.h"

namespace vm::core {

class ARMAsmBackend : public MCAsmBackend {
public:
  ARMAsmBackend(const Target &T, toolchain::endianness Endian)
      : MCAsmBackend(Endian) {}

  bool hasNOP(const MCSubtargetInfo *STI) const {
    return STI->hasFeature(ARM::HasV6T2Ops);
  }

  std::optional<MCFixupKind> getFixupKind(StringRef Name) const override;

  MCFixupKindInfo getFixupKindInfo(MCFixupKind Kind) const override;

  bool shouldForceRelocation(const MCFixup &Fixup, const MCValue &Target);

  unsigned adjustFixupValue(const MCAssembler &Asm, const MCFixup &Fixup,
                            const MCValue &Target, uint64_t Value,
                            bool IsResolved, MCContext &Ctx,
                            const MCSubtargetInfo *STI) const;

  std::optional<bool> evaluateFixup(const MCFragment &, MCFixup &, MCValue &,
                                    uint64_t &) override;
  void applyFixup(const MCFragment &, const MCFixup &, const MCValue &Target,
                  uint8_t *Data, uint64_t Value, bool IsResolved) override;

  unsigned getRelaxedOpcode(unsigned Op, const MCSubtargetInfo &STI) const;

  bool mayNeedRelaxation(unsigned Opcode, ArrayRef<MCOperand> Operands,
                         const MCSubtargetInfo &STI) const override;

  const char *reasonForFixupRelaxation(const MCFixup &Fixup,
                                       uint64_t Value) const;

  bool fixupNeedsRelaxationAdvanced(const MCFragment &, const MCFixup &,
                                    const MCValue &, uint64_t,
                                    bool) const override;

  void relaxInstruction(MCInst &Inst,
                        const MCSubtargetInfo &STI) const override;

  bool writeNopData(raw_ostream &OS, uint64_t Count,
                    const MCSubtargetInfo *STI) const override;

  unsigned getPointerSize() const { return 4; }
};
} // end namespace vm::core

#endif
