//===- OptUtils.cpp - MLIR Execution Engine optimization pass utilities ---===//
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
// This file implements the utility functions to trigger LLVM optimizations from
// MLIR Execution Engine.
//
//===----------------------------------------------------------------------===//

#include "mlir/ExecutionEngine/OptUtils.h"

#include "vm/core/Analysis/TargetTransformInfo.h"
#include "vm/core/IR/Module.h"
#include "vm/core/Passes/OptimizationLevel.h"
#include "vm/core/Passes/PassBuilder.h"
#include "vm/core/Support/Error.h"
#include "vm/core/Support/FormatVariadic.h"
#include "vm/core/Target/TargetMachine.h"
#include <optional>

using namespace vm::core;

static std::optional<OptimizationLevel> mapToLevel(unsigned optLevel,
                                                   unsigned sizeLevel) {
  switch (optLevel) {
  case 0:
    return OptimizationLevel::O0;

  case 1:
    return OptimizationLevel::O1;

  case 2:
    switch (sizeLevel) {
    case 0:
      return OptimizationLevel::O2;

    case 1:
      return OptimizationLevel::Os;

    case 2:
      return OptimizationLevel::Oz;
    }
    break;
  case 3:
    return OptimizationLevel::O3;
  }
  return std::nullopt;
}
// Create and return a lambda that uses LLVM pass manager builder to set up
// optimizations based on the given level.
std::function<Error(Module *)>
mlir::makeOptimizingTransformer(unsigned optLevel, unsigned sizeLevel,
                                TargetMachine *targetMachine) {
  return [optLevel, sizeLevel, targetMachine](Module *m) -> Error {
    std::optional<OptimizationLevel> ol = mapToLevel(optLevel, sizeLevel);
    if (!ol) {
      return make_error<StringError>(
          formatv("invalid optimization/size level {0}/{1}", optLevel,
                  sizeLevel)
              .str(),
          inconvertibleErrorCode());
    }
    LoopAnalysisManager lam;
    FunctionAnalysisManager fam;
    CGSCCAnalysisManager cgam;
    ModuleAnalysisManager mam;

    PipelineTuningOptions tuningOptions;
    tuningOptions.LoopUnrolling = true;
    tuningOptions.LoopInterleaving = true;
    tuningOptions.LoopVectorization = true;
    tuningOptions.SLPVectorization = true;

    PassBuilder pb(targetMachine, tuningOptions);

    pb.registerModuleAnalyses(mam);
    pb.registerCGSCCAnalyses(cgam);
    pb.registerFunctionAnalyses(fam);
    pb.registerLoopAnalyses(lam);
    pb.crossRegisterProxies(lam, fam, cgam, mam);

    ModulePassManager mpm;
    mpm.addPass(pb.buildPerModuleDefaultPipeline(*ol));
    mpm.run(*m, mam);
    return Error::success();
  };
}
