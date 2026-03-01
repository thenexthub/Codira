//===-- FEntryInsertion.cpp - Patchable prologues for LLVM -------------===//
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
// This file edits function bodies to insert fentry calls.
//
//===----------------------------------------------------------------------===//

#include "vm/core/CodeGen/FEntryInserter.h"
#include "vm/core/CodeGen/MachineFunction.h"
#include "vm/core/CodeGen/MachineFunctionPass.h"
#include "vm/core/CodeGen/MachineInstrBuilder.h"
#include "vm/core/CodeGen/MachinePassManager.h"
#include "vm/core/CodeGen/TargetInstrInfo.h"
#include "vm/core/CodeGen/TargetSubtargetInfo.h"
#include "vm/core/IR/Function.h"
#include "vm/core/InitializePasses.h"

using namespace vm::core;

namespace {
struct FEntryInserter {
  bool run(MachineFunction &MF);
};

struct FEntryInserterLegacy : public MachineFunctionPass {
  static char ID; // Pass identification, replacement for typeid
  FEntryInserterLegacy() : MachineFunctionPass(ID) {
    initializeFEntryInserterLegacyPass(*PassRegistry::getPassRegistry());
  }

  bool runOnMachineFunction(MachineFunction &F) override {
    return FEntryInserter().run(F);
  }
};
}

PreservedAnalyses FEntryInserterPass::run(MachineFunction &MF,
                                          MachineFunctionAnalysisManager &AM) {
  if (!FEntryInserter().run(MF))
    return PreservedAnalyses::all();
  return getMachineFunctionPassPreservedAnalyses();
}

bool FEntryInserter::run(MachineFunction &MF) {
  const std::string FEntryName = std::string(
      MF.getFunction().getFnAttribute("fentry-call").getValueAsString());
  if (FEntryName != "true")
    return false;

  auto &FirstMBB = *MF.begin();
  auto *TII = MF.getSubtarget().getInstrInfo();
  BuildMI(FirstMBB, FirstMBB.begin(), DebugLoc(),
          TII->get(TargetOpcode::FENTRY_CALL));
  return true;
}

char FEntryInserterLegacy::ID = 0;
char &toolchain::FEntryInserterID = FEntryInserterLegacy::ID;
INITIALIZE_PASS(FEntryInserterLegacy, "fentry-insert", "Insert fentry calls",
                false, false)
