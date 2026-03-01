//===----- HexagonMCShuffler.cpp - MC bundle shuffling --------------------===//
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
// This implements the shuffling of insns inside a bundle according to the
// packet formation rules of the Hexagon ISA.
//
//===----------------------------------------------------------------------===//

#include "MCTargetDesc/HexagonMCShuffler.h"
#include "MCTargetDesc/HexagonMCInstrInfo.h"
#include "MCTargetDesc/HexagonShuffler.h"
#include "vm/core/MC/MCInst.h"
#include "vm/core/MC/MCInstrDesc.h"
#include "vm/core/MC/MCInstrInfo.h"
#include "vm/core/Support/CommandLine.h"
#include "vm/core/Support/Debug.h"
#include "vm/core/Support/raw_ostream.h"
#include <cassert>

#define DEBUG_TYPE "hexagon-shuffle"

using namespace vm::core;

static cl::opt<bool>
    DisableShuffle("disable-hexagon-shuffle", cl::Hidden, cl::init(false),
                   cl::desc("Disable Hexagon instruction shuffling"));

void HexagonMCShuffler::init(MCInst &MCB) {
  if (HexagonMCInstrInfo::isBundle(MCB)) {
    MCInst const *Extender = nullptr;
    // Copy the bundle for the shuffling.
    for (const auto &I : HexagonMCInstrInfo::bundleInstructions(MCB)) {
      MCInst &MI = *const_cast<MCInst *>(I.getInst());
      LLVM_DEBUG(dbgs() << "Shuffling: " << MCII.getName(MI.getOpcode())
                        << '\n');
      assert(!HexagonMCInstrInfo::getDesc(MCII, MI).isPseudo());

      if (!HexagonMCInstrInfo::isImmext(MI)) {
        append(MI, Extender, HexagonMCInstrInfo::getUnits(MCII, STI, MI));
        Extender = nullptr;
      } else
        Extender = &MI;
    }
  }

  Loc = MCB.getLoc();
  BundleFlags = MCB.getOperand(0).getImm();
}

void HexagonMCShuffler::init(MCInst &MCB, MCInst const &AddMI,
                             bool bInsertAtFront) {
  if (HexagonMCInstrInfo::isBundle(MCB)) {
    if (bInsertAtFront)
      append(AddMI, nullptr, HexagonMCInstrInfo::getUnits(MCII, STI, AddMI));
    MCInst const *Extender = nullptr;
    // Copy the bundle for the shuffling.
    for (auto const &I : HexagonMCInstrInfo::bundleInstructions(MCB)) {
      assert(!HexagonMCInstrInfo::getDesc(MCII, *I.getInst()).isPseudo());
      MCInst &MI = *const_cast<MCInst *>(I.getInst());
      if (!HexagonMCInstrInfo::isImmext(MI)) {
        append(MI, Extender, HexagonMCInstrInfo::getUnits(MCII, STI, MI));
        Extender = nullptr;
      } else
        Extender = &MI;
    }
    if (!bInsertAtFront)
      append(AddMI, nullptr, HexagonMCInstrInfo::getUnits(MCII, STI, AddMI));
  }

  Loc = MCB.getLoc();
  BundleFlags = MCB.getOperand(0).getImm();
}

void HexagonMCShuffler::copyTo(MCInst &MCB) {
  MCB.clear();
  MCB.addOperand(MCOperand::createImm(BundleFlags));
  MCB.setLoc(Loc);
  // Copy the results into the bundle.
  for (auto &I : *this) {
    MCInst const &MI = I.getDesc();
    MCInst const *Extender = I.getExtender();
    if (Extender)
      MCB.addOperand(MCOperand::createInst(Extender));
    MCB.addOperand(MCOperand::createInst(&MI));
  }
}

bool HexagonMCShuffler::reshuffleTo(MCInst &MCB) {
  if (shuffle()) {
    // Copy the results into the bundle.
    copyTo(MCB);
    return true;
  }
  LLVM_DEBUG(MCB.dump());
  return false;
}

