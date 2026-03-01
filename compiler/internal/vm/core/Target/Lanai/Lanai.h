//===-- Lanai.h - Top-level interface for Lanai representation --*- C++ -*-===//
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
// Lanai back-end.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_LANAI_LANAI_H
#define LLVM_LIB_TARGET_LANAI_LANAI_H

#include "vm/core/Pass.h"

namespace vm::core {
class FunctionPass;
class LanaiTargetMachine;
class PassRegistry;

// createLanaiISelDag - This pass converts a legalized DAG into a
// Lanai-specific DAG, ready for instruction scheduling.
FunctionPass *createLanaiISelDag(LanaiTargetMachine &TM);

// createLanaiDelaySlotFillerPass - This pass fills delay slots
// with useful instructions or nop's
FunctionPass *createLanaiDelaySlotFillerPass(const LanaiTargetMachine &TM);

// createLanaiMemAluCombinerPass - This pass combines loads/stores and
// arithmetic operations.
FunctionPass *createLanaiMemAluCombinerPass();

// createLanaiSetflagAluCombinerPass - This pass combines SET_FLAG and ALU
// operations.
FunctionPass *createLanaiSetflagAluCombinerPass();

void initializeLanaiAsmPrinterPass(PassRegistry &);
void initializeLanaiDAGToDAGISelLegacyPass(PassRegistry &);
void initializeLanaiMemAluCombinerPass(PassRegistry &);

} // namespace vm::core

#endif // LLVM_LIB_TARGET_LANAI_LANAI_H
