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

#include "transforms/passes/prune_deopt.h"
#include "transforms/transform_utils.h"

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/InlineAsm.h>
#include <llvm/IR/MDBuilder.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Pass.h>
#include <llvm/Transforms/Utils/BasicBlockUtils.h>

#define DEBUG_TYPE "prune-deopt"

// Basic classes
using llvm::ArrayRef;
using llvm::BasicBlock;
using llvm::Function;
using llvm::FunctionAnalysisManager;
using llvm::OperandBundleDef;
using llvm::OperandBundleUse;
using llvm::Use;
// Instructions
using llvm::CallInst;
using llvm::ConstantInt;
using llvm::Instruction;

namespace ark::llvmbackend::passes {

llvm::PreservedAnalyses PruneDeopt::run(Function &function, FunctionAnalysisManager & /*analysisManager*/)
{
    LLVM_DEBUG(llvm::dbgs() << "Pruning Deopts for: " << function.getName() << "\n");
    bool changed = false;
    for (auto &block : function) {
        for (auto iter = block.begin(); iter != block.end();) {
            auto &inst = *iter;
            iter++;
            auto call = llvm::dyn_cast<CallInst>(&inst);
            if (call == nullptr) {
                continue;
            }
            auto bundle = call->getOperandBundle(llvm::LLVMContext::OB_deopt);
            if (bundle == llvm::None) {
                continue;
            }
            auto noReturn = IsNoReturn(bundle->Inputs);
            auto updated = GetUpdatedCallInst(call, bundle.getValue());
            changed = true;
            if (noReturn) {
                MakeUnreachableAfter(&block, updated);
                break;
            }
        }
    }
    return changed ? llvm::PreservedAnalyses::none() : llvm::PreservedAnalyses::all();
}

CallInst *PruneDeopt::GetUpdatedCallInst(CallInst *call, const OperandBundleUse &bundle)
{
    CallInst *updated;
    if (!IsCaughtDeoptimization(bundle.Inputs) && !call->hasFnAttr("may-deoptimize")) {
        LLVM_DEBUG(llvm::dbgs() << "Pruning deopt for: " << *call << "\n");
        ASSERT(call->getNumOperandBundles() == 1);
        OperandBundleDef emptyBundle {"deopt", llvm::None};
        updated = CallInst::Create(call, emptyBundle);
        auto iinfo = GetInlineInfo(bundle.Inputs);
        if (!iinfo.empty()) {
            updated->addFnAttr(llvm::Attribute::get(updated->getContext(), "inline-info", iinfo));
        }
    } else {
        LLVM_DEBUG(llvm::dbgs() << "Encoding deopt bundle: " << *call << "\n");
        ASSERT(call->getNumOperandBundles() == 1);
        OperandBundleDef encodedBundle {"deopt", EncodeDeoptBundle(call, bundle)};
        updated = CallInst::Create(call, {encodedBundle});
    }
    ReplaceInstWithInst(call, updated);
    LLVM_DEBUG(llvm::dbgs() << "Replaced with: " << *updated << "\n");
    return updated;
}

bool PruneDeopt::IsCaughtDeoptimization(ArrayRef<Use> inputs) const
{
    constexpr auto CAUGHT_FLAG_IDX = 3;
    for (uint32_t i = 0; i < inputs.size(); ++i) {
        if (llvm::isa<Function>(inputs[i])) {
            ASSERT((i + CAUGHT_FLAG_IDX) < inputs.size());
            uint32_t tryFlag = llvm::cast<ConstantInt>(inputs[i + CAUGHT_FLAG_IDX])->getZExtValue();
            if ((tryFlag & 1U) > 0) {
                return true;
            }
        }
    }
    return false;
}

bool PruneDeopt::IsNoReturn(ArrayRef<Use> inputs) const
{
    constexpr auto CAUGHT_FLAG_IDX = 3;
    for (uint32_t i = 0; i < inputs.size(); ++i) {
        if (llvm::isa<Function>(inputs[i])) {
            ASSERT((i + CAUGHT_FLAG_IDX) < inputs.size());
            uint32_t tryFlag = llvm::cast<ConstantInt>(inputs[i + CAUGHT_FLAG_IDX])->getZExtValue();
            if ((tryFlag & 2U) > 0) {
                return true;
            }
        }
    }
    return false;
}

PruneDeopt::EncodedDeoptBundle PruneDeopt::EncodeDeoptBundle(CallInst *call, const OperandBundleUse &bundle) const
{
    EncodedDeoptBundle encoded;
    // Reserve place for function counter
    encoded.push_back(nullptr);
    // Reserve space for function indexes
    for (const auto &ops : bundle.Inputs) {
        if (llvm::isa<Function>(ops)) {
            encoded.push_back(nullptr);
        }
    }
    bool mayBeDeoptIf = call->hasFnAttr("may-deoptimize");
    llvm::IRBuilder<> builder(call);
    // Set amount of functions
    encoded[0] = builder.getInt32(encoded.size() - 1);
    auto functionIndex = 1;
    constexpr auto REGMAP_FLAG = 1U;
    constexpr auto REGMAP_FLAG_IDX = 3;
    size_t offs = REGMAP_FLAG_IDX;
    for (const auto &ops : bundle.Inputs) {
        if (llvm::isa<Function>(ops)) {
            // Record position of the next function
            encoded[functionIndex++] = builder.getInt32(encoded.size());
            offs = 0;
        } else {
            offs++;
            if (offs == REGMAP_FLAG_IDX && mayBeDeoptIf) {
                encoded.push_back(builder.getInt32(llvm::cast<ConstantInt>(ops)->getZExtValue() | REGMAP_FLAG));
            } else {
                encoded.push_back(ops);
            }
        }
    }
    return encoded;
}

std::string PruneDeopt::GetInlineInfo(ArrayRef<Use> inputs) const
{
    constexpr auto METHOD_ID_IDX = 1;
    std::string inlineInfo;
    for (uint32_t i = 0; i < inputs.size(); i++) {
        if (llvm::isa<Function>(inputs[i])) {
            ASSERT((i + METHOD_ID_IDX) < inputs.size());
            if (!inlineInfo.empty()) {
                inlineInfo.append(",");
            }

            auto methodId = llvm::cast<ConstantInt>(inputs[i + METHOD_ID_IDX])->getZExtValue();
            inlineInfo.append(std::to_string(methodId));
        }
    }
    return inlineInfo;
}

void PruneDeopt::MakeUnreachableAfter(BasicBlock *block, Instruction *after) const
{
    // Remove the BLOCK from phi instructions
    for (auto succ : successors(block)) {
        for (auto phii = succ->phis().begin(), end = succ->phis().end(); phii != end;) {
            auto &phi = *phii++;
            auto idx = phi.getBasicBlockIndex(block);
            if (idx != -1) {
                phi.removeIncomingValue(idx);
            }
        }
    }
    auto maybeCall = llvm::dyn_cast<llvm::CallInst>(after);
    auto deoptimize = maybeCall != nullptr && maybeCall->getIntrinsicID() == llvm::Intrinsic::experimental_deoptimize;

    // Remove all instructions after AFTER
    for (auto iter = block->rbegin(); (&(*iter) != after);) {
        auto &toRemove = *iter++;
        Instruction *inst = &toRemove;
        // Do not remove ret instruction after deoptimize call
        if (deoptimize && toRemove.isTerminator()) {
            continue;
        }
        inst->replaceAllUsesWith(llvm::UndefValue::get(inst->getType()));
        inst->eraseFromParent();
    }
    if (!deoptimize) {
        // Create unreachable after AFTER
        llvm::IRBuilder<> builder(block);
        builder.CreateUnreachable();
    }
}
}  // namespace ark::llvmbackend::passes
