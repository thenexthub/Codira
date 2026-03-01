//===-- CodeGen/AsmPrinter/ARMException.cpp - ARM EHABI Exception Impl ----===//
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
#include "vm/core/ADT/Twine.h"
#include "vm/core/CodeGen/AsmPrinter.h"
#include "vm/core/CodeGen/MachineFunction.h"
#include "vm/core/IR/Function.h"
#include "vm/core/MC/MCAsmInfo.h"
#include "vm/core/MC/MCStreamer.h"
using namespace vm::core;

ARMException::ARMException(AsmPrinter *A) : EHStreamer(A) {}

ARMException::~ARMException() = default;

ARMTargetStreamer &ARMException::getTargetStreamer() {
  MCTargetStreamer &TS = *Asm->OutStreamer->getTargetStreamer();
  return static_cast<ARMTargetStreamer &>(TS);
}

void ARMException::beginFunction(const MachineFunction *MF) {
  if (Asm->MAI->getExceptionHandlingType() == ExceptionHandling::ARM)
    getTargetStreamer().emitFnStart();
  // See if we need call frame info.
  AsmPrinter::CFISection CFISecType = Asm->getFunctionCFISectionType(*MF);
  assert(CFISecType != AsmPrinter::CFISection::EH &&
         "non-EH CFI not yet supported in prologue with EHABI lowering");

  if (CFISecType == AsmPrinter::CFISection::Debug) {
    if (!hasEmittedCFISections) {
      if (Asm->getModuleCFISectionType() == AsmPrinter::CFISection::Debug)
        Asm->OutStreamer->emitCFISections(false, true, false);
      hasEmittedCFISections = true;
    }

    shouldEmitCFI = true;
    Asm->OutStreamer->emitCFIStartProc(false);
  }
}

void ARMException::markFunctionEnd() {
  if (shouldEmitCFI)
    Asm->OutStreamer->emitCFIEndProc();
}

/// endFunction - Gather and emit post-function exception information.
///
void ARMException::endFunction(const MachineFunction *MF) {
  ARMTargetStreamer &ATS = getTargetStreamer();
  const Function &F = MF->getFunction();
  const Function *Per = nullptr;
  if (F.hasPersonalityFn())
    Per = dyn_cast<Function>(F.getPersonalityFn()->stripPointerCasts());
  bool forceEmitPersonality =
    F.hasPersonalityFn() && !isNoOpWithoutInvoke(classifyEHPersonality(Per)) &&
    F.needsUnwindTableEntry();
  bool shouldEmitPersonality = forceEmitPersonality ||
    !MF->getLandingPads().empty();
  if (!Asm->MF->getFunction().needsUnwindTableEntry() &&
      !shouldEmitPersonality)
    ATS.emitCantUnwind();
  else if (shouldEmitPersonality) {
    // Emit references to personality.
    if (Per) {
      MCSymbol *PerSym = Asm->getSymbol(Per);
      ATS.emitPersonality(PerSym);
    }

    // Emit .handlerdata directive.
    ATS.emitHandlerData();

    // Emit actual exception table
    emitExceptionTable();
  }

  if (Asm->MAI->getExceptionHandlingType() == ExceptionHandling::ARM)
    ATS.emitFnEnd();
}

void ARMException::emitTypeInfos(unsigned TTypeEncoding,
                                 MCSymbol *TTBaseLabel) {
  const MachineFunction *MF = Asm->MF;
  const std::vector<const GlobalValue *> &TypeInfos = MF->getTypeInfos();
  const std::vector<unsigned> &FilterIds = MF->getFilterIds();

  bool VerboseAsm = Asm->OutStreamer->isVerboseAsm();

  int Entry = 0;
  // Emit the Catch TypeInfos.
  if (VerboseAsm && !TypeInfos.empty()) {
    Asm->OutStreamer->AddComment(">> Catch TypeInfos <<");
    Asm->OutStreamer->addBlankLine();
    Entry = TypeInfos.size();
  }

  for (const GlobalValue *GV : reverse(TypeInfos)) {
    if (VerboseAsm)
      Asm->OutStreamer->AddComment("TypeInfo " + Twine(Entry--));
    Asm->emitTTypeReference(GV, TTypeEncoding);
  }

  Asm->OutStreamer->emitLabel(TTBaseLabel);

  // Emit the Exception Specifications.
  if (VerboseAsm && !FilterIds.empty()) {
    Asm->OutStreamer->AddComment(">> Filter TypeInfos <<");
    Asm->OutStreamer->addBlankLine();
    Entry = 0;
  }
  for (std::vector<unsigned>::const_iterator
         I = FilterIds.begin(), E = FilterIds.end(); I < E; ++I) {
    unsigned TypeID = *I;
    if (VerboseAsm) {
      --Entry;
      if (TypeID != 0)
        Asm->OutStreamer->AddComment("FilterInfo " + Twine(Entry));
    }

    Asm->emitTTypeReference((TypeID == 0 ? nullptr : TypeInfos[TypeID - 1]),
                            TTypeEncoding);
  }
}
