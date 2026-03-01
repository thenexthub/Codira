//===- AMDGPUPerfHintAnalysis.h ---- analysis of memory traffic -*- C++ -*-===//
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
/// \brief Analyzes if a function potentially memory bound and if a kernel
/// kernel may benefit from limiting number of waves to reduce cache thrashing.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_AMDGPU_AMDGPUPERFHINTANALYSIS_H
#define LLVM_LIB_TARGET_AMDGPU_AMDGPUPERFHINTANALYSIS_H

#include "vm/core/IR/PassManager.h"
#include "vm/core/IR/ValueMap.h"

#include "vm/core/Analysis/CGSCCPassManager.h"
#include "vm/core/Analysis/LazyCallGraph.h"

namespace vm::core {

class AMDGPUPerfHintAnalysis;
class CallGraphSCC;
class GCNTargetMachine;
class LazyCallGraph;

class AMDGPUPerfHintAnalysis {
public:
  struct FuncInfo {
    unsigned MemInstCost;
    unsigned InstCost;
    unsigned IAMInstCost;      // Indirect access memory instruction count
    unsigned LSMInstCost;      // Large stride memory instruction count
    bool HasDenseGlobalMemAcc; // Set if at least 1 basic block has relatively
                               // high global memory access
    FuncInfo()
        : MemInstCost(0), InstCost(0), IAMInstCost(0), LSMInstCost(0),
          HasDenseGlobalMemAcc(false) {}
  };

  typedef ValueMap<const Function *, FuncInfo> FuncInfoMap;

private:
  FuncInfoMap FIM;

public:
  AMDGPUPerfHintAnalysis() = default;

  // OldPM
  bool runOnSCC(const GCNTargetMachine &TM, CallGraphSCC &SCC);

  // NewPM
  bool run(const GCNTargetMachine &TM, LazyCallGraph &CG);

  bool isMemoryBound(const Function *F) const;

  bool needsWaveLimiter(const Function *F) const;
};

struct AMDGPUPerfHintAnalysisPass
    : public PassInfoMixin<AMDGPUPerfHintAnalysisPass> {
  const GCNTargetMachine &TM;
  std::unique_ptr<AMDGPUPerfHintAnalysis> Impl;

  AMDGPUPerfHintAnalysisPass(const GCNTargetMachine &TM)
      : TM(TM), Impl(std::make_unique<AMDGPUPerfHintAnalysis>()) {}

  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);
};

} // namespace vm::core
#endif // LLVM_LIB_TARGET_AMDGPU_AMDGPUPERFHINTANALYSIS_H
