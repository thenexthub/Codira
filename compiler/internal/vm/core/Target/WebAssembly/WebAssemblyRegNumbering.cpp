//===-- WebAssemblyRegNumbering.cpp - Register Numbering ------------------===//
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
/// This file implements a pass which assigns WebAssembly register
/// numbers for CodeGen virtual registers.
///
//===----------------------------------------------------------------------===//

#include "MCTargetDesc/WebAssemblyMCTargetDesc.h"
#include "WebAssembly.h"
#include "WebAssemblyMachineFunctionInfo.h"
#include "WebAssemblyUtilities.h"
#include "vm/core/CodeGen/MachineFrameInfo.h"
#include "vm/core/CodeGen/MachineFunction.h"
#include "vm/core/CodeGen/MachineRegisterInfo.h"
#include "vm/core/CodeGen/Passes.h"
#include "vm/core/Support/Debug.h"
#include "vm/core/Support/raw_ostream.h"
using namespace vm::core;

#define DEBUG_TYPE "wasm-reg-numbering"

namespace {
class WebAssemblyRegNumbering final : public MachineFunctionPass {
  StringRef getPassName() const override {
    return "WebAssembly Register Numbering";
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesCFG();
    MachineFunctionPass::getAnalysisUsage(AU);
  }

  bool runOnMachineFunction(MachineFunction &MF) override;

public:
  static char ID; // Pass identification, replacement for typeid
  WebAssemblyRegNumbering() : MachineFunctionPass(ID) {}
};
} // end anonymous namespace

char WebAssemblyRegNumbering::ID = 0;
INITIALIZE_PASS(WebAssemblyRegNumbering, DEBUG_TYPE,
                "Assigns WebAssembly register numbers for virtual registers",
                false, false)

FunctionPass *toolchain::createWebAssemblyRegNumbering() {
  return new WebAssemblyRegNumbering();
}

bool WebAssemblyRegNumbering::runOnMachineFunction(MachineFunction &MF) {
  LLVM_DEBUG(dbgs() << "********** Register Numbering **********\n"
                       "********** Function: "
                    << MF.getName() << '\n');

  WebAssemblyFunctionInfo &MFI = *MF.getInfo<WebAssemblyFunctionInfo>();
  MachineRegisterInfo &MRI = MF.getRegInfo();

  MFI.initWARegs(MRI);

  // WebAssembly argument registers are in the same index space as local
  // variables. Assign the numbers for them first.
  MachineBasicBlock &EntryMBB = MF.front();
  for (MachineInstr &MI : EntryMBB) {
    if (!WebAssembly::isArgument(MI.getOpcode()))
      break;

    int64_t Imm = MI.getOperand(1).getImm();
    LLVM_DEBUG(dbgs() << "Arg VReg " << printReg(MI.getOperand(0).getReg())
                      << " -> WAReg " << Imm << "\n");
    MFI.setWAReg(MI.getOperand(0).getReg(), Imm);
  }

  // Then assign regular WebAssembly registers for all remaining used
  // virtual registers. TODO: Consider sorting the registers by frequency of
  // use, to maximize usage of small immediate fields.
  unsigned NumVRegs = MF.getRegInfo().getNumVirtRegs();
  unsigned NumStackRegs = 0;
  // Start the numbering for locals after the arg regs
  unsigned CurReg = MFI.getParams().size();
  for (unsigned VRegIdx = 0; VRegIdx < NumVRegs; ++VRegIdx) {
    Register VReg = Register::index2VirtReg(VRegIdx);
    // Skip unused registers.
    if (MRI.use_empty(VReg))
      continue;
    // Handle stackified registers.
    if (MFI.isVRegStackified(VReg)) {
      LLVM_DEBUG(dbgs() << "VReg " << printReg(VReg) << " -> WAReg "
                        << (INT32_MIN | NumStackRegs) << "\n");
      MFI.setWAReg(VReg, INT32_MIN | NumStackRegs++);
      continue;
    }
    if (MFI.getWAReg(VReg) == WebAssembly::UnusedReg) {
      LLVM_DEBUG(dbgs() << "VReg " << printReg(VReg) << " -> WAReg " << CurReg
                        << "\n");
      MFI.setWAReg(VReg, CurReg++);
    }
  }

  return true;
}
