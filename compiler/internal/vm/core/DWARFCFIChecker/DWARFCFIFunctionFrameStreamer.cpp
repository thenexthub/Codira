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

#include "vm/core/DWARFCFIChecker/DWARFCFIFunctionFrameStreamer.h"
#include "vm/core/ADT/ArrayRef.h"
#include "vm/core/MC/MCContext.h"
#include "vm/core/MC/MCDwarf.h"
#include "vm/core/MC/MCInst.h"
#include "vm/core/MC/MCInstrInfo.h"
#include "vm/core/MC/MCStreamer.h"
#include <optional>

using namespace vm::core;

void CFIFunctionFrameStreamer::updateReceiver(
    const std::optional<MCInst> &NewInst) {
  assert(hasUnfinishedDwarfFrameInfo() &&
         "should have an unfinished DWARF frame here");
  assert(!FrameIndices.empty() &&
         "there should be an index available for the current frame");
  assert(FrameIndices.size() == LastInstructions.size());
  assert(LastInstructions.size() == LastDirectiveIndices.size());

  auto Frames = getDwarfFrameInfos();
  assert(FrameIndices.back() < Frames.size());
  unsigned LastDirectiveIndex = LastDirectiveIndices.back();
  unsigned CurrentDirectiveIndex =
      Frames[FrameIndices.back()].Instructions.size();
  assert(CurrentDirectiveIndex >= LastDirectiveIndex);

  const MCDwarfFrameInfo *LastFrame = &Frames[FrameIndices.back()];
  ArrayRef<MCCFIInstruction> Directives;
  if (LastDirectiveIndex < CurrentDirectiveIndex) {
    Directives = ArrayRef<MCCFIInstruction>(LastFrame->Instructions);
    Directives =
        Directives.drop_front(LastDirectiveIndex)
            .drop_back(LastFrame->Instructions.size() - CurrentDirectiveIndex);
  }

  auto MaybeLastInstruction = LastInstructions.back();
  if (MaybeLastInstruction)
    // The directives are associated with an instruction.
    Receiver->emitInstructionAndDirectives(*MaybeLastInstruction, Directives);
  else
    // The directives are the prologue directives.
    Receiver->startFunctionFrame(false /* TODO: should put isEH here */,
                                 Directives);

  // Update the internal state for the top frame.
  LastInstructions.back() = NewInst;
  LastDirectiveIndices.back() = CurrentDirectiveIndex;
}

void CFIFunctionFrameStreamer::emitInstruction(const MCInst &Inst,
                                               const MCSubtargetInfo &STI) {
  if (hasUnfinishedDwarfFrameInfo())
    // Send the last instruction with the unsent directives already in the frame
    // to the receiver.
    updateReceiver(Inst);
}

void CFIFunctionFrameStreamer::emitCFIStartProcImpl(MCDwarfFrameInfo &Frame) {
  LastInstructions.push_back(std::nullopt);
  LastDirectiveIndices.push_back(0);
  FrameIndices.push_back(getNumFrameInfos());

  MCStreamer::emitCFIStartProcImpl(Frame);
}

void CFIFunctionFrameStreamer::emitCFIEndProcImpl(MCDwarfFrameInfo &CurFrame) {
  // Send the last instruction with the final directives of the current frame to
  // the receiver.
  updateReceiver(std::nullopt);

  assert(!FrameIndices.empty() && "There should be at least one frame to pop");
  LastDirectiveIndices.pop_back();
  LastInstructions.pop_back();
  FrameIndices.pop_back();

  Receiver->finishFunctionFrame();

  MCStreamer::emitCFIEndProcImpl(CurFrame);
}
