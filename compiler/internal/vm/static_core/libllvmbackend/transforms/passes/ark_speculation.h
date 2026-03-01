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

#ifndef LIBLLVMBACKEND_TRANSFORMS_PASSES_ARK_SPECULATION_H
#define LIBLLVMBACKEND_TRANSFORMS_PASSES_ARK_SPECULATION_H

#include <llvm/Transforms/Scalar/SpeculativeExecution.h>
#include <llvm/IR/PassManager.h>

namespace ark::llvmbackend {
struct LLVMCompilerOptions;
}  // namespace ark::llvmbackend

namespace ark::llvmbackend::passes {

class ArkSpeculativeExecution : public llvm::SpeculativeExecutionPass {
    static constexpr bool ONLY_IF_DIVERGENT_TARGET = true;

public:
    static constexpr llvm::StringRef ARG_NAME = "wrap-speculative-execution";

    explicit ArkSpeculativeExecution() : llvm::SpeculativeExecutionPass(ONLY_IF_DIVERGENT_TARGET) {}
    static bool ShouldInsert([[maybe_unused]] const ark::llvmbackend::LLVMCompilerOptions *options)
    {
        return true;
    }
};

}  // namespace ark::llvmbackend::passes

#endif  // LIBLLVMBACKEND_TRANSFORMS_PASSES_ARK_SPECULATION_H
