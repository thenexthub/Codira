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

#include "gc_barriers.h"

#include "llvm_ark_interface.h"
#include "metadata.h"
#include "transforms/builtins.h"
#include "transforms/runtime_calls.h"

#include "compiler/optimizer/ir/basicblock.h"

#include <llvm/IR/MDBuilder.h>
#include <llvm/ADT/SmallVector.h>
#include <llvm/Transforms/Utils/BasicBlockUtils.h>

namespace ark::llvmbackend::gc_barriers {

void EmitPreWRB(llvm::IRBuilder<> *builder, llvm::Value *mem, bool isVolatileMem, llvm::BasicBlock *outBb,
                LLVMArkInterface *arkInterface, llvm::Value *threadRegValue)
{
    auto func = builder->GetInsertBlock()->getParent();
    auto module = func->getParent();
    auto &ctx = module->getContext();
    auto initialBb = builder->GetInsertBlock();

    auto createUniqBasicBlockName = [&initialBb](const std::string &suffix) {
        return ark::llvmbackend::LLVMArkInterface::GetUniqueBasicBlockName(initialBb->getName().str(), suffix);
    };
    auto createBasicBlock = [&ctx, &initialBb, &createUniqBasicBlockName](const std::string &suffix) {
        auto name = createUniqBasicBlockName(suffix);
        auto funcIbb = initialBb->getParent();
        return llvm::BasicBlock::Create(ctx, name, funcIbb);
    };

    auto loadValueBb = createBasicBlock("pre_wrb_load_value");
    auto callRuntimeBb = createBasicBlock("pre_wrb_call_runtime");
    auto threadStructPtr = builder->CreateIntToPtr(threadRegValue, builder->getPtrTy());
    auto entrypointOffset = arkInterface->GetTlsPreWrbEntrypointOffset();
    auto entrypointPtr = builder->CreateConstInBoundsGEP1_32(builder->getInt8Ty(), threadStructPtr, entrypointOffset);

    // Check if entrypoint is null
    auto entrypoint =
        builder->CreateLoad(builder->getPtrTy(), entrypointPtr, "__panda_entrypoint_PreWrbFuncNoBridge_addr");
    auto hasEntrypoint = builder->CreateIsNotNull(entrypoint);
    builder->CreateCondBr(hasEntrypoint, loadValueBb, outBb);

    // Load old value, similar to LLVMIrConstructor::CreateLoadWithOrdering
    builder->SetInsertPoint(loadValueBb);
    auto load = builder->CreateLoad(builder->getPtrTy(LLVMArkInterface::GC_ADDR_SPACE), mem);
    if (isVolatileMem) {
        auto alignment = module->getDataLayout().getPrefTypeAlignment(load->getType());
        load->setOrdering(LLVMArkInterface::VOLATILE_ORDER);
        load->setAlignment(llvm::Align(alignment));
    }
    auto objectIsNull = builder->CreateIsNotNull(load);
    builder->CreateCondBr(objectIsNull, callRuntimeBb, outBb);

    // Call Runtime
    builder->SetInsertPoint(callRuntimeBb);
    static constexpr auto VAR_ARGS = true;
    auto functionType =
        llvm::FunctionType::get(builder->getVoidTy(), {builder->getPtrTy(LLVMArkInterface::GC_ADDR_SPACE)}, !VAR_ARGS);
    builder->CreateCall(functionType, entrypoint, {load});
    builder->CreateBr(outBb);

    builder->SetInsertPoint(outBb);
}

void EmitPostWRB(llvm::IRBuilder<> *builder, llvm::Value *mem, llvm::Value *offset, llvm::Value *value,
                 LLVMArkInterface *arkInterface, llvm::Value *threadRegValue, llvm::Value *frameRegValue)
{
    auto tlsOffset = arkInterface->GetManagedThreadPostWrbOneObjectOffset();

    auto gcPtrTy = builder->getPtrTy(LLVMArkInterface::GC_ADDR_SPACE);
    auto ptrTy = builder->getPtrTy();
    auto memTy = mem->getType();
    auto int32Ty = builder->getInt32Ty();
    auto threadRegPtr = builder->CreateIntToPtr(threadRegValue, ptrTy);
    auto addr = builder->CreateConstInBoundsGEP1_64(builder->getInt8Ty(), threadRegPtr, tlsOffset);
    auto callee = builder->CreateLoad(ptrTy, addr, "post_wrb_one_object_addr");

    ASSERT(mem->getType()->isPointerTy());
    ASSERT(value->getType()->isPointerTy() &&
           value->getType()->getPointerAddressSpace() == LLVMArkInterface::GC_ADDR_SPACE);
    ASSERT(offset->getType() == int32Ty);

    if (!arkInterface->IsIrtocMode()) {
        // LLVM AOT, only 3 parameters
        auto funcTy = llvm::FunctionType::get(builder->getVoidTy(), {memTy, int32Ty, gcPtrTy}, false);
        auto call = builder->CreateCall(funcTy, callee, {mem, offset, value});
        call->setCallingConv(llvm::CallingConv::ArkFast3);
        return;
    }
    if (arkInterface->IsArm64()) {
        // Arm64 Irtoc, 4 params (add thread)
        auto funcTy = llvm::FunctionType::get(builder->getVoidTy(), {memTy, int32Ty, gcPtrTy, ptrTy}, false);
        auto call = builder->CreateCall(funcTy, callee, {mem, offset, value, threadRegPtr});
        call->setCallingConv(llvm::CallingConv::ArkFast3);
        return;
    }
    // X86_64 Irtoc, 5 params (add thread, fp)
    ASSERT(frameRegValue != nullptr);
    auto funcTy = llvm::FunctionType::get(builder->getVoidTy(), {memTy, int32Ty, gcPtrTy, ptrTy, ptrTy}, false);
    auto frameRegPtr = builder->CreateIntToPtr(frameRegValue, ptrTy);
    auto call = builder->CreateCall(funcTy, callee, {mem, offset, value, threadRegPtr, frameRegPtr});
    call->setCallingConv(llvm::CallingConv::ArkFast3);
}

}  // namespace ark::llvmbackend::gc_barriers
