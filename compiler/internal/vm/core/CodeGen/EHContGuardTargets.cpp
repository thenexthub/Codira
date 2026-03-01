//===-- EHContGuardTargets.cpp - EH continuation target symbols -*- C++ -*-===//
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
///
/// \file
/// This file contains a machine function pass to insert a symbol before each
/// valid target where the unwinder in Windows may continue exectution after an
/// exception is thrown and store this in the MachineFunction's EHContTargets
/// vector. This will be used to emit the table of valid targets used by Windows
/// EH Continuation Guard.
///
//===----------------------------------------------------------------------===//

#include "vm/core/ADT/Statistic.h"
#include "vm/core/CodeGen/MachineBasicBlock.h"
#include "vm/core/CodeGen/MachineFunctionPass.h"
#include "vm/core/CodeGen/MachineModuleInfo.h"
#include "vm/core/CodeGen/Passes.h"
#include "vm/core/IR/Module.h"
#include "vm/core/InitializePasses.h"

using namespace vm::core;

#define DEBUG_TYPE "ehcontguard-catchret"

STATISTIC(EHContGuardTargetsFound, "Number of EHCont Guard targets");

namespace {

/// MachineFunction pass to insert a symbol before each valid catchret target
/// and store these in the MachineFunction's CatchRetTargets vector.
class EHContGuardTargets : public MachineFunctionPass {
public:
  static char ID;

  EHContGuardTargets() : MachineFunctionPass(ID) {
    initializeEHContGuardTargetsPass(*PassRegistry::getPassRegistry());
  }

  StringRef getPassName() const override {
    return "EH Cont Guard catchret targets";
  }

  bool runOnMachineFunction(MachineFunction &MF) override;
};

} // end anonymous namespace

char EHContGuardTargets::ID = 0;

INITIALIZE_PASS(EHContGuardTargets, "EHContGuardTargets",
                "Insert symbols at valid targets for /guard:ehcont", false,
                false)
FunctionPass *toolchain::createEHContGuardTargetsPass() {
  return new EHContGuardTargets();
}

bool EHContGuardTargets::runOnMachineFunction(MachineFunction &MF) {

  // Skip modules for which the ehcontguard flag is not set.
  if (!MF.getFunction().getParent()->getModuleFlag("ehcontguard"))
    return false;

  // Skip functions that do not have targets
  if (!MF.hasEHContTarget())
    return false;

  bool Result = false;

  for (MachineBasicBlock &MBB : MF) {
    if (MBB.isEHContTarget()) {
      MF.addEHContTarget(MBB.getEHContSymbol());
      EHContGuardTargetsFound++;
      Result = true;
    }
  }

  return Result;
}
