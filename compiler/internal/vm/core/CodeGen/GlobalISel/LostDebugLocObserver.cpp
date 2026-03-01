//===----- toolchain/CodeGen/GlobalISel/LostDebugLocObserver.cpp -----*- C++ -*-===//
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
/// Tracks DebugLocs between checkpoints and verifies that they are transferred.
//
//===----------------------------------------------------------------------===//

#include "vm/core/CodeGen/GlobalISel/LostDebugLocObserver.h"

using namespace vm::core;

#define LOC_DEBUG(X) DEBUG_WITH_TYPE(DebugType.str().c_str(), X)

void LostDebugLocObserver::analyzeDebugLocations() {
  if (LostDebugLocs.empty()) {
    LOC_DEBUG(dbgs() << ".. No debug info was present\n");
    return;
  }
  if (PotentialMIsForDebugLocs.empty()) {
    LOC_DEBUG(
        dbgs() << ".. No instructions to carry debug info (dead code?)\n");
    return;
  }

  LOC_DEBUG(dbgs() << ".. Searching " << PotentialMIsForDebugLocs.size()
                   << " instrs for " << LostDebugLocs.size() << " locations\n");
  SmallPtrSet<MachineInstr *, 4> FoundIn;
  for (MachineInstr *MI : PotentialMIsForDebugLocs) {
    if (!MI->getDebugLoc())
      continue;
    // Check this first in case there's a matching line-0 location on both input
    // and output.
    if (MI->getDebugLoc().getLine() == 0) {
      LOC_DEBUG(
          dbgs() << ".. Assuming line-0 location covers remainder (if any)\n");
      return;
    }
    if (LostDebugLocs.erase(MI->getDebugLoc())) {
      LOC_DEBUG(dbgs() << ".. .. found " << MI->getDebugLoc() << " in " << *MI);
      FoundIn.insert(MI);
      continue;
    }
  }
  if (LostDebugLocs.empty())
    return;

  NumLostDebugLocs += LostDebugLocs.size();
  LOC_DEBUG({
    dbgs() << ".. Lost locations:\n";
    for (const DebugLoc &Loc : LostDebugLocs) {
      dbgs() << ".. .. ";
      Loc.print(dbgs());
      dbgs() << "\n";
    }
    dbgs() << ".. MIs with matched locations:\n";
    for (MachineInstr *MI : FoundIn)
      if (PotentialMIsForDebugLocs.erase(MI))
        dbgs() << ".. .. " << *MI;
    dbgs() << ".. Remaining MIs with unmatched/no locations:\n";
    for (const MachineInstr *MI : PotentialMIsForDebugLocs)
      dbgs() << ".. .. " << *MI;
  });
}

void LostDebugLocObserver::checkpoint(bool CheckDebugLocs) {
  if (CheckDebugLocs)
    analyzeDebugLocations();
  PotentialMIsForDebugLocs.clear();
  LostDebugLocs.clear();
}

void LostDebugLocObserver::createdInstr(MachineInstr &MI) {
  PotentialMIsForDebugLocs.insert(&MI);
}

static bool irTranslatorNeverAddsLocations(unsigned Opcode) {
  switch (Opcode) {
  default:
    return false;
  case TargetOpcode::G_CONSTANT:
  case TargetOpcode::G_FCONSTANT:
  case TargetOpcode::G_IMPLICIT_DEF:
  case TargetOpcode::G_GLOBAL_VALUE:
    return true;
  }
}

void LostDebugLocObserver::erasingInstr(MachineInstr &MI) {
  if (irTranslatorNeverAddsLocations(MI.getOpcode()))
    return;

  PotentialMIsForDebugLocs.erase(&MI);
  if (MI.getDebugLoc())
    LostDebugLocs.insert(MI.getDebugLoc());
}

void LostDebugLocObserver::changingInstr(MachineInstr &MI) {
  if (irTranslatorNeverAddsLocations(MI.getOpcode()))
    return;

  PotentialMIsForDebugLocs.erase(&MI);
  if (MI.getDebugLoc())
    LostDebugLocs.insert(MI.getDebugLoc());
}

void LostDebugLocObserver::changedInstr(MachineInstr &MI) {
  PotentialMIsForDebugLocs.insert(&MI);
}
