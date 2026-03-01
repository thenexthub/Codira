//=-- Hexagon.h - Top-level interface for Hexagon representation --*- C++ -*-=//
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
// Hexagon back-end.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_HEXAGON_HEXAGON_H
#define LLVM_LIB_TARGET_HEXAGON_HEXAGON_H

#include "vm/core/Support/CodeGen.h"

namespace vm::core {
class HexagonTargetMachine;
class ImmutablePass;
class PassRegistry;
class FunctionPass;
class Pass;

extern char &HexagonCopyHoistingID;
extern char &HexagonExpandCondsetsID;
extern char &HexagonTfrCleanupID;
void initializeHexagonAsmPrinterPass(PassRegistry &);
void initializeHexagonBitSimplifyPass(PassRegistry &);
void initializeHexagonBranchRelaxationPass(PassRegistry &);
void initializeHexagonCFGOptimizerPass(PassRegistry &);
void initializeHexagonCommonGEPPass(PassRegistry &);
void initializeHexagonCopyHoistingPass(PassRegistry &);
void initializeHexagonConstExtendersPass(PassRegistry &);
void initializeHexagonConstPropagationPass(PassRegistry &);
void initializeHexagonCopyToCombinePass(PassRegistry &);
void initializeHexagonDAGToDAGISelLegacyPass(PassRegistry &);
void initializeHexagonEarlyIfConversionPass(PassRegistry &);
void initializeHexagonExpandCondsetsPass(PassRegistry &);
void initializeHexagonGenMemAbsolutePass(PassRegistry &);
void initializeHexagonGenMuxPass(PassRegistry &);
void initializeHexagonHardwareLoopsPass(PassRegistry &);
void initializeHexagonLoopIdiomRecognizeLegacyPassPass(PassRegistry &);
void initializeHexagonLoopAlignPass(PassRegistry &);
void initializeHexagonLoopReschedulingPass(PassRegistry &);
void initializeHexagonMaskPass(PassRegistry &);
void initializeHexagonMergeActivateWeightPass(PassRegistry &);
void initializeHexagonNewValueJumpPass(PassRegistry &);
void initializeHexagonOptAddrModePass(PassRegistry &);
void initializeHexagonPacketizerPass(PassRegistry &);
void initializeHexagonRDFOptPass(PassRegistry &);
void initializeHexagonSplitDoubleRegsPass(PassRegistry &);
void initializeHexagonTfrCleanupPass(PassRegistry &);
void initializeHexagonVExtractPass(PassRegistry &);
void initializeHexagonVectorCombineLegacyPass(PassRegistry &);
void initializeHexagonVectorLoopCarriedReuseLegacyPassPass(PassRegistry &);
void initializeHexagonFixupHwLoopsPass(PassRegistry &);
void initializeHexagonCallFrameInformationPass(PassRegistry &);
void initializeHexagonGenExtractPass(PassRegistry &);
void initializeHexagonGenInsertPass(PassRegistry &);
void initializeHexagonGenPredicatePass(PassRegistry &);
void initializeHexagonLoadWideningPass(PassRegistry &);
void initializeHexagonStoreWideningPass(PassRegistry &);
void initializeHexagonOptimizeSZextendsPass(PassRegistry &);
void initializeHexagonPeepholePass(PassRegistry &);
void initializeHexagonSplitConst32AndConst64Pass(PassRegistry &);
void initializeHexagonVectorPrintPass(PassRegistry &);

void initializeHexagonQFPOptimizerPass(PassRegistry &);

Pass *createHexagonLoopIdiomPass();
Pass *createHexagonVectorLoopCarriedReuseLegacyPass();

/// Creates a Hexagon-specific Target Transformation Info pass.
ImmutablePass *
createHexagonTargetTransformInfoPass(const HexagonTargetMachine *TM);

FunctionPass *createHexagonBitSimplify();
FunctionPass *createHexagonBranchRelaxation();
FunctionPass *createHexagonCallFrameInformation();
FunctionPass *createHexagonCFGOptimizer();
FunctionPass *createHexagonCommonGEP();
FunctionPass *createHexagonConstExtenders();
FunctionPass *createHexagonConstPropagationPass();
FunctionPass *createHexagonCopyHoisting();
FunctionPass *createHexagonCopyToCombine();
FunctionPass *createHexagonEarlyIfConversion();
FunctionPass *createHexagonFixupHwLoops();
FunctionPass *createHexagonGenExtract();
FunctionPass *createHexagonGenInsert();
FunctionPass *createHexagonGenMemAbsolute();
FunctionPass *createHexagonGenMux();
FunctionPass *createHexagonGenPredicate();
FunctionPass *createHexagonHardwareLoops();
FunctionPass *createHexagonISelDag(HexagonTargetMachine &TM,
                                   CodeGenOptLevel OptLevel);
FunctionPass *createHexagonLoopAlign();
FunctionPass *createHexagonLoopRescheduling();
FunctionPass *createHexagonMask();
FunctionPass *createHexagonMergeActivateWeight();
FunctionPass *createHexagonNewValueJump();
FunctionPass *createHexagonOptAddrMode();
FunctionPass *createHexagonOptimizeSZextends();
FunctionPass *createHexagonPacketizer(bool Minimal);
FunctionPass *createHexagonPeephole();
FunctionPass *createHexagonRDFOpt();
FunctionPass *createHexagonSplitConst32AndConst64();
FunctionPass *createHexagonSplitDoubleRegs();
FunctionPass *createHexagonStoreWidening();
FunctionPass *createHexagonLoadWidening();
FunctionPass *createHexagonTfrCleanup();
FunctionPass *createHexagonVectorCombineLegacyPass();
FunctionPass *createHexagonVectorPrint();
FunctionPass *createHexagonVExtract();
FunctionPass *createHexagonExpandCondsets();
FunctionPass *createHexagonQFPOptimizer();

} // end namespace vm::core;

#endif
