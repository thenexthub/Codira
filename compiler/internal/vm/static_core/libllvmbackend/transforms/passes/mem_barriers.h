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

#ifndef LIBLLVMBACKEND_TRANSFORMS_PASSES_MEM_BARRIERS_H
#define LIBLLVMBACKEND_TRANSFORMS_PASSES_MEM_BARRIERS_H

#include "llvm_ark_interface.h"

#include <llvm/IR/Instruction.h>
#include <llvm/IR/PassManager.h>
#include <llvm/IR/Instructions.h>

namespace ark::llvmbackend {
struct LLVMCompilerOptions;
}  // namespace ark::llvmbackend

namespace ark::llvmbackend::passes {

class MemBarriers : public llvm::PassInfoMixin<MemBarriers> {
public:
    explicit MemBarriers(LLVMArkInterface *arkInterface, bool optimize);

    static bool ShouldInsert([[maybe_unused]] const ark::llvmbackend::LLVMCompilerOptions *options)
    {
        return true;
    }

    static MemBarriers Create(LLVMArkInterface *arkInterface, const ark::llvmbackend::LLVMCompilerOptions *options);

    // NOLINTNEXTLINE(readability-identifier-naming)
    llvm::PreservedAnalyses run(llvm::Function &function, llvm::FunctionAnalysisManager &analysisManager);

private:
    bool GrabsGuarded(llvm::Instruction *inst, llvm::SmallVector<llvm::Instruction *> &mergeSet);
    void MergeBarriers(llvm::SmallVector<llvm::Instruction *> &mergeSet,
                       llvm::SmallVector<llvm::Instruction *> &needsBarrier);
    bool RelaxBarriers(llvm::Function &function);
    bool IsProtectedPath(llvm::BasicBlock &block, llvm::SmallVector<llvm::Instruction *> &mergeSet);
    bool SinkBarriers(llvm::BasicBlock &block, llvm::SmallVector<llvm::Instruction *> &mergeSet);

private:
    LLVMArkInterface *arkInterface_;
    bool optimize_;

public:
    static constexpr llvm::StringRef ARG_NAME = "mem-barriers";
};

}  // namespace ark::llvmbackend::passes

#endif  // LIBLLVMBACKEND_TRANSFORMS_PASSES_MEM_BARRIERS_H
