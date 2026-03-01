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

#ifndef LIBLLVMBACKEND_TRANSFORMS_PASSES_PANDA_RUNTIME_LOWERING_H
#define LIBLLVMBACKEND_TRANSFORMS_PASSES_PANDA_RUNTIME_LOWERING_H

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/PassManager.h>

namespace ark::llvmbackend {
class LLVMArkInterface;
struct LLVMCompilerOptions;
}  // namespace ark::llvmbackend

namespace llvm {
class CallInst;
class Instruction;
}  // namespace llvm

namespace ark::llvmbackend::passes {

class PandaRuntimeLowering : public llvm::PassInfoMixin<PandaRuntimeLowering> {
public:
    explicit PandaRuntimeLowering(LLVMArkInterface *arkInterface = nullptr);

    static bool ShouldInsert([[maybe_unused]] const ark::llvmbackend::LLVMCompilerOptions *options)
    {
        return true;
    }

    static PandaRuntimeLowering Create(LLVMArkInterface *arkInterface,
                                       const ark::llvmbackend::LLVMCompilerOptions *options);

    // NOLINTNEXTLINE(readability-identifier-naming)
    llvm::PreservedAnalyses run(llvm::Function &function, llvm::FunctionAnalysisManager &am);

private:
    llvm::Value *GetMethodOrResolverPtr(llvm::IRBuilder<> *builder, llvm::CallInst *inst);
    bool NeedsToBeLowered(llvm::CallInst *call);
    void LowerBuiltin(llvm::CallInst *inst);
    void LowerCallStatic(llvm::CallInst *inst);
    void LowerCallVirtual(llvm::CallInst *inst);
    void LowerDeoptimizeIntrinsic(llvm::CallInst *deoptimize);

private:
    LLVMArkInterface *arkInterface_;

public:
    static constexpr llvm::StringRef ARG_NAME = "runtime-calls-lowering";
};

}  // namespace ark::llvmbackend::passes

#endif  //  LIBLLVMBACKEND_TRANSFORMS_PASSES_PANDA_RUNTIME_LOWERING_H
