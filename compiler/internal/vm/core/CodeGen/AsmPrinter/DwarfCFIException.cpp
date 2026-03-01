//===-- CodeGen/AsmPrinter/DwarfException.cpp - Dwarf Exception Impl ------===//
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
// This file contains support for writing DWARF exception info into asm files.
//
//===----------------------------------------------------------------------===//

#include "DwarfException.h"
#include "vm/core/BinaryFormat/Dwarf.h"
#include "vm/core/CodeGen/AsmPrinter.h"
#include "vm/core/CodeGen/MachineFunction.h"
#include "vm/core/CodeGen/MachineModuleInfo.h"
#include "vm/core/IR/Function.h"
#include "vm/core/MC/MCAsmInfo.h"
#include "vm/core/MC/MCContext.h"
#include "vm/core/MC/MCStreamer.h"
#include "vm/core/Target/TargetLoweringObjectFile.h"
#include "vm/core/Target/TargetMachine.h"
#include "vm/core/Target/TargetOptions.h"
using namespace vm::core;

DwarfCFIException::DwarfCFIException(AsmPrinter *A) : EHStreamer(A) {}

DwarfCFIException::~DwarfCFIException() = default;

void DwarfCFIException::addPersonality(const GlobalValue *Personality) {
  if (!toolchain::is_contained(Personalities, Personality))
    Personalities.push_back(Personality);
}

/// endModule - Emit all exception information that should come after the
/// content.
void DwarfCFIException::endModule() {
  // SjLj uses this pass and it doesn't need this info.
  if (!Asm->MAI->usesCFIForEH())
    return;

  const TargetLoweringObjectFile &TLOF = Asm->getObjFileLowering();

  unsigned PerEncoding = TLOF.getPersonalityEncoding();

  if ((PerEncoding & 0x80) != dwarf::DW_EH_PE_indirect)
    return;

  // Emit indirect reference table for all used personality functions
  for (const GlobalValue *Personality : Personalities) {
    MCSymbol *Sym = Asm->getSymbol(Personality);
    TLOF.emitPersonalityValue(*Asm->OutStreamer, Asm->getDataLayout(), Sym,
                              Asm->MMI);
  }
  Personalities.clear();
}

void DwarfCFIException::beginFunction(const MachineFunction *MF) {
  shouldEmitPersonality = shouldEmitLSDA = false;
  const Function &F = MF->getFunction();

  // If any landing pads survive, we need an EH table.
  bool hasLandingPads = !MF->getLandingPads().empty();

  // See if we need frame move info.
  bool shouldEmitMoves =
      Asm->getFunctionCFISectionType(*MF) != AsmPrinter::CFISection::None;

  const TargetLoweringObjectFile &TLOF = Asm->getObjFileLowering();
  unsigned PerEncoding = TLOF.getPersonalityEncoding();
  const GlobalValue *Per = nullptr;
  if (F.hasPersonalityFn())
    Per = dyn_cast<GlobalValue>(F.getPersonalityFn()->stripPointerCasts());

  // Emit a personality function even when there are no landing pads
  forceEmitPersonality =
      // ...if a personality function is explicitly specified
      F.hasPersonalityFn() &&
      // ... and it's not known to be a noop in the absence of invokes
      !isNoOpWithoutInvoke(classifyEHPersonality(Per)) &&
      // ... and we're not explicitly asked not to emit it
      F.needsUnwindTableEntry();

  shouldEmitPersonality =
      (forceEmitPersonality ||
       (hasLandingPads && PerEncoding != dwarf::DW_EH_PE_omit)) &&
      Per;

  unsigned LSDAEncoding = TLOF.getLSDAEncoding();
  shouldEmitLSDA = shouldEmitPersonality &&
    LSDAEncoding != dwarf::DW_EH_PE_omit;

  const MCAsmInfo &MAI = *MF->getContext().getAsmInfo();
  if (MAI.getExceptionHandlingType() != ExceptionHandling::None)
    shouldEmitCFI =
        MAI.usesCFIForEH() && (shouldEmitPersonality || shouldEmitMoves);
  else
    shouldEmitCFI = Asm->usesCFIWithoutEH() && shouldEmitMoves;
}

void DwarfCFIException::beginBasicBlockSection(const MachineBasicBlock &MBB) {
  if (!shouldEmitCFI)
    return;

  if (!hasEmittedCFISections) {
    AsmPrinter::CFISection CFISecType = Asm->getModuleCFISectionType();
    // If we don't say anything it implies `.cfi_sections .eh_frame`, so we
    // chose not to be verbose in that case. And with `ForceDwarfFrameSection`,
    // we should always emit .debug_frame.
    if (CFISecType == AsmPrinter::CFISection::Debug ||
        Asm->TM.Options.ForceDwarfFrameSection ||
        Asm->TM.Options.MCOptions.EmitSFrameUnwind)
      Asm->OutStreamer->emitCFISections(
          CFISecType == AsmPrinter::CFISection::EH, true,
          Asm->TM.Options.MCOptions.EmitSFrameUnwind);
    hasEmittedCFISections = true;
  }

  Asm->OutStreamer->emitCFIStartProc(/*IsSimple=*/false);

  // Indicate personality routine, if any.
  if (!shouldEmitPersonality)
    return;

  auto &F = MBB.getParent()->getFunction();
  auto *P = dyn_cast<GlobalValue>(F.getPersonalityFn()->stripPointerCasts());
  assert(P && "Expected personality function");
  // Record the personality function.
  addPersonality(P);

  const TargetLoweringObjectFile &TLOF = Asm->getObjFileLowering();
  unsigned PerEncoding = TLOF.getPersonalityEncoding();
  const MCSymbol *Sym = TLOF.getCFIPersonalitySymbol(P, Asm->TM, MMI);
  Asm->OutStreamer->emitCFIPersonality(Sym, PerEncoding);

  // Provide LSDA information.
  if (shouldEmitLSDA)
    Asm->OutStreamer->emitCFILsda(Asm->getMBBExceptionSym(MBB),
                                  TLOF.getLSDAEncoding());
}

void DwarfCFIException::endBasicBlockSection(const MachineBasicBlock &MBB) {
  if (shouldEmitCFI)
    Asm->OutStreamer->emitCFIEndProc();
}

/// endFunction - Gather and emit post-function exception information.
///
void DwarfCFIException::endFunction(const MachineFunction *MF) {
  if (!shouldEmitPersonality)
    return;

  emitExceptionTable();
}
