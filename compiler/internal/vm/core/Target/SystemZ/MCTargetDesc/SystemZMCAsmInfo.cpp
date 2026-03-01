//===-- SystemZMCAsmInfo.cpp - SystemZ asm properties ---------------------===//
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

#include "SystemZMCAsmInfo.h"
#include "vm/core/MC/MCContext.h"
#include "vm/core/MC/MCExpr.h"
#include "vm/core/MC/MCValue.h"

using namespace vm::core;

const MCAsmInfo::AtSpecifier atSpecifiers[] = {
    {SystemZ::S_DTPOFF, "DTPOFF"}, {SystemZ::S_GOT, "GOT"},
    {SystemZ::S_GOTENT, "GOTENT"}, {SystemZ::S_INDNTPOFF, "INDNTPOFF"},
    {SystemZ::S_NTPOFF, "NTPOFF"}, {SystemZ::S_PLT, "PLT"},
    {SystemZ::S_TLSGD, "TLSGD"},   {SystemZ::S_TLSLD, "TLSLD"},
    {SystemZ::S_TLSLDM, "TLSLDM"},
};

SystemZMCAsmInfoELF::SystemZMCAsmInfoELF(const Triple &TT) {
  AssemblerDialect = AD_GNU;
  CalleeSaveStackSlotSize = 8;
  CodePointerSize = 8;
  Data64bitsDirective = "\t.quad\t";
  ExceptionsType = ExceptionHandling::DwarfCFI;
  IsLittleEndian = false;
  MaxInstLength = 6;
  SupportsDebugInformation = true;
  UsesELFSectionDirectiveForBSS = true;
  ZeroDirective = "\t.space\t";

  initializeAtSpecifiers(atSpecifiers);
}

SystemZMCAsmInfoGOFF::SystemZMCAsmInfoGOFF(const Triple &TT) {
  AllowAdditionalComments = false;
  AllowAtInName = true;
  AllowAtAtStartOfIdentifier = true;
  AllowDollarAtStartOfIdentifier = true;
  AssemblerDialect = AD_HLASM;
  CalleeSaveStackSlotSize = 8;
  CodePointerSize = 8;
  CommentString = "*";
  UsesSetToEquateSymbol = true;
  ExceptionsType = ExceptionHandling::ZOS;
  IsHLASM = true;
  IsLittleEndian = false;
  MaxInstLength = 6;
  SupportsDebugInformation = true;

  initializeAtSpecifiers(atSpecifiers);
}

bool SystemZMCAsmInfoGOFF::isAcceptableChar(char C) const {
  return MCAsmInfo::isAcceptableChar(C) || C == '#';
}

void SystemZMCAsmInfoGOFF::printSpecifierExpr(
    raw_ostream &OS, const MCSpecifierExpr &Expr) const {
  switch (Expr.getSpecifier()) {
  case SystemZ::S_None:
    OS << "AD";
    break;
  case SystemZ::S_QCon:
    OS << "QD";
    break;
  case SystemZ::S_RCon:
    OS << "RD";
    break;
  case SystemZ::S_VCon:
    OS << "VD";
    break;
  default:
    llvm_unreachable("Invalid kind");
  }
  OS << '(';
  printExpr(OS, *Expr.getSubExpr());
  OS << ')';
}

bool SystemZMCAsmInfoGOFF::evaluateAsRelocatableImpl(
    const MCSpecifierExpr &Expr, MCValue &Res, const MCAssembler *Asm) const {
  if (!Expr.getSubExpr()->evaluateAsRelocatable(Res, Asm))
    return false;
  Res.setSpecifier(Expr.getSpecifier());
  return true;
}