bool toolchain::HexagonMCShuffle(MCContext &Context, bool ReportErrors,
                            MCInstrInfo const &MCII, MCSubtargetInfo const &STI,
                            MCInst &MCB) {
  HexagonMCShuffler MCS(Context, ReportErrors, MCII, STI, MCB);

  if (DisableShuffle)
    // Ignore if user chose so.
    return false;

  if (!HexagonMCInstrInfo::bundleSize(MCB)) {
    // There once was a bundle:
    //    BUNDLE implicit-def %d2, implicit-def %r4, implicit-def %r5,
    //    implicit-def %d7, ...
    //      * %d2 = IMPLICIT_DEF; flags:
    //      * %d7 = IMPLICIT_DEF; flags:
    // After the IMPLICIT_DEFs were removed by the asm printer, the bundle
    // became empty.
    LLVM_DEBUG(dbgs() << "Skipping empty bundle");
    return false;
  } else if (!HexagonMCInstrInfo::isBundle(MCB)) {
    LLVM_DEBUG(dbgs() << "Skipping stand-alone insn");
    return false;
  }

  return MCS.reshuffleTo(MCB);
}

bool toolchain::HexagonMCShuffle(MCContext &Context, MCInstrInfo const &MCII,
                            MCSubtargetInfo const &STI, MCInst &MCB,
                            SmallVector<DuplexCandidate, 8> possibleDuplexes) {

  if (DisableShuffle || possibleDuplexes.size() == 0)
    return false;

  if (!HexagonMCInstrInfo::bundleSize(MCB)) {
    // There once was a bundle:
    //    BUNDLE implicit-def %d2, implicit-def %r4, implicit-def %r5,
    //    implicit-def %d7, ...
    //      * %d2 = IMPLICIT_DEF; flags:
    //      * %d7 = IMPLICIT_DEF; flags:
    // After the IMPLICIT_DEFs were removed by the asm printer, the bundle
    // became empty.
    LLVM_DEBUG(dbgs() << "Skipping empty bundle");
    return false;
  } else if (!HexagonMCInstrInfo::isBundle(MCB)) {
    LLVM_DEBUG(dbgs() << "Skipping stand-alone insn");
    return false;
  }

  bool doneShuffling = false;
  while (possibleDuplexes.size() > 0 && (!doneShuffling)) {
    // case of Duplex Found
    DuplexCandidate duplexToTry = possibleDuplexes.pop_back_val();
    MCInst Attempt(MCB);
    HexagonMCInstrInfo::replaceDuplex(Context, Attempt, duplexToTry);
    HexagonMCShuffler MCS(Context, false, MCII, STI, Attempt); // copy packet to the shuffler
    if (MCS.size() == 1) {                     // case of one duplex
      // copy the created duplex in the shuffler to the bundle
      MCS.copyTo(MCB);
      return false;
    }
    // try shuffle with this duplex
    doneShuffling = MCS.reshuffleTo(MCB);

    if (doneShuffling)
      break;
  }

  if (!doneShuffling) {
    HexagonMCShuffler MCS(Context, false, MCII, STI, MCB);
    doneShuffling = MCS.reshuffleTo(MCB); // shuffle
  }

  return doneShuffling;
}

bool toolchain::HexagonMCShuffle(MCContext &Context, MCInstrInfo const &MCII,
                            MCSubtargetInfo const &STI, MCInst &MCB,
                            MCInst const &AddMI, int fixupCount) {
  if (!HexagonMCInstrInfo::isBundle(MCB))
    return false;

  // if fixups present, make sure we don't insert too many nops that would
  // later prevent an extender from being inserted.
  unsigned int bundleSize = HexagonMCInstrInfo::bundleSize(MCB);
  if (bundleSize >= HEXAGON_PACKET_SIZE)
    return false;
  bool bhasDuplex = HexagonMCInstrInfo::hasDuplex(MCII, MCB);
  if (fixupCount >= 2) {
    if (bhasDuplex) {
      if (bundleSize >= HEXAGON_PACKET_SIZE - 1) {
        return false;
      }
    } else {
      return false;
    }
  } else {
    if (bundleSize == HEXAGON_PACKET_SIZE - 1 && fixupCount)
      return false;
  }

  if (DisableShuffle)
    return false;

  // mgl: temporary code (shuffler doesn't take into account the fact that
  // a duplex takes up two slots.  for example, 3 nops can be put into a packet
  // containing a duplex oversubscribing slots by 1).
  unsigned maxBundleSize = (HexagonMCInstrInfo::hasImmExt(MCB))
                               ? HEXAGON_PACKET_SIZE
                               : HEXAGON_PACKET_SIZE - 1;
  if (bhasDuplex && bundleSize >= maxBundleSize)
    return false;

  HexagonMCShuffler MCS(Context, false, MCII, STI, MCB, AddMI, false);
  return MCS.reshuffleTo(MCB);
}
