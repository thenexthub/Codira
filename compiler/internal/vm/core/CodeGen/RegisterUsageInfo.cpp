//===- RegisterUsageInfo.cpp - Register Usage Information Storage ---------===//
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
//===----------------------------------------------------------------------===//

#include "vm/core/CodeGen/RegisterUsageInfo.h"
#include "vm/core/ADT/SmallVector.h"
#include "vm/core/CodeGen/MachineOperand.h"
#include "vm/core/CodeGen/TargetRegisterInfo.h"
#include "vm/core/CodeGen/TargetSubtargetInfo.h"
#include "vm/core/IR/Analysis.h"
#include "vm/core/IR/Function.h"
#include "vm/core/IR/Module.h"
#include "vm/core/IR/PassManager.h"
#include "vm/core/Pass.h"
#include "vm/core/Support/CommandLine.h"
#include "vm/core/Support/raw_ostream.h"
#include "vm/core/Target/TargetMachine.h"
#include <cstdint>
#include <utility>
#include <vector>

using namespace vm::core;

// Defined in TargetPassConfig.cpp
extern cl::opt<bool> PrintRegUsage;

INITIALIZE_PASS(PhysicalRegisterUsageInfoWrapperLegacy, "reg-usage-info",
                "Register Usage Information Storage", false, true)

char PhysicalRegisterUsageInfoWrapperLegacy::ID = 0;

void PhysicalRegisterUsageInfo::setTargetMachine(const TargetMachine &TM) {
  this->TM = &TM;
}

bool PhysicalRegisterUsageInfo::doInitialization(Module &M) {
  RegMasks.reserve(M.size());
  return false;
}

bool PhysicalRegisterUsageInfo::doFinalization(Module &M) {
  if (PrintRegUsage)
    print(errs());

  RegMasks.shrink_and_clear();
  return false;
}

void PhysicalRegisterUsageInfo::storeUpdateRegUsageInfo(
    const Function &FP, ArrayRef<uint32_t> RegMask) {
  RegMasks[&FP] = RegMask;
}

ArrayRef<uint32_t>
PhysicalRegisterUsageInfo::getRegUsageInfo(const Function &FP) {
  auto It = RegMasks.find(&FP);
  if (It != RegMasks.end())
    return ArrayRef<uint32_t>(It->second);
  return ArrayRef<uint32_t>();
}

void PhysicalRegisterUsageInfo::print(raw_ostream &OS, const Module *M) const {
  using FuncPtrRegMaskPair = std::pair<const Function *, std::vector<uint32_t>>;

  // Create a vector of pointer to RegMasks entries
  SmallVector<const FuncPtrRegMaskPair *, 64> FPRMPairVector(
      toolchain::make_pointer_range(RegMasks));

  // sort the vector to print analysis in alphabatic order of function name.
  toolchain::sort(
      FPRMPairVector,
      [](const FuncPtrRegMaskPair *A, const FuncPtrRegMaskPair *B) -> bool {
        return A->first->getName() < B->first->getName();
      });

  for (const FuncPtrRegMaskPair *FPRMPair : FPRMPairVector) {
    OS << FPRMPair->first->getName() << " "
       << "Clobbered Registers: ";
    const TargetRegisterInfo *TRI
        = TM->getSubtarget<TargetSubtargetInfo>(*(FPRMPair->first))
          .getRegisterInfo();

    for (unsigned PReg = 1, PRegE = TRI->getNumRegs(); PReg < PRegE; ++PReg) {
      if (MachineOperand::clobbersPhysReg(&(FPRMPair->second[0]), PReg))
        OS << printReg(PReg, TRI) << " ";
    }
    OS << "\n";
  }
}

bool PhysicalRegisterUsageInfo::invalidate(
    Module &M, const PreservedAnalyses &PA,
    ModuleAnalysisManager::Invalidator &) {
  auto PAC = PA.getChecker<PhysicalRegisterUsageAnalysis>();
  return !PAC.preservedWhenStateless();
}

AnalysisKey PhysicalRegisterUsageAnalysis::Key;
PhysicalRegisterUsageInfo
PhysicalRegisterUsageAnalysis::run(Module &M, ModuleAnalysisManager &) {
  PhysicalRegisterUsageInfo PRUI;
  PRUI.doInitialization(M);
  return PRUI;
}

PreservedAnalyses
PhysicalRegisterUsageInfoPrinterPass::run(Module &M,
                                          ModuleAnalysisManager &AM) {
  auto *PRUI = &AM.getResult<PhysicalRegisterUsageAnalysis>(M);
  PRUI->print(OS, &M);
  return PreservedAnalyses::all();
}
