//===- HexagonMCShuffler.h --------------------------------------*- C++ -*-===//
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
// This declares the shuffling of insns inside a bundle according to the
// packet formation rules of the Hexagon ISA.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_HEXAGON_MCTARGETDESC_HEXAGONMCSHUFFLER_H
#define LLVM_LIB_TARGET_HEXAGON_MCTARGETDESC_HEXAGONMCSHUFFLER_H

#include "MCTargetDesc/HexagonMCInstrInfo.h"
#include "MCTargetDesc/HexagonShuffler.h"
#include "vm/core/ADT/SmallVector.h"

namespace vm::core {

class MCContext;
class MCInst;
class MCInstrInfo;
class MCSubtargetInfo;

// Insn bundle shuffler.
class HexagonMCShuffler : public HexagonShuffler {
public:
  HexagonMCShuffler(MCContext &Context, bool ReportErrors,
                    MCInstrInfo const &MCII, MCSubtargetInfo const &STI,
                    MCInst &MCB)
      : HexagonShuffler(Context, ReportErrors, MCII, STI) {
    init(MCB);
  }

  HexagonMCShuffler(MCContext &Context, bool ReportErrors,
                    MCInstrInfo const &MCII, MCSubtargetInfo const &STI,
                    MCInst &MCB, MCInst const &AddMI, bool InsertAtFront)
      : HexagonShuffler(Context, ReportErrors, MCII, STI) {
    init(MCB, AddMI, InsertAtFront);
  }

  // Copy reordered bundle to another.
  void copyTo(MCInst &MCB);

  // Reorder and copy result to another.
  bool reshuffleTo(MCInst &MCB);

private:
  void init(MCInst &MCB);
  void init(MCInst &MCB, MCInst const &AddMI, bool InsertAtFront);
};

// Invocation of the shuffler.  Returns true if the shuffle succeeded.  If
// true, MCB will contain the newly-shuffled packet.
bool HexagonMCShuffle(MCContext &Context, bool ReportErrors,
                      MCInstrInfo const &MCII, MCSubtargetInfo const &STI,
                      MCInst &MCB);
bool HexagonMCShuffle(MCContext &Context, MCInstrInfo const &MCII,
                      MCSubtargetInfo const &STI, MCInst &MCB,
                      MCInst const &AddMI, int fixupCount);
bool HexagonMCShuffle(MCContext &Context, MCInstrInfo const &MCII,
                      MCSubtargetInfo const &STI, MCInst &MCB,
                      SmallVector<DuplexCandidate, 8> possibleDuplexes);

} // end namespace vm::core

#endif // LLVM_LIB_TARGET_HEXAGON_MCTARGETDESC_HEXAGONMCSHUFFLER_H
