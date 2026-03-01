//==-- SystemZTargetStreamer.cpp - SystemZ Target Streamer Methods ----------=//
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
///
/// \file
/// This file defines SystemZ-specific target streamer classes.
/// These are for implementing support for target-specific assembly directives.
///
//===----------------------------------------------------------------------===//

#include "SystemZTargetStreamer.h"
#include "SystemZHLASMAsmStreamer.h"
#include "vm/core/MC/MCAsmInfo.h"
#include "vm/core/MC/MCGOFFStreamer.h"
#include "vm/core/MC/MCObjectFileInfo.h"

using namespace vm::core;

void SystemZTargetStreamer::emitConstantPools() {
  // Emit EXRL target instructions.
  if (EXRLTargets2Sym.empty())
    return;
  // Switch to the .text section.
  const MCObjectFileInfo &OFI = *Streamer.getContext().getObjectFileInfo();
  Streamer.switchSection(OFI.getTextSection());
  for (auto &I : EXRLTargets2Sym) {
    Streamer.emitLabel(I.second);
    const MCInstSTIPair &MCI_STI = I.first;
    Streamer.emitInstruction(MCI_STI.first, *MCI_STI.second);
  }
  EXRLTargets2Sym.clear();
}

SystemZHLASMAsmStreamer &SystemZTargetHLASMStreamer::getHLASMStreamer() {
  return static_cast<SystemZHLASMAsmStreamer &>(getStreamer());
}

// HLASM statements can only perform a single operation at a time
const MCExpr *SystemZTargetHLASMStreamer::createWordDiffExpr(
    MCContext &Ctx, const MCSymbol *Hi, const MCSymbol *Lo) {
  assert(Hi && Lo && "Symbols required to calculate expression");
  MCSymbol *Temp = Ctx.createTempSymbol();
  OS << Temp->getName() << " EQU ";
  const MCBinaryExpr *TempExpr = MCBinaryExpr::createSub(
      MCSymbolRefExpr::create(Hi, Ctx), MCSymbolRefExpr::create(Lo, Ctx), Ctx);
  Ctx.getAsmInfo()->printExpr(OS, *TempExpr);
  OS << "\n";
  return MCBinaryExpr::createLShr(MCSymbolRefExpr::create(Temp, Ctx),
                                  MCConstantExpr::create(1, Ctx), Ctx);
}

const MCExpr *SystemZTargetGOFFStreamer::createWordDiffExpr(
    MCContext &Ctx, const MCSymbol *Hi, const MCSymbol *Lo) {
  assert(Hi && Lo && "Symbols required to calculate expression");
  return MCBinaryExpr::createLShr(
      MCBinaryExpr::createSub(MCSymbolRefExpr::create(Hi, Ctx),
                              MCSymbolRefExpr::create(Lo, Ctx), Ctx),
      MCConstantExpr::create(1, Ctx), Ctx);
}
