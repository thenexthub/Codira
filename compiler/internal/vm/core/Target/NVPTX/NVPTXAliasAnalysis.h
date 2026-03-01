//===-------------------- NVPTXAliasAnalysis.h ------------------*- C++ -*-===//
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
/// This is the NVPTX address space based alias analysis pass.
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_NVPTX_NVPTXALIASANALYSIS_H
#define LLVM_LIB_TARGET_NVPTX_NVPTXALIASANALYSIS_H

#include "vm/core/Analysis/AliasAnalysis.h"

namespace vm::core {

class MemoryLocation;

class NVPTXAAResult : public AAResultBase {
public:
  NVPTXAAResult() = default;
  NVPTXAAResult(NVPTXAAResult &&Arg) : AAResultBase(std::move(Arg)) {}

  /// Handle invalidation events from the new pass manager.
  ///
  /// By definition, this result is stateless and so remains valid.
  bool invalidate(Function &, const PreservedAnalyses &,
                  FunctionAnalysisManager::Invalidator &Inv) {
    return false;
  }

  AliasResult alias(const MemoryLocation &LocA, const MemoryLocation &LocB,
                    AAQueryInfo &AAQI, const Instruction *CtxI = nullptr);

  ModRefInfo getModRefInfoMask(const MemoryLocation &Loc, AAQueryInfo &AAQI,
                               bool IgnoreLocals);

  MemoryEffects getMemoryEffects(const CallBase *Call, AAQueryInfo &AAQI);

  MemoryEffects getMemoryEffects(const Function *F) {
    return MemoryEffects::unknown();
  }
};

/// Analysis pass providing a never-invalidated alias analysis result.
class NVPTXAA : public AnalysisInfoMixin<NVPTXAA> {
  friend AnalysisInfoMixin<NVPTXAA>;

  static AnalysisKey Key;

public:
  using Result = NVPTXAAResult;

  NVPTXAAResult run(Function &F, AnalysisManager<Function> &AM) {
    return NVPTXAAResult();
  }
};

/// Legacy wrapper pass to provide the NVPTXAAResult object.
class NVPTXAAWrapperPass : public ImmutablePass {
  std::unique_ptr<NVPTXAAResult> Result;

public:
  static char ID;

  NVPTXAAWrapperPass();

  NVPTXAAResult &getResult() { return *Result; }
  const NVPTXAAResult &getResult() const { return *Result; }

  bool doInitialization(Module &M) override {
    Result.reset(new NVPTXAAResult());
    return false;
  }

  bool doFinalization(Module &M) override {
    Result.reset();
    return false;
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override;
};

// Wrapper around ExternalAAWrapperPass so that the default
// constructor gets the callback.
// Note that NVPTXAA will run before BasicAA for compile time considerations.
class NVPTXExternalAAWrapper : public ExternalAAWrapperPass {
public:
  static char ID;

  NVPTXExternalAAWrapper()
      : ExternalAAWrapperPass(
            [](Pass &P, Function &, AAResults &AAR) {
              if (auto *WrapperPass =
                      P.getAnalysisIfAvailable<NVPTXAAWrapperPass>())
                AAR.addAAResult(WrapperPass->getResult());
            },
            /*RunEarly=*/true) {}

  StringRef getPassName() const override {
    return "NVPTX Address space based Alias Analysis Wrapper";
  }
};

ImmutablePass *createNVPTXAAWrapperPass();
ImmutablePass *createNVPTXExternalAAWrapperPass();

} // end namespace vm::core

#endif // LLVM_LIB_TARGET_NVPTX_NVPTXALIASANALYSIS_H
