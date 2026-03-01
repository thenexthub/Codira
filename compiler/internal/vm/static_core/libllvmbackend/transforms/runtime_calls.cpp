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

#include "runtime_calls.h"
#include "llvm_ark_interface.h"
#include "transforms/transform_utils.h"

#include <llvm/IR/IRBuilder.h>

namespace ark::llvmbackend::runtime_calls {

llvm::Value *GetAddressToTLS(llvm::IRBuilder<> *builder, LLVMArkInterface *arkInterface, uintptr_t tlsOffset)
{
    auto threadRegValue = GetThreadRegValue(builder, arkInterface);
    auto threadRegPtr = builder->CreateIntToPtr(threadRegValue, builder->getPtrTy());
    return builder->CreateConstInBoundsGEP1_64(builder->getInt8Ty(), threadRegPtr, tlsOffset);
}

llvm::Value *LoadTLSValue(llvm::IRBuilder<> *builder, LLVMArkInterface *arkInterface, uintptr_t tlsOffset,
                          llvm::Type *type)
{
    auto addr = GetAddressToTLS(builder, arkInterface, tlsOffset);
    return builder->CreateLoad(type, addr);
}

llvm::FunctionCallee GetPandaRuntimeFunctionCallee(int entrypoint, llvm::FunctionType *functionProto,
                                                   llvm::IRBuilder<> *builder, llvm::StringRef prefix)
{
    auto module = builder->GetInsertBlock()->getModule();
    auto table = module->getGlobalVariable("__aot_got");
    auto arrayType = llvm::ArrayType::get(builder->getInt64Ty(), 0);
    auto epAddrPtr = builder->CreateConstInBoundsGEP2_32(arrayType, table, 0, entrypoint);
    auto entrypointAddress = builder->CreateLoad(builder->getInt64Ty(), epAddrPtr, prefix + "_addr");
    auto calleePointer = builder->CreateIntToPtr(entrypointAddress, llvm::PointerType::get(functionProto, 0));
    return llvm::FunctionCallee(functionProto, calleePointer);
}

llvm::CallInst *CreateEntrypointCallCommon(llvm::IRBuilder<> *builder, llvm::Value *threadRegValue,
                                           LLVMArkInterface *arkInterface, EntrypointId eid,
                                           llvm::ArrayRef<llvm::Value *> arguments,
                                           llvm::ArrayRef<llvm::OperandBundleDef> bundle)
{
    ASSERT(arkInterface->DeoptsEnabled() || bundle.empty());
    auto entrypointsTablePtrOffset = arkInterface->GetEntrypointsTablePointerTlsOffset();
    auto entrypointOffset = arkInterface->GetEntrypointOffset(eid);
    auto [functionProto, functionName] = arkInterface->GetEntrypointCallee(eid);

    auto threadRegPtr = builder->CreateIntToPtr(threadRegValue, builder->getPtrTy());
    auto entrypointsTablePtrAddr =
        builder->CreateConstInBoundsGEP1_64(builder->getInt8Ty(), threadRegPtr, entrypointsTablePtrOffset);
    auto entrypointsTableAddr = builder->CreateLoad(builder->getPtrTy(), entrypointsTablePtrAddr);
    auto addr = builder->CreateConstInBoundsGEP1_64(builder->getInt8Ty(), entrypointsTableAddr, entrypointOffset);
    auto callee = builder->CreateLoad(builder->getPtrTy(), addr, functionName + "_addr");

    auto calleeFuncTy = llvm::cast<llvm::FunctionType>(functionProto);
    auto call = builder->CreateCall(calleeFuncTy, callee, arguments, bundle);

    auto bridgeType = arkInterface->GetBridgeType(eid);
    ASSERT(bridgeType != BridgeType::SLOW_PATH && bridgeType != BridgeType::ODD_SAVED);
    ASSERT(call->getCallingConv() == llvm::CallingConv::C);

    // Entrypoint bridges preserve a lot of registers, so we can put appropriate ArkFast convention for them.
    if (bridgeType == BridgeType::ENTRYPOINT) {
        llvm::CallingConv::ID cc = llvm::CallingConv::C;
        switch (arguments.size()) {
            case 0U:
                cc = llvm::CallingConv::ArkFast0;
                break;
            case 1U:
            case 2U:
                cc = llvm::CallingConv::ArkFast2;
                break;
            case 3U:
            case 4U:
                cc = llvm::CallingConv::ArkFast4;
                break;
            case 5U:
            case 6U:
                cc = llvm::CallingConv::ArkFast6;
                break;
            default:
                llvm_unreachable("Entrypoints with 7 and more arguments are not supported");
        }
        call->setCallingConv(cc);
    }

    return call;
}

llvm::Value *GetThreadRegValue(llvm::IRBuilder<> *builder, LLVMArkInterface *arkInterface)
{
    ASSERT(!arkInterface->IsIrtocMode());
    auto func = builder->GetInsertBlock()->getParent();
    auto &ctx = func->getContext();
    auto regMd = llvm::MDNode::get(ctx, {llvm::MDString::get(ctx, arkInterface->GetThreadRegister())});
    auto threadReg = llvm::MetadataAsValue::get(ctx, regMd);
    auto readReg =
        llvm::Intrinsic::getDeclaration(func->getParent(), llvm::Intrinsic::read_register, builder->getInt64Ty());
    return builder->CreateCall(readReg, {threadReg});
}

llvm::Value *GetRealFrameRegValue(llvm::IRBuilder<> *builder, LLVMArkInterface *arkInterface)
{
    ASSERT(!arkInterface->IsIrtocMode());
    auto func = builder->GetInsertBlock()->getParent();
    auto &ctx = func->getContext();
    auto regMd = llvm::MDNode::get(ctx, {llvm::MDString::get(ctx, arkInterface->GetFramePointerRegister())});
    auto frameReg = llvm::MetadataAsValue::get(ctx, regMd);
    auto readReg =
        llvm::Intrinsic::getDeclaration(func->getParent(), llvm::Intrinsic::read_register, builder->getInt64Ty());
    return builder->CreateCall(readReg, {frameReg});
}

}  // namespace ark::llvmbackend::runtime_calls
