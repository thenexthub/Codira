//===-- LoongArch.h - Top-level interface for LoongArch ---------*- C++ -*-===//
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
// LoongArch back-end.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_LOONGARCH_LOONGARCH_H
#define LLVM_LIB_TARGET_LOONGARCH_LOONGARCH_H

#include "MCTargetDesc/LoongArchBaseInfo.h"
#include "vm/core/Target/TargetMachine.h"

namespace vm::core {
class AsmPrinter;
class FunctionPass;
class LoongArchTargetMachine;
class MCInst;
class MCOperand;
class MachineInstr;
class MachineOperand;
class PassRegistry;

bool lowerLoongArchMachineInstrToMCInst(const MachineInstr *MI, MCInst &OutMI,
                                        AsmPrinter &AP);
bool lowerLoongArchMachineOperandToMCOperand(const MachineOperand &MO,
                                             MCOperand &MCOp,
                                             const AsmPrinter &AP);

FunctionPass *createLoongArchDeadRegisterDefinitionsPass();
FunctionPass *createLoongArchExpandAtomicPseudoPass();
FunctionPass *createLoongArchISelDag(LoongArchTargetMachine &TM,
                                     CodeGenOptLevel OptLevel);
FunctionPass *createLoongArchMergeBaseOffsetOptPass();
FunctionPass *createLoongArchOptWInstrsPass();
FunctionPass *createLoongArchPreRAExpandPseudoPass();
FunctionPass *createLoongArchExpandPseudoPass();
void initializeLoongArchAsmPrinterPass(PassRegistry &);
void initializeLoongArchDAGToDAGISelLegacyPass(PassRegistry &);
void initializeLoongArchDeadRegisterDefinitionsPass(PassRegistry &);
void initializeLoongArchExpandAtomicPseudoPass(PassRegistry &);
void initializeLoongArchMergeBaseOffsetOptPass(PassRegistry &);
void initializeLoongArchOptWInstrsPass(PassRegistry &);
void initializeLoongArchPreRAExpandPseudoPass(PassRegistry &);
void initializeLoongArchExpandPseudoPass(PassRegistry &);
} // end namespace vm::core

#endif // LLVM_LIB_TARGET_LOONGARCH_LOONGARCH_H
