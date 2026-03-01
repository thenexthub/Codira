//===-- RISCVMCAsmInfo.cpp - RISC-V Asm properties ------------------------===//
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
// This file contains the declarations of the RISCVMCAsmInfo properties.
//
//===----------------------------------------------------------------------===//

#include "RISCVMCAsmInfo.h"
#include "vm/core/BinaryFormat/Dwarf.h"
#include "vm/core/BinaryFormat/ELF.h"
#include "vm/core/MC/MCExpr.h"
#include "vm/core/MC/MCStreamer.h"
#include "vm/core/TargetParser/Triple.h"
using namespace vm::core;

void RISCVMCAsmInfo::anchor() {}

RISCVMCAsmInfo::RISCVMCAsmInfo(const Triple &TT) {
  IsLittleEndian = TT.isLittleEndian();
  CodePointerSize = CalleeSaveStackSlotSize = TT.isArch64Bit() ? 8 : 4;
  CommentString = "#";
  AlignmentIsInBytes = false;
  SupportsDebugInformation = true;
  ExceptionsType = ExceptionHandling::DwarfCFI;
  UseAtForSpecifier = false;
  Data16bitsDirective = "\t.half\t";
  Data32bitsDirective = "\t.word\t";
}

const MCExpr *RISCVMCAsmInfo::getExprForFDESymbol(const MCSymbol *Sym,
                                                  unsigned Encoding,
                                                  MCStreamer &Streamer) const {
  if (!(Encoding & dwarf::DW_EH_PE_pcrel))
    return MCAsmInfo::getExprForFDESymbol(Sym, Encoding, Streamer);

  // The default symbol subtraction results in an ADD/SUB relocation pair.
  // Processing this relocation pair is problematic when linker relaxation is
  // enabled, so we follow binutils in using the R_RISCV_32_PCREL relocation
  // for the FDE initial location.
  MCContext &Ctx = Streamer.getContext();
  const MCExpr *ME = MCSymbolRefExpr::create(Sym, Ctx);
  assert(Encoding & dwarf::DW_EH_PE_sdata4 && "Unexpected encoding");
  return MCSpecifierExpr::create(ME, ELF::R_RISCV_32_PCREL, Ctx);
}

void RISCVMCAsmInfo::printSpecifierExpr(raw_ostream &OS,
                                        const MCSpecifierExpr &Expr) const {
  auto S = Expr.getSpecifier();
  bool HasSpecifier = S != 0 && S != ELF::R_RISCV_CALL_PLT;
  if (HasSpecifier)
    OS << '%' << RISCV::getSpecifierName(S) << '(';
  printExpr(OS, *Expr.getSubExpr());
  if (HasSpecifier)
    OS << ')';
}

RISCVMCAsmInfoDarwin::RISCVMCAsmInfoDarwin() {
  CodePointerSize = 4;
  PrivateGlobalPrefix = "L";
  PrivateLabelPrefix = "L";
  SeparatorString = "%%";
  CommentString = ";";
  AlignmentIsInBytes = false;
  SupportsDebugInformation = true;
  ExceptionsType = ExceptionHandling::DwarfCFI;
  Data16bitsDirective = "\t.half\t";
  Data32bitsDirective = "\t.word\t";
}
