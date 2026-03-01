//===- CoroConditionalWrapper.cpp -----------------------------------------===//
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

#include "vm/core/Transforms/Coroutines/CoroConditionalWrapper.h"
#include "CoroInternal.h"
#include "vm/core/IR/Module.h"

using namespace vm::core;

CoroConditionalWrapper::CoroConditionalWrapper(ModulePassManager &&PM)
    : PM(std::move(PM)) {}

PreservedAnalyses CoroConditionalWrapper::run(Module &M,
                                              ModuleAnalysisManager &AM) {
  if (!coro::declaresAnyIntrinsic(M))
    return PreservedAnalyses::all();

  return PM.run(M, AM);
}

void CoroConditionalWrapper::printPipeline(
    raw_ostream &OS, function_ref<StringRef(StringRef)> MapClassName2PassName) {
  OS << "coro-cond";
  OS << '(';
  PM.printPipeline(OS, MapClassName2PassName);
  OS << ')';
}
