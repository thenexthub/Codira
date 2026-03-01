//===-- GCMetadata.cpp - Garbage collector metadata -----------------------===//
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
// This file implements the GCFunctionInfo class and GCModuleInfo pass.
//
//===----------------------------------------------------------------------===//

#include "vm/core/CodeGen/GCMetadata.h"
#include "vm/core/IR/Function.h"
#include "vm/core/IR/Module.h"
#include "vm/core/InitializePasses.h"
#include "vm/core/Pass.h"
#include <cassert>
#include <memory>
#include <string>

using namespace vm::core;

bool GCStrategyMap::invalidate(Module &M, const PreservedAnalyses &PA,
                               ModuleAnalysisManager::Invalidator &) {
  for (const auto &F : M) {
    if (F.isDeclaration() || !F.hasGC())
      continue;
    if (!contains(F.getGC()))
      return true;
  }
  return false;
}

AnalysisKey CollectorMetadataAnalysis::Key;

CollectorMetadataAnalysis::Result
CollectorMetadataAnalysis::run(Module &M, ModuleAnalysisManager &MAM) {
  Result StrategyMap;
  for (auto &F : M) {
    if (F.isDeclaration() || !F.hasGC())
      continue;
    StringRef GCName = F.getGC();
    auto [It, Inserted] = StrategyMap.try_emplace(GCName);
    if (Inserted) {
      It->second = getGCStrategy(GCName);
      It->second->Name = GCName;
    }
  }
  return StrategyMap;
}

AnalysisKey GCFunctionAnalysis::Key;

GCFunctionAnalysis::Result
GCFunctionAnalysis::run(Function &F, FunctionAnalysisManager &FAM) {
  assert(!F.isDeclaration() && "Can only get GCFunctionInfo for a definition!");
  assert(F.hasGC() && "Function doesn't have GC!");

  auto &MAMProxy = FAM.getResult<ModuleAnalysisManagerFunctionProxy>(F);
  assert(
      MAMProxy.cachedResultExists<CollectorMetadataAnalysis>(*F.getParent()) &&
      "This pass need module analysis `collector-metadata`!");
  auto &Map =
      *MAMProxy.getCachedResult<CollectorMetadataAnalysis>(*F.getParent());
  GCStrategy &S = *Map.try_emplace(F.getGC()).first->second;
  GCFunctionInfo Info(F, S);
  return Info;
}

INITIALIZE_PASS(GCModuleInfo, "collector-metadata",
                "Create Garbage Collector Module Metadata", false, true)

// -----------------------------------------------------------------------------

GCFunctionInfo::GCFunctionInfo(const Function &F, GCStrategy &S)
    : F(F), S(S), FrameSize(~0LL) {}

GCFunctionInfo::~GCFunctionInfo() = default;

bool GCFunctionInfo::invalidate(Function &F, const PreservedAnalyses &PA,
                                FunctionAnalysisManager::Invalidator &) {
  auto PAC = PA.getChecker<GCFunctionAnalysis>();
  return !PAC.preserved() && !PAC.preservedSet<AllAnalysesOn<Function>>();
}

// -----------------------------------------------------------------------------

char GCModuleInfo::ID = 0;

GCModuleInfo::GCModuleInfo() : ImmutablePass(ID) {
  initializeGCModuleInfoPass(*PassRegistry::getPassRegistry());
}

GCFunctionInfo &GCModuleInfo::getFunctionInfo(const Function &F) {
  assert(!F.isDeclaration() && "Can only get GCFunctionInfo for a definition!");
  assert(F.hasGC());

  finfo_map_type::iterator I = FInfoMap.find(&F);
  if (I != FInfoMap.end())
    return *I->second;

  GCStrategy *S = getGCStrategy(F.getGC());
  Functions.push_back(std::make_unique<GCFunctionInfo>(F, *S));
  GCFunctionInfo *GFI = Functions.back().get();
  FInfoMap[&F] = GFI;
  return *GFI;
}

void GCModuleInfo::clear() {
  Functions.clear();
  FInfoMap.clear();
  GCStrategyList.clear();
}

// -----------------------------------------------------------------------------

GCStrategy *GCModuleInfo::getGCStrategy(const StringRef Name) {
  // TODO: Arguably, just doing a linear search would be faster for small N
  auto NMI = GCStrategyMap.find(Name);
  if (NMI != GCStrategyMap.end())
    return NMI->getValue();

  std::unique_ptr<GCStrategy> S = toolchain::getGCStrategy(Name);
  S->Name = std::string(Name);
  GCStrategyMap[Name] = S.get();
  GCStrategyList.push_back(std::move(S));
  return GCStrategyList.back().get();
}
