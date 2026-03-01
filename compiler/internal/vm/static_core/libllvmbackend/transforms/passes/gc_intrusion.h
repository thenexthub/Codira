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

#ifndef LIBLLVMBACKEND_TRANSFORMS_PASSES_GC_INTRUSION_H
#define LIBLLVMBACKEND_TRANSFORMS_PASSES_GC_INTRUSION_H

#include <llvm/ADT/SetVector.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/PassManager.h>

namespace ark::llvmbackend {
struct LLVMCompilerOptions;
}  // namespace ark::llvmbackend

namespace ark::llvmbackend::passes {

class GcRefLiveness;

class GcIntrusion : public llvm::PassInfoMixin<GcIntrusion> {
public:
    static bool ShouldInsert(const ark::llvmbackend::LLVMCompilerOptions *options);

    // NOLINTNEXTLINE(readability-identifier-naming)
    llvm::PreservedAnalyses run(llvm::Function &function, llvm::FunctionAnalysisManager &analysisManager);

private:
    using SSAVarRelocs = llvm::DenseMap<llvm::Value *, llvm::DenseMap<llvm::BasicBlock *, llvm::Value *>>;
    using InstructionOrderMap = llvm::DenseMap<llvm::Value *, uint64_t>;
    using SortedUses = llvm::DenseMap<llvm::Value *, std::list<llvm::Use *>>;
    using RPOMap = llvm::DenseMap<llvm::BasicBlock *, uint32_t>;
    struct GcIntrusionContext {
        SSAVarRelocs relocs;
        InstructionOrderMap orderMap;
        SortedUses sortedUses;
        RPOMap rpoMap;
    };

    void RewriteWithGcInBlock(llvm::BasicBlock *block, GcRefLiveness *liveness, llvm::SetVector<llvm::Value *> *alive,
                              llvm::DenseSet<llvm::BasicBlock *> *visited, GcIntrusionContext *gcContext);

    static bool ComesBefore(llvm::Value *a, llvm::Value *b, InstructionOrderMap *orderMap);

    void HoistForRelocation(llvm::Function *function);

    bool MoveComparisons(llvm::Function *function);

    void FixupEscapedUsages(llvm::Instruction *inst, const llvm::SmallVector<llvm::Instruction *> &casts);

    llvm::Value *CreateBackwardCasts(llvm::IRBuilder<> *builder, llvm::Value *from,
                                     const llvm::SmallVector<llvm::Instruction *> &casts);

    uint32_t GetStatepointId(const llvm::Instruction &inst);

    void ReplaceWithPhi(llvm::Value *var, llvm::BasicBlock *block, GcIntrusionContext *gcContext);

    void PropagateRelocs(GcRefLiveness *liveness, llvm::DenseSet<llvm::BasicBlock *> *visited, llvm::BasicBlock *block,
                         GcIntrusionContext *gcContext);

    void CopySinglePredRelocs(GcRefLiveness *liveness, llvm::BasicBlock *block, GcIntrusionContext *gcContext);

    llvm::Value *GetUniqueLiveOut(SSAVarRelocs *relocs, llvm::BasicBlock *block, llvm::Value *var) const;

    void ReplaceWithRelocated(llvm::CallInst *call, llvm::CallInst *gcCall, llvm::Value *inst, llvm::Value *relocated,
                              GcIntrusionContext *gcContext);

    void RewriteWithGc(llvm::CallInst *call, GcRefLiveness *liveness, llvm::SetVector<llvm::Value *> *refs,
                       GcIntrusionContext *gcContext);

    std::vector<llvm::Value *> GetDeoptsFromInlineInfo(llvm::IRBuilder<> &builder, llvm::CallInst *call);

    void CreateSortedUseList(llvm::BasicBlock *block, llvm::Value *from, GcIntrusionContext *gcContext);

    void ReplaceDominatedUses(llvm::Value *from, llvm::Value *to, llvm::BasicBlock *block,
                              GcIntrusionContext *gcContext);

    void UpdatePhiInputs(llvm::BasicBlock *block, SSAVarRelocs *relocs);

public:
    static constexpr llvm::StringRef ARG_NAME = "gc-intrusion";
};

}  // namespace ark::llvmbackend::passes

#endif  //  LIBLLVMBACKEND_TRANSFORMS_PASSES_GC_INTRUSION_H
