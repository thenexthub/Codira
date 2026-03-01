//===- AMDGPUResourceUsageAnalysis.h ---- analysis of resources -*- C++ -*-===//
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
/// \file
/// \brief Analyzes how many registers and other resources are used by
/// functions.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_AMDGPU_AMDGPURESOURCEUSAGEANALYSIS_H
#define LLVM_LIB_TARGET_AMDGPU_AMDGPURESOURCEUSAGEANALYSIS_H

#include "vm/core/ADT/SmallVector.h"
#include "vm/core/CodeGen/MachineFunctionPass.h"
#include "vm/core/IR/PassManager.h"

namespace vm::core {

class GCNSubtarget;
class MachineFunction;
class GCNTargetMachine;

struct AMDGPUResourceUsageAnalysisImpl {
public:
  static char ID;
  // Track resource usage for callee functions.
  struct SIFunctionResourceInfo {
    // Track the number of explicitly used VGPRs. Special registers reserved at
    // the end are tracked separately.
    int32_t NumVGPR = 0;
    int32_t NumAGPR = 0;
    int32_t NumExplicitSGPR = 0;
    int32_t NumNamedBarrier = 0;
    uint64_t CalleeSegmentSize = 0;
    uint64_t PrivateSegmentSize = 0;
    bool UsesVCC = false;
    bool UsesFlatScratch = false;
    bool HasDynamicallySizedStack = false;
    bool HasRecursion = false;
    bool HasIndirectCall = false;
    SmallVector<const Function *, 16> Callees;
  };

  SIFunctionResourceInfo
  analyzeResourceUsage(const MachineFunction &MF,
                       uint32_t AssumedStackSizeForDynamicSizeObjects,
                       uint32_t AssumedStackSizeForExternalCall) const;
};

struct AMDGPUResourceUsageAnalysisWrapperPass : public MachineFunctionPass {
  using FunctionResourceInfo =
      AMDGPUResourceUsageAnalysisImpl::SIFunctionResourceInfo;
  FunctionResourceInfo ResourceInfo;

public:
  static char ID;
  AMDGPUResourceUsageAnalysisWrapperPass() : MachineFunctionPass(ID) {}

  bool runOnMachineFunction(MachineFunction &MF) override;

  const FunctionResourceInfo &getResourceInfo() const { return ResourceInfo; }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesAll();
    MachineFunctionPass::getAnalysisUsage(AU);
  }
};

class AMDGPUResourceUsageAnalysis
    : public AnalysisInfoMixin<AMDGPUResourceUsageAnalysis> {
  friend AnalysisInfoMixin<AMDGPUResourceUsageAnalysis>;
  static AnalysisKey Key;

  const GCNTargetMachine &TM;

public:
  using Result = AMDGPUResourceUsageAnalysisImpl::SIFunctionResourceInfo;
  Result run(MachineFunction &MF, MachineFunctionAnalysisManager &MFAM);

  AMDGPUResourceUsageAnalysis(const GCNTargetMachine &TM_) : TM(TM_) {}
};

} // namespace vm::core
#endif // LLVM_LIB_TARGET_AMDGPU_AMDGPURESOURCEUSAGEANALYSIS_H
