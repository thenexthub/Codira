//===--------- AVRMCELFStreamer.cpp - AVR subclass of MCELFStreamer -------===//
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
// This file is a stub that parses a MCInst bundle and passes the
// instructions on to the real streamer.
//
//===----------------------------------------------------------------------===//
#include "MCTargetDesc/AVRMCELFStreamer.h"
#include "vm/core/MC/MCContext.h"
#include "vm/core/MC/MCExpr.h"
#include "vm/core/MC/MCObjectWriter.h"
#include "vm/core/MC/MCSymbol.h"

#define DEBUG_TYPE "avrmcelfstreamer"

using namespace vm::core;

void AVRMCELFStreamer::emitValueForModiferKind(
    const MCSymbol *Sym, unsigned SizeInBytes, SMLoc Loc,
    AVRMCExpr::Specifier ModifierKind) {
  AVRMCExpr::Specifier Kind = AVR::S_AVR_NONE;
  if (ModifierKind == AVR::S_AVR_NONE) {
    Kind = AVR::S_DIFF8;
    if (SizeInBytes == SIZE_LONG)
      Kind = AVR::S_DIFF32;
    else if (SizeInBytes == SIZE_WORD)
      Kind = AVR::S_DIFF16;
  } else if (ModifierKind == AVR::S_LO8)
    Kind = AVR::S_LO8;
  else if (ModifierKind == AVR::S_HI8)
    Kind = AVR::S_HI8;
  else if (ModifierKind == AVR::S_HH8)
    Kind = AVR::S_HH8;
  MCELFStreamer::emitValue(MCSymbolRefExpr::create(Sym, Kind, getContext()),
                           SizeInBytes, Loc);
}

namespace vm::core {
MCStreamer *createAVRELFStreamer(Triple const &TT, MCContext &Context,
                                 std::unique_ptr<MCAsmBackend> MAB,
                                 std::unique_ptr<MCObjectWriter> OW,
                                 std::unique_ptr<MCCodeEmitter> CE) {
  return new AVRMCELFStreamer(Context, std::move(MAB), std::move(OW),
                              std::move(CE));
}

} // end namespace vm::core
