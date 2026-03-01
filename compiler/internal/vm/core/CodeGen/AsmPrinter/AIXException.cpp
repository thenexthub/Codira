//===-- CodeGen/AsmPrinter/AIXException.cpp - AIX Exception Impl ----------===//
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
// This file contains support for writing AIX exception info into asm files.
//
//===----------------------------------------------------------------------===//

#include "DwarfException.h"
#include "vm/core/CodeGen/AsmPrinter.h"
#include "vm/core/CodeGen/MachineModuleInfo.h"
#include "vm/core/CodeGen/TargetLoweringObjectFileImpl.h"
#include "vm/core/IR/Module.h"
#include "vm/core/MC/MCSectionXCOFF.h"
#include "vm/core/MC/MCStreamer.h"
#include "vm/core/Target/TargetLoweringObjectFile.h"
#include "vm/core/Target/TargetMachine.h"

using namespace vm::core;

AIXException::AIXException(AsmPrinter *A) : EHStreamer(A) {}

void AIXException::emitExceptionInfoTable(const MCSymbol *LSDA,
                                          const MCSymbol *PerSym) {
  // Generate EH Info Table.
  // The EH Info Table, aka, 'compat unwind section' on AIX, have the following
  // format: struct eh_info_t {
  //   unsigned version;           /* EH info verion 0 */
  // #if defined(__64BIT__)
  //   char _pad[4];               /* padding */
  // #endif
  //   unsigned long lsda;         /* Pointer to LSDA */
  //   unsigned long personality;  /* Pointer to the personality routine */
  //   }

  auto *EHInfo = static_cast<MCSectionXCOFF *>(
      Asm->getObjFileLowering().getCompactUnwindSection());
  if (Asm->TM.getFunctionSections()) {
    // If option -ffunction-sections is on, append the function name to the
    // name of EH Info Table csect so that each function has its own EH Info
    // Table csect. This helps the linker to garbage-collect EH info of unused
    // functions.
    SmallString<128> NameStr = EHInfo->getName();
    raw_svector_ostream(NameStr) << '.' << Asm->MF->getFunction().getName();
    EHInfo = Asm->OutContext.getXCOFFSection(NameStr, EHInfo->getKind(),
                                             EHInfo->getCsectProp());
  }
  Asm->OutStreamer->switchSection(EHInfo);
  MCSymbol *EHInfoLabel =
      TargetLoweringObjectFileXCOFF::getEHInfoTableSymbol(Asm->MF);
  Asm->OutStreamer->emitLabel(EHInfoLabel);

  // Version number.
  Asm->emitInt32(0);

  const DataLayout &DL = MMI->getModule()->getDataLayout();
  const unsigned PointerSize = DL.getPointerSize();

  // Add necessary paddings in 64 bit mode.
  Asm->OutStreamer->emitValueToAlignment(Align(PointerSize));

  // LSDA location.
  Asm->OutStreamer->emitValue(MCSymbolRefExpr::create(LSDA, Asm->OutContext),
                              PointerSize);

  // Personality routine.
  Asm->OutStreamer->emitValue(MCSymbolRefExpr::create(PerSym, Asm->OutContext),
                              PointerSize);
}

void AIXException::endFunction(const MachineFunction *MF) {
  // There is no easy way to access register information in `AIXException`
  // class. when ShouldEmitEHBlock is false and VRs are saved, A dumy eh info
  // table are emitted in PPCAIXAsmPrinter::emitFunctionBodyEnd.
  if (!TargetLoweringObjectFileXCOFF::ShouldEmitEHBlock(MF))
    return;

  const MCSymbol *LSDALabel = emitExceptionTable();

  const Function &F = MF->getFunction();
  assert(F.hasPersonalityFn() &&
         "Landingpads are presented, but no personality routine is found.");
  const auto *Per =
      cast<GlobalValue>(F.getPersonalityFn()->stripPointerCasts());
  const MCSymbol *PerSym = Asm->TM.getSymbol(Per);

  emitExceptionInfoTable(LSDALabel, PerSym);
}
