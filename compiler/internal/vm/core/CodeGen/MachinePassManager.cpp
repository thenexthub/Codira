//===---------- MachinePassManager.cpp ------------------------------------===//
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
// This file contains the pass management machinery for machine functions.
//
//===----------------------------------------------------------------------===//

#include "vm/core/CodeGen/MachinePassManager.h"
#include "vm/core/CodeGen/MachineFunction.h"
#include "vm/core/CodeGen/MachineFunctionAnalysis.h"
#include "vm/core/CodeGen/MachineModuleInfo.h"
#include "vm/core/IR/Function.h"
#include "vm/core/IR/Module.h"
#include "vm/core/IR/PassManagerImpl.h"
#include "vm/core/Support/Compiler.h"

using namespace vm::core;

AnalysisKey FunctionAnalysisManagerMachineFunctionProxy::Key;

namespace vm::core {
template class LLVM_EXPORT_TEMPLATE AnalysisManager<MachineFunction>;
template class PassManager<MachineFunction>;
template class LLVM_EXPORT_TEMPLATE
    InnerAnalysisManagerProxy<MachineFunctionAnalysisManager, Module>;
template class LLVM_EXPORT_TEMPLATE
    InnerAnalysisManagerProxy<MachineFunctionAnalysisManager, Function>;
template class LLVM_EXPORT_TEMPLATE
    OuterAnalysisManagerProxy<ModuleAnalysisManager, MachineFunction>;
} // namespace vm::core

bool FunctionAnalysisManagerMachineFunctionProxy::Result::invalidate(
    MachineFunction &IR, const PreservedAnalyses &PA,
    MachineFunctionAnalysisManager::Invalidator &Inv) {
  // MachineFunction passes should not invalidate Function analyses.
  // TODO: verify that PA doesn't invalidate Function analyses.
  return false;
}

template <>
bool MachineFunctionAnalysisManagerModuleProxy::Result::invalidate(
    Module &M, const PreservedAnalyses &PA,
    ModuleAnalysisManager::Invalidator &Inv) {
  // If literally everything is preserved, we're done.
  if (PA.areAllPreserved())
    return false; // This is still a valid proxy.

  // If this proxy isn't marked as preserved, then even if the result remains
  // valid, the key itself may no longer be valid, so we clear everything.
  //
  // Note that in order to preserve this proxy, a module pass must ensure that
  // the MFAM has been completely updated to handle the deletion of functions.
  // Specifically, any MFAM-cached results for those functions need to have been
  // forcibly cleared. When preserved, this proxy will only invalidate results
  // cached on functions *still in the module* at the end of the module pass.
  auto PAC = PA.getChecker<MachineFunctionAnalysisManagerModuleProxy>();
  if (!PAC.preserved() && !PAC.preservedSet<AllAnalysesOn<Module>>()) {
    InnerAM->clear();
    return true;
  }

  // FIXME: be more precise, see
  // FunctionAnalysisManagerModuleProxy::Result::invalidate.
  if (!PA.allAnalysesInSetPreserved<AllAnalysesOn<MachineFunction>>()) {
    InnerAM->clear();
    return true;
  }

  // Return false to indicate that this result is still a valid proxy.
  return false;
}

template <>
bool MachineFunctionAnalysisManagerFunctionProxy::Result::invalidate(
    Function &F, const PreservedAnalyses &PA,
    FunctionAnalysisManager::Invalidator &Inv) {
  // If literally everything is preserved, we're done.
  if (PA.areAllPreserved())
    return false; // This is still a valid proxy.

  // If this proxy isn't marked as preserved, then even if the result remains
  // valid, the key itself may no longer be valid, so we clear everything.
  //
  // Note that in order to preserve this proxy, a module pass must ensure that
  // the MFAM has been completely updated to handle the deletion of functions.
  // Specifically, any MFAM-cached results for those functions need to have been
  // forcibly cleared. When preserved, this proxy will only invalidate results
  // cached on functions *still in the module* at the end of the module pass.
  auto PAC = PA.getChecker<MachineFunctionAnalysisManagerFunctionProxy>();
  if (!PAC.preserved() && !PAC.preservedSet<AllAnalysesOn<Function>>()) {
    InnerAM->clear();
    return true;
  }

  // FIXME: be more precise, see
  // FunctionAnalysisManagerModuleProxy::Result::invalidate.
  if (!PA.allAnalysesInSetPreserved<AllAnalysesOn<MachineFunction>>()) {
    InnerAM->clear();
    return true;
  }

  // Return false to indicate that this result is still a valid proxy.
  return false;
}

PreservedAnalyses
FunctionToMachineFunctionPassAdaptor::run(Function &F,
                                          FunctionAnalysisManager &FAM) {
  MachineFunctionAnalysisManager &MFAM =
      FAM.getResult<MachineFunctionAnalysisManagerFunctionProxy>(F)
          .getManager();
  PassInstrumentation PI = FAM.getResult<PassInstrumentationAnalysis>(F);
  PreservedAnalyses PA = PreservedAnalyses::all();
  // Do not codegen any 'available_externally' functions at all, they have
  // definitions outside the translation unit.
  if (F.isDeclaration() || F.hasAvailableExternallyLinkage())
    return PreservedAnalyses::all();

  MachineFunction &MF = FAM.getResult<MachineFunctionAnalysis>(F).getMF();

  if (!PI.runBeforePass<MachineFunction>(*Pass, MF))
    return PreservedAnalyses::all();
  PreservedAnalyses PassPA = Pass->run(MF, MFAM);
  MFAM.invalidate(MF, PassPA);
  PI.runAfterPass(*Pass, MF, PassPA);
  PA.intersect(std::move(PassPA));

  return PA;
}

void FunctionToMachineFunctionPassAdaptor::printPipeline(
    raw_ostream &OS, function_ref<StringRef(StringRef)> MapClassName2PassName) {
  OS << "machine-function(";
  Pass->printPipeline(OS, MapClassName2PassName);
  OS << ')';
}

template <>
PreservedAnalyses
PassManager<MachineFunction>::run(MachineFunction &MF,
                                  AnalysisManager<MachineFunction> &MFAM) {
  PassInstrumentation PI = MFAM.getResult<PassInstrumentationAnalysis>(MF);
  PreservedAnalyses PA = PreservedAnalyses::all();
  for (auto &Pass : Passes) {
    if (!PI.runBeforePass<MachineFunction>(*Pass, MF))
      continue;

    PreservedAnalyses PassPA = Pass->run(MF, MFAM);
    MFAM.invalidate(MF, PassPA);
    PI.runAfterPass(*Pass, MF, PassPA);
    PA.intersect(std::move(PassPA));
  }
  PA.preserveSet<AllAnalysesOn<MachineFunction>>();
  return PA;
}

PreservedAnalyses toolchain::getMachineFunctionPassPreservedAnalyses() {
  PreservedAnalyses PA;
  // Machine function passes are not allowed to modify the LLVM
  // representation, therefore we should preserve all IR analyses.
  PA.template preserveSet<AllAnalysesOn<Module>>();
  PA.template preserveSet<AllAnalysesOn<Function>>();
  return PA;
}
