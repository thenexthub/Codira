//===- MachineFunctionAnalysis.cpp ----------------------------------------===//
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
// This file contains the definitions of the MachineFunctionAnalysis
// members.
//
//===----------------------------------------------------------------------===//

#include "vm/core/CodeGen/MachineFunctionAnalysis.h"
#include "vm/core/CodeGen/MachineFunction.h"
#include "vm/core/CodeGen/MachineModuleInfo.h"
#include "vm/core/IR/Function.h"
#include "vm/core/Target/TargetMachine.h"

using namespace vm::core;

AnalysisKey MachineFunctionAnalysis::Key;

toolchain::MachineFunctionAnalysis::Result::Result(
    std::unique_ptr<MachineFunction> MF)
    : MF(std::move(MF)) {}

bool MachineFunctionAnalysis::Result::invalidate(
    Function &, const PreservedAnalyses &PA,
    FunctionAnalysisManager::Invalidator &) {
  // Unless it is invalidated explicitly, it should remain preserved.
  auto PAC = PA.getChecker<MachineFunctionAnalysis>();
  return !PAC.preservedWhenStateless();
}

MachineFunctionAnalysis::Result
MachineFunctionAnalysis::run(Function &F, FunctionAnalysisManager &FAM) {
  auto &Context = F.getContext();
  const TargetSubtargetInfo &STI = *TM->getSubtargetImpl(F);
  auto &MMI = FAM.getResult<ModuleAnalysisManagerFunctionProxy>(F)
                  .getCachedResult<MachineModuleAnalysis>(*F.getParent())
                  ->getMMI();
  auto MF = std::make_unique<MachineFunction>(
      F, *TM, STI, MMI.getContext(), Context.generateMachineFunctionNum(F));
  MF->initTargetMachineFunctionInfo(STI);

  // MRI callback for target specific initializations.
  TM->registerMachineRegisterInfoCallback(*MF);

  return Result(std::move(MF));
}

PreservedAnalyses FreeMachineFunctionPass::run(Function &F,
                                               FunctionAnalysisManager &FAM) {
  FAM.clearAnalysis<MachineFunctionAnalysis>(F);
  return PreservedAnalyses::all();
}
