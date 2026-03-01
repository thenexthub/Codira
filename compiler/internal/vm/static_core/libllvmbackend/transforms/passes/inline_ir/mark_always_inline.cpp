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

#include "mark_always_inline.h"
#include "llvm_ark_interface.h"
#include "llvm_compiler_options.h"

#include <llvm/Pass.h>
#include <llvm/IR/InlineAsm.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Transforms/Utils/BasicBlockUtils.h>
#include <llvm/Transforms/IPO/FunctionImport.h>

#include "transforms/transform_utils.h"

#define DEBUG_TYPE "mark-always-inline"

using llvm::Attribute;
using llvm::AttributeList;
using llvm::CallInst;
using llvm::dyn_cast;
using llvm::Function;
using llvm::StringRef;

namespace ark::llvmbackend::passes {

bool MarkAlwaysInline::ShouldInsert(const ark::llvmbackend::LLVMCompilerOptions *options)
{
    return options->doIrtocInline;
}

llvm::PreservedAnalyses MarkAlwaysInline::run(llvm::Function &function, llvm::FunctionAnalysisManager & /*AM*/)
{
    if (function.hasMetadata(LLVMArkInterface::FUNCTION_MD_INLINE_MODULE)) {
        return llvm::PreservedAnalyses::all();
    }

    // Experiments showed best improvement on this inlining level
    static constexpr int32_t DEFAULT_MAX_INLINING_LEVEL = 2;

    auto changed = InlineCallTree(&function, 0, DEFAULT_MAX_INLINING_LEVEL);
    return changed ? llvm::PreservedAnalyses::none() : llvm::PreservedAnalyses::all();
}

bool MarkAlwaysInline::InlineCallTree(Function *function, int32_t level, int32_t maxLevel)
{
    ASSERT(function != nullptr);

    if (level == maxLevel) {
        return false;
    }
    ASSERT(level < maxLevel);

    bool changed = false;
    for (auto &basicBlock : *function) {
        for (auto &instruction : basicBlock) {
            auto call = dyn_cast<CallInst>(&instruction);
            if (call == nullptr) {
                continue;
            }
            if (call->hasFnAttr(Attribute::AlwaysInline) || call->hasFnAttr(Attribute::NoInline)) {
                continue;
            }
            auto calledFunction = call->getCalledFunction();
            static constexpr std::array EXCLUSIONS = {
                // Because:
                // 1. Called in all interpreter handlers
                // 2. ~x5 compilation time increase
                StringRef("DebugPrintEntrypoint"),  //
            };
            if (calledFunction == nullptr || calledFunction->isDeclaration() ||
                std::find(EXCLUSIONS.cbegin(), EXCLUSIONS.cend(), calledFunction->getName()) != EXCLUSIONS.cend()) {
                continue;
            }
            call->addAttributeAtIndex(AttributeList::FunctionIndex, Attribute::AlwaysInline);
            changed = true;
            LLVM_DEBUG(llvm::dbgs() << "Set AlwaysInline to a call. Caller = '" << call->getFunction()->getName()
                                    << "', callee = '" << call->getCalledFunction()->getName() << "'\n");
            InlineCallTree(call->getCalledFunction(), level + 1, maxLevel);
        }
    }
    return changed;
}
}  // namespace ark::llvmbackend::passes
