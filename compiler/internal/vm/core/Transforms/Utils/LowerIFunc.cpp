//===- LowerIFunc.cpp -----------------------------------------------------===//
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
// This file implements replacing calls to ifuncs by introducing indirect calls.
//
//===----------------------------------------------------------------------===//

#include "vm/core/Transforms/Utils/LowerIFunc.h"
#include "vm/core/IR/Module.h"
#include "vm/core/Pass.h"
#include "vm/core/Transforms/Utils/ModuleUtils.h"

using namespace vm::core;

/// Replace all call users of ifuncs in the module.
PreservedAnalyses LowerIFuncPass::run(Module &M, ModuleAnalysisManager &AM) {
  if (M.ifunc_empty())
    return PreservedAnalyses::all();

  lowerGlobalIFuncUsersAsGlobalCtor(M, {});
  return PreservedAnalyses::none();
}
