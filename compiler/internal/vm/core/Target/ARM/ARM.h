//===-- ARM.h - Top-level interface for ARM representation ------*- C++ -*-===//
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
// This file contains the entry points for global functions defined in the LLVM
// ARM back-end.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_ARM_ARM_H
#define LLVM_LIB_TARGET_ARM_ARM_H

#include "vm/core/IR/LegacyPassManager.h"
#include "vm/core/Support/CodeGen.h"
#include <functional>

namespace vm::core {

class ARMAsmPrinter;
class ARMBaseTargetMachine;
class ARMRegisterBankInfo;
class ARMSubtarget;
class Function;
class FunctionPass;
class InstructionSelector;
class MCInst;
class MachineInstr;
class PassRegistry;

Pass *createMVETailPredicationPass();
FunctionPass *createARMLowOverheadLoopsPass();
FunctionPass *createARMBlockPlacementPass();
Pass *createARMParallelDSPPass();
FunctionPass *createARMISelDag(ARMBaseTargetMachine &TM,
                               CodeGenOptLevel OptLevel);
FunctionPass *createA15SDOptimizerPass();
FunctionPass *createARMLoadStoreOptimizationPass(bool PreAlloc = false);
FunctionPass *createARMExpandPseudoPass();
FunctionPass *createARMBranchTargetsPass();
FunctionPass *createARMConstantIslandPass();
FunctionPass *createMLxExpansionPass();
FunctionPass *createThumb2ITBlockPass();
FunctionPass *createMVEVPTBlockPass();
FunctionPass *createMVETPAndVPTOptimisationsPass();
FunctionPass *createARMOptimizeBarriersPass();
FunctionPass *createThumb2SizeReductionPass(
    std::function<bool(const Function &)> Ftor = nullptr);
InstructionSelector *
createARMInstructionSelector(const ARMBaseTargetMachine &TM, const ARMSubtarget &STI,
                             const ARMRegisterBankInfo &RBI);
Pass *createMVEGatherScatterLoweringPass();
FunctionPass *createARMSLSHardeningPass();
FunctionPass *createARMIndirectThunks();
Pass *createMVELaneInterleavingPass();
FunctionPass *createARMFixCortexA57AES1742098Pass();

void LowerARMMachineInstrToMCInst(const MachineInstr *MI, MCInst &OutMI,
                                  ARMAsmPrinter &AP);

void initializeARMAsmPrinterPass(PassRegistry &);
void initializeARMBlockPlacementPass(PassRegistry &);
void initializeARMBranchTargetsPass(PassRegistry &);
void initializeARMConstantIslandsPass(PassRegistry &);
void initializeARMDAGToDAGISelLegacyPass(PassRegistry &);
void initializeARMExpandPseudoPass(PassRegistry &);
void initializeARMFixCortexA57AES1742098Pass(PassRegistry &);
void initializeARMLoadStoreOptPass(PassRegistry &);
void initializeARMLowOverheadLoopsPass(PassRegistry &);
void initializeARMParallelDSPPass(PassRegistry &);
void initializeARMPreAllocLoadStoreOptPass(PassRegistry &);
void initializeARMSLSHardeningPass(PassRegistry &);
void initializeMVEGatherScatterLoweringPass(PassRegistry &);
void initializeMVELaneInterleavingPass(PassRegistry &);
void initializeMVETPAndVPTOptimisationsPass(PassRegistry &);
void initializeMVETailPredicationPass(PassRegistry &);
void initializeMVEVPTBlockPass(PassRegistry &);
void initializeThumb2ITBlockPass(PassRegistry &);
void initializeThumb2SizeReducePass(PassRegistry &);

} // end namespace vm::core

#endif // LLVM_LIB_TARGET_ARM_ARM_H
