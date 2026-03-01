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

#include "transforms/passes/ark_gvn.h"
#include "transforms/transform_utils.h"

#include "llvm_ark_interface.h"
#include "transforms/builtins.h"
#include "utils.h"

#include <llvm/Pass.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Verifier.h>
#include <llvm/IR/Dominators.h>
#include <llvm/Transforms/Utils/BasicBlockUtils.h>
#include <llvm/ADT/DenseMapInfo.h>
#include <llvm/ADT/Hashing.h>
#include <llvm/ADT/StringSwitch.h>

namespace ark::llvmbackend::passes {

using ark::llvmbackend::utils::CopyDebugLoc;
using ark::llvmbackend::utils::CopyDeoptBundle;
using ark::llvmbackend::utils::GetMethodIdFromAttr;
using builtins::LoadClass;
using builtins::LoadInitClass;
using builtins::LoadString;
using builtins::ResolveVirtual;

ArkGVN ArkGVN::Create(LLVMArkInterface *arkInterface,
                      [[maybe_unused]] const ark::llvmbackend::LLVMCompilerOptions *options)
{
    return ArkGVN(arkInterface);
}

ArkGVN::ArkGVN(LLVMArkInterface *arkInterface) : arkInterface_ {arkInterface} {}

llvm::PreservedAnalyses ArkGVN::run(llvm::Function &function, llvm::FunctionAnalysisManager &analysisManager)
{
    ASSERT(workStack_.empty());
    ASSERT(bbTables_.empty());
    GvnBuiltins builtins = {LoadClass(function.getParent()), LoadInitClass(function.getParent()),
                            LoadString(function.getParent()), ResolveVirtual(function.getParent())};

    auto &tree = analysisManager.getResult<llvm::DominatorTreeAnalysis>(function);
    bool changed = RunOnFunction(tree, builtins);
    ASSERT(workStack_.empty());
    bbTables_.clear();
    return changed ? llvm::PreservedAnalyses::none() : llvm::PreservedAnalyses::all();
}

void CreateCallBuiltin(llvm::CallInst *callInst, uint32_t compiledEntrypointOffset)
{
    auto isLaunchCall = callInst->hasFnAttr("is-launch-call");
    llvm::Value *thiz = callInst->getArgOperand(isLaunchCall ? callInst->arg_size() - 1 : 1);
    ASSERT(thiz->getType()->isPointerTy());

    auto builder = llvm::IRBuilder<>(callInst);
    auto builtin = ResolveVirtual(callInst->getModule());
    auto zero = builder.getInt64(0);
    auto arrayType = llvm::ArrayType::get(builder.getInt64Ty(), 0);
    auto offset = builder.CreateIntToPtr(zero, arrayType->getPointerTo());

    auto methodId = GetMethodIdFromAttr(callInst);
    auto builtinCallInst =
        builder.CreateCall(builtin, {thiz, builder.getInt64(methodId), offset}, CopyDeoptBundle(callInst));
    CopyDebugLoc(callInst, builtinCallInst);
    if (callInst->hasFnAttr("inline-info")) {
        builtinCallInst->addFnAttr(callInst->getFnAttr("inline-info"));
    }

    if (!isLaunchCall) {
        auto method = builder.CreateIntToPtr(builtinCallInst, builder.getPtrTy());
        auto *fType = callInst->getFunctionType();
        auto entrypointPtr = builder.CreateConstInBoundsGEP1_32(builder.getInt8Ty(), method, compiledEntrypointOffset);
        auto entrypoint = builder.CreateLoad(builder.getPtrTy(), entrypointPtr);
        callInst->setCalledFunction(fType, entrypoint);
    }
    callInst->setArgOperand(0, builtinCallInst);
    callInst->setAttributes(callInst->getAttributes().removeFnAttribute(callInst->getContext(), "original-method-id"));
}

bool ArkGVN::RunOnFunction(const llvm::DominatorTree &tree, const GvnBuiltins &builtins)
{
    bool changed = false;
    workStack_.push_back(tree.getRoot());
    while (!workStack_.empty()) {
        auto block = workStack_.back();
        changed |= RunOnBasicBlock(block, tree, builtins);
        workStack_.pop_back();
        for (const auto &child : tree.getNode(block)->children()) {
            workStack_.push_back(child->getBlock());
        }
    }
    return changed;
}

bool ArkGVN::RunOnBasicBlock(llvm::BasicBlock *block, const llvm::DominatorTree &tree, const GvnBuiltins &builtins)
{
    bool changed = false;
    llvm::SmallVector<llvm::CallInst *> replace;
    for (auto &inst : *block) {
        auto callInst = llvm::dyn_cast<llvm::CallInst>(&inst);
        if (callInst != nullptr && callInst->hasFnAttr("original-method-id") &&
            (arkInterface_->IsInterfaceMethod(callInst) || callInst->hasFnAttr("is-launch-call"))) {
            replace.push_back(callInst);
        }
    }
    for (auto &inst : replace) {
        CreateCallBuiltin(inst, arkInterface_->GetCompiledEntryPointOffset());
        changed = true;
    }
    for (auto iter = block->begin(), endIter = block->end(); iter != endIter;) {
        auto callInst = llvm::dyn_cast<llvm::CallInst>(&*(iter++));
        if (callInst == nullptr) {
            continue;
        }

        auto builtinKey = ParseBuiltin(callInst, builtins);
        if (builtinKey.builtinTy == BuiltinType::NONE) {
            continue;
        }

        llvm::Value *alternative = FindDominantCall(builtinKey, block, tree);
        if (alternative != nullptr) {
            ASSERT(tree.dominates(alternative, callInst));
            // need replace instruction by another
            callInst->replaceAllUsesWith(alternative);
            callInst->eraseFromParent();
            changed = true;
            continue;
        }
        if (builtinKey.builtinTy == RESOLVE_VIRTUAL_METHOD && arkInterface_->IsArm64() &&
            llvm::isa<llvm::ConstantInt>(callInst->getOperand(1))) {
            auto slotId = arkInterface_->CreateIntfInlineCacheSlotId(callInst->getFunction());
            auto aotGot = callInst->getModule()->getGlobalVariable("__aot_got");
            auto builder = llvm::IRBuilder<>(callInst);
            auto arrayType = llvm::ArrayType::get(builder.getInt64Ty(), 0);
            llvm::Value *slot = builder.CreateConstInBoundsGEP2_64(arrayType, aotGot, 0, slotId);
            callInst->setArgOperand(2U, slot);
            changed = true;
        }

        bbTables_[block].insert({builtinKey, callInst});
        if (builtinKey.builtinTy == LOAD_AND_INIT_CLASS) {
            BuiltinKey loadClass = builtinKey;
            loadClass.builtinTy = LOAD_CLASS;
            bbTables_[block].insert({loadClass, callInst});
        }
    }
    return changed;
}

ArkGVN::BuiltinKey ArkGVN::ParseBuiltin(const llvm::CallInst *callInst, const GvnBuiltins &builtins)
{
    BuiltinKey result;
    if (callInst == nullptr) {
        return result;
    }
    auto calledFunc = callInst->getCalledFunction();
    if (calledFunc == nullptr) {
        return result;
    }
    auto builtin = std::find(builtins.begin(), builtins.end(), calledFunc);
    if (builtin == builtins.end()) {
        return result;
    }

    auto builtinTy = static_cast<BuiltinType>(builtin - builtins.begin());
    // Parse arguments
    if (builtinTy == LOAD_AND_INIT_CLASS || builtinTy == LOAD_CLASS || builtinTy == LOAD_STRING) {
        auto typeId = callInst->getOperand(0U);
        if (!llvm::isa<llvm::ConstantInt>(typeId)) {
            return result;
        }
        result.args.push_back(typeId);
    } else if (builtinTy == RESOLVE_VIRTUAL_METHOD) {
        auto thiz = callInst->getOperand(0U);
        auto methodId = callInst->getOperand(1U);

        result.args.push_back(thiz);
        result.args.push_back(methodId);
    } else {
        llvm_unreachable("Unsupported builtin type");
    }

    result.builtinTy = builtinTy;

    return result;
}

llvm::Value *ArkGVN::FindDominantCall(const ArkGVN::BuiltinKey &curBuiltin, llvm::BasicBlock *block,
                                      const llvm::DominatorTree &tree)
{
    do {
        auto table = bbTables_[block];
        auto dominant = table.find(curBuiltin);
        if (dominant != table.end()) {
            return dominant->second;
        }
        auto idom = tree.getNode(block)->getIDom();
        block = idom == nullptr ? nullptr : idom->getBlock();
    } while (block != nullptr);
    return nullptr;
}

}  // namespace ark::llvmbackend::passes
