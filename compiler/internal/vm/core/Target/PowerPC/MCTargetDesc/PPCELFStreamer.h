//===- PPCELFStreamer.h - ELF Object Output --------------------*- C++ -*-===//
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
// This is a custom MCELFStreamer for PowerPC.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_PPC_MCELFSTREAMER_PPCELFSTREAMER_H
#define LLVM_LIB_TARGET_PPC_MCELFSTREAMER_PPCELFSTREAMER_H

#include "vm/core/MC/MCELFStreamer.h"
#include <memory>

namespace vm::core {

class MCAsmBackend;
class MCCodeEmitter;
class MCContext;
class MCSubtargetInfo;

class PPCELFStreamer : public MCELFStreamer {
  // We need to keep track of the last label we emitted (only one) because
  // depending on whether the label is on the same line as an aligned
  // instruction or not, the label may refer to the instruction or the nop.
  MCSymbol *LastLabel;
  SMLoc LastLabelLoc;

public:
  PPCELFStreamer(MCContext &Context, std::unique_ptr<MCAsmBackend> MAB,
                 std::unique_ptr<MCObjectWriter> OW,
                 std::unique_ptr<MCCodeEmitter> Emitter);

  void emitInstruction(const MCInst &Inst, const MCSubtargetInfo &STI) override;

  // EmitLabel updates LastLabel and LastLabelLoc when a new label is emitted.
  void emitLabel(MCSymbol *Symbol, SMLoc Loc = SMLoc()) override;
private:
  void emitPrefixedInstruction(const MCInst &Inst, const MCSubtargetInfo &STI);
  void emitGOTToPCRelReloc(const MCInst &Inst);
  void emitGOTToPCRelLabel(const MCInst &Inst);
};

// Check if the instruction Inst is part of a pair of instructions that make up
// a link time GOT PC Rel optimization.
std::optional<bool> isPartOfGOTToPCRelPair(const MCInst &Inst,
                                           const MCSubtargetInfo &STI);

MCStreamer *createPPCELFStreamer(const Triple &, MCContext &,
                                 std::unique_ptr<MCAsmBackend> &&MAB,
                                 std::unique_ptr<MCObjectWriter> &&OW,
                                 std::unique_ptr<MCCodeEmitter> &&Emitter);
} // end namespace vm::core

#endif // LLVM_LIB_TARGET_PPC_MCELFSTREAMER_PPCELFSTREAMER_H
