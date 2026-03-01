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

#include "TypeCheckerImpl.h"

#include "Codira/AST/ASTCasting.h"

using namespace Codira;
using namespace AST;

namespace {
// Find the closest loop expression where the jump expression locates.
Ptr<Expr> FindLoopExpr(const ASTContext& ctx, const JumpExpr& jumpExpr)
{
    auto sym = ScopeManager::GetRefLoopSymbol(ctx, jumpExpr);
    return sym ? DynamicCast<Expr*>(sym->node) : nullptr;
}
} // namespace

Ptr<Ty> TypeChecker::TypeCheckerImpl::SynLoopControlExpr(const ASTContext& ctx, JumpExpr& je) const
{
    je.refLoop = FindLoopExpr(ctx, je);
    // je.refLoop may be a null pointer, but the errors are already reported by CheckReturnAndJump in PreCheck
    je.ty = je.refLoop ? RawStaticCast<Ty*>(TypeManager::GetNothingTy()) : TypeManager::GetInvalidTy();
    return je.ty;
}

bool TypeChecker::TypeCheckerImpl::ChkLoopControlExpr(const ASTContext& ctx, JumpExpr& je) const
{
    SynLoopControlExpr(ctx, je);
    return je.refLoop != nullptr;
}
