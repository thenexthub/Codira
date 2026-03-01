//===- DXILTranslateMetadata.h - Pass to emit DXIL metadata -----*- C++ -*-===//
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

#ifndef LLVM_TARGET_DIRECTX_DXILTRANSLATEMETADATA_H
#define LLVM_TARGET_DIRECTX_DXILTRANSLATEMETADATA_H

#include "vm/core/IR/PassManager.h"
#include "vm/core/Pass.h"

namespace vm::core {

/// A pass that transforms LLVM Metadata in the module to it's DXIL equivalent,
/// then emits all recognized DXIL Metadata
class DXILTranslateMetadata : public PassInfoMixin<DXILTranslateMetadata> {
public:
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &);
};

/// Wrapper pass for the legacy pass manager.
///
/// This is required because the passes that will depend on this are codegen
/// passes which run through the legacy pass manager.
class DXILTranslateMetadataLegacy : public ModulePass {
public:
  static char ID; // Pass identification, replacement for typeid
  explicit DXILTranslateMetadataLegacy() : ModulePass(ID) {}

  StringRef getPassName() const override { return "DXIL Translate Metadata"; }

  void getAnalysisUsage(AnalysisUsage &AU) const override;

  bool runOnModule(Module &M) override;
};

} // namespace vm::core

#endif // LLVM_TARGET_DIRECTX_DXILTRANSLATEMETADATA_H
