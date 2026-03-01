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

#ifndef LIBLLVMBACKEND_TRANSFORMS_PASSES_INFER_FLAGS_H
#define LIBLLVMBACKEND_TRANSFORMS_PASSES_INFER_FLAGS_H

#include <llvm/IR/PassManager.h>
#include <llvm/Analysis/LoopAnalysisManager.h>
#include <llvm/Transforms/Scalar/LoopPassManager.h>

namespace ark::llvmbackend {
struct LLVMCompilerOptions;
}  // namespace ark::llvmbackend

namespace ark::llvmbackend {
class LLVMArkInterface;
}  // namespace ark::llvmbackend

namespace ark::llvmbackend::passes {

/// Simple pass, that infers "nsw" flag (no signed wrap) to help llvm perform optimizations
class InferFlags : public llvm::PassInfoMixin<InferFlags> {
public:
    explicit InferFlags() = default;

    static bool ShouldInsert(const ark::llvmbackend::LLVMCompilerOptions *options);

    // NOLINTNEXTLINE(readability-identifier-naming)
    llvm::PreservedAnalyses run(llvm::Function &function, llvm::FunctionAnalysisManager &analysisManager);

private:
    bool RunOnLoop(llvm::Loop *loop, llvm::ScalarEvolution *scalarEvolution);

    bool RunOnBasicBlock(llvm::Loop *loop, llvm::BasicBlock *basicBlock, llvm::ScalarEvolution *scalarEvolution);

public:
    static constexpr llvm::StringRef ARG_NAME = "infer-flags";
};

}  // namespace ark::llvmbackend::passes

#endif  //  LIBLLVMBACKEND_TRANSFORMS_PASSES_INFER_FLAGS_H
