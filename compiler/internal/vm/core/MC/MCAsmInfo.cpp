//===- MCAsmInfo.cpp - Asm Info -------------------------------------------===//
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
// This file defines target asm properties related what form asm statements
// should take.
//
//===----------------------------------------------------------------------===//

#include "vm/core/MC/MCAsmInfo.h"
#include "vm/core/ADT/StringExtras.h"
#include "vm/core/BinaryFormat/Dwarf.h"
#include "vm/core/MC/MCContext.h"
#include "vm/core/MC/MCExpr.h"
#include "vm/core/MC/MCStreamer.h"
#include "vm/core/MC/MCValue.h"
#include "vm/core/Support/Casting.h"
#include "vm/core/Support/CommandLine.h"

using namespace vm::core;

namespace {
enum DefaultOnOff { Default, Enable, Disable };
}
static cl::opt<DefaultOnOff> DwarfExtendedLoc(
    "dwarf-extended-loc", cl::Hidden,
    cl::desc("Disable emission of the extended flags in .loc directives."),
    cl::values(clEnumVal(Default, "Default for platform"),
               clEnumVal(Enable, "Enabled"), clEnumVal(Disable, "Disabled")),
    cl::init(Default));

namespace vm::core {
cl::opt<cl::boolOrDefault> UseLEB128Directives(
    "use-leb128-directives", cl::Hidden,
    cl::desc(
        "Disable the usage of LEB128 directives, and generate .byte instead."),
    cl::init(cl::BOU_UNSET));
}

MCAsmInfo::MCAsmInfo() {
  SeparatorString = ";";
  CommentString = "#";
  LabelSuffix = ":";
  PrivateGlobalPrefix = "L";
  PrivateLabelPrefix = PrivateGlobalPrefix;
  LinkerPrivateGlobalPrefix = "";
  InlineAsmStart = "APP";
  InlineAsmEnd = "NO_APP";
  ZeroDirective = "\t.zero\t";
  AsciiDirective = "\t.ascii\t";
  AscizDirective = "\t.asciz\t";
  Data8bitsDirective = "\t.byte\t";
  Data16bitsDirective = "\t.short\t";
  Data32bitsDirective = "\t.long\t";
  Data64bitsDirective = "\t.quad\t";
  GlobalDirective = "\t.globl\t";
  WeakDirective = "\t.weak\t";
  if (DwarfExtendedLoc != Default)
    SupportsExtendedDwarfLocDirective = DwarfExtendedLoc == Enable;
  if (UseLEB128Directives != cl::BOU_UNSET)
    HasLEB128Directives = UseLEB128Directives == cl::BOU_TRUE;
  UseIntegratedAssembler = true;
  ParseInlineAsmUsingAsmParser = false;
  PreserveAsmComments = true;
  PPCUseFullRegisterNames = false;
}

MCAsmInfo::~MCAsmInfo() = default;

void MCAsmInfo::addInitialFrameState(const MCCFIInstruction &Inst) {
  InitialFrameState.push_back(Inst);
}

const MCExpr *
MCAsmInfo::getExprForPersonalitySymbol(const MCSymbol *Sym,
                                       unsigned Encoding,
                                       MCStreamer &Streamer) const {
  return getExprForFDESymbol(Sym, Encoding, Streamer);
}

const MCExpr *
MCAsmInfo::getExprForFDESymbol(const MCSymbol *Sym,
                               unsigned Encoding,
                               MCStreamer &Streamer) const {
  if (!(Encoding & dwarf::DW_EH_PE_pcrel))
    return MCSymbolRefExpr::create(Sym, Streamer.getContext());

  MCContext &Context = Streamer.getContext();
  const MCExpr *Res = MCSymbolRefExpr::create(Sym, Context);
  MCSymbol *PCSym = Context.createTempSymbol();
  Streamer.emitLabel(PCSym);
  const MCExpr *PC = MCSymbolRefExpr::create(PCSym, Context);
  return MCBinaryExpr::createSub(Res, PC, Context);
}

bool MCAsmInfo::isAcceptableChar(char C) const {
  if (C == '@')
    return doesAllowAtInName();

  return isAlnum(C) || C == '_' || C == '$' || C == '.';
}

bool MCAsmInfo::isValidUnquotedName(StringRef Name) const {
  if (Name.empty())
    return false;

  // If any of the characters in the string is an unacceptable character, force
  // quotes.
  for (char C : Name) {
    if (!isAcceptableChar(C))
      return false;
  }

  return true;
}

bool MCAsmInfo::shouldOmitSectionDirective(StringRef SectionName) const {
  // FIXME: Does .section .bss/.data/.text work everywhere??
  return SectionName == ".text" || SectionName == ".data" ||
        (SectionName == ".bss" && !usesELFSectionDirectiveForBSS());
}

void MCAsmInfo::initializeAtSpecifiers(ArrayRef<AtSpecifier> Descs) {
  assert(AtSpecifierToName.empty() && "cannot initialize twice");
  for (auto Desc : Descs) {
    [[maybe_unused]] auto It =
        AtSpecifierToName.try_emplace(Desc.Kind, Desc.Name);
    assert(It.second && "duplicate Kind");
    [[maybe_unused]] auto It2 =
        NameToAtSpecifier.try_emplace(Desc.Name.lower(), Desc.Kind);
    assert(It2.second);
  }
}

StringRef MCAsmInfo::getSpecifierName(uint32_t S) const {
  auto It = AtSpecifierToName.find(S);
  assert(It != AtSpecifierToName.end() &&
         "ensure the specifier is set in initializeVariantKinds");
  return It->second;
}

std::optional<uint32_t> MCAsmInfo::getSpecifierForName(StringRef Name) const {
  auto It = NameToAtSpecifier.find(Name.lower());
  if (It != NameToAtSpecifier.end())
    return It->second;
  return {};
}

void MCAsmInfo::printExpr(raw_ostream &OS, const MCExpr &Expr) const {
  if (auto *SE = dyn_cast<MCSpecifierExpr>(&Expr))
    printSpecifierExpr(OS, *SE);
  else
    Expr.print(OS, this);
}

bool MCAsmInfo::evaluateAsRelocatableImpl(const MCSpecifierExpr &E,
                                          MCValue &Res,
                                          const MCAssembler *Asm) const {
  if (!E.getSubExpr()->evaluateAsRelocatable(Res, Asm))
    return false;

  Res.setSpecifier(E.getSpecifier());
  return !Res.getSubSym();
}
