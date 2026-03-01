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

#include "libarkbase/macros.h"
#include "utils.h"

namespace ark::llvmbackend::utils {

int64_t GetMethodIdFromAttr(llvm::CallInst *call)
{
    ASSERT(call != nullptr);
    ASSERT(call->hasFnAttr("original-method-id"));
    auto attr = call->getFnAttr("original-method-id");
    int64_t integer = -1;
    [[maybe_unused]] auto error = attr.getValueAsString().getAsInteger(0, integer);
    ASSERT(!error);
    ASSERT(integer >= 0);
    return integer;
}

bool HasCallsWithDeopt(llvm::Function &func)
{
    for (auto &block : func) {
        for (auto &instruction : block) {
            auto call = llvm::dyn_cast<llvm::CallInst>(&instruction);
            if (call != nullptr && call->getOperandBundle(llvm::LLVMContext::OB_deopt)) {
                return true;
            }
        }
    }
    return false;
}

llvm::SmallVector<llvm::OperandBundleDef> CopyDeoptBundle(llvm::CallInst *from)
{
    llvm::SmallVector<llvm::OperandBundleDef, 1> bundles;
    auto deoptBundle = from->getOperandBundle(llvm::LLVMContext::OB_deopt);
    if (deoptBundle) {
        llvm::SmallVector<llvm::Value *, 0> deoptVals;
        for (auto &user : deoptBundle->Inputs) {
            deoptVals.push_back(user.get());
        }
        bundles.push_back(llvm::OperandBundleDef {"deopt", deoptVals});
    }
    return bundles;
}

void CopyDebugLoc(llvm::CallInst *from, llvm::CallInst *to)
{
    auto &debugLoc = from->getDebugLoc();
    auto line = debugLoc ? debugLoc.getLine() : 0;
    auto inlinedAt = debugLoc ? debugLoc.getInlinedAt() : nullptr;
    auto func = from->getParent()->getParent();
    to->setDebugLoc(llvm::DILocation::get(func->getContext(), line, 1, func->getSubprogram(), inlinedAt));
}

llvm::Value *CreateLoadClassFromObject(llvm::Value *object, llvm::IRBuilder<> *builder,
                                       ark::llvmbackend::LLVMArkInterface *arkInterface)
{
    ASSERT(object->getType()->isPointerTy());

    auto dataOff = arkInterface->GetObjectClassOffset();
    auto ptrData = builder->CreateConstInBoundsGEP1_32(builder->getInt8Ty(), object, dataOff);
    auto classAddress = builder->CreateLoad(builder->getInt32Ty(), ptrData);
    return builder->CreateIntToPtr(classAddress, builder->getPtrTy());
}

llvm::Value *CreateLoadMethodUsingVTable(llvm::Value *thiz, llvm::Function *caller, size_t methodId,
                                         llvm::IRBuilder<> *builder, LLVMArkInterface *arkInterface)
{
    ASSERT(thiz != nullptr);
    ASSERT(caller != nullptr);

    auto classPtr = CreateLoadClassFromObject(thiz, builder, arkInterface);
    auto offset = arkInterface->GetVTableOffset(caller, methodId);
    auto methodPtr = builder->CreateConstInBoundsGEP1_32(builder->getInt8Ty(), classPtr, offset);
    return builder->CreateLoad(builder->getPtrTy(), methodPtr);
}

}  // namespace ark::llvmbackend::utils
