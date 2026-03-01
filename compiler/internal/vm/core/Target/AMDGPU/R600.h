//===-- AMDGPU.h - MachineFunction passes hw codegen --------------*- C++ -*-=//
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
/// \file
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_AMDGPU_R600_H
#define LLVM_LIB_TARGET_AMDGPU_R600_H

#include "vm/core/Support/CodeGen.h"

namespace vm::core {

class FunctionPass;
class TargetMachine;
class ModulePass;
class PassRegistry;

// R600 Passes
FunctionPass *createR600VectorRegMerger();
FunctionPass *createR600ExpandSpecialInstrsPass();
FunctionPass *createR600EmitClauseMarkers();
FunctionPass *createR600ClauseMergePass();
FunctionPass *createR600Packetizer();
FunctionPass *createR600ControlFlowFinalizer();
FunctionPass *createR600MachineCFGStructurizerPass();
FunctionPass *createR600ISelDag(TargetMachine &TM, CodeGenOptLevel OptLevel);
ModulePass *createR600OpenCLImageTypeLoweringPass();

void initializeR600ClauseMergePassPass(PassRegistry &);
extern char &R600ClauseMergePassID;

void initializeR600ControlFlowFinalizerPass(PassRegistry &);
extern char &R600ControlFlowFinalizerID;

void initializeR600ExpandSpecialInstrsPassPass(PassRegistry &);
extern char &R600ExpandSpecialInstrsPassID;

void initializeR600VectorRegMergerPass(PassRegistry &);
extern char &R600VectorRegMergerID;

void initializeR600PacketizerPass(PassRegistry &);
extern char &R600PacketizerID;

void initializeR600EmitClauseMarkersPass(PassRegistry &);
void initializeR600MachineCFGStructurizerPass(PassRegistry &);

} // End namespace vm::core

#endif
