//===- AMDGPUAliasAnalysis --------------------------------------*- C++ -*-===//
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
/// \file
/// This is the AMGPU address space based alias analysis pass.
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_AMDGPU_AMDGPUALIASANALYSIS_H
#define LLVM_LIB_TARGET_AMDGPU_AMDGPUALIASANALYSIS_H

#include "vm/core/Analysis/AliasAnalysis.h"
#include "vm/core/IR/Module.h"

namespace vm::core {

class DataLayout;
class MemoryLocation;

/// A simple AA result that uses TBAA metadata to answer queries.
class AMDGPUAAResult : public AAResultBase {
  const DataLayout &DL;

public:
  explicit AMDGPUAAResult(const DataLayout &DL) : DL(DL) {}
  AMDGPUAAResult(AMDGPUAAResult &&Arg)
      : AAResultBase(std::move(Arg)), DL(Arg.DL) {}

  /// Handle invalidation events from the new pass manager.
  ///
  /// By definition, this result is stateless and so remains valid.
  bool invalidate(Function &, const PreservedAnalyses &,
                  FunctionAnalysisManager::Invalidator &Inv) {
    return false;
  }

  AliasResult alias(const MemoryLocation &LocA, const MemoryLocation &LocB,
                    AAQueryInfo &AAQI, const Instruction *CtxI);
  ModRefInfo getModRefInfoMask(const MemoryLocation &Loc, AAQueryInfo &AAQI,
                               bool IgnoreLocals);
};

/// Analysis pass providing a never-invalidated alias analysis result.
class AMDGPUAA : public AnalysisInfoMixin<AMDGPUAA> {
  friend AnalysisInfoMixin<AMDGPUAA>;

  static AnalysisKey Key;

public:
  using Result = AMDGPUAAResult;

  AMDGPUAAResult run(Function &F, AnalysisManager<Function> &AM) {
    return AMDGPUAAResult(F.getDataLayout());
  }
};

/// Legacy wrapper pass to provide the AMDGPUAAResult object.
class AMDGPUAAWrapperPass : public ImmutablePass {
  std::unique_ptr<AMDGPUAAResult> Result;

public:
  static char ID;

  AMDGPUAAWrapperPass();

  AMDGPUAAResult &getResult() { return *Result; }
  const AMDGPUAAResult &getResult() const { return *Result; }

  bool doInitialization(Module &M) override {
    Result.reset(new AMDGPUAAResult(M.getDataLayout()));
    return false;
  }

  bool doFinalization(Module &M) override {
    Result.reset();
    return false;
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override;
};

// Wrapper around ExternalAAWrapperPass so that the default constructor gets the
// callback.
class AMDGPUExternalAAWrapper : public ExternalAAWrapperPass {
public:
  static char ID;

  AMDGPUExternalAAWrapper() : ExternalAAWrapperPass(
    [](Pass &P, Function &, AAResults &AAR) {
      if (auto *WrapperPass = P.getAnalysisIfAvailable<AMDGPUAAWrapperPass>())
        AAR.addAAResult(WrapperPass->getResult());
    }) {}
};

} // end namespace vm::core

#endif // LLVM_LIB_TARGET_AMDGPU_AMDGPUALIASANALYSIS_H
