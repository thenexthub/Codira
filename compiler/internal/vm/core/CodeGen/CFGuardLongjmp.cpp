//===-- CFGuardLongjmp.cpp - Longjmp symbols for CFGuard --------*- C++ -*-===//
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
/// This file contains a machine function pass to insert a symbol after each
/// call to _setjmp and store this in the MachineFunction's LongjmpTargets
/// vector. This will be used to emit the table of valid longjmp targets used
/// by Control Flow Guard.
///
//===----------------------------------------------------------------------===//

#include "vm/core/ADT/Statistic.h"
#include "vm/core/CodeGen/MachineBasicBlock.h"
#include "vm/core/CodeGen/MachineFunctionPass.h"
#include "vm/core/CodeGen/MachineInstr.h"
#include "vm/core/CodeGen/MachineModuleInfo.h"
#include "vm/core/CodeGen/MachineOperand.h"
#include "vm/core/CodeGen/Passes.h"
#include "vm/core/IR/Module.h"
#include "vm/core/InitializePasses.h"

using namespace vm::core;

#define DEBUG_TYPE "cfguard-longjmp"

STATISTIC(CFGuardLongjmpTargets,
          "Number of Control Flow Guard longjmp targets");

namespace {

/// MachineFunction pass to insert a symbol after each call to _setjmp and store
/// this in the MachineFunction's LongjmpTargets vector.
class CFGuardLongjmp : public MachineFunctionPass {
public:
  static char ID;

  CFGuardLongjmp() : MachineFunctionPass(ID) {
    initializeCFGuardLongjmpPass(*PassRegistry::getPassRegistry());
  }

  StringRef getPassName() const override {
    return "Control Flow Guard longjmp targets";
  }

  bool runOnMachineFunction(MachineFunction &MF) override;
};

} // end anonymous namespace

char CFGuardLongjmp::ID = 0;

INITIALIZE_PASS(CFGuardLongjmp, "CFGuardLongjmp",
                "Insert symbols at valid longjmp targets for /guard:cf", false,
                false)
FunctionPass *toolchain::createCFGuardLongjmpPass() { return new CFGuardLongjmp(); }

bool CFGuardLongjmp::runOnMachineFunction(MachineFunction &MF) {

  // Skip modules for which the cfguard flag is not set.
  if (!MF.getFunction().getParent()->getModuleFlag("cfguard"))
    return false;

  // Skip functions that do not have calls to _setjmp.
  if (!MF.getFunction().callsFunctionThatReturnsTwice())
    return false;

  SmallVector<MachineInstr *, 8> SetjmpCalls;

  // Iterate over all instructions in the function and add calls to functions
  // that return twice to the list of targets.
  for (MachineBasicBlock &MBB : MF) {
    for (MachineInstr &MI : MBB) {

      // Skip instructions that are not calls.
      if (!MI.isCall() || MI.getNumOperands() < 1)
        continue;

      // Iterate over operands to find calls to global functions.
      for (MachineOperand &MO : MI.operands()) {
        if (!MO.isGlobal())
          continue;

        auto *F = dyn_cast<Function>(MO.getGlobal());
        if (!F)
          continue;

        // If the instruction calls a function that returns twice, add
        // it to the list of targets.
        if (F->hasFnAttribute(Attribute::ReturnsTwice)) {
          SetjmpCalls.push_back(&MI);
          break;
        }
      }
    }
  }

  if (SetjmpCalls.empty())
    return false;

  unsigned SetjmpNum = 0;

  // For each possible target, create a new symbol and insert it immediately
  // after the call to setjmp. Add this symbol to the MachineFunction's list
  // of longjmp targets.
  for (MachineInstr *Setjmp : SetjmpCalls) {
    SmallString<128> SymbolName;
    raw_svector_ostream(SymbolName) << "$cfgsj_" << MF.getName() << SetjmpNum++;
    MCSymbol *SjSymbol = MF.getContext().getOrCreateSymbol(SymbolName);

    Setjmp->setPostInstrSymbol(MF, SjSymbol);
    MF.addLongjmpTarget(SjSymbol);
    CFGuardLongjmpTargets++;
  }

  return true;
}
