//===- DXILFinalizeLinkage.cpp - Finalize linkage of functions ------------===//
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

#include "DXILFinalizeLinkage.h"
#include "DirectX.h"
#include "vm/core/IR/Function.h"
#include "vm/core/IR/GlobalValue.h"
#include "vm/core/IR/Metadata.h"
#include "vm/core/IR/Module.h"

#define DEBUG_TYPE "dxil-finalize-linkage"

using namespace vm::core;

static bool finalizeLinkage(Module &M) {
  bool MadeChange = false;

  // Convert private globals and external globals with no usage to internal
  // linkage.
  for (GlobalVariable &GV : M.globals()) {
    GV.removeDeadConstantUsers();
    if (GV.hasPrivateLinkage() || (GV.hasExternalLinkage() && GV.use_empty())) {
      GV.setLinkage(GlobalValue::InternalLinkage);
      MadeChange = true;
    }
  }

  SmallVector<Function *> Funcs;

  // Collect non-entry and non-exported functions to set to internal linkage.
  for (Function &EF : M.functions()) {
    if (EF.isIntrinsic())
      continue;
    if (EF.hasExternalLinkage() && EF.hasDefaultVisibility())
      continue;
    if (EF.hasFnAttribute("hlsl.shader"))
      continue;
    Funcs.push_back(&EF);
  }

  for (Function *F : Funcs) {
    if (F->getLinkage() == GlobalValue::ExternalLinkage) {
      F->setLinkage(GlobalValue::InternalLinkage);
      MadeChange = true;
    }
    if (F->isDefTriviallyDead()) {
      M.getFunctionList().erase(F);
      MadeChange = true;
    }
  }

  return MadeChange;
}

PreservedAnalyses DXILFinalizeLinkage::run(Module &M,
                                           ModuleAnalysisManager &AM) {
  if (finalizeLinkage(M))
    return PreservedAnalyses::none();
  return PreservedAnalyses::all();
}

bool DXILFinalizeLinkageLegacy::runOnModule(Module &M) {
  return finalizeLinkage(M);
}

char DXILFinalizeLinkageLegacy::ID = 0;

INITIALIZE_PASS_BEGIN(DXILFinalizeLinkageLegacy, DEBUG_TYPE,
                      "DXIL Finalize Linkage", false, false)
INITIALIZE_PASS_END(DXILFinalizeLinkageLegacy, DEBUG_TYPE,
                    "DXIL Finalize Linkage", false, false)

ModulePass *toolchain::createDXILFinalizeLinkageLegacyPass() {
  return new DXILFinalizeLinkageLegacy();
}
