//===-- CodeGen.cpp -------------------------------------------------------===//
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
// This file implements the common initialization routines for the
// CodeGen library.
//
//===----------------------------------------------------------------------===//

#include "vm/core/InitializePasses.h"
#include "vm/core/PassRegistry.h"

using namespace vm::core;

/// initializeCodeGen - Initialize all passes linked into the CodeGen library.
void toolchain::initializeCodeGen(PassRegistry &Registry) {
  initializeAssignmentTrackingAnalysisPass(Registry);
  initializeAtomicExpandLegacyPass(Registry);
  initializeBasicBlockPathCloningPass(Registry);
  initializeBasicBlockSectionsPass(Registry);
  initializeBranchFolderLegacyPass(Registry);
  initializeBranchRelaxationLegacyPass(Registry);
  initializeBreakFalseDepsPass(Registry);
  initializeCallBrPreparePass(Registry);
  initializeCFGuardLongjmpPass(Registry);
  initializeCFIFixupPass(Registry);
  initializeCFIInstrInserterPass(Registry);
  initializeCheckDebugMachineModulePass(Registry);
  initializeCodeGenPrepareLegacyPassPass(Registry);
  initializeDeadMachineInstructionElimPass(Registry);
  initializeDebugifyMachineModulePass(Registry);
  initializeDetectDeadLanesLegacyPass(Registry);
  initializeDwarfEHPrepareLegacyPassPass(Registry);
  initializeEarlyIfConverterLegacyPass(Registry);
  initializeEarlyIfPredicatorPass(Registry);
  initializeEarlyMachineLICMPass(Registry);
  initializeEarlyTailDuplicateLegacyPass(Registry);
  initializeExpandIRInstsLegacyPassPass(Registry);
  initializeExpandMemCmpLegacyPassPass(Registry);
  initializeExpandPostRALegacyPass(Registry);
  initializeFEntryInserterLegacyPass(Registry);
  initializeFinalizeISelPass(Registry);
  initializeFixupStatepointCallerSavedLegacyPass(Registry);
  initializeFuncletLayoutPass(Registry);
  initializeGCMachineCodeAnalysisPass(Registry);
  initializeGCModuleInfoPass(Registry);
  initializeHardwareLoopsLegacyPass(Registry);
  initializeIfConverterPass(Registry);
  initializeImplicitNullChecksPass(Registry);
  initializeIndirectBrExpandLegacyPassPass(Registry);
  initializeInitUndefLegacyPass(Registry);
  initializeInterleavedLoadCombinePass(Registry);
  initializeInterleavedAccessPass(Registry);
  initializeJMCInstrumenterPass(Registry);
  initializeLibcallLoweringInfoWrapperPass(Registry);
  initializeLiveDebugValuesLegacyPass(Registry);
  initializeLiveDebugVariablesWrapperLegacyPass(Registry);
  initializeLiveIntervalsWrapperPassPass(Registry);
  initializeLiveRangeShrinkPass(Registry);
  initializeLiveStacksWrapperLegacyPass(Registry);
  initializeLiveVariablesWrapperPassPass(Registry);
  initializeLocalStackSlotPassPass(Registry);
  initializeLowerGlobalDtorsLegacyPassPass(Registry);
  initializeLowerIntrinsicsPass(Registry);
  initializeMIRAddFSDiscriminatorsPass(Registry);
  initializeMIRCanonicalizerPass(Registry);
  initializeMIRNamerPass(Registry);
  initializeMIRProfileLoaderPassPass(Registry);
  initializeMachineBlockFrequencyInfoWrapperPassPass(Registry);
  initializeMachineBlockPlacementLegacyPass(Registry);
  initializeMachineBlockPlacementStatsLegacyPass(Registry);
  initializeMachineCFGPrinterPass(Registry);
  initializeMachineCSELegacyPass(Registry);
  initializeMachineCombinerPass(Registry);
  initializeMachineCopyPropagationLegacyPass(Registry);
  initializeMachineCycleInfoPrinterLegacyPass(Registry);
  initializeMachineCycleInfoWrapperPassPass(Registry);
  initializeMachineDominatorTreeWrapperPassPass(Registry);
  initializeMachineFunctionPrinterPassPass(Registry);
  initializeMachineFunctionSplitterPass(Registry);
  initializeMachineLateInstrsCleanupLegacyPass(Registry);
  initializeMachineLICMPass(Registry);
  initializeMachineLoopInfoWrapperPassPass(Registry);
  initializeMachineModuleInfoWrapperPassPass(Registry);
  initializeMachineOptimizationRemarkEmitterPassPass(Registry);
  initializeMachineOutlinerPass(Registry);
  initializeMachinePipelinerPass(Registry);
  initializeMachineSanitizerBinaryMetadataLegacyPass(Registry);
  initializeModuloScheduleTestPass(Registry);
  initializeMachinePostDominatorTreeWrapperPassPass(Registry);
  initializeMachineRegionInfoPassPass(Registry);
  initializeMachineSchedulerLegacyPass(Registry);
  initializeMachineSinkingLegacyPass(Registry);
  initializeMachineUniformityAnalysisPassPass(Registry);
  initializeMIR2VecVocabLegacyAnalysisPass(Registry);
  initializeMIR2VecVocabPrinterLegacyPassPass(Registry);
  initializeMIR2VecPrinterLegacyPassPass(Registry);
  initializeMachineUniformityInfoPrinterPassPass(Registry);
  initializeMachineVerifierLegacyPassPass(Registry);
  initializeObjCARCContractLegacyPassPass(Registry);
  initializeOptimizePHIsLegacyPass(Registry);
  initializePEILegacyPass(Registry);
  initializePHIEliminationPass(Registry);
  initializePatchableFunctionLegacyPass(Registry);
  initializePeepholeOptimizerLegacyPass(Registry);
  initializePostMachineSchedulerLegacyPass(Registry);
  initializePostRAMachineSinkingLegacyPass(Registry);
  initializePostRAHazardRecognizerLegacyPass(Registry);
  initializePostRASchedulerLegacyPass(Registry);
  initializePreISelIntrinsicLoweringLegacyPassPass(Registry);
  initializeProcessImplicitDefsLegacyPass(Registry);
  initializeRABasicPass(Registry);
  initializeRAGreedyLegacyPass(Registry);
  initializeReachingDefInfoWrapperPassPass(Registry);
  initializeRegAllocFastPass(Registry);
  initializeRegUsageInfoCollectorLegacyPass(Registry);
  initializeRegUsageInfoPropagationLegacyPass(Registry);
  initializeRegisterCoalescerLegacyPass(Registry);
  initializeRemoveLoadsIntoFakeUsesLegacyPass(Registry);
  initializeRemoveRedundantDebugValuesLegacyPass(Registry);
  initializeRenameIndependentSubregsLegacyPass(Registry);
  initializeSafeStackLegacyPassPass(Registry);
  initializeSelectOptimizePass(Registry);
  initializeShadowStackGCLoweringPass(Registry);
  initializeShrinkWrapLegacyPass(Registry);
  initializeSjLjEHPreparePass(Registry);
  initializeSlotIndexesWrapperPassPass(Registry);
  initializeStackColoringLegacyPass(Registry);
  initializeStackFrameLayoutAnalysisLegacyPass(Registry);
  initializeStackMapLivenessPass(Registry);
  initializeStackProtectorPass(Registry);
  initializeStackSlotColoringLegacyPass(Registry);
  initializeStaticDataSplitterPass(Registry);
  initializeStaticDataAnnotatorPass(Registry);
  initializeStripDebugMachineModulePass(Registry);
  initializeTailDuplicateLegacyPass(Registry);
  initializeTargetPassConfigPass(Registry);
  initializeTwoAddressInstructionLegacyPassPass(Registry);
  initializeTypePromotionLegacyPass(Registry);
  initializeUnpackMachineBundlesPass(Registry);
  initializeUnreachableBlockElimLegacyPassPass(Registry);
  initializeUnreachableMachineBlockElimLegacyPass(Registry);
  initializeVirtRegMapWrapperLegacyPass(Registry);
  initializeVirtRegRewriterLegacyPass(Registry);
  initializeWasmEHPreparePass(Registry);
  initializeWinEHPreparePass(Registry);
  initializeXRayInstrumentationLegacyPass(Registry);
}
