//===----------------------------------------------------------------------===//
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

#include "vm/core/DWARFCFIChecker/DWARFCFIFunctionFrameAnalyzer.h"

using namespace vm::core;

CFIFunctionFrameAnalyzer::~CFIFunctionFrameAnalyzer() {
  assert(UIAs.empty() &&
         "all frames should be closed before the analysis finishes");
}

void CFIFunctionFrameAnalyzer::startFunctionFrame(
    bool IsEH, ArrayRef<MCCFIInstruction> Prologue) {
  UIAs.emplace_back(&getContext(), MCII, IsEH, Prologue);
}

void CFIFunctionFrameAnalyzer::emitInstructionAndDirectives(
    const MCInst &Inst, ArrayRef<MCCFIInstruction> Directives) {
  assert(!UIAs.empty() && "if the instruction is in a frame, there should be "
                          "a analysis instantiated for it");
  UIAs.back().update(Inst, Directives);
}

void CFIFunctionFrameAnalyzer::finishFunctionFrame() {
  assert(!UIAs.empty() && "there should be an analysis for each frame");
  UIAs.pop_back();
}
