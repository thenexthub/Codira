//===-------- PPCXCOFFStreamer.cpp - XCOFF Object Output ------------------===//
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
// This is a custom MCXCOFFStreamer for PowerPC.
//
// The purpose of the custom XCOFF streamer is to allow us to intercept
// instructions as they are being emitted and align all 8 byte instructions
// to a 64 byte boundary if required (by adding a 4 byte nop). This is important
// because 8 byte instructions are not allowed to cross 64 byte boundaries
// and by aligning anything that is within 4 bytes of the boundary we can
// guarantee that the 8 byte instructions do not cross that boundary.
//
//===----------------------------------------------------------------------===//

#include "PPCXCOFFStreamer.h"
#include "PPCMCCodeEmitter.h"
#include "vm/core/MC/MCAsmBackend.h"
#include "vm/core/MC/MCAssembler.h"
#include "vm/core/MC/MCCodeEmitter.h"
#include "vm/core/MC/MCObjectWriter.h"
#include "vm/core/MC/TargetRegistry.h"

using namespace vm::core;

PPCXCOFFStreamer::PPCXCOFFStreamer(MCContext &Context,
                                   std::unique_ptr<MCAsmBackend> MAB,
                                   std::unique_ptr<MCObjectWriter> OW,
                                   std::unique_ptr<MCCodeEmitter> Emitter)
    : MCXCOFFStreamer(Context, std::move(MAB), std::move(OW),
                      std::move(Emitter)) {}

void PPCXCOFFStreamer::emitPrefixedInstruction(const MCInst &Inst,
                                               const MCSubtargetInfo &STI) {
  // Prefixed instructions must not cross a 64-byte boundary (i.e. prefix is
  // before the boundary and the remaining 4-bytes are after the boundary). In
  // order to achieve this, a nop is added prior to any such boundary-crossing
  // prefixed instruction. Align to 64 bytes if possible but add a maximum of 4
  // bytes when trying to do that. If alignment requires adding more than 4
  // bytes then the instruction won't be aligned.
  emitCodeAlignment(Align(64), &STI, 4);

  // Emit the instruction.
  // Since the previous emit created a new fragment then adding this instruction
  // also forces the addition of a new fragment. Inst is now the first
  // instruction in that new fragment.
  MCXCOFFStreamer::emitInstruction(Inst, STI);
}

void PPCXCOFFStreamer::emitInstruction(const MCInst &Inst,
                                       const MCSubtargetInfo &STI) {
  PPCMCCodeEmitter *Emitter =
      static_cast<PPCMCCodeEmitter *>(getAssembler().getEmitterPtr());

  // Special handling is only for prefixed instructions.
  if (!Emitter->isPrefixedInstruction(Inst)) {
    MCXCOFFStreamer::emitInstruction(Inst, STI);
    return;
  }
  emitPrefixedInstruction(Inst, STI);
}

MCStreamer *
toolchain::createPPCXCOFFStreamer(const Triple &, MCContext &C,
                             std::unique_ptr<MCAsmBackend> &&MAB,
                             std::unique_ptr<MCObjectWriter> &&OW,
                             std::unique_ptr<MCCodeEmitter> &&Emitter) {
  return new PPCXCOFFStreamer(C, std::move(MAB), std::move(OW),
                              std::move(Emitter));
}
