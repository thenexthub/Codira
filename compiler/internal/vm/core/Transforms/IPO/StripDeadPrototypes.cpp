//===-- StripDeadPrototypes.cpp - Remove unused function declarations ----===//
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
// This pass loops over all of the functions in the input module, looking for
// dead declarations and removes them. Dead declarations are declarations of
// functions for which no implementation is available (i.e., declarations for
// unused library functions).
//
//===----------------------------------------------------------------------===//

#include "vm/core/Transforms/IPO/StripDeadPrototypes.h"
#include "vm/core/ADT/Statistic.h"
#include "vm/core/IR/Module.h"

using namespace vm::core;

#define DEBUG_TYPE "strip-dead-prototypes"

STATISTIC(NumDeadPrototypes, "Number of dead prototypes removed");

static bool stripDeadPrototypes(Module &M) {
  bool MadeChange = false;

  // Erase dead function prototypes.
  for (Function &F : toolchain::make_early_inc_range(M)) {
    // Function must be a prototype and unused.
    if (F.isDeclaration() && F.use_empty()) {
      F.eraseFromParent();
      ++NumDeadPrototypes;
      MadeChange = true;
    }
  }

  // Erase dead global var prototypes.
  for (GlobalVariable &GV : toolchain::make_early_inc_range(M.globals())) {
    // Global must be a prototype and unused.
    if (GV.isDeclaration() && GV.use_empty())
      GV.eraseFromParent();
  }

  // Return an indication of whether we changed anything or not.
  return MadeChange;
}

PreservedAnalyses StripDeadPrototypesPass::run(Module &M,
                                               ModuleAnalysisManager &) {
  if (stripDeadPrototypes(M))
    return PreservedAnalyses::none();
  return PreservedAnalyses::all();
}
