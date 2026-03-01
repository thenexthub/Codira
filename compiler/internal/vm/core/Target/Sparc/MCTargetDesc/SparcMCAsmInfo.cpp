//===- SparcMCAsmInfo.cpp - Sparc asm properties --------------------------===//
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
// This file contains the declarations of the SparcMCAsmInfo properties.
//
//===----------------------------------------------------------------------===//

#include "SparcMCAsmInfo.h"
#include "vm/core/BinaryFormat/Dwarf.h"
#include "vm/core/BinaryFormat/ELF.h"
#include "vm/core/MC/MCExpr.h"
#include "vm/core/MC/MCStreamer.h"
#include "vm/core/MC/MCTargetOptions.h"
#include "vm/core/TargetParser/Triple.h"

using namespace vm::core;

void SparcELFMCAsmInfo::anchor() {}

SparcELFMCAsmInfo::SparcELFMCAsmInfo(const Triple &TheTriple) {
  bool isV9 = (TheTriple.getArch() == Triple::sparcv9);
  IsLittleEndian = (TheTriple.getArch() == Triple::sparcel);

  if (isV9) {
    CodePointerSize = CalleeSaveStackSlotSize = 8;
  }

  Data16bitsDirective = "\t.half\t";
  Data32bitsDirective = "\t.word\t";
  // .xword is only supported by V9.
  Data64bitsDirective = (isV9) ? "\t.xword\t" : nullptr;
  ZeroDirective = "\t.skip\t";
  CommentString = "!";
  SupportsDebugInformation = true;

  ExceptionsType = ExceptionHandling::DwarfCFI;

  UsesELFSectionDirectiveForBSS = true;
}

const MCExpr*
SparcELFMCAsmInfo::getExprForPersonalitySymbol(const MCSymbol *Sym,
                                               unsigned Encoding,
                                               MCStreamer &Streamer) const {
  if (Encoding & dwarf::DW_EH_PE_pcrel) {
    MCContext &Ctx = Streamer.getContext();
    return MCSpecifierExpr::create(Sym, ELF::R_SPARC_DISP32, Ctx);
  }

  return MCAsmInfo::getExprForPersonalitySymbol(Sym, Encoding, Streamer);
}

const MCExpr*
SparcELFMCAsmInfo::getExprForFDESymbol(const MCSymbol *Sym,
                                       unsigned Encoding,
                                       MCStreamer &Streamer) const {
  if (Encoding & dwarf::DW_EH_PE_pcrel) {
    MCContext &Ctx = Streamer.getContext();
    return MCSpecifierExpr::create(Sym, ELF::R_SPARC_DISP32, Ctx);
  }
  return MCAsmInfo::getExprForFDESymbol(Sym, Encoding, Streamer);
}

void SparcELFMCAsmInfo::printSpecifierExpr(raw_ostream &OS,
                                           const MCSpecifierExpr &Expr) const {
  StringRef S = Sparc::getSpecifierName(Expr.getSpecifier());
  if (!S.empty())
    OS << '%' << S << '(';
  printExpr(OS, *Expr.getSubExpr());
  if (!S.empty())
    OS << ')';
}
