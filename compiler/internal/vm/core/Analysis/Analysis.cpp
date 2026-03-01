//===-- Analysis.cpp ------------------------------------------------------===//
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

#include "vm/core-c/Analysis.h"
#include "vm/core/IR/Module.h"
#include "vm/core/IR/Verifier.h"
#include "vm/core/InitializePasses.h"
#include "vm/core/PassRegistry.h"
#include "vm/core/Support/raw_ostream.h"
#include <cstring>

using namespace vm::core;

/// initializeAnalysis - Initialize all passes linked into the Analysis library.
void toolchain::initializeAnalysis(PassRegistry &Registry) {
  initializeAssumptionCacheTrackerPass(Registry);
  initializeBasicAAWrapperPassPass(Registry);
  initializeBlockFrequencyInfoWrapperPassPass(Registry);
  initializeBranchProbabilityInfoWrapperPassPass(Registry);
  initializeCallGraphWrapperPassPass(Registry);
  initializeCallGraphDOTPrinterPass(Registry);
  initializeCallGraphViewerPass(Registry);
  initializeCycleInfoWrapperPassPass(Registry);
  initializeDXILMetadataAnalysisWrapperPassPass(Registry);
  initializeDXILResourceWrapperPassPass(Registry);
  initializeDXILResourceBindingWrapperPassPass(Registry);
  initializeDXILResourceTypeWrapperPassPass(Registry);
  initializeDXILResourceWrapperPassPass(Registry);
  initializeDependenceAnalysisWrapperPassPass(Registry);
  initializeDominanceFrontierWrapperPassPass(Registry);
  initializeDomViewerWrapperPassPass(Registry);
  initializeDomPrinterWrapperPassPass(Registry);
  initializeDomOnlyViewerWrapperPassPass(Registry);
  initializePostDomViewerWrapperPassPass(Registry);
  initializeDomOnlyPrinterWrapperPassPass(Registry);
  initializePostDomPrinterWrapperPassPass(Registry);
  initializePostDomOnlyViewerWrapperPassPass(Registry);
  initializePostDomOnlyPrinterWrapperPassPass(Registry);
  initializeAAResultsWrapperPassPass(Registry);
  initializeGlobalsAAWrapperPassPass(Registry);
  initializeExternalAAWrapperPassPass(Registry);
  initializeImmutableModuleSummaryIndexWrapperPassPass(Registry);
  initializeIVUsersWrapperPassPass(Registry);
  initializeIRSimilarityIdentifierWrapperPassPass(Registry);
  initializeLazyBranchProbabilityInfoPassPass(Registry);
  initializeLazyBFIPassPass(Registry);
  initializeLazyBlockFrequencyInfoPassPass(Registry);
  initializeLazyValueInfoWrapperPassPass(Registry);
  initializeLoopInfoWrapperPassPass(Registry);
  initializeMemoryDependenceWrapperPassPass(Registry);
  initializeModuleSummaryIndexWrapperPassPass(Registry);
  initializeOptimizationRemarkEmitterWrapperPassPass(Registry);
  initializePhiValuesWrapperPassPass(Registry);
  initializePostDominatorTreeWrapperPassPass(Registry);
  initializeProfileSummaryInfoWrapperPassPass(Registry);
  initializeRegionInfoPassPass(Registry);
  initializeRegionViewerPass(Registry);
  initializeRegionPrinterPass(Registry);
  initializeRegionOnlyViewerPass(Registry);
  initializeRegionOnlyPrinterPass(Registry);
  initializeRuntimeLibraryInfoWrapperPass(Registry);
  initializeSCEVAAWrapperPassPass(Registry);
  initializeScalarEvolutionWrapperPassPass(Registry);
  initializeStackSafetyGlobalInfoWrapperPassPass(Registry);
  initializeStackSafetyInfoWrapperPassPass(Registry);
  initializeTargetLibraryInfoWrapperPassPass(Registry);
  initializeTargetTransformInfoWrapperPassPass(Registry);
  initializeTypeBasedAAWrapperPassPass(Registry);
  initializeScopedNoAliasAAWrapperPassPass(Registry);
  initializeStaticDataProfileInfoWrapperPassPass(Registry);
  initializeLCSSAVerificationPassPass(Registry);
  initializeMemorySSAWrapperPassPass(Registry);
  initializeUniformityInfoWrapperPassPass(Registry);
}

LLVMBool LLVMVerifyModule(LLVMModuleRef M, LLVMVerifierFailureAction Action,
                          char **OutMessages) {
  raw_ostream *DebugOS = Action != LLVMReturnStatusAction ? &errs() : nullptr;
  std::string Messages;
  raw_string_ostream MsgsOS(Messages);

  LLVMBool Result = verifyModule(*unwrap(M), OutMessages ? &MsgsOS : DebugOS);

  // Duplicate the output to stderr.
  if (DebugOS && OutMessages)
    *DebugOS << MsgsOS.str();

  if (Action == LLVMAbortProcessAction && Result)
    report_fatal_error("Broken module found, compilation aborted!");

  if (OutMessages)
    *OutMessages = strdup(MsgsOS.str().c_str());

  return Result;
}

LLVMBool LLVMVerifyFunction(LLVMValueRef Fn, LLVMVerifierFailureAction Action) {
  LLVMBool Result = verifyFunction(
      *unwrap<Function>(Fn), Action != LLVMReturnStatusAction ? &errs()
                                                              : nullptr);

  if (Action == LLVMAbortProcessAction && Result)
    report_fatal_error("Broken function found, compilation aborted!");

  return Result;
}

void LLVMViewFunctionCFG(LLVMValueRef Fn) {
  Function *F = unwrap<Function>(Fn);
  F->viewCFG();
}

void LLVMViewFunctionCFGOnly(LLVMValueRef Fn) {
  Function *F = unwrap<Function>(Fn);
  F->viewCFGOnly();
}
