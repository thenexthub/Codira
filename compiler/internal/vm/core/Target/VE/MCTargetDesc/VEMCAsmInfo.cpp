//===- VEMCAsmInfo.cpp - VE asm properties --------------------------------===//
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
// This file contains the declarations of the VEMCAsmInfo properties.
//
//===----------------------------------------------------------------------===//

#include "VEMCAsmInfo.h"
#include "vm/core/MC/MCExpr.h"
#include "vm/core/MC/MCStreamer.h"
#include "vm/core/MC/MCValue.h"
#include "vm/core/TargetParser/Triple.h"

using namespace vm::core;

const MCAsmInfo::AtSpecifier atSpecifiers[] = {
    {VE::S_HI32, "hi"},
    {VE::S_LO32, "lo"},
    {VE::S_PC_HI32, "pc_hi"},
    {VE::S_PC_LO32, "pc_lo"},
    {VE::S_GOT_HI32, "got_hi"},
    {VE::S_GOT_LO32, "got_lo"},
    {VE::S_GOTOFF_HI32, "gotoff_hi"},
    {VE::S_GOTOFF_LO32, "gotoff_lo"},
    {VE::S_PLT_HI32, "plt_hi"},
    {VE::S_PLT_LO32, "plt_lo"},
    {VE::S_TLS_GD_HI32, "tls_gd_hi"},
    {VE::S_TLS_GD_LO32, "tls_gd_lo"},
    {VE::S_TPOFF_HI32, "tpoff_hi"},
    {VE::S_TPOFF_LO32, "tpoff_lo"},
};

VE::Fixups VE::getFixupKind(uint8_t S) {
  switch (S) {
  default:
    llvm_unreachable("Unhandled VEMCExpr::Specifier");
  case VE::S_REFLONG:
    return VE::fixup_ve_reflong;
  case VE::S_HI32:
    return VE::fixup_ve_hi32;
  case VE::S_LO32:
    return VE::fixup_ve_lo32;
  case VE::S_PC_HI32:
    return VE::fixup_ve_pc_hi32;
  case VE::S_PC_LO32:
    return VE::fixup_ve_pc_lo32;
  case VE::S_GOT_HI32:
    return VE::fixup_ve_got_hi32;
  case VE::S_GOT_LO32:
    return VE::fixup_ve_got_lo32;
  case VE::S_GOTOFF_HI32:
    return VE::fixup_ve_gotoff_hi32;
  case VE::S_GOTOFF_LO32:
    return VE::fixup_ve_gotoff_lo32;
  case VE::S_PLT_HI32:
    return VE::fixup_ve_plt_hi32;
  case VE::S_PLT_LO32:
    return VE::fixup_ve_plt_lo32;
  case VE::S_TLS_GD_HI32:
    return VE::fixup_ve_tls_gd_hi32;
  case VE::S_TLS_GD_LO32:
    return VE::fixup_ve_tls_gd_lo32;
  case VE::S_TPOFF_HI32:
    return VE::fixup_ve_tpoff_hi32;
  case VE::S_TPOFF_LO32:
    return VE::fixup_ve_tpoff_lo32;
  }
}

void VEELFMCAsmInfo::anchor() {}

VEELFMCAsmInfo::VEELFMCAsmInfo(const Triple &TheTriple) {

  CodePointerSize = CalleeSaveStackSlotSize = 8;
  MaxInstLength = MinInstAlignment = 8;

  // VE uses ".*byte" directive for unaligned data.
  Data8bitsDirective = "\t.byte\t";
  Data16bitsDirective = "\t.2byte\t";
  Data32bitsDirective = "\t.4byte\t";
  Data64bitsDirective = "\t.8byte\t";

  // Uses '.section' before '.bss' directive.  VE requires this although
  // assembler manual says sinple '.bss' is supported.
  UsesELFSectionDirectiveForBSS = true;

  SupportsDebugInformation = true;

  initializeAtSpecifiers(atSpecifiers);
}

void VEELFMCAsmInfo::printSpecifierExpr(raw_ostream &OS,
                                        const MCSpecifierExpr &Expr) const {
  printExpr(OS, *Expr.getSubExpr());
  auto specifier = Expr.getSpecifier();
  if (specifier && specifier != VE::S_REFLONG)
    OS << '@' << getSpecifierName(specifier);
}

bool VEELFMCAsmInfo::evaluateAsRelocatableImpl(const MCSpecifierExpr &Expr,
                                               MCValue &Res,
                                               const MCAssembler *Asm) const {
  if (!Expr.getSubExpr()->evaluateAsRelocatable(Res, Asm))
    return false;
  Res.setSpecifier(Expr.getSpecifier());
  return true;
}
