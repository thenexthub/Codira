//===-- WebAssemblyLowerBrUnless.cpp - Lower br_unless --------------------===//
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
/// This file lowers br_unless into br_if with an inverted condition.
///
/// br_unless is not currently in the spec, but it's very convenient for LLVM
/// to use. This pass allows LLVM to use it, for now.
///
//===----------------------------------------------------------------------===//

#include "MCTargetDesc/WebAssemblyMCTargetDesc.h"
#include "WebAssembly.h"
#include "WebAssemblyMachineFunctionInfo.h"
#include "WebAssemblySubtarget.h"
#include "vm/core/CodeGen/MachineFunctionPass.h"
#include "vm/core/CodeGen/MachineInstrBuilder.h"
#include "vm/core/Support/Debug.h"
#include "vm/core/Support/raw_ostream.h"
using namespace vm::core;

#define DEBUG_TYPE "wasm-lower-br_unless"

namespace {
class WebAssemblyLowerBrUnless final : public MachineFunctionPass {
  StringRef getPassName() const override {
    return "WebAssembly Lower br_unless";
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesCFG();
    MachineFunctionPass::getAnalysisUsage(AU);
  }

  bool runOnMachineFunction(MachineFunction &MF) override;

public:
  static char ID; // Pass identification, replacement for typeid
  WebAssemblyLowerBrUnless() : MachineFunctionPass(ID) {}
};
} // end anonymous namespace

char WebAssemblyLowerBrUnless::ID = 0;
INITIALIZE_PASS(WebAssemblyLowerBrUnless, DEBUG_TYPE,
                "Lowers br_unless into inverted br_if", false, false)

FunctionPass *toolchain::createWebAssemblyLowerBrUnless() {
  return new WebAssemblyLowerBrUnless();
}

bool WebAssemblyLowerBrUnless::runOnMachineFunction(MachineFunction &MF) {
  LLVM_DEBUG(dbgs() << "********** Lowering br_unless **********\n"
                       "********** Function: "
                    << MF.getName() << '\n');

  auto &MFI = *MF.getInfo<WebAssemblyFunctionInfo>();
  const auto &TII = *MF.getSubtarget<WebAssemblySubtarget>().getInstrInfo();
  auto &MRI = MF.getRegInfo();

  for (auto &MBB : MF) {
    for (MachineInstr &MI : toolchain::make_early_inc_range(MBB)) {
      if (MI.getOpcode() != WebAssembly::BR_UNLESS)
        continue;

      Register Cond = MI.getOperand(1).getReg();
      bool Inverted = false;

      // Attempt to invert the condition in place.
      if (MFI.isVRegStackified(Cond)) {
        assert(MRI.hasOneDef(Cond));
        MachineInstr *Def = MRI.getVRegDef(Cond);
        switch (Def->getOpcode()) {
          using namespace WebAssembly;
        case EQ_I32:
          Def->setDesc(TII.get(NE_I32));
          Inverted = true;
          break;
        case NE_I32:
          Def->setDesc(TII.get(EQ_I32));
          Inverted = true;
          break;
        case GT_S_I32:
          Def->setDesc(TII.get(LE_S_I32));
          Inverted = true;
          break;
        case GE_S_I32:
          Def->setDesc(TII.get(LT_S_I32));
          Inverted = true;
          break;
        case LT_S_I32:
          Def->setDesc(TII.get(GE_S_I32));
          Inverted = true;
          break;
        case LE_S_I32:
          Def->setDesc(TII.get(GT_S_I32));
          Inverted = true;
          break;
        case GT_U_I32:
          Def->setDesc(TII.get(LE_U_I32));
          Inverted = true;
          break;
        case GE_U_I32:
          Def->setDesc(TII.get(LT_U_I32));
          Inverted = true;
          break;
        case LT_U_I32:
          Def->setDesc(TII.get(GE_U_I32));
          Inverted = true;
          break;
        case LE_U_I32:
          Def->setDesc(TII.get(GT_U_I32));
          Inverted = true;
          break;
        case EQ_I64:
          Def->setDesc(TII.get(NE_I64));
          Inverted = true;
          break;
        case NE_I64:
          Def->setDesc(TII.get(EQ_I64));
          Inverted = true;
          break;
        case GT_S_I64:
          Def->setDesc(TII.get(LE_S_I64));
          Inverted = true;
          break;
        case GE_S_I64:
          Def->setDesc(TII.get(LT_S_I64));
          Inverted = true;
          break;
        case LT_S_I64:
          Def->setDesc(TII.get(GE_S_I64));
          Inverted = true;
          break;
        case LE_S_I64:
          Def->setDesc(TII.get(GT_S_I64));
          Inverted = true;
          break;
        case GT_U_I64:
          Def->setDesc(TII.get(LE_U_I64));
          Inverted = true;
          break;
        case GE_U_I64:
          Def->setDesc(TII.get(LT_U_I64));
          Inverted = true;
          break;
        case LT_U_I64:
          Def->setDesc(TII.get(GE_U_I64));
          Inverted = true;
          break;
        case LE_U_I64:
          Def->setDesc(TII.get(GT_U_I64));
          Inverted = true;
          break;
        case EQ_F32:
          Def->setDesc(TII.get(NE_F32));
          Inverted = true;
          break;
        case NE_F32:
          Def->setDesc(TII.get(EQ_F32));
          Inverted = true;
          break;
        case EQ_F64:
          Def->setDesc(TII.get(NE_F64));
          Inverted = true;
          break;
        case NE_F64:
          Def->setDesc(TII.get(EQ_F64));
          Inverted = true;
          break;
        case EQZ_I32: {
          // Invert an eqz by replacing it with its operand.
          Cond = Def->getOperand(1).getReg();
          Def->eraseFromParent();
          Inverted = true;
          break;
        }
        default:
          break;
        }
      }

      // If we weren't able to invert the condition in place. Insert an
      // instruction to invert it.
      if (!Inverted) {
        Register Tmp = MRI.createVirtualRegister(&WebAssembly::I32RegClass);
        BuildMI(MBB, &MI, MI.getDebugLoc(), TII.get(WebAssembly::EQZ_I32), Tmp)
            .addReg(Cond);
        MFI.stackifyVReg(MRI, Tmp);
        Cond = Tmp;
        Inverted = true;
      }

      // The br_unless condition has now been inverted. Insert a br_if and
      // delete the br_unless.
      assert(Inverted);
      BuildMI(MBB, &MI, MI.getDebugLoc(), TII.get(WebAssembly::BR_IF))
          .add(MI.getOperand(0))
          .addReg(Cond);
      MBB.erase(&MI);
    }
  }

  return true;
}
