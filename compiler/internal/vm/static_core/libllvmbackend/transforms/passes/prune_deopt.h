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

#ifndef LIBLLVMBACKEND_TRANSFORMS_PASSES_PRUNE_DEOPT_H
#define LIBLLVMBACKEND_TRANSFORMS_PASSES_PRUNE_DEOPT_H

#include <llvm/IR/PassManager.h>

namespace ark::llvmbackend {
struct LLVMCompilerOptions;
}  // namespace ark::llvmbackend

namespace llvm {
struct OperandBundleUse;
}  // namespace llvm

namespace ark::llvmbackend::passes {

class PruneDeopt : public llvm::PassInfoMixin<PruneDeopt> {
public:
    static bool ShouldInsert([[maybe_unused]] const ark::llvmbackend::LLVMCompilerOptions *options)
    {
        return true;
    }

    // NOLINTNEXTLINE(readability-identifier-naming)
    llvm::PreservedAnalyses run(llvm::Function &function, llvm::FunctionAnalysisManager &analysisManager);

private:
    llvm::CallInst *GetUpdatedCallInst(llvm::CallInst *call, const llvm::OperandBundleUse &bundle);

    using EncodedDeoptBundle = llvm::SmallVector<llvm::Value *, 0>;

    bool IsCaughtDeoptimization(llvm::ArrayRef<llvm::Use> inputs) const;

    bool IsNoReturn(llvm::ArrayRef<llvm::Use> inputs) const;

    EncodedDeoptBundle EncodeDeoptBundle(llvm::CallInst *call, const llvm::OperandBundleUse &bundle) const;

    std::string GetInlineInfo(llvm::ArrayRef<llvm::Use> inputs) const;

    void MakeUnreachableAfter(llvm::BasicBlock *block, llvm::Instruction *after) const;

public:
    static constexpr llvm::StringRef ARG_NAME = "prune-deopt";
};

}  // namespace ark::llvmbackend::passes

#endif  //  LIBLLVMBACKEND_TRANSFORMS_PASSES_PRUNE_DEOPT_H
