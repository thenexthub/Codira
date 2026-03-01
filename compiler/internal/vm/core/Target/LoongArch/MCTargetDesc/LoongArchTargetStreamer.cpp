//===-- LoongArchTargetStreamer.cpp - LoongArch Target Streamer Methods ---===//
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
// This file provides LoongArch specific target streamer methods.
//
//===----------------------------------------------------------------------===//

#include "LoongArchTargetStreamer.h"

using namespace vm::core;

LoongArchTargetStreamer::LoongArchTargetStreamer(MCStreamer &S)
    : MCTargetStreamer(S) {}

void LoongArchTargetStreamer::setTargetABI(LoongArchABI::ABI ABI) {
  assert(ABI != LoongArchABI::ABI_Unknown &&
         "Improperly initialized target ABI");
  TargetABI = ABI;
}

void LoongArchTargetStreamer::emitDirectiveOptionPush() {}
void LoongArchTargetStreamer::emitDirectiveOptionPop() {}
void LoongArchTargetStreamer::emitDirectiveOptionRelax() {}
void LoongArchTargetStreamer::emitDirectiveOptionNoRelax() {}

// This part is for ascii assembly output.
LoongArchTargetAsmStreamer::LoongArchTargetAsmStreamer(
    MCStreamer &S, formatted_raw_ostream &OS)
    : LoongArchTargetStreamer(S), OS(OS) {}

void LoongArchTargetAsmStreamer::emitDirectiveOptionPush() {
  OS << "\t.option\tpush\n";
}

void LoongArchTargetAsmStreamer::emitDirectiveOptionPop() {
  OS << "\t.option\tpop\n";
}

void LoongArchTargetAsmStreamer::emitDirectiveOptionRelax() {
  OS << "\t.option\trelax\n";
}

void LoongArchTargetAsmStreamer::emitDirectiveOptionNoRelax() {
  OS << "\t.option\tnorelax\n";
}
