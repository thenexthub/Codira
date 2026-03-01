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
#include "transforms/passes/devirt.h"
#include "transforms/transform_utils.h"
#include "utils.h"
#include "llvm_ark_interface.h"
#include "llvm_compiler_options.h"

namespace ark::llvmbackend::passes {

Devirt Devirt::Create(LLVMArkInterface *arkInterface,
                      [[maybe_unused]] const ark::llvmbackend::LLVMCompilerOptions *options)
{
    return Devirt(arkInterface);
}

Devirt::Devirt(LLVMArkInterface *arkInterface) : arkInterface_ {arkInterface} {}

bool Devirt::ShouldInsert(const ark::llvmbackend::LLVMCompilerOptions *options)
{
    return options->doVirtualInline;
}

llvm::ConstantInt *GetObjectClassId(llvm::CallInst *call)
{
    auto thisArg = llvm::dyn_cast<llvm::Instruction>(call->getArgOperand(1));
    if (thisArg == nullptr) {
        return nullptr;
    }
    auto allocate = llvm::dyn_cast<llvm::CallInst>(thisArg);
    // NOTE: handle entrypoints
    if (allocate == nullptr || allocate->arg_size() == 0) {
        return nullptr;
    }
    auto loadAndInit = llvm::dyn_cast<llvm::CallInst>(allocate->getArgOperand(0));
    // NOTE: handle entrypoints
    if (loadAndInit == nullptr) {
        return nullptr;
    }
    // NOTE: support more sophisticated cases (maybe with type propagation)
    auto loadAndInitFunc = loadAndInit->getCalledFunction();
    auto module = call->getModule();
    if (loadAndInitFunc != ark::llvmbackend::builtins::LoadInitClass(module)) {
        return nullptr;
    }
    auto *objectKlassId = llvm::dyn_cast<llvm::ConstantInt>(loadAndInit->getArgOperand(0));
    if (objectKlassId == nullptr) {
        return nullptr;
    }
    return objectKlassId;
}

llvm::PreservedAnalyses Devirt::run(llvm::Function &function, llvm::FunctionAnalysisManager & /*analysis_manager*/)
{
    ASSERT(arkInterface_ != nullptr);
    bool changed = false;
    for (auto &block : function) {
        for (auto &instruction : block) {
            auto *call = llvm::dyn_cast<llvm::CallInst>(&instruction);
            if (call == nullptr || call->getCalledFunction() == nullptr || call->arg_size() < 2U ||
                call->getCalledFunction()->isIntrinsic()) {
                continue;
            }

            if (arkInterface_->IsRememberedCall(call->getFunction(), call->getCalledFunction())) {
                continue;
            }

            auto *objectKlassId = GetObjectClassId(call);
            if (objectKlassId == nullptr) {
                continue;
            }

            auto methodPtr = arkInterface_->ResolveVirtual(objectKlassId->getZExtValue(), call);
            if (methodPtr == nullptr) {
                continue;
            }

            auto methodName = arkInterface_->GetUniqMethodName(methodPtr);
            auto func = function.getParent()->getFunction(methodName);
            if (func == nullptr || func == call->getCalledFunction() || func->isDeclaration()) {
                continue;
            }

            auto ftype = call->getFunctionType();
            call->setCalledFunction(ftype, func);
            if (arkInterface_->IsExternal(call) && ark::llvmbackend::utils::HasCallsWithDeopt(*func)) {
                call->addAttributeAtIndex(llvm::AttributeList::FunctionIndex, llvm::Attribute::NoInline);
            }

            arkInterface_->PutVirtualFunction(methodPtr, func);
            changed = true;
        }
    }
    return changed ? llvm::PreservedAnalyses::none() : llvm::PreservedAnalyses::all();
}

}  // namespace ark::llvmbackend::passes
