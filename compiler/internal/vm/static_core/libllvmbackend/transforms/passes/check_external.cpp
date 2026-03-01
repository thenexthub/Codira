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

#include "transforms/builtins.h"
#include "transforms/passes/check_external.h"
#include "transforms/transform_utils.h"
#include "utils.h"
#include "llvm_ark_interface.h"
#include "llvm_compiler_options.h"

namespace ark::llvmbackend::passes {

CheckExternal CheckExternal::Create()
{
    return CheckExternal();
}

bool CheckExternal::ShouldInsert(const ark::llvmbackend::LLVMCompilerOptions *options)
{
    return options->inlining;
}

llvm::PreservedAnalyses CheckExternal::run(llvm::Function &function,
                                           llvm::FunctionAnalysisManager & /*analysis_manager*/)
{
    bool changed = false;
    llvm::SmallVector<llvm::CallInst *> remove;
    for (auto &block : function) {
        for (auto &instruction : block) {
            auto *call = llvm::dyn_cast<llvm::CallInst>(&instruction);
            if (call == nullptr) {
                continue;
            }
            auto callee = call->getCalledFunction();
            if (callee == nullptr || callee->isIntrinsic()) {
                continue;
            }
            // Remove calls to KeepThis here since it's a convenient place:
            // 1) It is a function pass
            // 2) It is after DeadArgElim
            // 3) It is before inlining
            if (callee == ark::llvmbackend::builtins::KeepThis(function.getParent())) {
                remove.push_back(call);
                continue;
            }
            if (call->hasFnAttr(llvm::Attribute::NoInline) && !callee->isDeclaration() &&
                !call->hasFnAttr("keep-noinline") && !ark::llvmbackend::utils::HasCallsWithDeopt(*callee)) {
                changed = true;
                call->removeAttributeAtIndex(llvm::AttributeList::FunctionIndex, llvm::Attribute::NoInline);
            }
        }
    }
    for (auto &inst : remove) {
        changed = true;
        inst->eraseFromParent();
    }
    return changed ? llvm::PreservedAnalyses::none() : llvm::PreservedAnalyses::all();
}

}  // namespace ark::llvmbackend::passes
