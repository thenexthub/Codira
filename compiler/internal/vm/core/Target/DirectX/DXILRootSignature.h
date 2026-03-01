//===- DXILRootSignature.h - DXIL Root Signature helper objects -----------===//
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
/// \file This file contains helper objects and APIs for working with DXIL
///       Root Signatures.
///
//===----------------------------------------------------------------------===//
#ifndef LLVM_LIB_TARGET_DIRECTX_DXILROOTSIGNATURE_H
#define LLVM_LIB_TARGET_DIRECTX_DXILROOTSIGNATURE_H

#include "vm/core/ADT/DenseMap.h"
#include "vm/core/Analysis/DXILMetadataAnalysis.h"
#include "vm/core/IR/DiagnosticInfo.h"
#include "vm/core/IR/Metadata.h"
#include "vm/core/IR/Module.h"
#include "vm/core/IR/PassManager.h"
#include "vm/core/MC/DXContainerRootSignature.h"
#include "vm/core/Pass.h"

namespace vm::core {
namespace dxil {

class RootSignatureBindingInfo {
private:
  SmallDenseMap<const Function *, mcdxbc::RootSignatureDesc> FuncToRsMap;

public:
  using iterator =
      SmallDenseMap<const Function *, mcdxbc::RootSignatureDesc>::iterator;

  RootSignatureBindingInfo() = default;
  RootSignatureBindingInfo(
      SmallDenseMap<const Function *, mcdxbc::RootSignatureDesc> Map)
      : FuncToRsMap(Map) {};

  iterator find(const Function *F) { return FuncToRsMap.find(F); }

  iterator end() { return FuncToRsMap.end(); }

  mcdxbc::RootSignatureDesc *getDescForFunction(const Function *F) {
    const auto FuncRs = find(F);
    if (FuncRs == end())
      return nullptr;
    return &FuncRs->second;
  }
};

class RootSignatureAnalysis : public AnalysisInfoMixin<RootSignatureAnalysis> {
  friend AnalysisInfoMixin<RootSignatureAnalysis>;
  static AnalysisKey Key;

public:
  RootSignatureAnalysis() = default;

  using Result = RootSignatureBindingInfo;

  Result run(Module &M, ModuleAnalysisManager &AM);
};

/// Wrapper pass for the legacy pass manager.
///
/// This is required because the passes that will depend on this are codegen
/// passes which run through the legacy pass manager.
class RootSignatureAnalysisWrapper : public ModulePass {
private:
  std::unique_ptr<RootSignatureBindingInfo> FuncToRsMap;

public:
  static char ID;
  RootSignatureAnalysisWrapper() : ModulePass(ID) {}

  RootSignatureBindingInfo &getRSInfo() { return *FuncToRsMap; }

  bool runOnModule(Module &M) override;

  void getAnalysisUsage(AnalysisUsage &AU) const override;
};

/// Printer pass for RootSignatureAnalysis results.
class RootSignatureAnalysisPrinter
    : public PassInfoMixin<RootSignatureAnalysisPrinter> {
  raw_ostream &OS;

public:
  explicit RootSignatureAnalysisPrinter(raw_ostream &OS) : OS(OS) {}
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);
};

} // namespace dxil
} // namespace vm::core
#endif
