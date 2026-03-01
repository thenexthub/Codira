//===- DXILUpgrade.cpp - Upgrade DXIL metadata to LLVM constructs ---------===//
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

#include "vm/core/Transforms/Utils/DXILUpgrade.h"
#include "vm/core/IR/Constants.h"
#include "vm/core/IR/Metadata.h"
#include "vm/core/IR/Module.h"
#include "vm/core/Support/Debug.h"

using namespace vm::core;

#define DEBUG_TYPE "dxil-upgrade"

static bool handleValVerMetadata(Module &M) {
  NamedMDNode *ValVer = M.getNamedMetadata("dx.valver");
  if (!ValVer)
    return false;

  LLVM_DEBUG({
    MDNode *N = ValVer->getOperand(0);
    auto X = mdconst::extract<ConstantInt>(N->getOperand(0))->getZExtValue();
    auto Y = mdconst::extract<ConstantInt>(N->getOperand(1))->getZExtValue();
    dbgs() << "DXIL: validation version: " << X << "." << Y << "\n";
  });
  // We don't need the validation version internally, so we drop it.
  ValVer->dropAllReferences();
  ValVer->eraseFromParent();
  return true;
}

PreservedAnalyses DXILUpgradePass::run(Module &M, ModuleAnalysisManager &AM) {
  PreservedAnalyses PA;
  // We never add, remove, or change functions here.
  PA.preserve<FunctionAnalysisManagerModuleProxy>();
  PA.preserveSet<AllAnalysesOn<Function>>();

  bool Changed = false;
  Changed |= handleValVerMetadata(M);

  if (!Changed)
    return PreservedAnalyses::all();
  return PA;
}
