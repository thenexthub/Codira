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

#include "llvm_compiler_options.h"
#include "transforms/passes/inline_devirt.h"
#include "transforms/transform_utils.h"
#include <llvm/Analysis/CGSCCPassManager.h>
#include <llvm/ADT/PriorityWorklist.h>

namespace ark::llvmbackend {
struct LLVMCompilerOptions;
}  // namespace ark::llvmbackend

namespace ark::llvmbackend::passes {

InlineDevirt InlineDevirt::Create(LLVMArkInterface *arkInterface, const ark::llvmbackend::LLVMCompilerOptions *options)
{
    return InlineDevirt(arkInterface, options->doVirtualInline);
}

bool InlineDevirt::ShouldInsert(const ark::llvmbackend::LLVMCompilerOptions *options)
{
    return options->inlining;
}

InlineDevirt::InlineDevirt(LLVMArkInterface *arkInterface, bool doVirtualInline)
    : arkInterface_ {arkInterface}, doVirtualInline_ {doVirtualInline}
{
}

void InlineDevirt::RunCheckExternal(ark::llvmbackend::passes::CheckExternal &externalPass)
{
    for (auto &node : *currentSCC_) {
        auto &func = node.getFunction();
        if (func.isDeclaration()) {
            continue;
        }
        [[maybe_unused]] auto shouldRunExternal =
            passInstrumentation_->runBeforePass<llvm::Function>(externalPass, func);
        ASSERT(shouldRunExternal);

        auto preservedAnalysesExternal = externalPass.run(func, *functionAnalysisManager_);
        passInstrumentation_->runAfterPass<llvm::Function>(externalPass, func, preservedAnalysesExternal);

        if (!preservedAnalysesExternal.areAllPreserved()) {
            currentSCC_ = &llvm::updateCGAndAnalysisManagerForCGSCCPass(
                *callGraph_, *currentSCC_, node, *analysisManager_, *updateResult_, *functionAnalysisManager_);
            functionAnalysisManager_->invalidate(func, preservedAnalysesExternal);
            preservedAnalyses_.intersect(std::move(preservedAnalysesExternal));
        }
    }
}

bool InlineDevirt::RunInlining(llvm::InlinerPass &inlinePass, llvm::SmallPtrSetImpl<llvm::Function *> &changedFunctions)
{
    [[maybe_unused]] auto shouldRunInlining =
        passInstrumentation_->runBeforePass<llvm::LazyCallGraph::SCC>(inlinePass, *currentSCC_);
    ASSERT(shouldRunInlining);

    auto preservedAnalysesInline = inlinePass.run(*currentSCC_, *analysisManager_, *callGraph_, *updateResult_);
    passInstrumentation_->runAfterPass<llvm::LazyCallGraph::SCC>(inlinePass, *currentSCC_, preservedAnalysesInline);

    ASSERT(updateResult_->InvalidatedSCCs.count(currentSCC_) == 0);
    if (!preservedAnalysesInline.areAllPreserved()) {
        for (auto &f : *currentSCC_) {
            changedFunctions.insert(&f.getFunction());
        }
        if (updateResult_->UpdatedC != nullptr && updateResult_->UpdatedC != currentSCC_) {
            currentSCC_ = updateResult_->UpdatedC;
        }
        // Function analyses get invalidated in inlinePass.run
        auto *resultFAMCP =
            &analysisManager_->getResult<llvm::FunctionAnalysisManagerCGSCCProxy>(*currentSCC_, *callGraph_);
        resultFAMCP->updateFAM(*functionAnalysisManager_);
        analysisManager_->invalidate(*currentSCC_, preservedAnalysesInline);
        preservedAnalyses_.intersect(std::move(preservedAnalysesInline));
        return true;
    }
    return false;
}

bool InlineDevirt::RunDevirt(ark::llvmbackend::passes::Devirt &devirtPass)
{
    auto devirtChanged = false;
    for (auto &node : *currentSCC_) {
        auto &func = node.getFunction();
        if (func.isDeclaration()) {
            continue;
        }
        [[maybe_unused]] auto shouldRunDevirt = passInstrumentation_->runBeforePass<llvm::Function>(devirtPass, func);
        ASSERT(shouldRunDevirt);

        auto preservedAnalysesDevirt = devirtPass.run(func, *functionAnalysisManager_);
        passInstrumentation_->runAfterPass<llvm::Function>(devirtPass, func, preservedAnalysesDevirt);

        if (!preservedAnalysesDevirt.areAllPreserved()) {
            devirtChanged = true;
            currentSCC_ = &llvm::updateCGAndAnalysisManagerForCGSCCPass(
                *callGraph_, *currentSCC_, node, *analysisManager_, *updateResult_, *functionAnalysisManager_);
            functionAnalysisManager_->invalidate(func, preservedAnalysesDevirt);
            preservedAnalyses_.intersect(std::move(preservedAnalysesDevirt));
        }
    }
    return devirtChanged;
}

llvm::PreservedAnalyses InlineDevirt::run(llvm::LazyCallGraph::SCC &initialSCC,
                                          llvm::CGSCCAnalysisManager &analysisManager, llvm::LazyCallGraph &callGraph,
                                          llvm::CGSCCUpdateResult &updateResult)
{
    passInstrumentation_ = &analysisManager.getResult<llvm::PassInstrumentationAnalysis>(initialSCC, callGraph);
    functionAnalysisManager_ =
        &analysisManager.getResult<llvm::FunctionAnalysisManagerCGSCCProxy>(initialSCC, callGraph).getManager();
    preservedAnalyses_ = llvm::PreservedAnalyses::all();
    updateResult_ = &updateResult;
    currentSCC_ = &initialSCC;
    analysisManager_ = &analysisManager;
    callGraph_ = &callGraph;

    llvm::InlinerPass inlinePass(false);
    ark::llvmbackend::passes::Devirt devirtPass(arkInterface_);
    ark::llvmbackend::passes::CheckExternal externalPass;
    static constexpr auto CHANGED_FUNCTIONS_SIZE = 16U;
    llvm::SmallPtrSet<llvm::Function *, CHANGED_FUNCTIONS_SIZE> changedFunctions;

    static constexpr auto MAX_ITERATIONS = 8U;
    for (uint32_t i = 0; i < MAX_ITERATIONS; i++) {
        RunCheckExternal(externalPass);
        auto inlineChanged = RunInlining(inlinePass, changedFunctions);
        auto devirtChanged = doVirtualInline_ && RunDevirt(devirtPass);
        // run CheckExternal and Devirt once more only if IR has changed
        if (!inlineChanged && !devirtChanged) {
            break;
        }
    }

    for (auto func : changedFunctions) {
        for (auto user : func->users()) {
            auto call = llvm::dyn_cast<llvm::CallInst>(user);
            if (call != nullptr && call->getCalledFunction() == func && call->getFunction() != func &&
                arkInterface_->IsExternal(call)) {
                auto parent = call->getFunction();
                auto pnode = callGraph_->lookup(*parent);
                ASSERT(pnode != nullptr);
                auto pscc = callGraph_->lookupSCC(*pnode);
                ASSERT(pscc != nullptr);
                updateResult.CWorklist.insert(pscc);
            }
        }
    }
    return preservedAnalyses_;
}

}  // namespace ark::llvmbackend::passes
