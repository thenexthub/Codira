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

#ifndef LIBLLVMBACKEND_TRANSFORMS_PASSES_INTRINSICS_LOWERING_H
#define LIBLLVMBACKEND_TRANSFORMS_PASSES_INTRINSICS_LOWERING_H

#include <unordered_map>

#include "llvm_ark_interface.h"

#include <llvm/IR/Intrinsics.h>
#include <llvm/IR/PassManager.h>

namespace ark::llvmbackend {
struct LLVMCompilerOptions;
}  // namespace ark::llvmbackend

namespace llvm {
class CallInst;
class Instruction;
}  // namespace llvm

namespace ark::llvmbackend::passes {

class IntrinsicsLowering : public llvm::PassInfoMixin<IntrinsicsLowering> {
public:
    explicit IntrinsicsLowering(LLVMArkInterface *arkInterface = nullptr);

    static bool ShouldInsert([[maybe_unused]] const ark::llvmbackend::LLVMCompilerOptions *options)
    {
        return true;
    }

    static IntrinsicsLowering Create(LLVMArkInterface *arkInterface,
                                     const ark::llvmbackend::LLVMCompilerOptions *options);

    // NOLINTNEXTLINE(readability-identifier-naming)
    llvm::PreservedAnalyses run(llvm::Function &function, llvm::FunctionAnalysisManager &analysisManager);

private:
    bool ReplaceWithLLVMIntrinsic(llvm::CallInst *call, llvm::Intrinsic::ID intrinsicId);

    void HandleMemCall(llvm::CallInst *call, llvm::FunctionCallee callee,
                       std::unordered_map<llvm::Instruction *, llvm::Instruction *> *instToReplaceWithInst);
    bool HandleCall(llvm::CallInst *call, LLVMArkInterface::IntrinsicId intrinsicId,
                    std::unordered_map<llvm::Instruction *, llvm::Instruction *> *instToReplaceWithInst);

    bool HandleFRem(llvm::Instruction *inst, LLVMArkInterface::IntrinsicId intrinsicId,
                    std::unordered_map<llvm::Instruction *, llvm::Instruction *> *instToReplaceWithInst);

private:
    LLVMArkInterface *arkInterface_;

public:
    static constexpr llvm::StringRef ARG_NAME = "intrinsics-lowering";
};

}  // namespace ark::llvmbackend::passes

#endif  // LIBLLVMBACKEND_TRANSFORMS_PASSES_INTRINSICS_LOWERING_H
