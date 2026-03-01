//===-- SPIRV.h - Top-level interface for SPIR-V representation -*- C++ -*-===//
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

#ifndef LLVM_LIB_TARGET_SPIRV_SPIRV_H
#define LLVM_LIB_TARGET_SPIRV_SPIRV_H

#include "MCTargetDesc/SPIRVMCTargetDesc.h"
#include "vm/core/CodeGen/MachineFunctionPass.h"
#include "vm/core/Target/TargetMachine.h"

namespace vm::core {
class SPIRVTargetMachine;
class SPIRVSubtarget;
class InstructionSelector;
class RegisterBankInfo;

ModulePass *createSPIRVPrepareFunctionsPass(const SPIRVTargetMachine &TM);
FunctionPass *createSPIRVStructurizerPass();
ModulePass *createSPIRVCBufferAccessLegacyPass();
ModulePass *createSPIRVPushConstantAccessLegacyPass(SPIRVTargetMachine *TM);
FunctionPass *createSPIRVMergeRegionExitTargetsPass();
FunctionPass *createSPIRVStripConvergenceIntrinsicsPass();
ModulePass *createSPIRVLegalizeImplicitBindingPass();
ModulePass *createSPIRVLegalizeZeroSizeArraysPass(const SPIRVTargetMachine &TM);
FunctionPass *createSPIRVLegalizePointerCastPass(SPIRVTargetMachine *TM);
FunctionPass *createSPIRVRegularizerPass();
FunctionPass *createSPIRVPreLegalizerCombiner();
FunctionPass *createSPIRVPreLegalizerPass();
FunctionPass *createSPIRVPostLegalizerPass();
ModulePass *createSPIRVEmitIntrinsicsPass(SPIRVTargetMachine *TM);
ModulePass *createSPIRVPrepareGlobalsPass();
MachineFunctionPass *createSPIRVEmitNonSemanticDIPass(SPIRVTargetMachine *TM);
InstructionSelector *
createSPIRVInstructionSelector(const SPIRVTargetMachine &TM,
                               const SPIRVSubtarget &Subtarget,
                               const RegisterBankInfo &RBI);

void initializeSPIRVModuleAnalysisPass(PassRegistry &);
void initializeSPIRVAsmPrinterPass(PassRegistry &);
void initializeSPIRVConvergenceRegionAnalysisWrapperPassPass(PassRegistry &);
void initializeSPIRVPreLegalizerPass(PassRegistry &);
void initializeSPIRVPreLegalizerCombinerPass(PassRegistry &);
void initializeSPIRVPostLegalizerPass(PassRegistry &);
void initializeSPIRVStructurizerPass(PassRegistry &);
void initializeSPIRVCBufferAccessLegacyPass(PassRegistry &);
void initializeSPIRVPushConstantAccessLegacyPass(PassRegistry &);
void initializeSPIRVEmitIntrinsicsPass(PassRegistry &);
void initializeSPIRVEmitNonSemanticDIPass(PassRegistry &);
void initializeSPIRVLegalizePointerCastPass(PassRegistry &);
void initializeSPIRVRegularizerPass(PassRegistry &);
void initializeSPIRVMergeRegionExitTargetsPass(PassRegistry &);
void initializeSPIRVPrepareFunctionsPass(PassRegistry &);
void initializeSPIRVPrepareGlobalsPass(PassRegistry &);
void initializeSPIRVStripConvergentIntrinsicsPass(PassRegistry &);
void initializeSPIRVLegalizeImplicitBindingPass(PassRegistry &);
void initializeSPIRVLegalizeZeroSizeArraysLegacyPass(PassRegistry &);
} // namespace vm::core

#endif // LLVM_LIB_TARGET_SPIRV_SPIRV_H
