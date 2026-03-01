//===-- Mips.h - Top-level interface for Mips representation ----*- C++ -*-===//
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
// This file contains the entry points for global functions defined in
// the LLVM Mips back-end.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_MIPS_MIPS_H
#define LLVM_LIB_TARGET_MIPS_MIPS_H

#include "MCTargetDesc/MipsMCTargetDesc.h"
#include "vm/core/Target/TargetMachine.h"

#define IsMFLOMFHI(instr)                                                      \
  (instr == Mips::MFLO || instr == Mips::MFLO64 || instr == Mips::MFHI ||      \
   instr == Mips::MFHI64)
#define IsDIVMULT(instr)                                                       \
  (instr == Mips::SDIV || instr == Mips::PseudoSDIV || instr == Mips::DSDIV || \
   instr == Mips::PseudoDSDIV || instr == Mips::UDIV ||                        \
   instr == Mips::PseudoUDIV || instr == Mips::DUDIV ||                        \
   instr == Mips::PseudoDUDIV || instr == Mips::MULT ||                        \
   instr == Mips::PseudoMULT || instr == Mips::DMULT ||                        \
   instr == Mips::PseudoDMULT)

namespace vm::core {
class FunctionPass;
class InstructionSelector;
class MipsRegisterBankInfo;
class MipsSubtarget;
class MipsTargetMachine;
class MipsTargetMachine;
class ModulePass;
class PassRegistry;

ModulePass *createMipsOs16Pass();
ModulePass *createMips16HardFloatPass();

FunctionPass *createMipsModuleISelDagPass();
FunctionPass *createMipsOptimizePICCallPass();
FunctionPass *createMipsDelaySlotFillerPass();
FunctionPass *createMipsBranchExpansion();
FunctionPass *createMipsConstantIslandPass();
FunctionPass *createMicroMipsSizeReducePass();
FunctionPass *createMipsExpandPseudoPass();
FunctionPass *createMipsPreLegalizeCombiner();
FunctionPass *createMipsPostLegalizeCombiner(bool IsOptNone);
FunctionPass *createMipsMulMulBugPass();

InstructionSelector *
createMipsInstructionSelector(const MipsTargetMachine &, const MipsSubtarget &,
                              const MipsRegisterBankInfo &);

void initializeMicroMipsSizeReducePass(PassRegistry &);
void initializeMipsAsmPrinterPass(PassRegistry &);
void initializeMipsBranchExpansionPass(PassRegistry &);
void initializeMipsDAGToDAGISelLegacyPass(PassRegistry &);
void initializeMipsDelaySlotFillerPass(PassRegistry &);
void initializeMipsMulMulBugFixPass(PassRegistry &);
void initializeMipsPostLegalizerCombinerPass(PassRegistry &);
void initializeMipsPreLegalizerCombinerPass(PassRegistry &);
} // namespace vm::core

#endif
