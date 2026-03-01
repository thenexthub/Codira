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

#include "inline_ir_utils.h"

#include <llvm/IR/Module.h>
#include <llvm/IR/Function.h>

using llvm::Constant;
using llvm::dyn_cast;
using llvm::GlobalAlias;
using llvm::GlobalValue;
using llvm::Module;
using llvm::SmallVector;

namespace ark::llvmbackend {

/**
 * Remove dangling alias from the module
 *
 * A dangled alias points to declaration instead of definitions.
 * LLVM IR verifier disallows these aliases
 *
 * @return true if at least one alias was removed
 */
bool RemoveDanglingAliases(Module &module)
{
    SmallVector<GlobalAlias *> aliases;
    for (GlobalAlias &alias : module.aliases()) {
        const Constant *aliasee = alias.getAliasee();
        const auto *gv = dyn_cast<GlobalValue>(aliasee);
        if (gv != nullptr && gv->isDeclarationForLinker()) {
            aliases.push_back(&alias);
        }
    }
    for (auto alias : aliases) {
        alias->replaceAllUsesWith(alias->getAliasee());
        alias->eraseFromParent();
    }
    return !aliases.empty();
}

}  // namespace ark::llvmbackend
