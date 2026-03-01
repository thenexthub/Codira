//===-- RISCV.h - Top-level interface for RISC-V ----------------*- C++ -*-===//
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
// RISC-V back-end.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_RISCV_RISCV_H
#define LLVM_LIB_TARGET_RISCV_RISCV_H

#include "MCTargetDesc/RISCVBaseInfo.h"
#include "vm/core/Target/TargetMachine.h"

namespace vm::core {
class FunctionPass;
class InstructionSelector;
class ModulePass;
class PassRegistry;
class RISCVRegisterBankInfo;
class RISCVSubtarget;
class RISCVTargetMachine;

class RISCVCodeGenPreparePass : public PassInfoMixin<RISCVCodeGenPreparePass> {
private:
  const RISCVTargetMachine *TM;

public:
  RISCVCodeGenPreparePass(const RISCVTargetMachine *TM) : TM(TM) {}
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &FAM);
};
FunctionPass *createRISCVCodeGenPrepareLegacyPass();
void initializeRISCVCodeGenPrepareLegacyPassPass(PassRegistry &);

FunctionPass *createRISCVDeadRegisterDefinitionsPass();
void initializeRISCVDeadRegisterDefinitionsPass(PassRegistry &);

FunctionPass *createRISCVIndirectBranchTrackingPass();
void initializeRISCVIndirectBranchTrackingPass(PassRegistry &);

FunctionPass *createRISCVLandingPadSetupPass();
void initializeRISCVLandingPadSetupPass(PassRegistry &);

FunctionPass *createRISCVISelDag(RISCVTargetMachine &TM,
                                 CodeGenOptLevel OptLevel);

FunctionPass *createRISCVLateBranchOptPass();
void initializeRISCVLateBranchOptPass(PassRegistry &);

FunctionPass *createRISCVMakeCompressibleOptPass();
void initializeRISCVMakeCompressibleOptPass(PassRegistry &);

FunctionPass *createRISCVGatherScatterLoweringPass();
void initializeRISCVGatherScatterLoweringPass(PassRegistry &);

FunctionPass *createRISCVVectorPeepholePass();
void initializeRISCVVectorPeepholePass(PassRegistry &);

FunctionPass *createRISCVOptWInstrsPass();
void initializeRISCVOptWInstrsPass(PassRegistry &);

FunctionPass *createRISCVFoldMemOffsetPass();
void initializeRISCVFoldMemOffsetPass(PassRegistry &);

FunctionPass *createRISCVMergeBaseOffsetOptPass();
void initializeRISCVMergeBaseOffsetOptPass(PassRegistry &);

FunctionPass *createRISCVExpandPseudoPass();
void initializeRISCVExpandPseudoPass(PassRegistry &);

FunctionPass *createRISCVPreRAExpandPseudoPass();
void initializeRISCVPreRAExpandPseudoPass(PassRegistry &);

FunctionPass *createRISCVExpandAtomicPseudoPass();
void initializeRISCVExpandAtomicPseudoPass(PassRegistry &);

FunctionPass *createRISCVInsertVSETVLIPass();
void initializeRISCVInsertVSETVLIPass(PassRegistry &);
extern char &RISCVInsertVSETVLIID;

FunctionPass *createRISCVPostRAExpandPseudoPass();
void initializeRISCVPostRAExpandPseudoPass(PassRegistry &);
FunctionPass *createRISCVInsertReadWriteCSRPass();
void initializeRISCVInsertReadWriteCSRPass(PassRegistry &);

FunctionPass *createRISCVInsertWriteVXRMPass();
void initializeRISCVInsertWriteVXRMPass(PassRegistry &);

FunctionPass *createRISCVRedundantCopyEliminationPass();
void initializeRISCVRedundantCopyEliminationPass(PassRegistry &);

FunctionPass *createRISCVMoveMergePass();
void initializeRISCVMoveMergePass(PassRegistry &);

FunctionPass *createRISCVPushPopOptimizationPass();
void initializeRISCVPushPopOptPass(PassRegistry &);
FunctionPass *createRISCVLoadStoreOptPass();
void initializeRISCVLoadStoreOptPass(PassRegistry &);

FunctionPass *createRISCVPreAllocZilsdOptPass();
void initializeRISCVPreAllocZilsdOptPass(PassRegistry &);

FunctionPass *createRISCVZacasABIFixPass();
void initializeRISCVZacasABIFixPass(PassRegistry &);

InstructionSelector *
createRISCVInstructionSelector(const RISCVTargetMachine &,
                               const RISCVSubtarget &,
                               const RISCVRegisterBankInfo &);
void initializeRISCVDAGToDAGISelLegacyPass(PassRegistry &);

FunctionPass *createRISCVPostLegalizerCombiner();
void initializeRISCVPostLegalizerCombinerPass(PassRegistry &);

FunctionPass *createRISCVO0PreLegalizerCombiner();
void initializeRISCVO0PreLegalizerCombinerPass(PassRegistry &);

FunctionPass *createRISCVPreLegalizerCombiner();
void initializeRISCVPreLegalizerCombinerPass(PassRegistry &);

ModulePass *createRISCVPromoteConstantPass();
void initializeRISCVPromoteConstantPass(PassRegistry &);

FunctionPass *createRISCVVLOptimizerPass();
void initializeRISCVVLOptimizerPass(PassRegistry &);

FunctionPass *createRISCVVMV0EliminationPass();
void initializeRISCVVMV0EliminationPass(PassRegistry &);

void initializeRISCVAsmPrinterPass(PassRegistry &);
} // namespace vm::core

#endif
