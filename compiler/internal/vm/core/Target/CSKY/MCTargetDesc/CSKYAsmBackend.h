//===-- CSKYAsmBackend.h - CSKY Assembler Backend -------------------------===//
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

#ifndef LLVM_LIB_TARGET_CSKY_MCTARGETDESC_CSKYASMBACKEND_H
#define LLVM_LIB_TARGET_CSKY_MCTARGETDESC_CSKYASMBACKEND_H

#include "MCTargetDesc/CSKYFixupKinds.h"
#include "vm/core/MC/MCAsmBackend.h"
#include "vm/core/MC/MCSubtargetInfo.h"
#include "vm/core/MC/MCTargetOptions.h"

namespace vm::core {

class CSKYAsmBackend : public MCAsmBackend {

public:
  CSKYAsmBackend(const MCSubtargetInfo &STI, const MCTargetOptions &OP)
      : MCAsmBackend(toolchain::endianness::little) {}

  std::optional<bool> evaluateFixup(const MCFragment &, MCFixup &, MCValue &,
                                    uint64_t &) override;
  void applyFixup(const MCFragment &, const MCFixup &, const MCValue &Target,
                  uint8_t *Data, uint64_t Value, bool IsResolved) override;

  MCFixupKindInfo getFixupKindInfo(MCFixupKind Kind) const override;

  bool fixupNeedsRelaxation(const MCFixup &Fixup,
                            uint64_t Value) const override;

  bool mayNeedRelaxation(unsigned Opcode, ArrayRef<MCOperand> Operands,
                         const MCSubtargetInfo &STI) const override;
  void relaxInstruction(MCInst &Inst,
                        const MCSubtargetInfo &STI) const override;

  bool fixupNeedsRelaxationAdvanced(const MCFragment &, const MCFixup &,
                                    const MCValue &, uint64_t,
                                    bool) const override;

  bool writeNopData(raw_ostream &OS, uint64_t Count,
                    const MCSubtargetInfo *STI) const override;

  bool shouldForceRelocation(const MCFixup &Fixup, const MCValue &Target);

  std::unique_ptr<MCObjectTargetWriter>
  createObjectTargetWriter() const override;
};
} // namespace vm::core

#endif // LLVM_LIB_TARGET_CSKY_MCTARGETDESC_CSKYASMBACKEND_H
