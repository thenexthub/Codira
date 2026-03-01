//===-- XtensaTargetStreamer.cpp - Xtensa Target Streamer Methods ---------===//
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
// This file provides Xtensa specific target streamer methods.
//
//===----------------------------------------------------------------------===//

#include "XtensaTargetStreamer.h"
#include "XtensaInstPrinter.h"
#include "vm/core/BinaryFormat/ELF.h"
#include "vm/core/MC/MCAssembler.h"
#include "vm/core/MC/MCContext.h"
#include "vm/core/MC/MCObjectFileInfo.h"
#include "vm/core/MC/MCSectionELF.h"
#include "vm/core/Support/Casting.h"
#include "vm/core/Support/FormattedStream.h"

using namespace vm::core;

static std::string getLiteralSectionName(StringRef CSectionName) {
  std::size_t Pos = CSectionName.find(".text");
  std::string SectionName;
  if (Pos != std::string::npos) {
    SectionName = CSectionName.substr(0, Pos);

    if (Pos > 0)
      SectionName += ".text";

    CSectionName = CSectionName.drop_front(Pos);
    CSectionName.consume_front(".text");

    SectionName += ".literal";
    SectionName += CSectionName;
  } else {
    SectionName = CSectionName;
    SectionName += ".literal";
  }
  return SectionName;
}

XtensaTargetStreamer::XtensaTargetStreamer(MCStreamer &S)
    : MCTargetStreamer(S) {}

XtensaTargetAsmStreamer::XtensaTargetAsmStreamer(MCStreamer &S,
                                                 formatted_raw_ostream &OS)
    : XtensaTargetStreamer(S), OS(OS) {}

void XtensaTargetAsmStreamer::emitLiteral(MCSymbol *LblSym, const MCExpr *Value,
                                          bool SwitchLiteralSection, SMLoc L) {
  SmallString<60> Str;
  raw_svector_ostream LiteralStr(Str);

  LiteralStr << "\t.literal " << LblSym->getName() << ", ";

  if (auto CE = dyn_cast<MCConstantExpr>(Value)) {
    LiteralStr << CE->getValue() << "\n";
  } else if (auto SRE = dyn_cast<MCSymbolRefExpr>(Value)) {
    const MCSymbol &Sym = SRE->getSymbol();
    LiteralStr << Sym.getName() << "\n";
  } else {
    llvm_unreachable("unexpected constant pool entry type");
  }

  OS << LiteralStr.str();
}

void XtensaTargetAsmStreamer::emitLiteralPosition() {
  OS << "\t.literal_position\n";
}

void XtensaTargetAsmStreamer::startLiteralSection(MCSection *BaseSection) {
  emitLiteralPosition();
}

XtensaTargetELFStreamer::XtensaTargetELFStreamer(MCStreamer &S)
    : XtensaTargetStreamer(S) {}

void XtensaTargetELFStreamer::emitLiteral(MCSymbol *LblSym, const MCExpr *Value,
                                          bool SwitchLiteralSection, SMLoc L) {
  MCStreamer &OutStreamer = getStreamer();
  if (SwitchLiteralSection) {
    MCContext &Context = OutStreamer.getContext();
    auto *CS = static_cast<MCSectionELF *>(OutStreamer.getCurrentSectionOnly());
    std::string SectionName = getLiteralSectionName(CS->getName());

    MCSection *ConstSection = Context.getELFSection(
        SectionName, ELF::SHT_PROGBITS, ELF::SHF_EXECINSTR | ELF::SHF_ALLOC);

    OutStreamer.pushSection();
    OutStreamer.switchSection(ConstSection);
  }

  OutStreamer.emitLabel(LblSym, L);
  OutStreamer.emitValue(Value, 4, L);

  if (SwitchLiteralSection) {
    OutStreamer.popSection();
  }
}

void XtensaTargetELFStreamer::startLiteralSection(MCSection *BaseSection) {
  MCContext &Context = getStreamer().getContext();

  std::string SectionName = getLiteralSectionName(BaseSection->getName());

  MCSection *ConstSection = Context.getELFSection(
      SectionName, ELF::SHT_PROGBITS, ELF::SHF_EXECINSTR | ELF::SHF_ALLOC);

  ConstSection->setAlignment(Align(4));
}

MCELFStreamer &XtensaTargetELFStreamer::getStreamer() {
  return static_cast<MCELFStreamer &>(Streamer);
}
