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

#include "propagate_lenarray.h"

#include <llvm/Analysis/LoopInfo.h>
#include <llvm/Analysis/ScalarEvolution.h>
#include <llvm/Analysis/ScalarEvolutionExpressions.h>
#include <llvm/Analysis/MemoryBuiltins.h>
#include <llvm/Analysis/ValueTracking.h>
#include <llvm/IR/Dominators.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/InstIterator.h>
#include <llvm/IR/Operator.h>
#include <llvm/Support/KnownBits.h>
#include <llvm/Transforms/Utils/LoopPeel.h>

#include "transforms/builtins.h"

#define DEBUG_TYPE "propagate-lenarray"

namespace ark::llvmbackend::passes {

bool PropagateLenArray::ShouldInsert([[maybe_unused]] const ark::llvmbackend::LLVMCompilerOptions *options)
{
    return true;
}

llvm::PreservedAnalyses PropagateLenArray::run(llvm::Function &function,
                                               [[maybe_unused]] llvm::FunctionAnalysisManager &analysisManager)
{
    LLVM_DEBUG(llvm::dbgs() << "Running on '" << function.getName() << "'\n");

    llvm::DenseMap<llvm::CallInst *, llvm::ConstantInt *> replace {};
    for (auto &inst : llvm::instructions(function)) {
        auto call = llvm::dyn_cast<llvm::CallInst>(&inst);
        if (call == nullptr || call->getCalledFunction() != llvmbackend::builtins::LenArray(function.getParent())) {
            continue;
        }

        auto maybeAlloc = llvm::dyn_cast<llvm::CallInst>(call->getArgOperand(0));
        // Matching AllocateArrayTlab*: first arg is klass, second is size
        // NOTE(zdenis): Attribute for size
        // NOTE(zdenis): Support MultiArray
        if (maybeAlloc != nullptr && maybeAlloc->arg_size() == 2U && llvm::isAllocLikeFn(maybeAlloc, nullptr)) {
            auto maybeSize = llvm::dyn_cast<llvm::ConstantInt>(maybeAlloc->getArgOperand(1U));
            if (maybeSize != nullptr) {
                replace.insert({call, maybeSize});
            }
        }
    }

    bool changed = false;
    for (auto &[lenArray, size] : replace) {
        LLVM_DEBUG(llvm::dbgs() << "Replacing " << *lenArray << " with " << *size << "\n");
        auto cast = llvm::CastInst::Create(llvm::CastInst::Trunc, size,
                                           llvmbackend::builtins::LenArray(function.getParent())->getReturnType());
        cast->insertBefore(lenArray);
        lenArray->replaceAllUsesWith(cast);
        changed = true;
    }

    return changed ? llvm::PreservedAnalyses::none() : llvm::PreservedAnalyses::all();
}
}  // namespace ark::llvmbackend::passes
