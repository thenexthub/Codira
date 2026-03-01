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

#include "aarch64_fixup_sdiv.h"

#include "llvm_ark_interface.h"

#include "transforms/transform_utils.h"

#define DEBUG_TYPE "aarch64-fixup-sdiv"

namespace ark::llvmbackend::passes {

AArch64FixupSDiv AArch64FixupSDiv::Create(LLVMArkInterface *arkInterface,
                                          [[maybe_unused]] const LLVMCompilerOptions *options)
{
    return AArch64FixupSDiv(arkInterface);
}

bool AArch64FixupSDiv::ReplaceSelect(llvm::Instruction *selectInst)
{
    auto cmp = llvm::cast<llvm::Instruction>(selectInst->getOperand(0U));
    auto sub = selectInst->getOperand(1U);
    auto sdiv = selectInst->getOperand(2U);

    // sub instruction may be replaced with value
    auto sdivInst = llvm::cast<llvm::Instruction>(sdiv);

    selectInst->replaceAllUsesWith(sdivInst);
    sdivInst->takeName(selectInst);
    ASSERT(!sdivInst->getDebugLoc());
    sdivInst->setDebugLoc(selectInst->getDebugLoc());
    selectInst->eraseFromParent();
    if (cmp->uses().empty()) {
        cmp->eraseFromParent();
    }
    if (sub->uses().empty() && llvm::isa<llvm::Instruction>(sub)) {
        auto subInst = llvm::cast<llvm::Instruction>(sub);
        subInst->eraseFromParent();
    }
    return true;
}

AArch64FixupSDiv::AArch64FixupSDiv(LLVMArkInterface *arkInterface) : arkInterface_(arkInterface) {}

llvm::PreservedAnalyses AArch64FixupSDiv::run(llvm::Function &function,
                                              [[maybe_unused]] llvm::FunctionAnalysisManager &analysisManager)
{
    if (!arkInterface_->IsArm64()) {
        return llvm::PreservedAnalyses::all();
    }
    bool changed = false;
    for (auto &basicBlock : function) {
        llvm::SmallVector<llvm::Instruction *> selectInsts;
        for (auto &instruction : basicBlock) {
            if (instruction.hasMetadata(LLVMArkInterface::AARCH64_SDIV_INST)) {
                ASSERT(instruction.getOpcode() == llvm::Instruction::Select);
                selectInsts.push_back(&instruction);
            }
        }
        for (auto selectInst : selectInsts) {
            changed |= ReplaceSelect(selectInst);
        }
    }

    return changed ? llvm::PreservedAnalyses::none() : llvm::PreservedAnalyses::all();
}

}  // namespace ark::llvmbackend::passes
