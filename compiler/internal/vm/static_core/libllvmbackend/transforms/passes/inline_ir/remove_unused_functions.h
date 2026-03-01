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

#ifndef LIBLLVMBACKEND_TRANSFORMS_PASSES_INLINE_IR_REMOVE_UNUSED_FUNCTIONS_H
#define LIBLLVMBACKEND_TRANSFORMS_PASSES_INLINE_IR_REMOVE_UNUSED_FUNCTIONS_H

#include <llvm/ADT/DenseSet.h>
#include <llvm/IR/PassManager.h>

namespace ark::llvmbackend {
struct LLVMCompilerOptions;
}  // namespace ark::llvmbackend

namespace llvm {
class Function;
class Module;
class Value;
}  // namespace llvm

namespace ark::llvmbackend::passes {

/**
 * Remove unused functions from the module.
 *
 * The function is unused if it is not used by any root function.
 * A root function is a function without FUNCTION_MD_INLINE_MODULE metadata
 */
class RemoveUnusedFunctions : public llvm::PassInfoMixin<RemoveUnusedFunctions> {
public:
    static bool ShouldInsert(const ark::llvmbackend::LLVMCompilerOptions *options);

    // NOLINTNEXTLINE(readability-identifier-naming)
    llvm::PreservedAnalyses run(llvm::Module &module, llvm::ModuleAnalysisManager &analysis_manager);

private:
    void VisitValue(llvm::DenseSet<llvm::Function *> &usedFunctions, llvm::Value &value,
                    llvm::DenseSet<llvm::Value *> &seenValues);

public:
    static constexpr llvm::StringRef ARG_NAME = "remove-unused-functions";
};

}  // namespace ark::llvmbackend::passes

#endif  // LIBLLVMBACKEND_TRANSFORMS_PASSES_INLINE_IR_REMOVE_UNUSED_FUNCTIONS_H
