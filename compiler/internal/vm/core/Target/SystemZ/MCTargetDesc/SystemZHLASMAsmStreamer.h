//===- SystemZHLASMAsmStreamer.h - HLASM Assembly Text Output ---*- C++ -*-===//
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
// This file declares the SystemZHLASMAsmStreamer class.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_SYSTEMZ_MCTARGETDESC_SYSTEMZHLASMASMSTREAMER_H
#define LLVM_LIB_TARGET_SYSTEMZ_MCTARGETDESC_SYSTEMZHLASMASMSTREAMER_H

#include "vm/core/ADT/SmallString.h"
#include "vm/core/ADT/StringRef.h"
#include "vm/core/MC/MCAsmBackend.h"
#include "vm/core/MC/MCAsmInfo.h"
#include "vm/core/MC/MCAssembler.h"
#include "vm/core/MC/MCCodeEmitter.h"
#include "vm/core/MC/MCContext.h"
#include "vm/core/MC/MCInst.h"
#include "vm/core/MC/MCInstPrinter.h"
#include "vm/core/MC/MCObjectWriter.h"
#include "vm/core/MC/MCStreamer.h"
#include "vm/core/MC/MCTargetOptions.h"
#include "vm/core/Support/FormattedStream.h"

namespace vm::core {
class MCSymbolGOFF;

class SystemZHLASMAsmStreamer final : public MCStreamer {
  constexpr static size_t InstLimit = 80;
  constexpr static size_t ContIndicatorColumn = 72;
  constexpr static size_t ContStartColumn = 15;
  constexpr static size_t ContLen = ContIndicatorColumn - ContStartColumn;
  std::unique_ptr<formatted_raw_ostream> FOSOwner;
  formatted_raw_ostream &FOS;
  std::string Str;
  raw_string_ostream OS;
  const MCAsmInfo *MAI;
  std::unique_ptr<MCInstPrinter> InstPrinter;
  std::unique_ptr<MCAssembler> Assembler;
  SmallString<128> CommentToEmit;
  raw_svector_ostream CommentStream;
  raw_null_ostream NullStream;
  bool IsVerboseAsm = false;

public:
  SystemZHLASMAsmStreamer(MCContext &Context,
                          std::unique_ptr<formatted_raw_ostream> os,
                          std::unique_ptr<MCInstPrinter> printer,
                          std::unique_ptr<MCCodeEmitter> emitter,
                          std::unique_ptr<MCAsmBackend> asmbackend)
      : MCStreamer(Context), FOSOwner(std::move(os)), FOS(*FOSOwner), OS(Str),
        MAI(Context.getAsmInfo()), InstPrinter(std::move(printer)),
        Assembler(std::make_unique<MCAssembler>(
            Context, std::move(asmbackend), std::move(emitter),
            (asmbackend) ? asmbackend->createObjectWriter(NullStream)
                         : nullptr)),
        CommentStream(CommentToEmit) {
    assert(InstPrinter);
    if (Assembler->getBackendPtr())
      setAllowAutoPadding(Assembler->getBackend().allowAutoPadding());

    Context.setUseNamesOnTempLabels(true);
    auto *TO = Context.getTargetOptions();
    if (!TO)
      return;
    IsVerboseAsm = TO->AsmVerbose;
    if (IsVerboseAsm)
      InstPrinter->setCommentStream(CommentStream);
  }

  MCAssembler &getAssembler() { return *Assembler; }

  void EmitEOL();
  void EmitComment();

  /// Add a comment that can be emitted to the generated .s file to make the
  /// output of the compiler more readable. This only affects the MCAsmStreamer
  /// and only when verbose assembly output is enabled.
  void AddComment(const Twine &T, bool EOL = true) override;

  void emitBytes(StringRef Data) override;

  void emitAlignmentDS(uint64_t ByteAlignment, std::optional<int64_t> Value,
                       unsigned ValueSize, unsigned MaxBytesToEmit);
  void emitValueToAlignment(Align Alignment, int64_t Fill, uint8_t FillLen,
                            unsigned MaxBytesToEmit) override;

  void emitCodeAlignment(Align Alignment, const MCSubtargetInfo *STI,
                         unsigned MaxBytesToEmit = 0) override;

  /// Return true if this streamer supports verbose assembly at all.
  bool isVerboseAsm() const override { return IsVerboseAsm; }

  /// Do we support EmitRawText?
  bool hasRawTextSupport() const override { return true; }

  /// @name MCStreamer Interface
  /// @{
  void visitUsedSymbol(const MCSymbol &Sym) override;

  void changeSection(MCSection *Section, uint32_t Subsection) override;

  void emitInstruction(const MCInst &Inst, const MCSubtargetInfo &STI) override;
  void emitLabel(MCSymbol *Symbol, SMLoc Loc) override;
  bool emitSymbolAttribute(MCSymbol *Symbol, MCSymbolAttr Attribute) override;

  void emitCommonSymbol(MCSymbol *Symbol, uint64_t Size,
                        Align ByteAlignment) override {}

  void emitZerofill(MCSection *Section, MCSymbol *Symbol = nullptr,
                    uint64_t Size = 0, Align ByteAlignment = Align(1),
                    SMLoc Loc = SMLoc()) override {}
  void emitRawTextImpl(StringRef String) override;
  void emitValueImpl(const MCExpr *Value, unsigned Size, SMLoc Loc) override;

  void emitHLASMValueImpl(const MCExpr *Value, unsigned Size,
                          bool Parens = false);
  /// @}

  void finishImpl() override;
};
} // namespace vm::core

#endif // LLVM_LIB_TARGET_SYSTEMZ_MCTARGETDESC_SYSTEMZHLASMASMSTREAMER_H
