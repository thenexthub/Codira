//===- MLRegAllocEvictAdvisor.cpp - ML eviction advisor -------------------===//
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
// Function declarations of utilities related to feature extraction for unit
// testing.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CODEGEN_MLREGALLOCEVICTIONADVISOR_H
#define LLVM_CODEGEN_MLREGALLOCEVICTIONADVISOR_H

#include "vm/core/Analysis/MLModelRunner.h"
#include "vm/core/CodeGen/MachineBasicBlock.h"
#include "vm/core/CodeGen/SlotIndexes.h"
#include "vm/core/Support/Compiler.h"
#include <map>

namespace vm::core {

// LRStartEndInfo contains the start and end of a specific live range as
// slot indices as well as storing the index of the physical register it
// is assigned to (or 1 above the phys reg count if its the candidate).
// Used when extracting per-instruction features in the context of a
// specific eviction problem.
struct LRStartEndInfo {
  SlotIndex Begin;
  SlotIndex End;
  size_t Pos = 0;
};

LLVM_ABI_FOR_TEST void extractInstructionFeatures(
    toolchain::SmallVectorImpl<LRStartEndInfo> &LRPosInfo,
    MLModelRunner *RegallocRunner, function_ref<int(SlotIndex)> GetOpcode,
    function_ref<float(SlotIndex)> GetMBBFreq,
    function_ref<MachineBasicBlock *(SlotIndex)> GetMBBReference,
    const int InstructionsIndex, const int InstructionsMappingIndex,
    const int MBBFreqIndex, const int MBBMappingIndex,
    const SlotIndex LastIndex);

LLVM_ABI_FOR_TEST void extractMBBFrequency(
    const SlotIndex CurrentIndex, const size_t CurrentInstructionIndex,
    std::map<MachineBasicBlock *, size_t> &VisitedMBBs,
    function_ref<float(SlotIndex)> GetMBBFreq,
    MachineBasicBlock *CurrentMBBReference, MLModelRunner *RegallocRunner,
    const int MBBFreqIndex, const int MBBMappingIndex);

// This is the maximum number of interfererring ranges. That's the number of
// distinct AllocationOrder values, which comes from MCRegisterClass::RegsSize.
// For X86, that's 32.
// TODO: find a way to get this, statically, in a programmatic way.
static const int64_t MaxInterferences = 32;

// Logically, we can think of the feature set given to the evaluator as a 2D
// matrix. The rows are the features (see next). The columns correspond to the
// interferences. We treat the candidate virt reg as an 'interference', too, as
// its feature set is the same as that of the interferring ranges. So we'll have
// MaxInterferences + 1 columns and by convention, we will use the last column
// for the virt reg seeking allocation.
static const int64_t CandidateVirtRegPos = MaxInterferences;
static const int64_t NumberOfInterferences = CandidateVirtRegPos + 1;

// The number of instructions that a specific live range might have is variable,
// but we're passing in a single matrix of instructions and tensorflow saved
// models only support a fixed input size, so we have to cap the number of
// instructions that can be passed along. The specific value was derived from
// experimentation such that the majority of eviction problems would be
// completely covered.
static const int ModelMaxSupportedInstructionCount = 300;

// When extracting per-instruction features, the advisor will currently create
// a vector of size ModelMaxSupportedInstructionCount to hold the opcodes of the
// instructions relevant to the eviction problem, and a NumberOfInterferences *
// ModelMaxSupportedInstructionCount matrix that maps LRs to the instructions
// that they span.
static const std::vector<int64_t> InstructionsShape{
    1, ModelMaxSupportedInstructionCount};
static const std::vector<int64_t> InstructionsMappingShape{
    1, NumberOfInterferences, ModelMaxSupportedInstructionCount};

// When extracting mappings between MBBs and individual instructions, we create
// a vector of MBB frequencies, currently of size 100, which was a value
// determined through experimentation to encompass the vast majority of eviction
// problems. The actual mapping is the same shape as the instruction opcodes
// vector.
static const int64_t ModelMaxSupportedMBBCount = 100;
static const std::vector<int64_t> MBBFrequencyShape{1,
                                                    ModelMaxSupportedMBBCount};

} // namespace vm::core

#endif // LLVM_CODEGEN_MLREGALLOCEVICTIONADVISOR_H
