//===-- RISCVELFStreamer.h - RISC-V ELF Target Streamer ---------*- C++ -*-===//
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

#ifndef LLVM_LIB_TARGET_RISCV_MCTARGETDESC_RISCVELFSTREAMER_H
#define LLVM_LIB_TARGET_RISCV_MCTARGETDESC_RISCVELFSTREAMER_H

#include "RISCVTargetStreamer.h"
#include "vm/core/MC/MCELFStreamer.h"

namespace vm::core {

class RISCVELFStreamer : public MCELFStreamer {
  void reset() override;
  void emitDataMappingSymbol();
  void emitInstructionsMappingSymbol();
  void emitMappingSymbol(StringRef Name);

  enum ElfMappingSymbol { EMS_None, EMS_Instructions, EMS_Data };

  DenseMap<const MCSection *, ElfMappingSymbol> LastMappingSymbols;
  ElfMappingSymbol LastEMS = EMS_None;

public:
  RISCVELFStreamer(MCContext &C, std::unique_ptr<MCAsmBackend> MAB,
                   std::unique_ptr<MCObjectWriter> MOW,
                   std::unique_ptr<MCCodeEmitter> MCE);

  void changeSection(MCSection *Section, uint32_t Subsection) override;
  void emitInstruction(const MCInst &Inst, const MCSubtargetInfo &STI) override;
  void emitBytes(StringRef Data) override;
  void emitFill(const MCExpr &NumBytes, uint64_t FillValue, SMLoc Loc) override;
  void emitValueImpl(const MCExpr *Value, unsigned Size, SMLoc Loc) override;
};

class RISCVTargetELFStreamer : public RISCVTargetStreamer {
private:
  StringRef CurrentVendor;

  MCSection *AttributeSection = nullptr;

  void emitAttribute(unsigned Attribute, unsigned Value) override;
  void emitTextAttribute(unsigned Attribute, StringRef String) override;
  void emitIntTextAttribute(unsigned Attribute, unsigned IntValue,
                            StringRef StringValue) override;
  void finishAttributeSection() override;

  void reset() override;

public:
  RISCVELFStreamer &getStreamer();
  RISCVTargetELFStreamer(MCStreamer &S, const MCSubtargetInfo &STI);

  void emitDirectiveOptionExact() override;
  void emitDirectiveOptionNoExact() override;
  void emitDirectiveOptionPIC() override;
  void emitDirectiveOptionNoPIC() override;
  void emitDirectiveOptionPop() override;
  void emitDirectiveOptionPush() override;
  void emitDirectiveOptionRelax() override;
  void emitDirectiveOptionNoRelax() override;
  void emitDirectiveOptionRVC() override;
  void emitDirectiveOptionNoRVC() override;
  void emitDirectiveVariantCC(MCSymbol &Symbol) override;

  void emitNoteGnuPropertySection(const uint32_t Feature1And);
  void finish() override;
};

MCStreamer *createRISCVELFStreamer(const Triple &, MCContext &C,
                                   std::unique_ptr<MCAsmBackend> &&MAB,
                                   std::unique_ptr<MCObjectWriter> &&MOW,
                                   std::unique_ptr<MCCodeEmitter> &&MCE);
} // namespace vm::core

#endif // LLVM_LIB_TARGET_RISCV_MCTARGETDESC_RISCVELFSTREAMER_H
