//===-- MSP430ELFStreamer.cpp - MSP430 ELF Target Streamer Methods --------===//
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
// This file provides MSP430 specific target streamer methods.
//
//===----------------------------------------------------------------------===//

#include "MSP430MCTargetDesc.h"
#include "vm/core/BinaryFormat/ELF.h"
#include "vm/core/MC/MCAssembler.h"
#include "vm/core/MC/MCContext.h"
#include "vm/core/MC/MCELFStreamer.h"
#include "vm/core/MC/MCSectionELF.h"
#include "vm/core/MC/MCStreamer.h"
#include "vm/core/MC/MCSubtargetInfo.h"
#include "vm/core/Support/MSP430Attributes.h"

using namespace vm::core;
using namespace vm::core::MSP430Attrs;

namespace vm::core {

class MSP430TargetELFStreamer : public MCTargetStreamer {
public:
  MCELFStreamer &getStreamer();
  MSP430TargetELFStreamer(MCStreamer &S, const MCSubtargetInfo &STI);
};

// This part is for ELF object output.
MSP430TargetELFStreamer::MSP430TargetELFStreamer(MCStreamer &S,
                                                 const MCSubtargetInfo &STI)
    : MCTargetStreamer(S) {
  // Emit build attributes section according to
  // MSP430 EABI (slaa534.pdf, part 13).
  MCSection *AttributeSection = getStreamer().getContext().getELFSection(
      ".MSP430.attributes", ELF::SHT_MSP430_ATTRIBUTES, 0);
  Streamer.switchSection(AttributeSection);

  // Format version.
  Streamer.emitInt8(0x41);
  // Subsection length.
  Streamer.emitInt32(22);
  // Vendor name string, zero-terminated.
  Streamer.emitBytes("mspabi");
  Streamer.emitInt8(0);

  // Attribute vector scope tag. 1 stands for the entire file.
  Streamer.emitInt8(1);
  // Attribute vector length.
  Streamer.emitInt32(11);

  Streamer.emitInt8(TagISA);
  Streamer.emitInt8(STI.hasFeature(MSP430::FeatureX) ? ISAMSP430X : ISAMSP430);
  Streamer.emitInt8(TagCodeModel);
  Streamer.emitInt8(CMSmall);
  Streamer.emitInt8(TagDataModel);
  Streamer.emitInt8(DMSmall);
  // Don't emit TagEnumSize, for full GCC compatibility.
}

MCELFStreamer &MSP430TargetELFStreamer::getStreamer() {
  return static_cast<MCELFStreamer &>(Streamer);
}

MCTargetStreamer *
createMSP430ObjectTargetStreamer(MCStreamer &S, const MCSubtargetInfo &STI) {
  const Triple &TT = STI.getTargetTriple();
  if (TT.isOSBinFormatELF())
    return new MSP430TargetELFStreamer(S, STI);
  return nullptr;
}

} // namespace vm::core
