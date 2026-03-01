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

#include "mark_inline_module.h"
#include "llvm_ark_interface.h"
#include "llvm_compiler_options.h"

#include <llvm/IR/Module.h>
#include <llvm/Support/Debug.h>

#define DEBUG_TYPE "mark-inline-module"

using llvm::GlobalObject;

namespace ark::llvmbackend::passes {

bool MarkInlineModule::ShouldInsert(const ark::llvmbackend::LLVMCompilerOptions *options)
{
    return options->doIrtocInline;
}

/// Mark all functions and global variables from inline module with FUNCTION_MD_INLINE_MODULE metadata
llvm::PreservedAnalyses MarkInlineModule::run(llvm::Module &module, llvm::ModuleAnalysisManager & /*AM*/)
{
    llvm::for_each(module.global_objects(), MarkInlineModule::Mark);
    return module.global_objects().empty() ? llvm::PreservedAnalyses::all() : llvm::PreservedAnalyses::none();
}

void MarkInlineModule::Mark(GlobalObject &object)
{
    LLVM_DEBUG(llvm::dbgs() << "Marked '" << object.getName() << "'\n");
    object.addMetadata(LLVMArkInterface::FUNCTION_MD_INLINE_MODULE, *llvm::MDNode::get(object.getContext(), {}));
}

}  // namespace ark::llvmbackend::passes
