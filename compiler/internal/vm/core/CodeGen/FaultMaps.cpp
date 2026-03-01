//===- FaultMaps.cpp ------------------------------------------------------===//
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

#include "vm/core/CodeGen/FaultMaps.h"
#include "vm/core/ADT/Twine.h"
#include "vm/core/CodeGen/AsmPrinter.h"
#include "vm/core/MC/MCContext.h"
#include "vm/core/MC/MCExpr.h"
#include "vm/core/MC/MCObjectFileInfo.h"
#include "vm/core/MC/MCStreamer.h"
#include "vm/core/Support/Debug.h"
#include "vm/core/Support/ErrorHandling.h"

using namespace vm::core;

#define DEBUG_TYPE "faultmaps"

static const int FaultMapVersion = 1;
const char *FaultMaps::WFMP = "Fault Maps: ";

FaultMaps::FaultMaps(AsmPrinter &AP) : AP(AP) {}

void FaultMaps::recordFaultingOp(FaultKind FaultTy,
                                 const MCSymbol *FaultingLabel,
                                 const MCSymbol *HandlerLabel) {
  MCContext &OutContext = AP.OutStreamer->getContext();

  const MCExpr *FaultingOffset = MCBinaryExpr::createSub(
      MCSymbolRefExpr::create(FaultingLabel, OutContext),
      MCSymbolRefExpr::create(AP.CurrentFnSymForSize, OutContext), OutContext);

  const MCExpr *HandlerOffset = MCBinaryExpr::createSub(
      MCSymbolRefExpr::create(HandlerLabel, OutContext),
      MCSymbolRefExpr::create(AP.CurrentFnSymForSize, OutContext), OutContext);

  FunctionInfos[AP.CurrentFnSym].emplace_back(FaultTy, FaultingOffset,
                                              HandlerOffset);
}

void FaultMaps::serializeToFaultMapSection() {
  if (FunctionInfos.empty())
    return;

  MCContext &OutContext = AP.OutStreamer->getContext();
  MCStreamer &OS = *AP.OutStreamer;

  // Create the section.
  MCSection *FaultMapSection =
      OutContext.getObjectFileInfo()->getFaultMapSection();
  OS.switchSection(FaultMapSection);

  // Emit a dummy symbol to force section inclusion.
  OS.emitLabel(OutContext.getOrCreateSymbol(Twine("__LLVM_FaultMaps")));

  LLVM_DEBUG(dbgs() << "********** Fault Map Output **********\n");

  // Header
  OS.emitIntValue(FaultMapVersion, 1); // Version.
  OS.emitIntValue(0, 1);               // Reserved.
  OS.emitInt16(0);                     // Reserved.

  LLVM_DEBUG(dbgs() << WFMP << "#functions = " << FunctionInfos.size() << "\n");
  OS.emitInt32(FunctionInfos.size());

  LLVM_DEBUG(dbgs() << WFMP << "functions:\n");

  for (const auto &FFI : FunctionInfos)
    emitFunctionInfo(FFI.first, FFI.second);
}

void FaultMaps::emitFunctionInfo(const MCSymbol *FnLabel,
                                 const FunctionFaultInfos &FFI) {
  MCStreamer &OS = *AP.OutStreamer;

  LLVM_DEBUG(dbgs() << WFMP << "  function addr: " << *FnLabel << "\n");
  OS.emitSymbolValue(FnLabel, 8);

  LLVM_DEBUG(dbgs() << WFMP << "  #faulting PCs: " << FFI.size() << "\n");
  OS.emitInt32(FFI.size());

  OS.emitInt32(0); // Reserved

  for (const auto &Fault : FFI) {
    OS.emitInt32(Fault.Kind);
    OS.emitValue(Fault.FaultingOffsetExpr, 4);
    OS.emitValue(Fault.HandlerOffsetExpr, 4);
  }
}

const char *FaultMaps::faultTypeToString(FaultMaps::FaultKind FT) {
  switch (FT) {
  default:
    llvm_unreachable("unhandled fault type!");
  case FaultMaps::FaultingLoad:
    return "FaultingLoad";
  case FaultMaps::FaultingLoadStore:
    return "FaultingLoadStore";
  case FaultMaps::FaultingStore:
    return "FaultingStore";
  }
}
