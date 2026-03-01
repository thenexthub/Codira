//=--- RegUsageInfoPropagate.cpp - Register Usage Informartion Propagation --=//
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
/// This pass is required to take advantage of the interprocedural register
/// allocation infrastructure.
///
/// This pass iterates through MachineInstrs in a given MachineFunction and at
/// each callsite queries RegisterUsageInfo for RegMask (calculated based on
/// actual register allocation) of the callee function, if the RegMask detail
/// is available then this pass will update the RegMask of the call instruction.
/// This updated RegMask will be used by the register allocator while allocating
/// the current MachineFunction.
///
//===----------------------------------------------------------------------===//

#include "vm/core/CodeGen/RegUsageInfoPropagate.h"
#include "vm/core/CodeGen/MachineBasicBlock.h"
#include "vm/core/CodeGen/MachineFrameInfo.h"
#include "vm/core/CodeGen/MachineFunctionPass.h"
#include "vm/core/CodeGen/MachineInstr.h"
#include "vm/core/CodeGen/MachineRegisterInfo.h"
#include "vm/core/CodeGen/Passes.h"
#include "vm/core/CodeGen/RegisterUsageInfo.h"
#include "vm/core/IR/Analysis.h"
#include "vm/core/IR/Module.h"
#include "vm/core/Pass.h"
#include "vm/core/Support/Debug.h"
#include "vm/core/Support/raw_ostream.h"

using namespace vm::core;

#define DEBUG_TYPE "ip-regalloc"

#define RUIP_NAME "Register Usage Information Propagation"

namespace {

class RegUsageInfoPropagation {
public:
  explicit RegUsageInfoPropagation(PhysicalRegisterUsageInfo *PRUI)
      : PRUI(PRUI) {}

  bool run(MachineFunction &MF);

private:
  PhysicalRegisterUsageInfo *PRUI;

  static void setRegMask(MachineInstr &MI, ArrayRef<uint32_t> RegMask) {
    assert(RegMask.size() ==
           MachineOperand::getRegMaskSize(MI.getParent()->getParent()
                                          ->getRegInfo().getTargetRegisterInfo()
                                          ->getNumRegs())
           && "expected register mask size");
    for (MachineOperand &MO : MI.operands()) {
      if (MO.isRegMask())
        MO.setRegMask(RegMask.data());
    }
  }
};

class RegUsageInfoPropagationLegacy : public MachineFunctionPass {
public:
  static char ID;
  RegUsageInfoPropagationLegacy() : MachineFunctionPass(ID) {
    PassRegistry &Registry = *PassRegistry::getPassRegistry();
    initializeRegUsageInfoPropagationLegacyPass(Registry);
  }

  StringRef getPassName() const override { return RUIP_NAME; }

  bool runOnMachineFunction(MachineFunction &MF) override;

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<PhysicalRegisterUsageInfoWrapperLegacy>();
    AU.setPreservesAll();
    MachineFunctionPass::getAnalysisUsage(AU);
  }
};

} // end of anonymous namespace

INITIALIZE_PASS_BEGIN(RegUsageInfoPropagationLegacy, "reg-usage-propagation",
                      RUIP_NAME, false, false)
INITIALIZE_PASS_DEPENDENCY(PhysicalRegisterUsageInfoWrapperLegacy)
INITIALIZE_PASS_END(RegUsageInfoPropagationLegacy, "reg-usage-propagation",
                    RUIP_NAME, false, false)

char RegUsageInfoPropagationLegacy::ID = 0;

// Assumes call instructions have a single reference to a function.
static const Function *findCalledFunction(const Module &M,
                                          const MachineInstr &MI) {
  for (const MachineOperand &MO : MI.operands()) {
    if (MO.isGlobal())
      return dyn_cast<const Function>(MO.getGlobal());

    if (MO.isSymbol())
      return M.getFunction(MO.getSymbolName());
  }

  return nullptr;
}

bool RegUsageInfoPropagationLegacy::runOnMachineFunction(MachineFunction &MF) {
  PhysicalRegisterUsageInfo *PRUI =
      &getAnalysis<PhysicalRegisterUsageInfoWrapperLegacy>().getPRUI();

  RegUsageInfoPropagation RUIP(PRUI);
  return RUIP.run(MF);
}

PreservedAnalyses
RegUsageInfoPropagationPass::run(MachineFunction &MF,
                                 MachineFunctionAnalysisManager &MFAM) {
  Module &MFA = *MF.getFunction().getParent();
  auto *PRUI = MFAM.getResult<ModuleAnalysisManagerMachineFunctionProxy>(MF)
                   .getCachedResult<PhysicalRegisterUsageAnalysis>(MFA);
  assert(PRUI && "PhysicalRegisterUsageAnalysis not available");
  RegUsageInfoPropagation(PRUI).run(MF);
  return PreservedAnalyses::all();
}

bool RegUsageInfoPropagation::run(MachineFunction &MF) {
  const Module &M = *MF.getFunction().getParent();

  LLVM_DEBUG(dbgs() << " ++++++++++++++++++++ " << RUIP_NAME
                    << " ++++++++++++++++++++  \n");
  LLVM_DEBUG(dbgs() << "MachineFunction : " << MF.getName() << "\n");

  const MachineFrameInfo &MFI = MF.getFrameInfo();
  if (!MFI.hasCalls() && !MFI.hasTailCall())
    return false;

  bool Changed = false;

  for (MachineBasicBlock &MBB : MF) {
    for (MachineInstr &MI : MBB) {
      if (!MI.isCall())
        continue;
      LLVM_DEBUG(
          dbgs()
          << "Call Instruction Before Register Usage Info Propagation : \n"
          << MI << "\n");

      auto UpdateRegMask = [&](const Function &F) {
        const ArrayRef<uint32_t> RegMask = PRUI->getRegUsageInfo(F);
        if (RegMask.empty())
          return;
        setRegMask(MI, RegMask);
        Changed = true;
      };

      if (const Function *F = findCalledFunction(M, MI)) {
        if (F->isDefinitionExact()) {
          UpdateRegMask(*F);
        } else {
          LLVM_DEBUG(dbgs() << "Function definition is not exact\n");
        }
      } else {
        LLVM_DEBUG(dbgs() << "Failed to find call target function\n");
      }

      LLVM_DEBUG(
          dbgs()
          << "Call Instruction After Register Usage Info Propagation : \n"
          << MI << '\n');
    }
  }

  LLVM_DEBUG(
      dbgs() << " +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"
                "++++++ \n");
  return Changed;
}

FunctionPass *toolchain::createRegUsageInfoPropPass() {
  return new RegUsageInfoPropagationLegacy();
}
