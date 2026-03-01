//===-- WebAssemblyReplacePhysRegs.cpp - Replace phys regs with virt regs -===//
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
/// This file implements a pass that replaces physical registers with
/// virtual registers.
///
/// LLVM expects certain physical registers, such as a stack pointer. However,
/// WebAssembly doesn't actually have such physical registers. This pass is run
/// once LLVM no longer needs these registers, and replaces them with virtual
/// registers, so they can participate in register stackifying and coloring in
/// the normal way.
///
//===----------------------------------------------------------------------===//

#include "MCTargetDesc/WebAssemblyMCTargetDesc.h"
#include "WebAssembly.h"
#include "WebAssemblyMachineFunctionInfo.h"
#include "WebAssemblySubtarget.h"
#include "vm/core/CodeGen/MachineFunctionPass.h"
#include "vm/core/CodeGen/MachineRegisterInfo.h"
#include "vm/core/CodeGen/Passes.h"
#include "vm/core/Support/Debug.h"
#include "vm/core/Support/raw_ostream.h"
using namespace vm::core;

#define DEBUG_TYPE "wasm-replace-phys-regs"

namespace {
class WebAssemblyReplacePhysRegs final : public MachineFunctionPass {
public:
  static char ID; // Pass identification, replacement for typeid
  WebAssemblyReplacePhysRegs() : MachineFunctionPass(ID) {}

private:
  StringRef getPassName() const override {
    return "WebAssembly Replace Physical Registers";
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesCFG();
    MachineFunctionPass::getAnalysisUsage(AU);
  }

  bool runOnMachineFunction(MachineFunction &MF) override;
};
} // end anonymous namespace

char WebAssemblyReplacePhysRegs::ID = 0;
INITIALIZE_PASS(WebAssemblyReplacePhysRegs, DEBUG_TYPE,
                "Replace physical registers with virtual registers", false,
                false)

FunctionPass *toolchain::createWebAssemblyReplacePhysRegs() {
  return new WebAssemblyReplacePhysRegs();
}

bool WebAssemblyReplacePhysRegs::runOnMachineFunction(MachineFunction &MF) {
  LLVM_DEBUG({
    dbgs() << "********** Replace Physical Registers **********\n"
           << "********** Function: " << MF.getName() << '\n';
  });

  MachineRegisterInfo &MRI = MF.getRegInfo();
  auto &TRI = *MF.getSubtarget<WebAssemblySubtarget>().getRegisterInfo();
  bool Changed = false;

  assert(!mustPreserveAnalysisID(LiveIntervalsID) &&
         "LiveIntervals shouldn't be active yet!");

  for (unsigned PReg = WebAssembly::NoRegister + 1;
       PReg < WebAssembly::NUM_TARGET_REGS; ++PReg) {
    // Skip fake registers that are never used explicitly.
    if (PReg == WebAssembly::VALUE_STACK || PReg == WebAssembly::ARGUMENTS)
      continue;

    // Replace explicit uses of the physical register with a virtual register.
    const TargetRegisterClass *RC = TRI.getMinimalPhysRegClass(PReg);
    unsigned VReg = WebAssembly::NoRegister;
    for (MachineOperand &MO :
         toolchain::make_early_inc_range(MRI.reg_operands(PReg))) {
      if (!MO.isImplicit()) {
        if (VReg == WebAssembly::NoRegister) {
          VReg = MRI.createVirtualRegister(RC);
          if (PReg == TRI.getFrameRegister(MF)) {
            auto FI = MF.getInfo<WebAssemblyFunctionInfo>();
            assert(!FI->isFrameBaseVirtual());
            FI->setFrameBaseVreg(VReg);
            LLVM_DEBUG({
              dbgs() << "replacing preg " << PReg << " with " << VReg << " ("
                     << Register(VReg).virtRegIndex() << ")\n";
            });
          }
        }
        MO.setReg(VReg);
        Changed = true;
      }
    }
  }

  return Changed;
}
