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

#include "transforms/passes/insert_safepoints.h"

#include "llvm_ark_interface.h"
#include "llvm_compiler_options.h"
#include "transforms/gc_utils.h"
#include "transforms/transform_utils.h"

#include <llvm/IR/Function.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/Pass.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/Transforms/Scalar.h>
#include <llvm/Transforms/Utils/Cloning.h>

#define DEBUG_TYPE "insert-safepoints"

using llvm::BasicBlock;
using llvm::CallInst;
using llvm::Function;
using llvm::Instruction;

// NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
static llvm::cl::opt<uint32_t> g_safepointOnEntryLimit("isp-on-entry-limit", llvm::cl::Hidden, llvm::cl::init(0));

static void InsertInlinedPoll(Function *poll, Instruction *point, bool afterPoint = false)
{
    CallInst *call;
    if (afterPoint) {
        call = llvm::CallInst::Create(poll);
        call->insertAfter(point);
    } else {
        call = llvm::CallInst::Create(poll, "", point);
    }
    llvm::InlineFunctionInfo info;
    // NOLINTNEXTLINE(clang-analyzer-cplusplus.NewDeleteLeaks)
    [[maybe_unused]] bool status = InlineFunction(*call, info).isSuccess();
    ASSERT(status && "Inlining of gc.safepoint_poll must succeed");
}

/// Insert an inlined poll if a function size is greater than g_safepointOnEntryLimit
static bool InsertSafepointOnEntry(Function &func, Function *poll)
{
    uint32_t count = 0;
    bool spOnEntry = false;
    for (BasicBlock &block : func) {
        count += block.size();
        if (count > g_safepointOnEntryLimit) {
            spOnEntry = true;
            break;
        }
    }
    if (spOnEntry) {
        auto insertInst = func.getEntryBlock().getFirstNonPHI();
        // Skip allocas so LLVM can merge them into prologue
        while (llvm::isa<llvm::AllocaInst>(insertInst)) {
            insertInst = insertInst->getNextNode();
        }
        InsertInlinedPoll(poll, insertInst);
    }
    return spOnEntry;
}

/// Insert inlined polls after instructions having attribute needs-extra-safepoint
static bool InsertSafepointAfterIntrinsics(Function &func, Function *poll)
{
    std::vector<CallInst *> calls;
    for (auto &block : func) {
        for (auto &inst : block) {
            auto call = llvm::dyn_cast<CallInst>(&inst);
            if (call != nullptr && call->hasFnAttr("needs-extra-safepoint")) {
                calls.push_back(call);
            }
        }
    }
    for (auto call : calls) {
        InsertInlinedPoll(poll, call, true);
    }
    return !calls.empty();
}

/// Run PlaceSafepoint using legacy pass manager as it hasn't been adapted to a new pass manager
static bool RunLegacyInserter(Function &func)
{
    llvm::legacy::FunctionPassManager manager(func.getParent());
    manager.add(llvm::createPlaceSafepointsPass());

    manager.doInitialization();
    auto changed = manager.run(func);
    manager.doFinalization();
    return changed;
}

namespace ark::llvmbackend::passes {

bool InsertSafepoints::ShouldInsert(const ark::llvmbackend::LLVMCompilerOptions *options)
{
    return options->useSafepoint;
}

llvm::PreservedAnalyses InsertSafepoints::run(llvm::Function &function,
                                              llvm::FunctionAnalysisManager & /*analysisManager*/)
{
    if (!gc_utils::IsGcFunction(function) || gc_utils::IsFunctionSupplemental(function)) {
        return llvm::PreservedAnalyses::all();
    }

    auto poll = function.getParent()->getFunction(ark::llvmbackend::LLVMArkInterface::GC_SAFEPOINT_POLL_NAME);

    auto changed = InsertSafepointOnEntry(function, poll);
    // It is a rare case when we need extra safe points. So optimize it by this way.
    if (function.hasFnAttribute("needs-extra-safepoint")) {
        changed |= InsertSafepointAfterIntrinsics(function, poll);
    }
    changed |= RunLegacyInserter(function);
    return changed ? llvm::PreservedAnalyses::none() : llvm::PreservedAnalyses::all();
}

}  // namespace ark::llvmbackend::passes
