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

#include "builtins.h"
#include "llvm_ark_interface.h"
#include "runtime_calls.h"
#include "utils.h"
#include "lowering/gc_barriers.h"
#include "lowering/metadata.h"

#include "compiler/optimizer/ir/basicblock.h"
#include "compiler/optimizer/ir/inst.h"

#include <llvm/IR/DebugInfoMetadata.h>
#include <llvm/IR/MDBuilder.h>
#include <llvm/ADT/SmallVector.h>
#include <llvm/Transforms/Utils/BasicBlockUtils.h>

using ark::llvmbackend::LLVMArkInterface;
using ark::llvmbackend::Metadata::BranchWeights::LIKELY_BRANCH_WEIGHT;
using ark::llvmbackend::Metadata::BranchWeights::UNLIKELY_BRANCH_WEIGHT;
using ark::llvmbackend::runtime_calls::GetThreadRegValue;
using ark::llvmbackend::utils::CopyDebugLoc;
using ark::llvmbackend::utils::CopyDeoptBundle;

namespace {
llvm::CallInst *CreateEntrypointCallHelper(llvm::IRBuilder<> *builder, ark::compiler::RuntimeInterface::EntrypointId id,
                                           llvm::ArrayRef<llvm::Value *> arguments, LLVMArkInterface *arkInterface,
                                           llvm::CallInst *inst)
{
    auto threadReg = GetThreadRegValue(builder, arkInterface);
    auto call = ark::llvmbackend::runtime_calls::CreateEntrypointCallCommon(
        builder, threadReg, arkInterface, static_cast<ark::llvmbackend::runtime_calls::EntrypointId>(id), arguments,
        CopyDeoptBundle(inst));
    CopyDebugLoc(inst, call);
    if (inst->hasFnAttr("inline-info")) {
        call->addFnAttr(inst->getFnAttr("inline-info"));
    }

    return call;
}
llvm::Value *PostWRBHelper(llvm::IRBuilder<> *builder, llvm::CallInst *inst, LLVMArkInterface *arkInterface)
{
    auto mem = inst->getOperand(0U);
    auto offset = inst->getOperand(1U);
    auto value = inst->getOperand(2U);

    ASSERT(!arkInterface->IsIrtocMode());
    auto threadReg = GetThreadRegValue(builder, arkInterface);
    ark::llvmbackend::gc_barriers::EmitPostWRB(builder, mem, offset, value, arkInterface, threadReg, nullptr);
    return nullptr;
}
llvm::Value *FastClassLoadingHelper(llvm::IRBuilder<> *builder, LLVMArkInterface *arkInterface,
                                    llvm::BasicBlock *continuation)
{
    auto function = continuation->getParent();

    llvm::Value *methodValue = function->arg_begin();

    auto offset = arkInterface->GetClassOffset();
    auto dataPtr = builder->CreateConstInBoundsGEP1_32(builder->getInt8Ty(), methodValue, offset);
    auto classAddr = builder->CreateLoad(builder->getInt32Ty(), dataPtr);
    auto result = builder->CreateIntToPtr(classAddr, builder->getPtrTy());
    builder->CreateBr(continuation);
    return result;
}

llvm::Value *SlowClassLoadingHelper(llvm::IRBuilder<> *builder, llvm::CallInst *inst, LLVMArkInterface *arkInterface,
                                    llvm::BasicBlock *continuation, bool forceInit)
{
    using llvm::Value;
    auto eid = forceInit ? ark::compiler::RuntimeInterface::EntrypointId::CLASS_INIT_RESOLVER
                         : ark::compiler::RuntimeInterface::EntrypointId::CLASS_RESOLVER;
    auto initialBb = builder->GetInsertBlock();
    auto function = initialBb->getParent();
    auto module = function->getParent();
    auto &ctx = builder->getContext();

    // Helper functions
    auto createUniqBasicBlockName = [&initialBb](const std::string &suffix) {
        return ark::llvmbackend::LLVMArkInterface::GetUniqueBasicBlockName(initialBb->getName().str(), suffix);
    };
    auto createBasicBlock = [&ctx, &initialBb, &createUniqBasicBlockName](const std::string &suffix) {
        auto name = createUniqBasicBlockName(suffix);
        auto func = initialBb->getParent();
        return llvm::BasicBlock::Create(ctx, name, func);
    };

    // Helper types & variables
    auto aotGot = module->getGlobalVariable("__aot_got");
    auto arrayType = llvm::ArrayType::get(builder->getInt64Ty(), 0);

    // Input args
    auto slotIdVal = inst->getOperand(1U);

    /* Start generating code */

    auto klassPtr = builder->CreateInBoundsGEP(arrayType, aotGot, {builder->getInt32(0), slotIdVal});
    auto cachedKlassTmp = builder->CreateLoad(builder->getInt64Ty(), klassPtr, "cached_klass");
    auto cachedKlass = builder->CreateIntToPtr(cachedKlassTmp, builder->getPtrTy());
    auto resolutionRequired = builder->CreateIsNull(cachedKlass, "resolutionRequired");

    auto weights = llvm::MDBuilder(ctx).createBranchWeights(UNLIKELY_BRANCH_WEIGHT,  // if resolution_required
                                                            LIKELY_BRANCH_WEIGHT);   // else
    auto slowPath = createBasicBlock(std::string("slow_path"));
    builder->CreateCondBr(resolutionRequired, slowPath, continuation, weights);

    builder->SetInsertPoint(slowPath);

    auto freshKlass = CreateEntrypointCallHelper(builder, eid, {klassPtr}, arkInterface, inst);
    ASSERT(freshKlass->getCallingConv() == llvm::CallingConv::C);
    freshKlass->setCallingConv(llvm::CallingConv::ArkResolver);
    builder->CreateBr(continuation);

    builder->SetInsertPoint(&continuation->front());
    auto result = builder->CreatePHI(builder->getPtrTy(), 2U, "klass");
    result->addIncoming(cachedKlass, initialBb);
    result->addIncoming(freshKlass, slowPath);

    return result;
}

llvm::Value *LowerLoadClassHelper(llvm::IRBuilder<> *builder, llvm::CallInst *inst, LLVMArkInterface *arkInterface,
                                  bool forceInit)
{
    auto initialBb = builder->GetInsertBlock();
    auto function = initialBb->getParent();

    auto typeIdVal = inst->getOperand(0U);

    llvm::BasicBlock *slowPath = initialBb;
    llvm::BasicBlock *continuation = llvm::SplitBlock(initialBb, inst);

    slowPath->back().eraseFromParent();
    builder->SetInsertPoint(slowPath);

    if (!llvm::isa<llvm::ConstantInt>(typeIdVal)) {
        return SlowClassLoadingHelper(builder, inst, arkInterface, continuation, forceInit);
    }

    // Try simple constant case
    auto functionMd = function->getMetadata(LLVMArkInterface::FUNCTION_MD_CLASS_ID);
    ASSERT(functionMd != nullptr);
    auto functionMdCasted = llvm::dyn_cast<llvm::ConstantAsMetadata>(functionMd->getOperand(0U))->getValue();
    auto methodKlassId = llvm::cast<llvm::ConstantInt>(functionMdCasted)->getZExtValue();
    auto requiredKlassId = llvm::cast<llvm::ConstantInt>(typeIdVal)->getZExtValue();

    return methodKlassId == requiredKlassId
               ? FastClassLoadingHelper(builder, arkInterface, continuation)
               : SlowClassLoadingHelper(builder, inst, arkInterface, continuation, forceInit);
}

llvm::Value *PreWRBHelper(llvm::IRBuilder<> *builder, llvm::CallInst *inst, LLVMArkInterface *arkInterface)
{
    ASSERT(!arkInterface->IsIrtocMode());
    auto initialBb = builder->GetInsertBlock();
    auto mem = inst->getOperand(0U);
    auto op1 = inst->getOperand(1U);
    ASSERT(llvm::isa<llvm::ConstantInt>(op1));
    auto isConst = llvm::cast<llvm::ConstantInt>(op1);
    auto isVolatileMem = !isConst->isZero();

    llvm::BasicBlock *continuation = llvm::SplitBlock(initialBb, inst);

    initialBb->back().eraseFromParent();
    builder->SetInsertPoint(initialBb);
    auto threadRegValue = GetThreadRegValue(builder, arkInterface);
    ark::llvmbackend::gc_barriers::EmitPreWRB(builder, mem, isVolatileMem, continuation, arkInterface, threadRegValue);
    return nullptr;
}
}  // namespace

