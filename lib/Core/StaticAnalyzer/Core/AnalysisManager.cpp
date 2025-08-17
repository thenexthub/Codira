//===-- AnalysisManager.cpp -------------------------------------*- C++ -*-===//
//
// Copyright (c) 2025, NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
// 
// Author: Tunjay Akbarli
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
// 
//     http://www.apache.org/licenses/LICENSE-2.0
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

#include "language/Core/StaticAnalyzer/Core/PathSensitive/AnalysisManager.h"

using namespace language::Core;
using namespace ento;

void AnalysisManager::anchor() { }

AnalysisManager::AnalysisManager(ASTContext &ASTCtx, Preprocessor &PP,
                                 PathDiagnosticConsumers PDC,
                                 StoreManagerCreator storemgr,
                                 ConstraintManagerCreator constraintmgr,
                                 CheckerManager *checkerMgr,
                                 AnalyzerOptions &Options,
                                 std::unique_ptr<CodeInjector> injector)
    : AnaCtxMgr(
          ASTCtx, Options.UnoptimizedCFG,
          Options.ShouldIncludeImplicitDtorsInCFG,
          /*addInitializers=*/true, Options.ShouldIncludeTemporaryDtorsInCFG,
          Options.ShouldIncludeLifetimeInCFG,
          // Adding LoopExit elements to the CFG is a requirement for loop
          // unrolling.
          Options.ShouldIncludeLoopExitInCFG || Options.ShouldUnrollLoops,
          Options.ShouldIncludeScopesInCFG, Options.ShouldSynthesizeBodies,
          Options.ShouldConditionalizeStaticInitializers,
          /*addCXXNewAllocator=*/true,
          Options.ShouldIncludeRichConstructorsInCFG,
          Options.ShouldElideConstructors,
          /*addVirtualBaseBranches=*/true, std::move(injector)),
      Ctx(ASTCtx), PP(PP), LangOpts(ASTCtx.getLangOpts()),
      PathConsumers(std::move(PDC)), CreateStoreMgr(storemgr),
      CreateConstraintMgr(constraintmgr), CheckerMgr(checkerMgr),
      options(Options) {
  AnaCtxMgr.getCFGBuildOptions().setAllAlwaysAdd();
  AnaCtxMgr.getCFGBuildOptions().OmitImplicitValueInitializers = true;
  AnaCtxMgr.getCFGBuildOptions().AddCXXDefaultInitExprInAggregates =
      Options.ShouldIncludeDefaultInitForAggregates;
}

AnalysisManager::~AnalysisManager() {
  FlushDiagnostics();
}

void AnalysisManager::FlushDiagnostics() {
  PathDiagnosticConsumer::FilesMade filesMade;
  for (const auto &Consumer : PathConsumers) {
    Consumer->FlushDiagnostics(&filesMade);
  }
}
