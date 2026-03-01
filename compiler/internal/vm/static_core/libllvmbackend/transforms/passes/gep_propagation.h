/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef LIBLLVMBACKEND_TRANSFORMS_PASSES_GEP_PROPAGATION_H
#define LIBLLVMBACKEND_TRANSFORMS_PASSES_GEP_PROPAGATION_H

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/PassManager.h>

namespace ark::llvmbackend {
struct LLVMCompilerOptions;
}  // namespace ark::llvmbackend

namespace llvm {
class Instruction;
}  // namespace llvm

namespace ark::llvmbackend::passes {

class GepPropagation : public llvm::PassInfoMixin<GepPropagation> {
public:
    static bool ShouldInsert([[maybe_unused]] const ark::llvmbackend::LLVMCompilerOptions *options)
    {
        return true;
    }

    // NOLINTNEXTLINE(readability-identifier-naming)
    llvm::PreservedAnalyses run(llvm::Function &function, llvm::FunctionAnalysisManager &analysisManager);

private:
    using SelectorSplitMap = llvm::DenseMap<llvm::Instruction *, std::pair<llvm::Instruction *, llvm::Instruction *>>;

    void AddToVector(llvm::Instruction *inst, llvm::SmallVector<llvm::Instruction *> *toExpand,
                     llvm::SmallVector<llvm::Instruction *> *selectors);

    void Propagate(llvm::Function *function);

    void SplitGepSelectors(llvm::Function *function, llvm::SmallVector<llvm::Instruction *> *selectors,
                           llvm::DenseMap<llvm::Instruction *, llvm::Instruction *> *sgeps);

    std::pair<llvm::Value *, llvm::Value *> GenerateInput(llvm::Value *input, llvm::Instruction *inst,
                                                          llvm::Instruction *inPoint, const SelectorSplitMap &mapping);

    void GenerateSelectorInputs(llvm::Instruction *inst, const SelectorSplitMap &mapping);

    llvm::Value *GetConstantOffset(llvm::Constant *offset, llvm::Type *type);

    std::pair<llvm::Value *, bool> GetBasePointer(llvm::Value *value);

    llvm::Value *GetConstantInput(llvm::Instruction *inst);

    void OptimizeGepoffs(SelectorSplitMap &mapping);

    void OptimizeSelectors(SelectorSplitMap &mapping);

    void ReplaceWithSplitGep(llvm::Instruction *inst, llvm::Instruction *gep);

    void ReplaceRecursively(llvm::Instruction *inst, llvm::SmallVector<llvm::Instruction *, 1> *seq);

    llvm::Instruction *CloneSequence(llvm::IRBuilder<> *builder, llvm::SmallVector<llvm::Instruction *, 1> *seq);

public:
    static constexpr llvm::StringRef ARG_NAME = "gep-propagation";
};

}  // namespace ark::llvmbackend::passes

#endif  //  LIBLLVMBACKEND_TRANSFORMS_PASSES_GEP_PROPAGATION_H
