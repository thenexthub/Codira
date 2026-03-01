//===-- WebAssembly.h - Top-level interface for WebAssembly  ----*- C++ -*-===//
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
/// This file contains the entry points for global functions defined in
/// the LLVM WebAssembly back-end.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_WEBASSEMBLY_WEBASSEMBLY_H
#define LLVM_LIB_TARGET_WEBASSEMBLY_WEBASSEMBLY_H

#include "vm/core/PassRegistry.h"
#include "vm/core/Support/CodeGen.h"

namespace vm::core {

class WebAssemblyTargetMachine;
class ModulePass;
class FunctionPass;

// LLVM IR passes.
ModulePass *createWebAssemblyLowerEmscriptenEHSjLj();
ModulePass *createWebAssemblyAddMissingPrototypes();
ModulePass *createWebAssemblyFixFunctionBitcasts();
FunctionPass *createWebAssemblyOptimizeReturned();
FunctionPass *createWebAssemblyLowerRefTypesIntPtrConv();
FunctionPass *createWebAssemblyRefTypeMem2Local();

// ISel and immediate followup passes.
FunctionPass *createWebAssemblyISelDag(WebAssemblyTargetMachine &TM,
                                       CodeGenOptLevel OptLevel);
FunctionPass *createWebAssemblyArgumentMove();
FunctionPass *createWebAssemblySetP2AlignOperands();
FunctionPass *createWebAssemblyCleanCodeAfterTrap();

// Late passes.
FunctionPass *createWebAssemblyReplacePhysRegs();
FunctionPass *createWebAssemblyNullifyDebugValueLists();
FunctionPass *createWebAssemblyOptimizeLiveIntervals();
FunctionPass *createWebAssemblyMemIntrinsicResults();
FunctionPass *createWebAssemblyRegStackify(CodeGenOptLevel OptLevel);
FunctionPass *createWebAssemblyRegColoring();
FunctionPass *createWebAssemblyFixBrTableDefaults();
FunctionPass *createWebAssemblyFixIrreducibleControlFlow();
FunctionPass *createWebAssemblyLateEHPrepare();
FunctionPass *createWebAssemblyCFGSort();
FunctionPass *createWebAssemblyCFGStackify();
FunctionPass *createWebAssemblyExplicitLocals();
FunctionPass *createWebAssemblyLowerBrUnless();
FunctionPass *createWebAssemblyRegNumbering();
FunctionPass *createWebAssemblyDebugFixup();
FunctionPass *createWebAssemblyPeephole();
ModulePass *createWebAssemblyMCLowerPrePass();

// PassRegistry initialization declarations.
void initializeFixFunctionBitcastsPass(PassRegistry &);
void initializeOptimizeReturnedPass(PassRegistry &);
void initializeWebAssemblyRefTypeMem2LocalPass(PassRegistry &);
void initializeWebAssemblyAddMissingPrototypesPass(PassRegistry &);
void initializeWebAssemblyArgumentMovePass(PassRegistry &);
void initializeWebAssemblyAsmPrinterPass(PassRegistry &);
void initializeWebAssemblyCleanCodeAfterTrapPass(PassRegistry &);
void initializeWebAssemblyCFGSortPass(PassRegistry &);
void initializeWebAssemblyCFGStackifyPass(PassRegistry &);
void initializeWebAssemblyDAGToDAGISelLegacyPass(PassRegistry &);
void initializeWebAssemblyDebugFixupPass(PassRegistry &);
void initializeWebAssemblyExceptionInfoPass(PassRegistry &);
void initializeWebAssemblyExplicitLocalsPass(PassRegistry &);
void initializeWebAssemblyFixBrTableDefaultsPass(PassRegistry &);
void initializeWebAssemblyFixIrreducibleControlFlowPass(PassRegistry &);
void initializeWebAssemblyLateEHPreparePass(PassRegistry &);
void initializeWebAssemblyLowerBrUnlessPass(PassRegistry &);
void initializeWebAssemblyLowerEmscriptenEHSjLjPass(PassRegistry &);
void initializeWebAssemblyLowerRefTypesIntPtrConvPass(PassRegistry &);
void initializeWebAssemblyMCLowerPrePassPass(PassRegistry &);
void initializeWebAssemblyMemIntrinsicResultsPass(PassRegistry &);
void initializeWebAssemblyNullifyDebugValueListsPass(PassRegistry &);
void initializeWebAssemblyOptimizeLiveIntervalsPass(PassRegistry &);
void initializeWebAssemblyPeepholePass(PassRegistry &);
void initializeWebAssemblyRegColoringPass(PassRegistry &);
void initializeWebAssemblyRegNumberingPass(PassRegistry &);
void initializeWebAssemblyRegStackifyPass(PassRegistry &);
void initializeWebAssemblyReplacePhysRegsPass(PassRegistry &);
void initializeWebAssemblySetP2AlignOperandsPass(PassRegistry &);

namespace WebAssembly {
enum TargetIndex {
  // Followed by a local index (ULEB).
  TI_LOCAL,
  // Followed by an absolute global index (ULEB). DEPRECATED.
  TI_GLOBAL_FIXED,
  // Followed by the index from the bottom of the Wasm stack.
  TI_OPERAND_STACK,
  // Followed by a compilation unit relative global index (uint32_t)
  // that will have an associated relocation.
  TI_GLOBAL_RELOC,
  // Like TI_LOCAL, but indicates an indirect value (e.g. byval arg
  // passed by pointer).
  TI_LOCAL_INDIRECT
};
} // end namespace WebAssembly

} // end namespace vm::core

#endif