namespace ark::llvmbackend::builtins {

llvm::Function *LenArray(llvm::Module *module)
{
    auto function = module->getFunction(LEN_ARRAY_BUILTIN);
    if (function != nullptr) {
        return function;
    }
    auto type = llvm::FunctionType::get(llvm::Type::getInt32Ty(module->getContext()),
                                        {llvm::PointerType::get(module->getContext(), LLVMArkInterface::GC_ADDR_SPACE),
                                         llvm::Type::getInt32Ty(module->getContext())},
                                        false);
    function = llvm::Function::Create(type, llvm::Function::ExternalLinkage, LEN_ARRAY_BUILTIN, module);
    function->setDoesNotThrow();
    function->setSectionPrefix(BUILTIN_SECTION);
    function->addFnAttr(llvm::Attribute::ReadNone);
    function->addFnAttr(llvm::Attribute::WillReturn);
    return function;
}

llvm::Function *KeepThis(llvm::Module *module)
{
    auto function = module->getFunction(KEEP_THIS_BUILTIN);
    if (function != nullptr) {
        return function;
    }

    auto type =
        llvm::FunctionType::get(llvm::Type::getVoidTy(module->getContext()),
                                {llvm::PointerType::get(module->getContext(), LLVMArkInterface::GC_ADDR_SPACE)}, false);
    function = llvm::Function::Create(type, llvm::Function::ExternalLinkage, KEEP_THIS_BUILTIN, module);
    function->setDoesNotThrow();
    function->setSectionPrefix(BUILTIN_SECTION);
    function->addFnAttr(llvm::Attribute::WillReturn);
    return function;
}

llvm::Function *BarrierReturnVoid(llvm::Module *module)
{
    auto function = module->getFunction(BARRIER_RETURN_VOID_BUILTIN);
    if (function != nullptr) {
        return function;
    }

    auto type = llvm::FunctionType::get(llvm::Type::getVoidTy(module->getContext()), {}, false);
    function = llvm::Function::Create(type, llvm::Function::ExternalLinkage, BARRIER_RETURN_VOID_BUILTIN, module);
    function->setDoesNotThrow();
    function->setSectionPrefix(BUILTIN_SECTION);
    function->addFnAttr(llvm::Attribute::WillReturn);
    return function;
}

llvm::Function *LoadClass(llvm::Module *module)
{
    auto function = module->getFunction(LOAD_CLASS_BUILTIN);
    if (function != nullptr) {
        return function;
    }

    auto type = llvm::FunctionType::get(
        llvm::PointerType::get(module->getContext(), 0),
        {llvm::Type::getInt64Ty(module->getContext()), llvm::Type::getInt32Ty(module->getContext())}, false);
    function = llvm::Function::Create(type, llvm::Function::ExternalLinkage, LOAD_CLASS_BUILTIN, module);
    function->setDoesNotThrow();
    function->setSectionPrefix(BUILTIN_SECTION);
    return function;
}

llvm::Function *LoadInitClass(llvm::Module *module)
{
    auto function = module->getFunction(LOAD_INIT_CLASS_BUILTIN);
    if (function != nullptr) {
        return function;
    }

    auto type = llvm::FunctionType::get(
        llvm::PointerType::get(module->getContext(), 0),
        {llvm::Type::getInt64Ty(module->getContext()), llvm::Type::getInt32Ty(module->getContext())}, false);
    function = llvm::Function::Create(type, llvm::Function::ExternalLinkage, LOAD_INIT_CLASS_BUILTIN, module);
    function->setDoesNotThrow();
    function->setSectionPrefix(BUILTIN_SECTION);
    return function;
}

llvm::Function *PreWRB(llvm::Module *module, unsigned addrSpace)
{
    auto builtinName = (addrSpace == LLVMArkInterface::GC_ADDR_SPACE) ? PRE_WRB_GCADR_BUILTIN : PRE_WRB_BUILTIN;
    auto function = module->getFunction(builtinName);
    if (function != nullptr) {
        return function;
    }
    auto &ctx = module->getContext();
    auto type = llvm::FunctionType::get(llvm::Type::getVoidTy(ctx),
                                        {llvm::PointerType::get(ctx, addrSpace), llvm::Type::getInt1Ty(ctx)}, false);
    function = llvm::Function::Create(type, llvm::Function::ExternalLinkage, builtinName, module);
    function->setDoesNotThrow();
    function->setSectionPrefix(BUILTIN_SECTION);
    function->addFnAttr(llvm::Attribute::ArgMemOnly);
    function->addFnAttr(llvm::Attribute::WillReturn);
    function->addParamAttr(0U, llvm::Attribute::ReadOnly);
    return function;
}

llvm::Function *PostWRB(llvm::Module *module, unsigned addrSpace)
{
    auto builtinName = (addrSpace == LLVMArkInterface::GC_ADDR_SPACE) ? POST_WRB_GCADR_BUILTIN : POST_WRB_BUILTIN;
    auto function = module->getFunction(builtinName);
    if (function != nullptr) {
        return function;
    }
    auto &ctx = module->getContext();
    auto type = llvm::FunctionType::get(llvm::Type::getVoidTy(ctx),
                                        {llvm::PointerType::get(ctx, addrSpace), llvm::Type::getInt32Ty(ctx),
                                         llvm::PointerType::get(ctx, LLVMArkInterface::GC_ADDR_SPACE)},
                                        false);
    function = llvm::Function::Create(type, llvm::Function::ExternalLinkage, builtinName, module);
    function->setDoesNotThrow();
    function->setSectionPrefix(BUILTIN_SECTION);
    function->addFnAttr(llvm::Attribute::ArgMemOnly);
    function->addFnAttr(llvm::Attribute::WillReturn);
    function->addParamAttr(0U, llvm::Attribute::ReadNone);
    function->addParamAttr(2U, llvm::Attribute::ReadNone);
    return function;
}

llvm::Function *LoadString(llvm::Module *module)
{
    auto function = module->getFunction(LOAD_STRING_BUILTIN);
    if (function != nullptr) {
        return function;
    }

    auto type = llvm::FunctionType::get(
        llvm::PointerType::get(module->getContext(), LLVMArkInterface::GC_ADDR_SPACE),
        {llvm::Type::getInt64Ty(module->getContext()), llvm::Type::getInt32Ty(module->getContext())}, false);
    function = llvm::Function::Create(type, llvm::Function::ExternalLinkage, LOAD_STRING_BUILTIN, module);
    function->setDoesNotThrow();
    function->setSectionPrefix(BUILTIN_SECTION);
    return function;
}

llvm::Function *ResolveVirtual(llvm::Module *module)
{
    auto function = module->getFunction(RESOLVE_VIRTUAL_BUILTIN);
    if (function != nullptr) {
        return function;
    }
    auto type = llvm::FunctionType::get(llvm::PointerType::get(module->getContext(), 0),
                                        {llvm::PointerType::get(module->getContext(), LLVMArkInterface::GC_ADDR_SPACE),
                                         llvm::Type::getInt64Ty(module->getContext()),
                                         llvm::PointerType::get(module->getContext(), 0)},
                                        false);
    function = llvm::Function::Create(type, llvm::Function::ExternalLinkage, RESOLVE_VIRTUAL_BUILTIN, module);
    function->setDoesNotThrow();
    function->setSectionPrefix(BUILTIN_SECTION);
    return function;
}

llvm::Value *LowerLenArray(llvm::IRBuilder<> *builder, llvm::CallInst *inst)
{
    auto array = inst->getOperand(0U);
    auto offset = inst->getOperand(1U);

    auto rawPtr = builder->CreateInBoundsGEP(builder->getInt8Ty(), array, offset);

    return builder->CreateLoad(builder->getInt32Ty(), rawPtr);
}

llvm::Value *LowerLoadString(llvm::IRBuilder<> *builder, llvm::CallInst *inst, LLVMArkInterface *arkInterface)
{
    auto module = inst->getModule();
    auto function = inst->getFunction();
    auto aotGot = module->getGlobalVariable("__aot_got");
    auto arrayType = llvm::ArrayType::get(builder->getInt64Ty(), 0);

    // Input args
    auto typeId = inst->getOperand(0U);
    auto slotId = inst->getOperand(1U);

    auto slotPtr = builder->CreateInBoundsGEP(arrayType, aotGot, {builder->getInt32(0), slotId});
    auto str = builder->CreateLoad(builder->getInt64Ty(), slotPtr);
    auto limit = compiler::RuntimeInterface::RESOLVE_STRING_AOT_COUNTER_LIMIT;
    auto cmp = builder->CreateICmpULT(str, builder->getInt64(limit));

    llvm::Instruction *ifi;
    llvm::Instruction *elsi;
    auto weights =
        llvm::MDBuilder(inst->getContext()).createBranchWeights(UNLIKELY_BRANCH_WEIGHT, LIKELY_BRANCH_WEIGHT);
    SplitBlockAndInsertIfThenElse(cmp, inst, &ifi, &elsi, weights);

    ifi->getParent()->setName("load_str_slow");
    elsi->getParent()->setName("load_str_fast");

    builder->SetInsertPoint(ifi);
    auto method = function->arg_begin();
    auto eid = compiler::RuntimeInterface::EntrypointId::RESOLVE_STRING_AOT;
    auto freshStr = CreateEntrypointCallHelper(builder, eid, {method, typeId, slotPtr}, arkInterface, inst);
    freshStr->setName("fresh_str");

    builder->SetInsertPoint(elsi);
    auto cachedStr = builder->CreateIntToPtr(str, inst->getType());
    cachedStr->setName("cached_str");

    builder->SetInsertPoint(inst);
    auto resStr = builder->CreatePHI(inst->getType(), 2U);
    resStr->addIncoming(freshStr, ifi->getParent());
    resStr->addIncoming(cachedStr, elsi->getParent());
    return resStr;
}

llvm::Value *LowerResolveVirtual(llvm::IRBuilder<> *builder, llvm::CallInst *inst, LLVMArkInterface *arkInterface)
{
    llvm::CallInst *entrypointAddress;
    auto method = inst->getFunction()->arg_begin();
    auto thiz = inst->getOperand(0U);
    auto methodId = inst->getOperand(1U);
    ASSERT(!llvm::isa<llvm::PoisonValue>(thiz));
    if (!arkInterface->IsArm64() || !llvm::isa<llvm::ConstantInt>(methodId)) {
        static constexpr auto ENTRYPOINT_ID = compiler::RuntimeInterface::EntrypointId::RESOLVE_VIRTUAL_CALL_AOT;
        auto offset = builder->getInt64(0);
        entrypointAddress =
            CreateEntrypointCallHelper(builder, ENTRYPOINT_ID, {method, thiz, methodId, offset}, arkInterface, inst);
    } else {
        auto offset = inst->getOperand(2U);
        static constexpr auto ENTRYPOINT_ID = compiler::RuntimeInterface::EntrypointId::INTF_INLINE_CACHE;
        entrypointAddress =
            CreateEntrypointCallHelper(builder, ENTRYPOINT_ID, {method, thiz, methodId, offset}, arkInterface, inst);
        ASSERT(entrypointAddress->getCallingConv() == llvm::CallingConv::C);
        entrypointAddress->setCallingConv(llvm::CallingConv::ArkFast4);
    }

    return builder->CreateIntToPtr(entrypointAddress, builder->getPtrTy());
}

llvm::Value *LowerBuiltin(llvm::IRBuilder<> *builder, llvm::CallInst *inst, LLVMArkInterface *arkInterface)
{
    auto function = inst->getCalledFunction();
    const auto &funcName = function->getName();
    if (funcName.equals(LEN_ARRAY_BUILTIN)) {
        return LowerLenArray(builder, inst);
    }
    if (funcName.equals(LOAD_CLASS_BUILTIN)) {
        return LowerLoadClassHelper(builder, inst, arkInterface, false);
    }
    if (funcName.equals(LOAD_INIT_CLASS_BUILTIN)) {
        return LowerLoadClassHelper(builder, inst, arkInterface, true);
    }
    if (funcName.equals(PRE_WRB_GCADR_BUILTIN) || funcName.equals(PRE_WRB_BUILTIN)) {
        return PreWRBHelper(builder, inst, arkInterface);
    }
    if (funcName.equals(POST_WRB_GCADR_BUILTIN) || funcName.equals(POST_WRB_BUILTIN)) {
        return PostWRBHelper(builder, inst, arkInterface);
    }
    if (funcName.equals(LOAD_STRING_BUILTIN)) {
        return LowerLoadString(builder, inst, arkInterface);
    }
    if (funcName.equals(RESOLVE_VIRTUAL_BUILTIN)) {
        return LowerResolveVirtual(builder, inst, arkInterface);
    }
    if (funcName.equals(BARRIER_RETURN_VOID_BUILTIN)) {
        return nullptr;
    }
    if (funcName.equals(KEEP_THIS_BUILTIN)) {
        return nullptr;
    }
    UNREACHABLE();
}
}  // namespace ark::llvmbackend::builtins
