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

#include "ark_inlining.h"
#include "llvm_compiler_options.h"

#include <llvm/Analysis/InlineAdvisor.h>
#include <llvm/Analysis/ReplayInlineAdvisor.h>

#include <llvm/IR/InstIterator.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/IntrinsicInst.h>
#include <llvm/Demangle/Demangle.h>

#include <vector>

namespace ark::llvmbackend::passes {

bool InlinePrepare::ShouldInsert(const ark::llvmbackend::LLVMCompilerOptions *options)
{
    return options->inlining;
}

InlinePrepare InlinePrepare::Create([[maybe_unused]] LLVMArkInterface *arkInterface,
                                    const ark::llvmbackend::LLVMCompilerOptions *options)
{
    static constexpr int INLINING_THRESHOLD = 500;
    auto inlineParams = llvm::getInlineParams(INLINING_THRESHOLD);
    inlineParams.AllowRecursiveCall = options->recursiveInlining;
    return InlinePrepare(inlineParams);
}

llvm::PreservedAnalyses InlinePrepare::run(llvm::Module &module, llvm::ModuleAnalysisManager &moduleAm)
{
    auto &advisorResult = moduleAm.getResult<llvm::InlineAdvisorAnalysis>(module);
    if (!advisorResult.tryCreate(
            inlineParams_, llvm::InliningAdvisorMode::Default, {},  // CC-OFF(G.FMT.06-CPP) project code style
            llvm::InlineContext {llvm::ThinOrFullLTOPhase::None, llvm::InlinePass::ModuleInliner})) {
        module.getContext().emitError("Could not setup Inlining Advisor for the requested mode and/or options");
    }
    return llvm::PreservedAnalyses::all();
}

bool IrtocInlineChecker::ShouldInsert(const ark::llvmbackend::LLVMCompilerOptions *options)
{
    return options->doIrtocInline;
}

void IrtocInlineChecker::CheckShouldInline(llvm::CallBase *callBase)
{
    using llvm::StringRef;
    if (!callBase->hasFnAttr(llvm::Attribute::AlwaysInline)) {
        return;
    }
    auto caller = callBase->getCaller();
    auto callee = callBase->getCalledFunction();
    std::string msg = "Unknown reason";
    if (callBase->hasFnAttr("inline-remark")) {
        msg = callBase->getFnAttr("inline-remark").getValueAsString();
    }

    auto demCallerName = llvm::demangle(std::string(caller->getName()));
    if (callee == nullptr) {
        llvm::report_fatal_error(llvm::Twine("Can't inline with alwaysinline attr  'nullptr") + "' into '" +
                                 demCallerName + " due to " + msg + "'");
        return;
    }
    auto demCalleeName = llvm::demangle(std::string(callee->getName()));
#ifdef __SANITIZE_THREAD__
    // The functions from EXCLUSIONS are come from panda runtime (array-inl.h and class.h)
    // These function are recursive (Thay are optimized by tail recursive in normal way
    // but not if PANDA_ENABLE_THREAD_SANITIZER)
    static constexpr std::array EXCLUSIONS = {StringRef("ark::coretypes::Array::CreateMultiDimensionalArray"),
                                              StringRef("ark::Class::IsAssignableFrom(ark::Class const*)")};
    if (std::find_if(EXCLUSIONS.cbegin(), EXCLUSIONS.cend(), [demCalleeName](StringRef pat) {
            return demCalleeName.find(pat) != std::string::npos;
        }) == EXCLUSIONS.cend()) {
        llvm::report_fatal_error(llvm::Twine("Can't inline with alwaysinline attr '") + demCalleeName + "' into '" +
                                 demCallerName + "' due to '" + msg + "'");
    }
#else
    llvm::report_fatal_error(llvm::Twine("Can't inline with alwaysinline attr '") + demCalleeName + "' into '" +
                             demCallerName + "' due to '" + msg + "'");
#endif
}

llvm::PreservedAnalyses IrtocInlineChecker::run(llvm::LazyCallGraph::SCC &component,
                                                llvm::CGSCCAnalysisManager & /*unused*/,
                                                llvm::LazyCallGraph & /*unused*/, llvm::CGSCCUpdateResult & /*unused*/)
{
    for (const auto &node : component) {
        auto &func = node.getFunction();
        if (func.isDeclaration()) {
            continue;
        }

        for (llvm::Instruction &inst : llvm::instructions(func)) {
            auto *callBase = llvm::dyn_cast<llvm::CallBase>(&inst);
            if (callBase == nullptr || llvm::isa<llvm::IntrinsicInst>(&inst)) {
                continue;
            }
            llvm::Function *callee = callBase->getCalledFunction();
            if (callee == nullptr || callee->isDeclaration()) {
                continue;
            }
            CheckShouldInline(callBase);
        }
    }

    return llvm::PreservedAnalyses::all();
}

}  // namespace ark::llvmbackend::passes
