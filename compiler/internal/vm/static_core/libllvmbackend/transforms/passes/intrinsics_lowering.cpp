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

#include <unordered_map>

#include "transforms/gc_utils.h"
#include "transforms/passes/intrinsics_lowering.h"
#include "transforms/runtime_calls.h"
#include "transforms/transform_utils.h"
#include "llvm_ark_interface.h"

#include <llvm/Pass.h>
#include <llvm/IR/IntrinsicInst.h>
#include <llvm/IR/Intrinsics.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Transforms/Utils/BasicBlockUtils.h>

using ark::llvmbackend::runtime_calls::GetPandaRuntimeFunctionCallee;
using llvm::Function;
using llvm::FunctionAnalysisManager;

namespace ark::llvmbackend::passes {

IntrinsicsLowering IntrinsicsLowering::Create(LLVMArkInterface *arkInterface,
                                              [[maybe_unused]] const ark::llvmbackend::LLVMCompilerOptions *options)
{
    return IntrinsicsLowering(arkInterface);
}

IntrinsicsLowering::IntrinsicsLowering(LLVMArkInterface *arkInterface) : arkInterface_ {arkInterface} {}

llvm::PreservedAnalyses IntrinsicsLowering::run(Function &function, FunctionAnalysisManager & /*analysisManager*/)
{
    ASSERT(arkInterface_ != nullptr);
    if (gc_utils::IsFunctionSupplemental(function)) {
        return llvm::PreservedAnalyses::all();
    }
    bool changed = false;
    std::unordered_map<llvm::Instruction *, llvm::Instruction *> instToReplaceWithInst;
    for (auto &block : function) {
        for (auto &instruction : block) {
            auto llvmIntrinsicId = arkInterface_->GetLLVMIntrinsicId(&instruction);
            if (llvmIntrinsicId != llvm::Intrinsic::not_intrinsic) {
                ASSERT(llvm::isa<llvm::CallInst>(&instruction));
                changed |= ReplaceWithLLVMIntrinsic(llvm::cast<llvm::CallInst>(&instruction), llvmIntrinsicId);
                continue;
            }
            auto intrinsicId = arkInterface_->GetIntrinsicId(&instruction);
            if (intrinsicId == LLVMArkInterface::NO_INTRINSIC_ID) {
                continue;
            }

            auto opcode = instruction.getOpcode();
            if (opcode == llvm::Instruction::Call) {
                changed |= HandleCall(llvm::cast<llvm::CallInst>(&instruction), intrinsicId, &instToReplaceWithInst);
            } else if (opcode == llvm::Instruction::FRem) {
                changed |= HandleFRem(&instruction, intrinsicId, &instToReplaceWithInst);
            } else {
                llvm_unreachable("Unexpected opcode while lowering intrinsics");
            }
        }
    }
    for (auto item : instToReplaceWithInst) {
        llvm::ReplaceInstWithInst(item.first, item.second);
    }
    return changed ? llvm::PreservedAnalyses::none() : llvm::PreservedAnalyses::all();
}
bool IntrinsicsLowering::ReplaceWithLLVMIntrinsic(llvm::CallInst *call, llvm::Intrinsic::ID intrinsicId)
{
    ASSERT(intrinsicId == llvm::Intrinsic::memcpy_inline || intrinsicId == llvm::Intrinsic::memset_inline);
    std::vector<llvm::Type *> argTypes;
    auto functionType = call->getFunctionType();
    if (intrinsicId == llvm::Intrinsic::memcpy_inline) {
        // Skip the 4-th `isvolatile` arg
        argTypes.push_back(functionType->getParamType(0U));  // Dst type
        argTypes.push_back(functionType->getParamType(1U));  // Src type
        argTypes.push_back(functionType->getParamType(2U));  // Size type
    } else if (intrinsicId == llvm::Intrinsic::memset_inline) {
        argTypes.push_back(functionType->getParamType(0U));  // Dst type
        argTypes.push_back(functionType->getParamType(2U));  // Size type
    } else {
        llvm_unreachable("Attempt to insert unsupported llvm intrinsic");
    }
    auto *module = call->getModule();
    auto intrinsicDecl = llvm::Intrinsic::getDeclaration(module, intrinsicId, argTypes);
    call->setCalledFunction(intrinsicDecl);
    return true;
}

void IntrinsicsLowering::HandleMemCall(
    llvm::CallInst *call, llvm::FunctionCallee callee,
    std::unordered_map<llvm::Instruction *, llvm::Instruction *> *instToReplaceWithInst)
{
    auto builder = llvm::IRBuilder<>(call);
    static constexpr unsigned DEST = 0U;
    static constexpr unsigned SRC_OR_CHAR = 1U;
    static constexpr unsigned COUNT = 2U;

    llvm::Value *op0 = call->getOperand(DEST);
    llvm::Value *op1 = call->getOperand(SRC_OR_CHAR);
    llvm::Value *op2 = call->getOperand(COUNT);

    ASSERT(op0->getType()->isPointerTy());
    if (op0->getType() != callee.getFunctionType()->getParamType(DEST)) {
        op0 = builder.CreateAddrSpaceCast(op0, callee.getFunctionType()->getParamType(DEST));
    }
    if (op1->getType() != callee.getFunctionType()->getParamType(SRC_OR_CHAR)) {
        ASSERT(op1->getType()->isPointerTy());
        op1 = builder.CreateAddrSpaceCast(op1, callee.getFunctionType()->getParamType(SRC_OR_CHAR));
    }

    ASSERT(callee.getFunctionType()->getParamType(COUNT)->isIntegerTy());
    auto realCountType = llvm::cast<llvm::IntegerType>(callee.getFunctionType()->getParamType(COUNT));
    if (llvm::cast<llvm::IntegerType>(op2->getType())->getBitWidth() < realCountType->getBitWidth()) {
        op2 = builder.CreateCast(llvm::Instruction::ZExt, op2, realCountType);
    }
    // Remove is_volatile last operand
    auto newCall = llvm::CallInst::Create(callee, {op0, op1, op2});
    instToReplaceWithInst->insert({call, newCall});
}

bool IntrinsicsLowering::HandleCall(llvm::CallInst *call, LLVMArkInterface::IntrinsicId intrinsicId,
                                    std::unordered_map<llvm::Instruction *, llvm::Instruction *> *instToReplaceWithInst)
{
    llvm::StringRef intrinsicName = arkInterface_->GetIntrinsicRuntimeFunctionName(intrinsicId);
    auto intrinsicFunctionTy = arkInterface_->GetRuntimeFunctionType(intrinsicName);

    ASSERT(intrinsicFunctionTy != nullptr);
    auto builder = llvm::IRBuilder<>(call);
    auto type = call->getType();

    auto callee = GetPandaRuntimeFunctionCallee(intrinsicId, intrinsicFunctionTy, &builder, intrinsicName);

    if (type->isVectorTy()) {
        llvm::Value *vec = llvm::UndefValue::get(type);
        auto vecLen = llvm::cast<llvm::VectorType>(type)->getElementCount().getKnownMinValue();
        std::vector<llvm::Value *> args;

        for (uint64_t i = 0; i < vecLen; i++) {
            for (auto &arg : call->args()) {
                args.push_back(builder.CreateExtractElement(arg, i));
            }

            auto newCall = builder.CreateCall(callee, llvm::makeArrayRef(args));
            if (i < vecLen - 1) {
                vec = builder.CreateInsertElement(vec, newCall, i);
            } else {
                vec = llvm::InsertElementInst::Create(vec, newCall, builder.getInt64(i));
            }

            args.clear();
        }

        auto result = llvm::cast<llvm::Instruction>(vec);
        instToReplaceWithInst->insert({call, result});
    } else {
        if (llvm::isa<llvm::MemCpyInst>(call) || llvm::isa<llvm::MemMoveInst>(call) ||
            llvm::isa<llvm::MemSetInst>(call)) {
            HandleMemCall(call, callee, instToReplaceWithInst);
        } else {
            call->setCalledFunction(callee);
        }
    }

    return true;
}

bool IntrinsicsLowering::HandleFRem(llvm::Instruction *inst, LLVMArkInterface::IntrinsicId intrinsicId,
                                    std::unordered_map<llvm::Instruction *, llvm::Instruction *> *instToReplaceWithInst)
{
    ASSERT(inst->getOpcode() == llvm::Instruction::FRem);
    llvm::StringRef intrinsicName = arkInterface_->GetIntrinsicRuntimeFunctionName(intrinsicId);
    auto intrinsicFunctionTy = arkInterface_->GetRuntimeFunctionType(intrinsicName);

    auto arg1 = inst->getOperand(0U);
    auto arg2 = inst->getOperand(1U);
    auto module = inst->getModule();

    auto builder = llvm::IRBuilder<>(inst);
    auto table = module->getGlobalVariable("__aot_got");
    auto arrayType = llvm::ArrayType::get(builder.getInt64Ty(), 0);
    auto pointerToEntrypointAddress = builder.CreateConstInBoundsGEP2_32(arrayType, table, 0, intrinsicId);
    auto entrypointAddress =
        builder.CreateLoad(builder.getInt64Ty(), pointerToEntrypointAddress, intrinsicName + "_address");
    auto calleePointer = builder.CreateIntToPtr(entrypointAddress, builder.getPtrTy(0));

    auto call = llvm::CallInst::Create(intrinsicFunctionTy, calleePointer, {arg1, arg2});
    instToReplaceWithInst->insert({inst, call});

    return true;
}

}  // namespace ark::llvmbackend::passes
