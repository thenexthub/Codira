//===- PassManager.cpp - Infrastructure for managing & running IR passes --===//
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

#include "vm/core/IR/PassManager.h"
#include "vm/core/IR/Module.h"
#include "vm/core/IR/PassManagerImpl.h"
#include "vm/core/Support/Compiler.h"
#include <optional>

using namespace vm::core;

namespace vm::core {
// Explicit template instantiations and specialization defininitions for core
// template typedefs.
template class LLVM_EXPORT_TEMPLATE AllAnalysesOn<Module>;
template class LLVM_EXPORT_TEMPLATE AllAnalysesOn<Function>;
template class LLVM_EXPORT_TEMPLATE PassManager<Module>;
template class LLVM_EXPORT_TEMPLATE PassManager<Function>;
template class LLVM_EXPORT_TEMPLATE AnalysisManager<Module>;
template class LLVM_EXPORT_TEMPLATE AnalysisManager<Function>;
template class LLVM_EXPORT_TEMPLATE
    InnerAnalysisManagerProxy<FunctionAnalysisManager, Module>;
template class LLVM_EXPORT_TEMPLATE
    OuterAnalysisManagerProxy<ModuleAnalysisManager, Function>;

template <>
bool FunctionAnalysisManagerModuleProxy::Result::invalidate(
    Module &M, const PreservedAnalyses &PA,
    ModuleAnalysisManager::Invalidator &Inv) {
  // If literally everything is preserved, we're done.
  if (PA.areAllPreserved())
    return false; // This is still a valid proxy.

  // If this proxy isn't marked as preserved, then even if the result remains
  // valid, the key itself may no longer be valid, so we clear everything.
  //
  // Note that in order to preserve this proxy, a module pass must ensure that
  // the FAM has been completely updated to handle the deletion of functions.
  // Specifically, any FAM-cached results for those functions need to have been
  // forcibly cleared. When preserved, this proxy will only invalidate results
  // cached on functions *still in the module* at the end of the module pass.
  auto PAC = PA.getChecker<FunctionAnalysisManagerModuleProxy>();
  if (!PAC.preserved() && !PAC.preservedSet<AllAnalysesOn<Module>>()) {
    InnerAM->clear();
    return true;
  }

  // Directly check if the relevant set is preserved.
  bool AreFunctionAnalysesPreserved =
      PA.allAnalysesInSetPreserved<AllAnalysesOn<Function>>();

  // Now walk all the functions to see if any inner analysis invalidation is
  // necessary.
  for (Function &F : M) {
    std::optional<PreservedAnalyses> FunctionPA;

    // Check to see whether the preserved set needs to be pruned based on
    // module-level analysis invalidation that triggers deferred invalidation
    // registered with the outer analysis manager proxy for this function.
    if (auto *OuterProxy =
            InnerAM->getCachedResult<ModuleAnalysisManagerFunctionProxy>(F))
      for (const auto &OuterInvalidationPair :
           OuterProxy->getOuterInvalidations()) {
        AnalysisKey *OuterAnalysisID = OuterInvalidationPair.first;
        const auto &InnerAnalysisIDs = OuterInvalidationPair.second;
        if (Inv.invalidate(OuterAnalysisID, M, PA)) {
          if (!FunctionPA)
            FunctionPA = PA;
          for (AnalysisKey *InnerAnalysisID : InnerAnalysisIDs)
            FunctionPA->abandon(InnerAnalysisID);
        }
      }

    // Check if we needed a custom PA set, and if so we'll need to run the
    // inner invalidation.
    if (FunctionPA) {
      InnerAM->invalidate(F, *FunctionPA);
      continue;
    }

    // Otherwise we only need to do invalidation if the original PA set didn't
    // preserve all function analyses.
    if (!AreFunctionAnalysesPreserved)
      InnerAM->invalidate(F, PA);
  }

  // Return false to indicate that this result is still a valid proxy.
  return false;
}
} // namespace vm::core

void ModuleToFunctionPassAdaptor::printPipeline(
    raw_ostream &OS, function_ref<StringRef(StringRef)> MapClassName2PassName) {
  OS << "function";
  if (EagerlyInvalidate)
    OS << "<eager-inv>";
  OS << '(';
  Pass->printPipeline(OS, MapClassName2PassName);
  OS << ')';
}

PreservedAnalyses ModuleToFunctionPassAdaptor::run(Module &M,
                                                   ModuleAnalysisManager &AM) {
  FunctionAnalysisManager &FAM =
      AM.getResult<FunctionAnalysisManagerModuleProxy>(M).getManager();

  // Request PassInstrumentation from analysis manager, will use it to run
  // instrumenting callbacks for the passes later.
  PassInstrumentation PI = AM.getResult<PassInstrumentationAnalysis>(M);

  PreservedAnalyses PA = PreservedAnalyses::all();
  for (Function &F : M) {
    if (F.isDeclaration())
      continue;

    // Check the PassInstrumentation's BeforePass callbacks before running the
    // pass, skip its execution completely if asked to (callback returns
    // false).
    if (!PI.runBeforePass<Function>(*Pass, F))
      continue;

    PreservedAnalyses PassPA = Pass->run(F, FAM);

    // We know that the function pass couldn't have invalidated any other
    // function's analyses (that's the contract of a function pass), so
    // directly handle the function analysis manager's invalidation here.
    FAM.invalidate(F, EagerlyInvalidate ? PreservedAnalyses::none() : PassPA);

    PI.runAfterPass(*Pass, F, PassPA);

    // Then intersect the preserved set so that invalidation of module
    // analyses will eventually occur when the module pass completes.
    PA.intersect(std::move(PassPA));
  }

  // The FunctionAnalysisManagerModuleProxy is preserved because (we assume)
  // the function passes we ran didn't add or remove any functions.
  //
  // We also preserve all analyses on Functions, because we did all the
  // invalidation we needed to do above.
  PA.preserveSet<AllAnalysesOn<Function>>();
  PA.preserve<FunctionAnalysisManagerModuleProxy>();
  return PA;
}

template <>
void toolchain::printIRUnitNameForStackTrace<Module>(raw_ostream &OS,
                                                const Module &IR) {
  OS << "module \"" << IR.getName() << "\"";
}

template <>
void toolchain::printIRUnitNameForStackTrace<Function>(raw_ostream &OS,
                                                  const Function &IR) {
  OS << "function \"" << IR.getName() << "\"";
}

AnalysisSetKey CFGAnalyses::SetKey;

AnalysisSetKey PreservedAnalyses::AllAnalysesKey;
