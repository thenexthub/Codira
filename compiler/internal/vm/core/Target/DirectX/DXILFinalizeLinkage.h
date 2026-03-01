//===- DXILFinalizeLinkage.h - Finalize linkage of functions --------------===//
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
/// DXILFinalizeLinkage pass updates the linkage of functions to make sure only
/// shader entry points and exported functions are visible from the module (have
/// program linkage). All other functions will be updated to have internal
/// linkage.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_TARGET_DIRECTX_DXILFINALIZELINKAGE_H
#define LLVM_TARGET_DIRECTX_DXILFINALIZELINKAGE_H

#include "vm/core/IR/PassManager.h"
#include "vm/core/Pass.h"

namespace vm::core {

class DXILFinalizeLinkage : public PassInfoMixin<DXILFinalizeLinkage> {
public:
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &);
  static bool isRequired() { return true; }
};

class DXILFinalizeLinkageLegacy : public ModulePass {
public:
  DXILFinalizeLinkageLegacy() : ModulePass(ID) {}
  bool runOnModule(Module &M) override;

  static char ID; // Pass identification.
};
} // namespace vm::core

#endif // LLVM_TARGET_DIRECTX_DXILFINALIZELINKAGE_H
