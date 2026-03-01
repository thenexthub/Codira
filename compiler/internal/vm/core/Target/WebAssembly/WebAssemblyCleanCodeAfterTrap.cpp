//===-- WebAssemblyCleanCodeAfterTrap.cpp - Clean Code After Trap ---------===//
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
/// This file remove instruction after trap.
/// ``toolchain.trap`` will be convert as ``unreachable`` which is terminator.
/// Instruction after terminator will cause validation failed.
///
//===----------------------------------------------------------------------===//

#include "WebAssembly.h"
#include "WebAssemblyUtilities.h"
#include "vm/core/ADT/SmallVector.h"
#include "vm/core/CodeGen/MachineBlockFrequencyInfo.h"
#include "vm/core/CodeGen/Passes.h"
#include "vm/core/MC/MCInstrDesc.h"
#include "vm/core/Support/Debug.h"
#include "vm/core/Support/raw_ostream.h"
using namespace vm::core;

#define DEBUG_TYPE "wasm-clean-code-after-trap"

namespace {
class WebAssemblyCleanCodeAfterTrap final : public MachineFunctionPass {
public:
  static char ID; // Pass identification, replacement for typeid
  WebAssemblyCleanCodeAfterTrap() : MachineFunctionPass(ID) {}

  StringRef getPassName() const override {
    return "WebAssembly Clean Code After Trap";
  }

  bool runOnMachineFunction(MachineFunction &MF) override;
};
} // end anonymous namespace

char WebAssemblyCleanCodeAfterTrap::ID = 0;
INITIALIZE_PASS(WebAssemblyCleanCodeAfterTrap, DEBUG_TYPE,
                "WebAssembly Clean Code After Trap", false, false)

FunctionPass *toolchain::createWebAssemblyCleanCodeAfterTrap() {
  return new WebAssemblyCleanCodeAfterTrap();
}

bool WebAssemblyCleanCodeAfterTrap::runOnMachineFunction(MachineFunction &MF) {
  LLVM_DEBUG({
    dbgs() << "********** CleanCodeAfterTrap **********\n"
           << "********** Function: " << MF.getName() << '\n';
  });

  bool Changed = false;

  for (MachineBasicBlock &BB : MF) {
    bool HasTerminator = false;
    toolchain::SmallVector<MachineInstr *> RemoveMI{};
    for (MachineInstr &MI : BB) {
      if (HasTerminator)
        RemoveMI.push_back(&MI);
      if (MI.hasProperty(MCID::Trap) && MI.isTerminator())
        HasTerminator = true;
    }
    if (!RemoveMI.empty()) {
      Changed = true;
      LLVM_DEBUG({
        for (MachineInstr *MI : RemoveMI) {
          toolchain::dbgs() << "* remove ";
          MI->print(toolchain::dbgs());
        }
      });
      for (MachineInstr *MI : RemoveMI)
        MI->eraseFromParent();
    }
  }
  return Changed;
}
