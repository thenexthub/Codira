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

#ifndef LIBLLVMBACKEND_TRANSFORMS_PASSES_GC_INTRUSION_CHECK_H
#define LIBLLVMBACKEND_TRANSFORMS_PASSES_GC_INTRUSION_CHECK_H

#include <llvm/IR/PassManager.h>

namespace ark::llvmbackend {
struct LLVMCompilerOptions;
}  // namespace ark::llvmbackend

namespace llvm {
class GCRelocateInst;
class GCStatepointInst;
}  // namespace llvm

namespace ark::llvmbackend::passes {

class GcIntrusionCheck : public llvm::PassInfoMixin<GcIntrusionCheck> {
public:
    static constexpr llvm::StringRef ARG_NAME = "gc-intrusion-check";

    static bool ShouldInsert(const ark::llvmbackend::LLVMCompilerOptions *options);

    // NOLINTNEXTLINE(readability-identifier-naming)
    llvm::PreservedAnalyses run(llvm::Function &function, llvm::FunctionAnalysisManager &analysisManager);

private:
    void CheckStatepoint(const llvm::Function &func, const llvm::GCStatepointInst &statepoint);

    void CheckRelocate(const llvm::Function &func, const llvm::GCRelocateInst &relocate);

    void CheckInstruction(const llvm::Function &func, const llvm::Instruction &inst);

    const llvm::Instruction *FindDefOrStatepoint(const llvm::Instruction *start, const llvm::Instruction *def);

    const llvm::Instruction *FindDefOrStatepointRecursive(const llvm::Instruction *start, const llvm::Instruction *def,
                                                          llvm::DenseSet<const llvm::BasicBlock *> *visited);

    bool IsHiddenGcRef(llvm::Value *ref);

    llvm::ArrayRef<llvm::Use> GetBundle(const llvm::GCStatepointInst &call, uint32_t id);
};

}  // namespace ark::llvmbackend::passes

#endif  //  LIBLLVMBACKEND_TRANSFORMS_PASSES_GC_INTRUSION_CHECK_H
