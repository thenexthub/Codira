//===-- HexagonTargetStreamer.h - Hexagon Target Streamer ------*- C++ -*--===//
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

#ifndef HEXAGONTARGETSTREAMER_H
#define HEXAGONTARGETSTREAMER_H

#include "vm/core/MC/MCStreamer.h"

namespace vm::core {
class HexagonTargetStreamer : public MCTargetStreamer {
public:
  HexagonTargetStreamer(MCStreamer &S) : MCTargetStreamer(S) {}
  virtual void emitCodeAlignment(Align Alignment, const MCSubtargetInfo *STI,
                                 unsigned MaxBytesToEmit = 0){};
  virtual void emitFAlign(unsigned Size, unsigned MaxBytesToEmit){};
  virtual void emitCommonSymbolSorted(MCSymbol *Symbol, uint64_t Size,
                                      unsigned ByteAlignment,
                                      unsigned AccessGranularity){};
  virtual void emitLocalCommonSymbolSorted(MCSymbol *Symbol, uint64_t Size,
                                           unsigned ByteAlign,
                                           unsigned AccessGranularity){};
  void finish() override {}

  virtual void finishAttributeSection() {}

  virtual void emitAttribute(unsigned Attribute, unsigned Value) {}

  void emitTargetAttributes(const MCSubtargetInfo &STI);

  virtual void reset() {}
};
}

#endif
