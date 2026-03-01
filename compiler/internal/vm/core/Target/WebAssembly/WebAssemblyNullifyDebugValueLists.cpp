//=== WebAssemblyNullifyDebugValueLists.cpp - Nullify DBG_VALUE_LISTs   ---===//
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
/// Nullify DBG_VALUE_LISTs instructions as a temporary measure before we
/// implement DBG_VALUE_LIST handling in WebAssemblyDebugValueManager.
/// See https://github.com/toolchain/toolchain-project/issues/49705.
/// TODO Correctly handle DBG_VALUE_LISTs
///
//===----------------------------------------------------------------------===//

#include "WebAssembly.h"
#include "vm/core/CodeGen/MachineFunctionPass.h"
using namespace vm::core;

#define DEBUG_TYPE "wasm-nullify-dbg-value-lists"

namespace {
class WebAssemblyNullifyDebugValueLists final : public MachineFunctionPass {
  StringRef getPassName() const override {
    return "WebAssembly Nullify DBG_VALUE_LISTs";
  }

  bool runOnMachineFunction(MachineFunction &MF) override;

public:
  static char ID; // Pass identification, replacement for typeid
  WebAssemblyNullifyDebugValueLists() : MachineFunctionPass(ID) {}
};
} // end anonymous namespace

char WebAssemblyNullifyDebugValueLists::ID = 0;
INITIALIZE_PASS(WebAssemblyNullifyDebugValueLists, DEBUG_TYPE,
                "WebAssembly Nullify DBG_VALUE_LISTs", false, false)

FunctionPass *toolchain::createWebAssemblyNullifyDebugValueLists() {
  return new WebAssemblyNullifyDebugValueLists();
}

bool WebAssemblyNullifyDebugValueLists::runOnMachineFunction(
    MachineFunction &MF) {
  LLVM_DEBUG(dbgs() << "********** Nullify DBG_VALUE_LISTs **********\n"
                       "********** Function: "
                    << MF.getName() << '\n');
  bool Changed = false;
  // Our backend, including WebAssemblyDebugValueManager, currently cannot
  // handle DBG_VALUE_LISTs correctly. So this makes them undefined, which will
  // appear as "optimized out".
  for (auto &MBB : MF) {
    for (auto &MI : MBB) {
      if (MI.getOpcode() == TargetOpcode::DBG_VALUE_LIST) {
        MI.setDebugValueUndef();
        Changed = true;
      }
    }
  }
  return Changed;
}
