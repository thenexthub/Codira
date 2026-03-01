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

#include "transforms/passes/mem_barriers.h"
#include "llvm_ark_interface.h"
#include "llvm_compiler_options.h"

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/IntrinsicsAArch64.h>
#include <llvm/Transforms/Utils/BasicBlockUtils.h>

#define DEBUG_TYPE "mem-barriers"

using llvm::Function;
using llvm::FunctionAnalysisManager;

namespace ark::llvmbackend::passes {

MemBarriers MemBarriers::Create([[maybe_unused]] LLVMArkInterface *arkInterface,
                                const ark::llvmbackend::LLVMCompilerOptions *options)
{
    return MemBarriers(arkInterface, options->optimize);
}

MemBarriers::MemBarriers(LLVMArkInterface *arkInterface, bool optimize)
    : arkInterface_ {arkInterface}, optimize_ {optimize}
{
}

llvm::PreservedAnalyses MemBarriers::run(Function &function, FunctionAnalysisManager & /*analysisManager*/)
{
    bool changed = false;
    uint32_t dmbSunk = 0;

    for (llvm::BasicBlock &block : function) {
        llvm::SmallVector<llvm::Instruction *> needsBarrier;
        llvm::SmallVector<llvm::Instruction *> mergeSet;
        for (auto &inst : block) {
            auto callInst = llvm::dyn_cast<llvm::CallInst>(&inst);
            if (callInst != nullptr && callInst->hasFnAttr("needs-mem-barrier")) {
                mergeSet.push_back(callInst);
                continue;
            }
            if (optimize_ && inst.mayWriteToMemory() && GrabsGuarded(&inst, mergeSet)) {
                MergeBarriers(mergeSet, needsBarrier);
            }
        }

        if (optimize_ && SinkBarriers(block, mergeSet)) {
            changed = true;  // sank dmb to successor
            dmbSunk++;
        } else {
            MergeBarriers(mergeSet, needsBarrier);  // not able to sink dmb
        }

        for (auto inst : needsBarrier) {
            auto builder = llvm::IRBuilder<>(inst->getNextNode());
            builder.CreateFence(llvm::AtomicOrdering::Release);
            changed = true;
        }
    }

    LLVM_DEBUG(llvm::dbgs() << "Number of dmb's sunk: " << dmbSunk << "\n");
    if (dmbSunk != 0) {
        LLVM_DEBUG(llvm::dbgs() << "Sunk function name: " << function.getName() << "\n");
    }

    changed |= RelaxBarriers(function);

    return changed ? llvm::PreservedAnalyses::none() : llvm::PreservedAnalyses::all();
}

bool MemBarriers::GrabsGuarded(llvm::Instruction *inst, llvm::SmallVector<llvm::Instruction *> &mergeSet)
{
    llvm::SmallVector<llvm::Value *> inputs;
    if (auto storeInst = llvm::dyn_cast<llvm::StoreInst>(inst)) {
        inputs.push_back(storeInst->getValueOperand());
    } else if (auto callInst = llvm::dyn_cast<llvm::CallInst>(inst)) {
        inputs.append(callInst->arg_begin(), callInst->arg_end());
    } else {
        inputs.append(inst->value_op_begin(), inst->value_op_end());
    }
    for (auto input : inputs) {
        auto inputInst = llvm::dyn_cast<llvm::Instruction>(input);
        if (inputInst != nullptr && std::find(mergeSet.begin(), mergeSet.end(), inputInst) != mergeSet.end()) {
            return true;
        }
    }
    return false;
}

void MemBarriers::MergeBarriers(llvm::SmallVector<llvm::Instruction *> &mergeSet,
                                llvm::SmallVector<llvm::Instruction *> &needsBarrier)
{
    if (mergeSet.empty()) {
        return;
    }
    if (optimize_) {
        needsBarrier.push_back(mergeSet.back());
    } else {
        needsBarrier.append(mergeSet);
    }
    mergeSet.clear();
}

bool MemBarriers::SinkBarriers(llvm::BasicBlock &block, llvm::SmallVector<llvm::Instruction *> &mergeSet)
{
    if (mergeSet.empty()) {
        return false;  // no dmb to sink
    }

    const uint32_t kExpectedSuccessors = 2;

    auto *br = llvm::dyn_cast<llvm::BranchInst>(block.getTerminator());
    if (br == nullptr || !br->isConditional() || br->getNumSuccessors() != kExpectedSuccessors) {
        return false;  // Terminator is not br with 2 branches
    }

    // get two successor paths
    llvm::BasicBlock *path0 = br->getSuccessor(0);
    llvm::BasicBlock *path1 = br->getSuccessor(1);
    if (path0 == nullptr || path1 == nullptr) {
        return false;
    }

    // get if both paths are protected
    bool pp0 = IsProtectedPath(*path0, mergeSet);
    bool pp1 = IsProtectedPath(*path1, mergeSet);
    if (!pp0 && !pp1) {
        return false;  // both path are not protected, no need to sink
    }
    if (pp0 && pp1) {
        LLVM_DEBUG(llvm::dbgs() << "both paths are protected!\n");
        return true;  // both path are protected, don't need to add any dmb
    }
    if (!pp0) {
        std::swap(path0, path1);  // so that path0 is always the protected path
    }

    // dmb sink to path1 if and only if path1 have exactly 1 predecessor
    if (path1->getSinglePredecessor() == nullptr) {
        return false;
    }

    // get the instruction that grabs protected object, or encounter deoptimization call
    llvm::Instruction *instToInsert = &(path1->back());
    for (auto &inst : *path1) {
        if (inst.mayWriteToMemory() && GrabsGuarded(&inst, mergeSet)) {
            instToInsert = &inst;
            break;
        }
        auto call = llvm::dyn_cast<llvm::CallInst>(&inst);
        if (call == nullptr) {
            continue;
        }
        if (call->getIntrinsicID() == llvm::Intrinsic::experimental_deoptimize) {
            instToInsert = &inst;
            break;
        }
    }

    auto builder = llvm::IRBuilder<>(instToInsert);
    builder.CreateFence(llvm::AtomicOrdering::Release);
    LLVM_DEBUG(llvm::dbgs() << "one path is protected, sinking to the other...\n");
    return true;
}

bool MemBarriers::IsProtectedPath(llvm::BasicBlock &block, llvm::SmallVector<llvm::Instruction *> &mergeSet)
{
    llvm::SmallVector<llvm::Instruction *> curMergeSet;
    for (auto &inst : block) {
        auto callInst = llvm::dyn_cast<llvm::CallInst>(&inst);
        // add to current merge set (for use to check if path would be protected)
        if (callInst != nullptr && callInst->hasFnAttr("needs-mem-barrier")) {
            curMergeSet.push_back(callInst);
            continue;
        }
        // If the path is protected
        if (inst.mayWriteToMemory() && GrabsGuarded(&inst, curMergeSet)) {
            return true;
        }
        // If the path is not protected
        if (inst.mayWriteToMemory() && GrabsGuarded(&inst, mergeSet)) {
            return false;
        }
    }
    return false;
}

bool MemBarriers::RelaxBarriers(llvm::Function &function)
{
    if (!arkInterface_->IsArm64() || !optimize_) {
        return false;
    }

    auto opcode = llvm::Intrinsic::AARCH64Intrinsics::aarch64_dmb;
    auto dmb = llvm::Intrinsic::getDeclaration(function.getParent(), opcode, {});
    static constexpr uint32_t ISHST = 10U;
    auto ishst = llvm::ConstantInt::get(llvm::Type::getInt32Ty(function.getContext()), ISHST);

    bool changed = false;
    for (auto &basicBlock : function) {
        llvm::SmallVector<llvm::FenceInst *> fences;
        for (auto &instruction : basicBlock) {
            auto fence = llvm::dyn_cast<llvm::FenceInst>(&instruction);
            if (fence != nullptr && fence->getOrdering() == llvm::AtomicOrdering::Release) {
                fences.push_back(fence);
            }
        }
        for (auto fence : fences) {
            auto upgraded = llvm::CallInst::Create(dmb, {ishst}, llvm::None);
            llvm::ReplaceInstWithInst(fence, upgraded);
        }
        changed |= !fences.empty();
    }
    return changed;
}

}  // namespace ark::llvmbackend::passes
