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

#include "fixup_poisons.h"

#include <llvm/IR/Constants.h>

#include "transforms/gc_utils.h"

#define DEBUG_TYPE "fixup-poisons"

using ark::llvmbackend::gc_utils::IsGcRefType;

namespace ark::llvmbackend::passes {

FixupPoisons::FixupPoisons() = default;

bool FixupPoisons::FixupInstructionOperands(llvm::Instruction &instruction)
{
    bool changed = false;
    for (auto operand : instruction.operand_values()) {
        auto poison = llvm::dyn_cast<llvm::PoisonValue>(operand);
        if (poison == nullptr) {
            continue;
        }
        if (IsGcRefType(poison->getType())) {
            LLVM_DEBUG(llvm::dbgs() << "Replacing poison in inst:\n");
            LLVM_DEBUG(instruction.print(llvm::dbgs()));
            LLVM_DEBUG(llvm::dbgs() << "with null pointer");

            auto replacement = llvm::Constant::getNullValue(poison->getType());
            poison->replaceAllUsesWith(replacement);
            changed = true;
        }
    }
    return changed;
}

llvm::PreservedAnalyses FixupPoisons::run(llvm::Function &function,
                                          [[maybe_unused]] llvm::FunctionAnalysisManager &analysisManager)
{
    bool changed = false;

    for (auto &basicBlock : function) {
        for (auto &instruction : basicBlock) {
            changed |= FixupInstructionOperands(instruction);
        }
    }

    return changed ? llvm::PreservedAnalyses::none() : llvm::PreservedAnalyses::all();
}

}  // namespace ark::llvmbackend::passes
