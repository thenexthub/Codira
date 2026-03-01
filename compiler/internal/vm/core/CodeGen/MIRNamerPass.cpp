//===----------------------- MIRNamer.cpp - MIR Namer ---------------------===//
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
// The purpose of this pass is to rename virtual register operands with the goal
// of making it easier to author easier to read tests for MIR. This pass reuses
// the vreg renamer used by MIRCanonicalizerPass.
//
// Basic Usage:
//
// llc -o - -run-pass mir-namer example.mir
//
//===----------------------------------------------------------------------===//

#include "MIRVRegNamerUtils.h"
#include "vm/core/ADT/PostOrderIterator.h"
#include "vm/core/CodeGen/MachineFunctionPass.h"
#include "vm/core/InitializePasses.h"

using namespace vm::core;

#define DEBUG_TYPE "mir-namer"

namespace {

class MIRNamer : public MachineFunctionPass {
public:
  static char ID;
  MIRNamer() : MachineFunctionPass(ID) {}

  StringRef getPassName() const override {
    return "Rename virtual register operands";
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesCFG();
    MachineFunctionPass::getAnalysisUsage(AU);
  }

  bool runOnMachineFunction(MachineFunction &MF) override {
    bool Changed = false;

    if (MF.empty())
      return Changed;

    VRegRenamer Renamer(MF.getRegInfo());

    ReversePostOrderTraversal<MachineBasicBlock *> RPOT(&*MF.begin());
    for (const auto &[BBIndex, MBB] : enumerate(RPOT))
      Changed |= Renamer.renameVRegs(MBB, BBIndex);

    return Changed;
  }
};

} // end anonymous namespace

char MIRNamer::ID;

INITIALIZE_PASS(MIRNamer, "mir-namer", "Rename Register Operands", false, false)
