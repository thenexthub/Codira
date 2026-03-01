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

#include "discard_inline_module.h"

#include "llvm_ark_interface.h"
#include "inline_ir_utils.h"
#include "llvm_compiler_options.h"

#include <llvm/IR/Module.h>
#include <llvm/IR/Function.h>
#include <llvm/Pass.h>
#include <llvm/Support/Debug.h>
#include <llvm/Transforms/IPO/FunctionImport.h>

#define DEBUG_TYPE "discard-inline-module"

using llvm::convertToDeclaration;

namespace ark::llvmbackend::passes {

bool DiscardInlineModule::ShouldInsert([[maybe_unused]] const ark::llvmbackend::LLVMCompilerOptions *options)
{
    return true;
}

/// Discard all functions and global variables from inline module
llvm::PreservedAnalyses DiscardInlineModule::run(llvm::Module &module, llvm::ModuleAnalysisManager & /*AM*/)
{
    bool changed = false;
    for (auto &object : module.global_objects()) {
        changed |= DiscardIfNecessary(&object);
    }
    changed |= RemoveDanglingAliases(module);
    return changed ? llvm::PreservedAnalyses::none() : llvm::PreservedAnalyses::all();
}
bool DiscardInlineModule::DiscardIfNecessary(llvm::GlobalObject *object)
{
    if (object->hasMetadata(LLVMArkInterface::FUNCTION_MD_INLINE_MODULE)) {
        if (!ShouldKeep(*object)) {
            LLVM_DEBUG(llvm::dbgs() << "Removed '" << object->getName() << "'\n");
            convertToDeclaration(*object);
            return true;
        }
    }
    LLVM_DEBUG(llvm::dbgs() << "Keeping '" << object->getName() << "'\n");
    return false;
}

bool DiscardInlineModule::ShouldKeep(const llvm::GlobalValue &globalValue) const
{
    // Example: static function, constant global variable
    return globalValue.hasLocalLinkage()
           /**
            * Examples:
            *
            * 1. Function template instantiations
            * 2. When a class declaration defines function. Example:
            *     class Foo {
            *         int bar() {
            *             return 42;
            *         }
            *     }
            *
            * We keep such functions because multiple definitions are allowed for them.
            * The linker will choose appropriate
            */
           || globalValue.hasLinkOnceLinkage();
}

}  // namespace ark::llvmbackend::passes
