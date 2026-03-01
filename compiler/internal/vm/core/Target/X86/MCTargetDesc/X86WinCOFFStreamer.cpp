//===-- X86WinCOFFStreamer.cpp - X86 Target WinCOFF Streamer ----*- C++ -*-===//
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

#include "X86MCTargetDesc.h"
#include "X86TargetStreamer.h"
#include "vm/core/MC/MCAsmBackend.h"
#include "vm/core/MC/MCAssembler.h"
#include "vm/core/MC/MCCodeEmitter.h"
#include "vm/core/MC/MCObjectWriter.h"
#include "vm/core/MC/MCWin64EH.h"
#include "vm/core/MC/MCWinCOFFStreamer.h"

using namespace vm::core;

namespace {
class X86WinCOFFStreamer : public MCWinCOFFStreamer {
  Win64EH::UnwindEmitter EHStreamer;
public:
  X86WinCOFFStreamer(MCContext &C, std::unique_ptr<MCAsmBackend> AB,
                     std::unique_ptr<MCCodeEmitter> CE,
                     std::unique_ptr<MCObjectWriter> OW)
      : MCWinCOFFStreamer(C, std::move(AB), std::move(CE), std::move(OW)) {}

  void emitInstruction(const MCInst &Inst, const MCSubtargetInfo &STI) override;
  void emitWinEHHandlerData(SMLoc Loc) override;
  void emitWindowsUnwindTables(WinEH::FrameInfo *Frame) override;
  void emitWindowsUnwindTables() override;
  void emitCVFPOData(const MCSymbol *ProcSym, SMLoc Loc) override;
  void finishImpl() override;
};

void X86WinCOFFStreamer::emitInstruction(const MCInst &Inst,
                                         const MCSubtargetInfo &STI) {
  X86_MC::emitInstruction(*this, Inst, STI);
}

void X86WinCOFFStreamer::emitWinEHHandlerData(SMLoc Loc) {
  MCStreamer::emitWinEHHandlerData(Loc);

  // We have to emit the unwind info now, because this directive
  // actually switches to the .xdata section.
  if (WinEH::FrameInfo *CurFrame = getCurrentWinFrameInfo())
    EHStreamer.EmitUnwindInfo(*this, CurFrame, /* HandlerData = */ true);
}

void X86WinCOFFStreamer::emitWindowsUnwindTables(WinEH::FrameInfo *Frame) {
  EHStreamer.EmitUnwindInfo(*this, Frame, /* HandlerData = */ false);
}

void X86WinCOFFStreamer::emitWindowsUnwindTables() {
  if (!getNumWinFrameInfos())
    return;
  EHStreamer.Emit(*this);
}

void X86WinCOFFStreamer::emitCVFPOData(const MCSymbol *ProcSym, SMLoc Loc) {
  X86TargetStreamer *XTS =
      static_cast<X86TargetStreamer *>(getTargetStreamer());
  XTS->emitFPOData(ProcSym, Loc);
}

void X86WinCOFFStreamer::finishImpl() {
  emitFrames();
  emitWindowsUnwindTables();

  MCWinCOFFStreamer::finishImpl();
}
} // namespace

MCStreamer *
toolchain::createX86WinCOFFStreamer(MCContext &C, std::unique_ptr<MCAsmBackend> &&AB,
                               std::unique_ptr<MCObjectWriter> &&OW,
                               std::unique_ptr<MCCodeEmitter> &&CE) {
  return new X86WinCOFFStreamer(C, std::move(AB), std::move(CE), std::move(OW));
}
