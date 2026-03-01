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

#include "Diags.h"

using namespace Codira;
using namespace Sema;

Ptr<Ty> TypeChecker::TypeCheckerImpl::SynIsExpr(ASTContext& ctx, IsExpr& ie)
{
    if (Ty::IsTyCorrect(Synthesize(ctx, ie.leftExpr.get())) && Ty::IsTyCorrect(Synthesize(ctx, ie.isType.get())) &&
        ReplaceIdealTy(*ie.leftExpr)) {
        ie.ty = TypeManager::GetPrimitiveTy(TypeKind::TYPE_BOOLEAN);
    } else {
        ie.ty = TypeManager::GetInvalidTy();
    }
    return ie.ty;
}

bool TypeChecker::TypeCheckerImpl::ChkIsExpr(ASTContext& ctx, Ty& target, IsExpr& ie)
{
    // Always type checking the expression even if the target type mismatches.
    auto ty = SynIsExpr(ctx, ie);
    if (!Ty::IsTyCorrect(ty)) {
        return false;
    }
    bool isWellTyped = ty->IsBoolean();

    auto boolTy = TypeManager::GetPrimitiveTy(TypeKind::TYPE_BOOLEAN);
    if (!typeManager.IsLitBoxableType(boolTy, &target)) {
        DiagMismatchedTypesWithFoundTy(diag, ie, target, *boolTy);
        isWellTyped = false;
    }

    ie.ty = isWellTyped ? ie.ty : TypeManager::GetInvalidTy();
    return isWellTyped;
}

Ptr<Ty> TypeChecker::TypeCheckerImpl::SynAsExpr(ASTContext& ctx, AsExpr& ae)
{
    if (Ty::IsTyCorrect(Synthesize(ctx, ae.leftExpr.get())) && Ty::IsTyCorrect(Synthesize(ctx, ae.asType.get())) &&
        ReplaceIdealTy(*ae.leftExpr)) {
        auto optionDecl = RawStaticCast<EnumDecl*>(importManager.GetCoreDecl("Option"));
        if (optionDecl) {
            ae.ty = typeManager.GetEnumTy(*optionDecl, {ae.asType->ty});
        } else {
            diag.Diagnose(ae, DiagKind::sema_no_core_object);
            ae.ty = TypeManager::GetInvalidTy();
        }
    } else {
        ae.ty = TypeManager::GetInvalidTy();
    }
    return ae.ty;
}

bool TypeChecker::TypeCheckerImpl::ChkAsExpr(ASTContext& ctx, Ty& target, AsExpr& ae)
{
    if (!Ty::IsTyCorrect(SynAsExpr(ctx, ae))) {
        return false;
    }
    if (!CheckOptionBox(target, *ae.ty)) {
        DiagMismatchedTypes(diag, ae, target);
        ae.ty = TypeManager::GetInvalidTy();
        return false;
    }
    return true;
}
