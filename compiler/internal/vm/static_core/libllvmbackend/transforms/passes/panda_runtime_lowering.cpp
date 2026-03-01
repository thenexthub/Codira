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

#include "transforms/passes/panda_runtime_lowering.h"

#include "llvm_ark_interface.h"
#include "lowering/metadata.h"
#include "transforms/runtime_calls.h"
#include "transforms/builtins.h"
#include "transforms/transform_utils.h"
#include "utils.h"

#include <llvm/IR/DebugInfoMetadata.h>
#include <llvm/IR/InlineAsm.h>
#include <llvm/IR/MDBuilder.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Transforms/Utils/BasicBlockUtils.h>
#include <llvm/Pass.h>

namespace ark::llvmbackend::passes {

PandaRuntimeLowering PandaRuntimeLowering::Create(llvmbackend::LLVMArkInterface *arkInterface,
                                                  [[maybe_unused]] const ark::llvmbackend::LLVMCompilerOptions *options)
{
    return PandaRuntimeLowering(arkInterface);
}

PandaRuntimeLowering::PandaRuntimeLowering(LLVMArkInterface *arkInterface) : arkInterface_ {arkInterface} {}

llvm::PreservedAnalyses PandaRuntimeLowering::run(llvm::Function &function, llvm::FunctionAnalysisManager & /*am*/)
{
    ASSERT(arkInterface_ != nullptr);
    bool changed = false;
    bool hasDeopt = false;
    for (llvm::BasicBlock &block : function) {
        llvm::SmallVector<llvm::CallInst *> calls;
        for (llvm::Instruction &inst : block) {
            auto call = llvm::dyn_cast<llvm::CallInst>(&inst);
            if (call == nullptr) {
                continue;
            }
            if (call->hasFnAttr("may-deoptimize")) {
                hasDeopt = true;
            }
            if (NeedsToBeLowered(call)) {
                calls.push_back(call);
            }
        }
        for (auto call : calls) {
            auto callee = call->getCalledFunction();
            ASSERT(callee != nullptr &&
                   (!callee->isIntrinsic() || callee->getIntrinsicID() == llvm::Intrinsic::experimental_deoptimize));

            // LLVM is able to process pure recursive calls, but they may confuse StackWalker after deoptimization
            if (callee == &function && !hasDeopt) {
                continue;
            }

            if (callee->getIntrinsicID() == llvm::Intrinsic::experimental_deoptimize) {
                LowerDeoptimizeIntrinsic(call);
            } else if (callee->getSectionPrefix() && callee->getSectionPrefix()->equals(builtins::BUILTIN_SECTION)) {
                LowerBuiltin(call);
            } else if (arkInterface_->IsRememberedCall(&function, callee)) {
                LowerCallStatic(call);
            } else if (call->hasFnAttr("original-method-id")) {
                LowerCallVirtual(call);
            } else {
                llvm_unreachable("Do not know how to lower a call - unknown call type.");
            }
            changed = true;
        }
    }
    return changed ? llvm::PreservedAnalyses::none() : llvm::PreservedAnalyses::all();
}

void PandaRuntimeLowering::LowerCallStatic(llvm::CallInst *inst)
{
    auto epOffset = llvm::ConstantInt::get(llvm::Type::getInt64Ty(inst->getContext()),
                                           arkInterface_->GetCompiledEntryPointOffset());

    auto builder = llvm::IRBuilder<>(inst);
    auto calleePtr = GetMethodOrResolverPtr(&builder, inst);
    // Calculate address of entry point
    auto calleeAddr = builder.CreateInBoundsGEP(builder.getInt8Ty(), calleePtr, epOffset, "ep_addr");
    // Cast entry point address to a pointer to callee function pointer
    auto ftype = inst->getFunctionType();
    // Load function pointer
    ASSERT(inst->getCalledFunction() != nullptr);
    auto calleeExec = builder.CreateLoad(builder.getPtrTy(), calleeAddr, {inst->getCalledFunction()->getName(), "_p"});
    // Update call inst
    inst->setCalledFunction(ftype, calleeExec);
    inst->setArgOperand(0, calleePtr);
    if (!arkInterface_->IsArm64()) {
        inst->setCallingConv(llvm::CallingConv::ArkPlt);
    }
    inst->addFnAttr(llvm::Attribute::get(inst->getContext(), "use-ark-spills"));
}

void PandaRuntimeLowering::LowerCallVirtual(llvm::CallInst *inst)
{
    ASSERT(!arkInterface_->IsInterfaceMethod(inst));
    auto builder = llvm::IRBuilder<>(inst);
    llvm::Value *thiz = inst->getArgOperand(1);
    auto methodId = ark::llvmbackend::utils::GetMethodIdFromAttr(inst);
    auto func = inst->getFunction();
    auto method = ark::llvmbackend::utils::CreateLoadMethodUsingVTable(thiz, func, methodId, &builder, arkInterface_);

    auto offset = arkInterface_->GetCompiledEntryPointOffset();
    auto calleeAdr = builder.CreateConstInBoundsGEP1_32(builder.getInt8Ty(), method, offset);
    auto calleeP = builder.CreateLoad(builder.getPtrTy(), calleeAdr);

    inst->setCalledFunction(inst->getFunctionType(), calleeP);
    inst->setArgOperand(0, method);
}

llvm::Value *PandaRuntimeLowering::GetMethodOrResolverPtr(llvm::IRBuilder<> *builder, llvm::CallInst *inst)
{
    auto slot = arkInterface_->GetPltSlotId(inst->getCaller(), inst->getCalledFunction());
    auto block = builder->GetInsertBlock();
    auto aotGot = block->getModule()->getGlobalVariable("__aot_got");
    auto arrayType = llvm::ArrayType::get(builder->getInt64Ty(), 0);
    auto methodPtr = builder->CreateConstInBoundsGEP2_64(arrayType, aotGot, 0, slot);
    auto cachedMethodAddr = builder->CreateLoad(builder->getInt64Ty(), methodPtr);
    return builder->CreateIntToPtr(cachedMethodAddr, builder->getPtrTy(), "method_ptr");
}

void PandaRuntimeLowering::LowerBuiltin(llvm::CallInst *inst)
{
    auto builder = llvm::IRBuilder<>(inst);
    auto lowered = builtins::LowerBuiltin(&builder, inst, arkInterface_);
    if (lowered != nullptr) {
        llvm::BasicBlock::iterator ii(inst);
        ReplaceInstWithValue(inst->getParent()->getInstList(), ii, lowered);
    } else {
        ASSERT(inst->use_empty());
        ASSERT(inst->getFunctionType()->getReturnType()->isVoidTy());
        inst->eraseFromParent();
    }
}

bool PandaRuntimeLowering::NeedsToBeLowered(llvm::CallInst *call)
{
    auto callee = call->getCalledFunction();
    if (callee == nullptr ||
        (callee->isIntrinsic() && callee->getIntrinsicID() != llvm::Intrinsic::experimental_deoptimize)) {
        return false;
    }
    ASSERT((callee->getSectionPrefix() && callee->getSectionPrefix()->equals(builtins::BUILTIN_SECTION)) ||
           arkInterface_->IsRememberedCall(call->getCaller(), callee) || call->hasFnAttr("original-method-id") ||
           call->getIntrinsicID() == llvm::Intrinsic::experimental_deoptimize);
    return true;
}

void PandaRuntimeLowering::LowerDeoptimizeIntrinsic(llvm::CallInst *deoptimize)
{
    ASSERT(deoptimize->getIntrinsicID() == llvm::Intrinsic::experimental_deoptimize);

    llvm::IRBuilder<> builder {deoptimize};
    ASSERT(deoptimize->arg_size() > 0);

    // The last argument is the entrypoint id
    llvm::Value *entrypointIdValue = deoptimize->getArgOperand(deoptimize->arg_size() - 1U);
    uint64_t entrypointId = llvm::cast<llvm::ConstantInt>(entrypointIdValue)->getZExtValue();

    // Drop last argument
    llvm::SmallVector<llvm::Value *> args;
    for (auto &arg : llvm::drop_end(deoptimize->args())) {
        args.push_back(arg.get());
    }

    auto entrypointCall = llvmbackend::runtime_calls::CreateEntrypointCallCommon(
        &builder, runtime_calls::GetThreadRegValue(&builder, arkInterface_), arkInterface_,
        static_cast<llvmbackend::runtime_calls::EntrypointId>(entrypointId), args, utils::CopyDeoptBundle(deoptimize));
    // Copy attributes
    entrypointCall->setDebugLoc(deoptimize->getDebugLoc());

    // Remove return after llvm.experimental.deoptimize call
    auto basicBlock = entrypointCall->getParent();
    auto terminator = basicBlock->getTerminator();
    ASSERT(llvm::isa<llvm::ReturnInst>(terminator));
    terminator->eraseFromParent();

    // Erase the llvm.experimental.deoptimize call
    ASSERT(deoptimize->getNumUses() == 0);
    deoptimize->eraseFromParent();

    // Create unreachable instead of removed return
    builder.SetInsertPoint(basicBlock);
    builder.CreateUnreachable();
}

}  // namespace ark::llvmbackend::passes
