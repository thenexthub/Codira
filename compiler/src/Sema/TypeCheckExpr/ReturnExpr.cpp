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

using namespace Codira;
using namespace AST;

// The function does not need a target type since bottom type is a subtype of all (target) types.
bool TypeChecker::TypeCheckerImpl::ChkReturnExpr(ASTContext& ctx, ReturnExpr& re)
{
    return Ty::IsTyCorrect(SynReturnExpr(ctx, re));
}

Ptr<Ty> TypeChecker::TypeCheckerImpl::SynReturnExpr(ASTContext& ctx, ReturnExpr& re)
{
    if (!re.refFuncBody || !re.refFuncBody->retType) {
        re.ty = TypeManager::GetInvalidTy();
        return re.ty;
    }

    CODEC_ASSERT(re.expr);
    bool isWellTyped = true;
    re.ty = TypeManager::GetInvalidTy();

    // Analyse re.expr.
    auto retTy = re.refFuncBody->retType->ty;
    if (Ty::IsTyCorrect(retTy) && !retTy->IsQuest()) {
        bool isInConstructor = re.refFuncBody->funcDecl && IsInstanceConstructor(*re.refFuncBody->funcDecl);
        if (isInConstructor) {
            isWellTyped = CheckReturnInConstructors(ctx, re) && isWellTyped;
        } else {
            isWellTyped = Check(ctx, retTy, re.expr.get());
        }
        if (isWellTyped) {
            ctx.targetTypeMap[re.expr.get()] = re.expr->ty;
        }
    } else {
        isWellTyped = Synthesize(ctx, re.expr.get()) && ReplaceIdealTy(*re.expr);
    }

    // Replace ClassThisTy to ClassTy when the function's outer declaration is not Class or Extend which extends class.
    if (!Is<ClassDecl>(re.refFuncBody->parentClassLike)) {
        if (auto ctt = DynamicCast<ClassThisTy*>(re.expr->ty); ctt && ctt->decl) {
            re.expr->ty = ctt->decl->ty;
        }
    }

    // Generic decls imported from foreign code and created by auto-sdk have no body, no need to check return.
    if (!isWellTyped && NeedCheckBodyReturn(*re.refFuncBody)) {
        re.ty = TypeManager::GetInvalidTy();
    } else {
        re.ty = TypeManager::GetNothingTy();
    }

    return re.ty;
}

bool TypeChecker::TypeCheckerImpl::CheckReturnInConstructors(ASTContext& ctx, const ReturnExpr& re)
{
    CODEC_NULLPTR_CHECK(re.expr);
    return Check(ctx, TypeManager::GetPrimitiveTy(TypeKind::TYPE_UNIT), re.expr.get());
}

bool TypeChecker::TypeCheckerImpl::NeedCheckBodyReturn(const FuncBody& fb) const
{
    if (fb.parentClassLike && HasJavaAttr(*fb.parentClassLike) &&
        fb.parentClassLike->TestAttr(Attribute::GENERIC, Attribute::IMPORTED)) {
        return false;
    }
    return !(fb.funcDecl && HasJavaAttr(*fb.funcDecl) &&
        fb.funcDecl->TestAttr(Attribute::GENERIC, Attribute::IMPORTED));
}
