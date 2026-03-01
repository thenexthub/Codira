/*
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
 * Middletown, DE 19709, New Castle County, USA.
 */

#include "loop_peeling.h"

#include <llvm/Analysis/LoopInfo.h>
#include <llvm/Analysis/MemoryBuiltins.h>
#include <llvm/IR/Dominators.h>
#include <llvm/IR/Operator.h>
#include <llvm/Support/KnownBits.h>
#include <llvm/Transforms/Utils/LoopPeel.h>

#include "transforms/builtins.h"
#include "transforms/transform_utils.h"

#define DEBUG_TYPE "ark-loop-peeling"

// Peel only loops with llvm.experimental.deoptimize calls
// NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
static llvm::cl::opt<bool> g_deoptimizeOnly("ark-loop-peeling-deoptimize-only", llvm::cl::Hidden, llvm::cl::init(true));

namespace ark::llvmbackend::passes {

llvm::PreservedAnalyses ArkLoopPeeling::run(llvm::Loop &loop,
                                            [[maybe_unused]] llvm::LoopAnalysisManager &analysisManager,
                                            llvm::LoopStandardAnalysisResults &loopStandardAnalysisResults,
                                            [[maybe_unused]] llvm::LPMUpdater &lpmUpdater)
{
    if (llvm::canPeel(&loop)) {
        if (!g_deoptimizeOnly || ContainsDeoptimize(&loop)) {
            [[maybe_unused]] bool result =
                llvm::peelLoop(&loop, 1U, &loopStandardAnalysisResults.LI, &loopStandardAnalysisResults.SE,
                               loopStandardAnalysisResults.DT, nullptr, true);
            // Always returns true, false is unexpected
            ASSERT(result);
            return llvm::PreservedAnalyses::none();
        }
    }
    return llvm::PreservedAnalyses::all();
}

bool ArkLoopPeeling::ContainsDeoptimize(llvm::Loop *loop)
{
    ASSERT(loop->getHeader() != nullptr);
    for (auto &block : loop->blocks()) {
        for (auto &inst : *block) {
            auto call = llvm::dyn_cast<llvm::CallInst>(&inst);
            if (call == nullptr) {
                continue;
            }
            if (call->getIntrinsicID() == llvm::Intrinsic::experimental_deoptimize) {
                return true;
            }
        }
    }
    return false;
}

bool ArkLoopPeeling::ShouldInsert([[maybe_unused]] const ark::llvmbackend::LLVMCompilerOptions *options)
{
    return true;
}
}  // namespace ark::llvmbackend::passes
