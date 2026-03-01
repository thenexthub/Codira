//===-- HexagonMCAsmInfo.cpp - Hexagon asm properties ---------------------===//
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
// This file contains the declarations of the HexagonMCAsmInfo properties.
//
//===----------------------------------------------------------------------===//

#include "HexagonMCAsmInfo.h"
#include "MCTargetDesc/HexagonMCExpr.h"
#include "vm/core/MC/MCExpr.h"

using namespace vm::core;

const MCAsmInfo::AtSpecifier atSpecifiers[] = {
    {HexagonMCExpr::VK_DTPREL, "DTPREL"}, {HexagonMCExpr::VK_GD_GOT, "GDGOT"},
    {HexagonMCExpr::VK_GD_PLT, "GDPLT"},  {HexagonMCExpr::VK_GOT, "GOT"},
    {HexagonMCExpr::VK_GOTREL, "GOTREL"}, {HexagonMCExpr::VK_IE, "IE"},
    {HexagonMCExpr::VK_IE_GOT, "IEGOT"},  {HexagonMCExpr::VK_LD_GOT, "LDGOT"},
    {HexagonMCExpr::VK_LD_PLT, "LDPLT"},  {HexagonMCExpr::VK_PCREL, "PCREL"},
    {HexagonMCExpr::VK_PLT, "PLT"},       {HexagonMCExpr::VK_TPREL, "TPREL"},
};

// Pin the vtable to this file.
void HexagonMCAsmInfo::anchor() {}

HexagonMCAsmInfo::HexagonMCAsmInfo(const Triple &TT) {
  Data16bitsDirective = "\t.half\t";
  Data32bitsDirective = "\t.word\t";
  Data64bitsDirective = nullptr;  // .xword is only supported by V9.
  CommentString = "//";
  SupportsDebugInformation = true;

  LCOMMDirectiveAlignmentType = LCOMM::ByteAlignment;
  InlineAsmStart = "# InlineAsm Start";
  InlineAsmEnd = "# InlineAsm End";
  UsesSetToEquateSymbol = true;
  ZeroDirective = "\t.space\t";
  AscizDirective = "\t.string\t";

  MinInstAlignment = 4;
  UsesELFSectionDirectiveForBSS  = true;
  ExceptionsType = ExceptionHandling::DwarfCFI;
  UseLogicalShr = false;

  initializeAtSpecifiers(atSpecifiers);
}
