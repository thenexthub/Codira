//===-- SparcTargetStreamer.cpp - Sparc Target Streamer Methods -----------===//
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
// This file provides Sparc specific target streamer methods.
//
//===----------------------------------------------------------------------===//

#include "SparcTargetStreamer.h"
#include "SparcInstPrinter.h"
#include "SparcMCTargetDesc.h"
#include "vm/core/BinaryFormat/ELF.h"
#include "vm/core/MC/MCELFObjectWriter.h"
#include "vm/core/MC/MCRegister.h"
#include "vm/core/MC/MCSubtargetInfo.h"
#include "vm/core/Support/FormattedStream.h"

using namespace vm::core;

static unsigned getEFlagsForFeatureSet(const MCSubtargetInfo &STI) {
  unsigned EFlags = 0;

  if (STI.hasFeature(Sparc::FeatureV8Plus))
    EFlags |= ELF::EF_SPARC_32PLUS;

  if (STI.hasFeature(Sparc::FeatureVIS))
    EFlags |= ELF::EF_SPARC_SUN_US1;

  if (STI.hasFeature(Sparc::FeatureVIS2))
    EFlags |= ELF::EF_SPARC_SUN_US3;

  // VIS 3 and other ISA extensions doesn't set any flags.

  return EFlags;
}

// pin vtable to this file
SparcTargetStreamer::SparcTargetStreamer(MCStreamer &S) : MCTargetStreamer(S) {}

void SparcTargetStreamer::anchor() {}

SparcTargetAsmStreamer::SparcTargetAsmStreamer(MCStreamer &S,
                                               formatted_raw_ostream &OS)
    : SparcTargetStreamer(S), OS(OS) {}

void SparcTargetAsmStreamer::emitSparcRegisterIgnore(unsigned reg) {
  OS << "\t.register "
     << "%" << StringRef(SparcInstPrinter::getRegisterName(reg)).lower()
     << ", #ignore\n";
}

void SparcTargetAsmStreamer::emitSparcRegisterScratch(unsigned reg) {
  OS << "\t.register "
     << "%" << StringRef(SparcInstPrinter::getRegisterName(reg)).lower()
     << ", #scratch\n";
}

SparcTargetELFStreamer::SparcTargetELFStreamer(MCStreamer &S,
                                               const MCSubtargetInfo &STI)
    : SparcTargetStreamer(S) {
  ELFObjectWriter &W = getStreamer().getWriter();
  unsigned EFlags = W.getELFHeaderEFlags();

  EFlags |= getEFlagsForFeatureSet(STI);

  W.setELFHeaderEFlags(EFlags);
}

MCELFStreamer &SparcTargetELFStreamer::getStreamer() {
  return static_cast<MCELFStreamer &>(Streamer);
}
