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

#include "remove_unused_functions.h"
#include "llvm_ark_interface.h"
#include "inline_ir_utils.h"
#include "llvm_compiler_options.h"

#include <llvm/Pass.h>
#include <llvm/IR/InlineAsm.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Transforms/Utils/BasicBlockUtils.h>
#include <llvm/Transforms/IPO/FunctionImport.h>

#define DEBUG_TYPE "remove-unused-functions"

using llvm::Argument;
using llvm::BasicBlock;
using llvm::cast;
using llvm::convertToDeclaration;
using llvm::DenseSet;
using llvm::Function;
using llvm::InlineAsm;
using llvm::isa;
using llvm::User;
using llvm::Value;

namespace ark::llvmbackend::passes {

bool RemoveUnusedFunctions::ShouldInsert(const ark::llvmbackend::LLVMCompilerOptions *options)
{
    return options->doIrtocInline;
}

llvm::PreservedAnalyses RemoveUnusedFunctions::run(llvm::Module &module, llvm::ModuleAnalysisManager & /*AM*/)
{
    DenseSet<Function *> usedFunctions;
    for (auto &function : module.functions()) {
        if (function.getMetadata(LLVMArkInterface::FUNCTION_MD_INLINE_MODULE) != nullptr) {
            LLVM_DEBUG(llvm::dbgs() << "Skip " << function.getName() << " from inline module\n");
            continue;
        }
        if (function.isDeclaration()) {
            continue;
        }
        LLVM_DEBUG(llvm::dbgs() << function.getName() << " is root\n");
        DenseSet<Value *> seen;
        VisitValue(usedFunctions, function, seen);
    }

    bool changed = false;
    for (auto &function : module.functions()) {
        if (!usedFunctions.contains(&function)) {
            LLVM_DEBUG(llvm::dbgs() << "Deleted body of " << function.getName() << "\n");
            convertToDeclaration(function);
            changed = true;
        }
    }

    changed |= ark::llvmbackend::RemoveDanglingAliases(module);
    return changed ? llvm::PreservedAnalyses::none() : llvm::PreservedAnalyses::all();
}
void RemoveUnusedFunctions::VisitValue(DenseSet<Function *> &usedFunctions, Value &value, DenseSet<Value *> &seenValues)
{
    if (seenValues.contains(&value)) {
        return;
    }

    seenValues.insert(&value);

    if (isa<Function>(value)) {
        auto &function = cast<Function>(value);
        if (usedFunctions.contains(&function)) {
            return;
        }

        usedFunctions.insert(&function);
        DenseSet<Value *> seen;
        for (auto &basicBlock : function) {
            VisitValue(usedFunctions, basicBlock, seen);
        }
    } else if (isa<BasicBlock>(value)) {
        auto &basicBlock = cast<BasicBlock>(value);
        for (auto &instruction : basicBlock) {
            VisitValue(usedFunctions, instruction, seenValues);
        }
    } else if (isa<User>(value)) {
        auto &user = cast<User>(value);
        for (auto operand : user.operand_values()) {
            VisitValue(usedFunctions, *operand, seenValues);
        }
    } else {
        if (isa<Argument, llvm::MetadataAsValue, InlineAsm>(value)) {
            return;
        }
        LLVM_DEBUG(llvm::dbgs() << "Value = ");
        LLVM_DEBUG(value.print(llvm::dbgs()));
        LLVM_DEBUG(llvm::dbgs() << "\n");
        llvm_unreachable("Unexpected value");
    }
}

}  // namespace ark::llvmbackend::passes
