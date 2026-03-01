//===- MipsELFStreamer.h - ELF Object Output --------------------*- C++ -*-===//
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
// This is a custom MCELFStreamer which allows us to insert some hooks before
// emitting data into an actual object file.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_MIPS_MCTARGETDESC_MIPSELFSTREAMER_H
#define LLVM_LIB_TARGET_MIPS_MCTARGETDESC_MIPSELFSTREAMER_H

#include "MipsOptionRecord.h"
#include "vm/core/ADT/SmallVector.h"
#include "vm/core/MC/MCELFStreamer.h"
#include <memory>

namespace vm::core {

class MCAsmBackend;
class MCCodeEmitter;
class MCContext;
class MCSubtargetInfo;
struct MCDwarfFrameInfo;

class MipsELFStreamer : public MCELFStreamer {
  SmallVector<std::unique_ptr<MipsOptionRecord>, 8> MipsOptionRecords;
  MipsRegInfoRecord *RegInfoRecord;
  SmallVector<MCSymbol*, 4> Labels;

public:
  MipsELFStreamer(MCContext &Context, std::unique_ptr<MCAsmBackend> MAB,
                  std::unique_ptr<MCObjectWriter> OW,
                  std::unique_ptr<MCCodeEmitter> Emitter);

  /// Overriding this function allows us to add arbitrary behaviour before the
  /// \p Inst is actually emitted. For example, we can inspect the operands and
  /// gather sufficient information that allows us to reason about the register
  /// usage for the translation unit.
  void emitInstruction(const MCInst &Inst, const MCSubtargetInfo &STI) override;

  /// Overriding this function allows us to record all labels that should be
  /// marked as microMIPS. Based on this data marking is done in
  /// EmitInstruction.
  void emitLabel(MCSymbol *Symbol, SMLoc Loc = SMLoc()) override;

  /// Overriding this function allows us to dismiss all labels that are
  /// candidates for marking as microMIPS when .section directive is processed.
  void switchSection(MCSection *Section, uint32_t Subsection = 0) override;

  /// Overriding these functions allows us to dismiss all labels that are
  /// candidates for marking as microMIPS when .word/.long/.4byte etc
  /// directives are emitted.
  void emitValueImpl(const MCExpr *Value, unsigned Size, SMLoc Loc) override;
  void emitIntValue(uint64_t Value, unsigned Size) override;

  // Overriding these functions allows us to avoid recording of these labels
  // in EmitLabel and later marking them as microMIPS.
  void emitCFIStartProcImpl(MCDwarfFrameInfo &Frame) override;
  void emitCFIEndProcImpl(MCDwarfFrameInfo &Frame) override;
  MCSymbol *emitCFILabel() override;

  /// Emits all the option records stored up until the point it's called.
  void EmitMipsOptionRecords();

  /// Mark labels as microMIPS, if necessary for the subtarget.
  void createPendingLabelRelocs();
};

MCELFStreamer *createMipsELFStreamer(MCContext &Context,
                                     std::unique_ptr<MCAsmBackend> MAB,
                                     std::unique_ptr<MCObjectWriter> OW,
                                     std::unique_ptr<MCCodeEmitter> Emitter);
} // end namespace vm::core

#endif // LLVM_LIB_TARGET_MIPS_MCTARGETDESC_MIPSELFSTREAMER_H
