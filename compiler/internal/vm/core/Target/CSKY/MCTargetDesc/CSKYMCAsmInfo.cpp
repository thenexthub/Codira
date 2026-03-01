//===-- CSKYMCAsmInfo.cpp - CSKY Asm properties ---------------------------===//
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
// This file contains the declarations of the CSKYMCAsmInfo properties.
//
//===----------------------------------------------------------------------===//

#include "CSKYMCAsmInfo.h"
#include "MCTargetDesc/CSKYMCAsmInfo.h"
#include "vm/core/BinaryFormat/Dwarf.h"
#include "vm/core/MC/MCExpr.h"
#include "vm/core/MC/MCStreamer.h"

using namespace vm::core;

const MCAsmInfo::AtSpecifier atSpecifiers[] = {
    {CSKY::S_GOT, "GOT"},       {CSKY::S_GOTOFF, "GOTOFF"},
    {CSKY::S_PLT, "PLT"},       {CSKY::S_TLSGD, "TLSGD"},
    {CSKY::S_TLSLDM, "TLSLDM"}, {CSKY::S_TPOFF, "TPOFF"},
};

void CSKYMCAsmInfo::anchor() {}

CSKYMCAsmInfo::CSKYMCAsmInfo(const Triple &TargetTriple) {
  AlignmentIsInBytes = false;
  SupportsDebugInformation = true;
  CommentString = "#";

  // Uses '.section' before '.bss' directive
  UsesELFSectionDirectiveForBSS = true;

  ExceptionsType = ExceptionHandling::DwarfCFI;

  initializeAtSpecifiers(atSpecifiers);
}

static StringRef getVariantKindName(uint8_t Kind) {
  using namespace CSKY;
  switch (Kind) {
  default:
    llvm_unreachable("Invalid ELF symbol kind");
  case S_None:
  case S_ADDR:
    return "";
  case S_ADDR_HI16:
    return "@HI16";
  case S_ADDR_LO16:
    return "@LO16";
  case S_GOT_IMM18_BY4:
  case S_GOT:
    return "@GOT";
  case S_GOTPC:
    return "@GOTPC";
  case S_GOTOFF:
    return "@GOTOFF";
  case S_PLT_IMM18_BY4:
  case S_PLT:
    return "@PLT";
  case S_TLSLE:
    return "@TPOFF";
  case S_TLSIE:
    return "@GOTTPOFF";
  case S_TLSGD:
    return "@TLSGD32";
  case S_TLSLDO:
    return "@TLSLDO32";
  case S_TLSLDM:
    return "@TLSLDM32";
  }
}

void CSKYMCAsmInfo::printSpecifierExpr(raw_ostream &OS,
                                       const MCSpecifierExpr &Expr) const {
  printExpr(OS, *Expr.getSubExpr());
  OS << getVariantKindName(Expr.getSpecifier());
}
