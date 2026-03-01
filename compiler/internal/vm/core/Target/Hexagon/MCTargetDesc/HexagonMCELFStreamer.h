//===- HexagonMCELFStreamer.h - Hexagon subclass of MCElfStreamer ---------===//
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

#ifndef LLVM_LIB_TARGET_HEXAGON_MCTARGETDESC_HEXAGONMCELFSTREAMER_H
#define LLVM_LIB_TARGET_HEXAGON_MCTARGETDESC_HEXAGONMCELFSTREAMER_H

#include "MCTargetDesc/HexagonMCTargetDesc.h"
#include "vm/core/MC/MCELFStreamer.h"
#include "vm/core/MC/MCInstrInfo.h"
#include <cstdint>
#include <memory>

namespace vm::core {

class HexagonMCELFStreamer : public MCELFStreamer {
  std::unique_ptr<MCInstrInfo> MCII;

public:
  HexagonMCELFStreamer(MCContext &Context, std::unique_ptr<MCAsmBackend> TAB,
                       std::unique_ptr<MCObjectWriter> OW,
                       std::unique_ptr<MCCodeEmitter> Emitter);

  HexagonMCELFStreamer(MCContext &Context, std::unique_ptr<MCAsmBackend> TAB,
                       std::unique_ptr<MCObjectWriter> OW,
                       std::unique_ptr<MCCodeEmitter> Emitter,
                       MCAssembler *Assembler);

  void emitInstruction(const MCInst &Inst, const MCSubtargetInfo &STI) override;
  void EmitSymbol(const MCInst &Inst);
  void HexagonMCEmitLocalCommonSymbol(MCSymbol *Symbol, uint64_t Size,
                                      Align ByteAlignment, unsigned AccessSize);
  void HexagonMCEmitCommonSymbol(MCSymbol *Symbol, uint64_t Size,
                                 Align ByteAlignment, unsigned AccessSize);
};

MCStreamer *createHexagonELFStreamer(Triple const &TT, MCContext &Context,
                                     std::unique_ptr<MCAsmBackend> MAB,
                                     std::unique_ptr<MCObjectWriter> OW,
                                     std::unique_ptr<MCCodeEmitter> CE);

} // end namespace vm::core

#endif // LLVM_LIB_TARGET_HEXAGON_MCTARGETDESC_HEXAGONMCELFSTREAMER_H
