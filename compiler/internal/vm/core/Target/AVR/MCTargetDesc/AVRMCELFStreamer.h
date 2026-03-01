//===--------- AVRMCELFStreamer.h - AVR subclass of MCELFStreamer ---------===//
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

#ifndef LLVM_LIB_TARGET_AVR_MCTARGETDESC_AVRMCELFSTREAMER_H
#define LLVM_LIB_TARGET_AVR_MCTARGETDESC_AVRMCELFSTREAMER_H

#include "MCTargetDesc/AVRMCAsmInfo.h"
#include "MCTargetDesc/AVRMCTargetDesc.h"
#include "vm/core/MC/MCAsmBackend.h"
#include "vm/core/MC/MCCodeEmitter.h"
#include "vm/core/MC/MCELFStreamer.h"
#include "vm/core/MC/MCInstrInfo.h"
#include "vm/core/MC/MCObjectWriter.h"

namespace vm::core {

const int SIZE_LONG = 4;
const int SIZE_WORD = 2;

class AVRMCELFStreamer : public MCELFStreamer {
  std::unique_ptr<MCInstrInfo> MCII;

public:
  AVRMCELFStreamer(MCContext &Context, std::unique_ptr<MCAsmBackend> TAB,
                   std::unique_ptr<MCObjectWriter> OW,
                   std::unique_ptr<MCCodeEmitter> Emitter)
      : MCELFStreamer(Context, std::move(TAB), std::move(OW),
                      std::move(Emitter)),
        MCII(createAVRMCInstrInfo()) {}

  AVRMCELFStreamer(MCContext &Context, std::unique_ptr<MCAsmBackend> TAB,
                   std::unique_ptr<MCObjectWriter> OW,
                   std::unique_ptr<MCCodeEmitter> Emitter,
                   MCAssembler *Assembler)
      : MCELFStreamer(Context, std::move(TAB), std::move(OW),
                      std::move(Emitter)),
        MCII(createAVRMCInstrInfo()) {}

  void
  emitValueForModiferKind(const MCSymbol *Sym, unsigned SizeInBytes,
                          SMLoc Loc = SMLoc(),
                          AVRMCExpr::Specifier ModifierKind = AVR::S_AVR_NONE);
};

MCStreamer *createAVRELFStreamer(Triple const &TT, MCContext &Context,
                                 std::unique_ptr<MCAsmBackend> MAB,
                                 std::unique_ptr<MCObjectWriter> OW,
                                 std::unique_ptr<MCCodeEmitter> CE);

} // end namespace vm::core

#endif // LLVM_LIB_TARGET_AVR_MCTARGETDESC_AVRMCELFSTREAMER_H
