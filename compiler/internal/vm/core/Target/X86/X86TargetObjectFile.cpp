//===-- X86TargetObjectFile.cpp - X86 Object Info -------------------------===//
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

#include "X86TargetObjectFile.h"
#include "MCTargetDesc/X86MCAsmInfo.h"
#include "vm/core/BinaryFormat/Dwarf.h"
#include "vm/core/MC/MCExpr.h"
#include "vm/core/MC/MCValue.h"
#include "vm/core/Target/TargetMachine.h"

using namespace vm::core;
using namespace dwarf;

const MCExpr *X86_64MachoTargetObjectFile::getTTypeGlobalReference(
    const GlobalValue *GV, unsigned Encoding, const TargetMachine &TM,
    MachineModuleInfo *MMI, MCStreamer &Streamer) const {

  // On Darwin/X86-64, we can reference dwarf symbols with foo@GOTPCREL+4, which
  // is an indirect pc-relative reference.
  if ((Encoding & DW_EH_PE_indirect) && (Encoding & DW_EH_PE_pcrel)) {
    const MCSymbol *Sym = TM.getSymbol(GV);
    const MCExpr *Res =
        MCSymbolRefExpr::create(Sym, X86::S_GOTPCREL, getContext());
    const MCExpr *Four = MCConstantExpr::create(4, getContext());
    return MCBinaryExpr::createAdd(Res, Four, getContext());
  }

  return TargetLoweringObjectFileMachO::getTTypeGlobalReference(
      GV, Encoding, TM, MMI, Streamer);
}

MCSymbol *X86_64MachoTargetObjectFile::getCFIPersonalitySymbol(
    const GlobalValue *GV, const TargetMachine &TM,
    MachineModuleInfo *MMI) const {
  return TM.getSymbol(GV);
}

const MCExpr *X86_64MachoTargetObjectFile::getIndirectSymViaGOTPCRel(
    const GlobalValue *GV, const MCSymbol *Sym, const MCValue &MV,
    int64_t Offset, MachineModuleInfo *MMI, MCStreamer &Streamer) const {
  // On Darwin/X86-64, we need to use foo@GOTPCREL+4 to access the got entry
  // from a data section. In case there's an additional offset, then use
  // foo@GOTPCREL+4+<offset>.
  unsigned FinalOff = Offset+MV.getConstant()+4;
  const MCExpr *Res =
      MCSymbolRefExpr::create(Sym, X86::S_GOTPCREL, getContext());
  const MCExpr *Off = MCConstantExpr::create(FinalOff, getContext());
  return MCBinaryExpr::createAdd(Res, Off, getContext());
}

X86ELFTargetObjectFile::X86ELFTargetObjectFile() {
  PLTRelativeSpecifier = X86::S_PLT;
}

const MCExpr *X86ELFTargetObjectFile::getDebugThreadLocalSymbol(
    const MCSymbol *Sym) const {
  return MCSymbolRefExpr::create(Sym, X86::S_DTPOFF, getContext());
}

const MCExpr *X86_64ELFTargetObjectFile::getIndirectSymViaGOTPCRel(
    const GlobalValue *GV, const MCSymbol *Sym, const MCValue &MV,
    int64_t Offset, MachineModuleInfo *MMI, MCStreamer &Streamer) const {
  int64_t FinalOffset = Offset + MV.getConstant();
  const MCExpr *Res =
      MCSymbolRefExpr::create(Sym, X86::S_GOTPCREL, getContext());
  const MCExpr *Off = MCConstantExpr::create(FinalOffset, getContext());
  return MCBinaryExpr::createAdd(Res, Off, getContext());
}
