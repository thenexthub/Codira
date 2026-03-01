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

#include "check_tail_calls.h"
#include <llvm/CodeGen/TargetInstrInfo.h>
#include <llvm/CodeGen/MachineFunctionPass.h>
#include <llvm/IR/Instructions.h>

#include "transforms/transform_utils.h"

#define DEBUG_TYPE "check-tail-calls"

namespace {

/**
 * Used to detect cycles while walking on CFG
 *
 * @tparam T type of set elements
 */
template <typename T>
class ScopedSetElement final {
public:
    explicit ScopedSetElement(llvm::SmallPtrSetImpl<T> *set, T value) : set_(set), value_(value)
    {
        ASSERT(set != nullptr);
        ASSERT(value != nullptr);
        set->insert(value);
    }

    ~ScopedSetElement()
    {
        set_->erase(value_);
    }

    ScopedSetElement(const ScopedSetElement &) = delete;
    ScopedSetElement &operator=(const ScopedSetElement &) = delete;
    ScopedSetElement(ScopedSetElement &&) = delete;
    ScopedSetElement &operator=(ScopedSetElement &&) = delete;

private:
    llvm::SmallPtrSetImpl<T> *set_;
    T value_;
};

using VisitedBasicBlocks = llvm::SmallPtrSet<llvm::MachineBasicBlock *, 4U>;
using VisitedBasicBlockElement = ScopedSetElement<VisitedBasicBlocks::value_type>;

// This pass checks: 1) tail calls in interpreter handlers 2) calls to SlowPathes made in Irtoc FastPathes (they
// should be tail calls to avoid miscompilation). `CheckTailCallsPass` checks that llvm was able to lower
// them into machine code correspondingly, not falling back to regular calls.
class CheckTailCallsPass : public llvm::MachineFunctionPass {
public:
    static constexpr llvm::StringRef PASS_NAME = "Check ARK Tail Calls";
    static constexpr llvm::StringRef ARG_NAME = "check-ark-tail-calls";

    explicit CheckTailCallsPass() : MachineFunctionPass(ID) {}

    llvm::StringRef getPassName() const override
    {
        return PASS_NAME;
    }

    // Almost exact copy of `getTerminatingMustTailCall` from llvm
    const llvm::CallInst *GetTerminatingTailCall(const llvm::BasicBlock *bb) const
    {
        if (bb->empty()) {
            return nullptr;
        }

        auto returnInst = llvm::dyn_cast<llvm::ReturnInst>(&bb->back());
        if (returnInst == nullptr || returnInst == &bb->front()) {
            return nullptr;
        }

        const llvm::Instruction *prev = returnInst->getPrevNode();
        if (prev == nullptr) {
            return nullptr;
        }

        if (llvm::Value *returnValue = returnInst->getReturnValue()) {
            if (returnValue != prev) {
                return nullptr;
            }
            // Look through the optional bitcast
            if (auto *bitcastInst = llvm::dyn_cast<llvm::BitCastInst>(prev)) {
                returnValue = bitcastInst->getOperand(0);
                prev = bitcastInst->getPrevNode();
                if (prev == nullptr || returnValue != prev) {
                    return nullptr;
                }
            }
        }
        if (auto *callInst = llvm::dyn_cast<llvm::CallInst>(prev)) {
            if (callInst->isTailCall()) {
                return callInst;
            }
        }
        return nullptr;
    }

    static bool IsRealTailCall(llvm::MachineBasicBlock *basicBlock, VisitedBasicBlocks *visitedBasicBlocks)
    {
        auto *instInfo = basicBlock->getParent()->getSubtarget().getInstrInfo();
        if (llvm::all_of(basicBlock->terminators(),
                         [&instInfo](llvm::MachineInstr &term) { return instInfo->isTailCall(term); })) {
            return true;
        }
        if (visitedBasicBlocks->contains(basicBlock)) {
            llvm::report_fatal_error("Cycle in CFG in '" + basicBlock->getParent()->getName() +
                                     "' prevents tail call check");
        }
        VisitedBasicBlockElement visitedBasicBlockElement {visitedBasicBlocks, basicBlock};
        return llvm::all_of(basicBlock->successors(), [&visitedBasicBlocks](llvm::MachineBasicBlock *succ) {
            return IsRealTailCall(succ, visitedBasicBlocks);
        });
    }

    bool runOnMachineFunction(llvm::MachineFunction &machineFunction) override
    {
        llvm::SmallSet<const llvm::Instruction *, 4U> confirmedTailCalls;
        for (auto &basicBlock : machineFunction) {
            auto irBasicBlock = basicBlock.getBasicBlock();
            if (irBasicBlock == nullptr) {
                continue;
            }
            auto callInst = GetTerminatingTailCall(irBasicBlock);
            if (callInst != nullptr && callInst->hasFnAttr("ark-tail-call")) {
                VisitedBasicBlocks visitedBasicBlocks;
                if (IsRealTailCall(&basicBlock, &visitedBasicBlocks)) {
                    confirmedTailCalls.insert(callInst);
                } else {
                    llvm::report_fatal_error("Cannot find tail call for  '" + machineFunction.getName() + "'");
                }
                ASSERT(visitedBasicBlocks.empty());
            }
        }
        for (auto &irBasicBlock : machineFunction.getFunction()) {
            for (auto &irInst : irBasicBlock) {
                auto *callInst = llvm::dyn_cast<llvm::CallInst>(&irInst);
                if (callInst != nullptr && callInst->hasFnAttr("ark-tail-call") &&
                    !confirmedTailCalls.contains(callInst)) {
                    llvm::report_fatal_error("Missing tail call in '" + machineFunction.getName() + "'");
                }
            }
        }
        return false;
    }

    static inline char ID = 0;  // NOLINT(readability-identifier-naming)
};
}  // namespace

llvm::MachineFunctionPass *ark::llvmbackend::CreateCheckTailCallsPass()
{
    return new CheckTailCallsPass();
}

// NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
static llvm::RegisterPass<CheckTailCallsPass> g_ctc(CheckTailCallsPass::ARG_NAME, CheckTailCallsPass::PASS_NAME, false,
                                                    false);
