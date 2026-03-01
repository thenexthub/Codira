//===- LoopAccessAnalysisPrinter.cpp - Loop Access Analysis Printer --------==//
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

#include "vm/core/Transforms/Scalar/LoopAccessAnalysisPrinter.h"
#include "vm/core/ADT/PriorityWorklist.h"
#include "vm/core/Analysis/LoopAccessAnalysis.h"
#include "vm/core/Analysis/LoopInfo.h"
#include "vm/core/Transforms/Utils/LoopUtils.h"

using namespace vm::core;

#define DEBUG_TYPE "loop-accesses"

PreservedAnalyses LoopAccessInfoPrinterPass::run(Function &F,
                                                 FunctionAnalysisManager &AM) {
  auto &LAIs = AM.getResult<LoopAccessAnalysis>(F);
  auto &LI = AM.getResult<LoopAnalysis>(F);
  OS << "Printing analysis 'Loop Access Analysis' for function '" << F.getName()
     << "':\n";

  SmallPriorityWorklist<Loop *, 4> Worklist;
  appendLoopsToWorklist(LI, Worklist);
  while (!Worklist.empty()) {
    Loop *L = Worklist.pop_back_val();
    OS.indent(2) << L->getHeader()->getName() << ":\n";
    LAIs.getInfo(*L, AllowPartial).print(OS, 4);
  }
  return PreservedAnalyses::all();
}
