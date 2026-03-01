//===-- VETargetStreamer.cpp - VE Target Streamer Methods -----------------===//
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
// This file provides VE specific target streamer methods.
//
//===----------------------------------------------------------------------===//

#include "VETargetStreamer.h"
#include "VEInstPrinter.h"
#include "vm/core/MC/MCRegister.h"

using namespace vm::core;

// pin vtable to this file
VETargetStreamer::VETargetStreamer(MCStreamer &S) : MCTargetStreamer(S) {}

void VETargetStreamer::anchor() {}

VETargetAsmStreamer::VETargetAsmStreamer(MCStreamer &S,
                                         formatted_raw_ostream &OS)
    : VETargetStreamer(S), OS(OS) {}

void VETargetAsmStreamer::emitVERegisterIgnore(unsigned reg) {
  OS << "\t.register "
     << "%" << StringRef(VEInstPrinter::getRegisterName(reg)).lower()
     << ", #ignore\n";
}

void VETargetAsmStreamer::emitVERegisterScratch(unsigned reg) {
  OS << "\t.register "
     << "%" << StringRef(VEInstPrinter::getRegisterName(reg)).lower()
     << ", #scratch\n";
}

VETargetELFStreamer::VETargetELFStreamer(MCStreamer &S) : VETargetStreamer(S) {}

MCELFStreamer &VETargetELFStreamer::getStreamer() {
  return static_cast<MCELFStreamer &>(Streamer);
}
