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

/**
 * @file
 *
 * This file declares Block Scope for codegen.
 */

#ifndef CODIRA_BLOCKSCOPEIMPL_H
#define CODIRA_BLOCKSCOPEIMPL_H

#include "llvm/IR/Value.h"

#include "IRBuilder.h"

namespace Codira {
namespace CHIR {
class Func;
}
namespace CodeGen {
class IRBuilder2;

class CodeGenBlockScope {
public:
    CodeGenBlockScope(IRBuilder2& irBuilder, llvm::BasicBlock* bb)
        : irBuilder(irBuilder), oldBb(irBuilder.GetInsertBlock())
    {
        irBuilder.SetInsertPoint(bb);
    }

    CodeGenBlockScope(IRBuilder2& irBuilder, const CHIR::Block& chirBlock)
            : irBuilder(irBuilder), oldBb(irBuilder.GetInsertBlock())
    {
        irBuilder.SetInsertPoint(irBuilder.GetCGModule().GetMappedBB(&chirBlock));
        irBuilder.SetInsertCGFunction(*irBuilder.GetCGModule().GetOrInsertCGFunction(chirBlock.GetTopLevelFunc()));
    }

    ~CodeGenBlockScope()
    {
        irBuilder.SetInsertPoint(oldBb);
    }

private:
    IRBuilder2& irBuilder;
    llvm::BasicBlock* oldBb;
};

class CodeGenFunctionScope {
public:
    CodeGenFunctionScope(IRBuilder2& irBuilder, llvm::Function* function)
        : block(irBuilder, &function->getEntryBlock())
    {
    }

    ~CodeGenFunctionScope() = default;

private:
    CodeGenBlockScope block;
};

class CodeGenUnwindBlockScope {
public:
    CodeGenUnwindBlockScope(CGModule& cgMod, llvm::BasicBlock* unwindBlock) : cgMod(cgMod)
    {
        cgMod.GetCGContext().PushUnwindBlockStack(unwindBlock);
    }

    ~CodeGenUnwindBlockScope()
    {
        cgMod.GetCGContext().PopUnwindBlockStack();
    }

private:
    CGModule& cgMod;
};

} // namespace CodeGen
} // namespace Codira
#endif // CODIRA_BLOCKSCOPEIMPL_H
