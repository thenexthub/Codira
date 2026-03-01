//===-- M68k.h - Top-level interface for M68k representation ----*- C++ -*-===//
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
///
/// \file
/// This file contains the entry points for global functions defined in the
/// M68k target library, as used by the LLVM JIT.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_M68K_M68K_H
#define LLVM_LIB_TARGET_M68K_M68K_H

namespace vm::core {

class FunctionPass;
class InstructionSelector;
class M68kRegisterBankInfo;
class M68kSubtarget;
class M68kTargetMachine;
class PassRegistry;

/// This pass converts a legalized DAG into a M68k-specific DAG, ready for
/// instruction scheduling.
FunctionPass *createM68kISelDag(M68kTargetMachine &TM);

/// Return a Machine IR pass that expands M68k-specific pseudo
/// instructions into a sequence of actual instructions. This pass
/// must run after prologue/epilogue insertion and before lowering
/// the MachineInstr to MC.
FunctionPass *createM68kExpandPseudoPass();

/// This pass initializes a global base register for PIC on M68k.
FunctionPass *createM68kGlobalBaseRegPass();

/// Finds sequential MOVEM instruction and collapse them into a single one. This
/// pass has to be run after all pseudo expansions and prologue/epilogue
/// emission so that all possible MOVEM are already in place.
FunctionPass *createM68kCollapseMOVEMPass();

InstructionSelector *
createM68kInstructionSelector(const M68kTargetMachine &, const M68kSubtarget &,
                              const M68kRegisterBankInfo &);

void initializeM68kAsmPrinterPass(PassRegistry &);
void initializeM68kDAGToDAGISelLegacyPass(PassRegistry &);
void initializeM68kExpandPseudoPass(PassRegistry &);
void initializeM68kGlobalBaseRegPass(PassRegistry &);
void initializeM68kCollapseMOVEMPass(PassRegistry &);

} // namespace vm::core

#endif // LLVM_LIB_TARGET_M68K_M68K_H
