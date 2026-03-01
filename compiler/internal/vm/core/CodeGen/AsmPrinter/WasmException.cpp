//===-- CodeGen/AsmPrinter/WasmException.cpp - Wasm Exception Impl --------===//
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
// This file contains support for writing WebAssembly exception info into asm
// files.
//
//===----------------------------------------------------------------------===//

#include "WasmException.h"
#include "vm/core/CodeGen/AsmPrinter.h"
#include "vm/core/CodeGen/MachineFunction.h"
#include "vm/core/IR/Mangler.h"
#include "vm/core/MC/MCContext.h"
#include "vm/core/MC/MCStreamer.h"
using namespace vm::core;

void WasmException::endFunction(const MachineFunction *MF) {
  bool ShouldEmitExceptionTable = false;
  for (const LandingPadInfo &Info : MF->getLandingPads()) {
    if (MF->hasWasmLandingPadIndex(Info.LandingPadBlock)) {
      ShouldEmitExceptionTable = true;
      break;
    }
  }
  if (!ShouldEmitExceptionTable)
    return;
  MCSymbol *LSDALabel = emitExceptionTable();
  assert(LSDALabel && ".GCC_exception_table has not been emitted!");

  // Wasm requires every data section symbol to have a .size set. So we emit an
  // end marker and set the size as the difference between the start end the end
  // marker.
  MCSymbol *LSDAEndLabel = Asm->createTempSymbol("GCC_except_table_end");
  Asm->OutStreamer->emitLabel(LSDAEndLabel);
  MCContext &OutContext = Asm->OutStreamer->getContext();
  const MCExpr *SizeExp = MCBinaryExpr::createSub(
      MCSymbolRefExpr::create(LSDAEndLabel, OutContext),
      MCSymbolRefExpr::create(LSDALabel, OutContext), OutContext);
  Asm->OutStreamer->emitELFSize(LSDALabel, SizeExp);
}

// Compute the call-site table for wasm EH. Even though we use the same function
// name to share the common routines, a call site entry in the table corresponds
// to not a call site for possibly-throwing functions but a landing pad. In wasm
// EH the VM is responsible for stack unwinding. After an exception occurs and
// the stack is unwound, the control flow is transferred to wasm 'catch'
// instruction by the VM, after which the personality function is called from
// the compiler-generated code. Refer to WasmEHPrepare pass for more
// information.
void WasmException::computeCallSiteTable(
    SmallVectorImpl<CallSiteEntry> &CallSites,
    SmallVectorImpl<CallSiteRange> &CallSiteRanges,
    const SmallVectorImpl<const LandingPadInfo *> &LandingPads,
    const SmallVectorImpl<unsigned> &FirstActions) {
  MachineFunction &MF = *Asm->MF;
  for (unsigned I = 0, N = LandingPads.size(); I < N; ++I) {
    const LandingPadInfo *Info = LandingPads[I];
    MachineBasicBlock *LPad = Info->LandingPadBlock;
    // We don't emit LSDA for single catch (...).
    if (!MF.hasWasmLandingPadIndex(LPad))
      continue;
    // Wasm EH must maintain the EH pads in the order assigned to them by the
    // WasmEHPrepare pass.
    unsigned LPadIndex = MF.getWasmLandingPadIndex(LPad);
    CallSiteEntry Site = {nullptr, nullptr, Info, FirstActions[I]};
    if (CallSites.size() < LPadIndex + 1)
      CallSites.resize(LPadIndex + 1);
    CallSites[LPadIndex] = Site;
  }
}
