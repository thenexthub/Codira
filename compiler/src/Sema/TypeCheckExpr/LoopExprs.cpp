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
#include "TypeCheckUtil.h"

using namespace Codira;
using namespace Sema;
using namespace TypeCheckUtil;

namespace {
Ptr<Ty> GetIterableTy(TypeManager& tyMgr, ImportManager& importManager, Promotion& promotion, Ty& ty)
{
    // Promote implemented iterable type except nothing type.
    if (ty.IsNothing()) {
        return TypeManager::GetInvalidTy();
    }
    auto iterableInterface = importManager.GetCoreDecl("Iterable");
    if (auto genTy = DynamicCast<GenericsTy*>(&ty); genTy && genTy->isPlaceholder) {
        if (auto placeholderItTy = tyMgr.ConstrainByCtor(*genTy, *iterableInterface->ty)) {
            return placeholderItTy;
        } else {
            return TypeManager::GetInvalidTy();
        }
    }
    if (iterableInterface) {
        auto prTys = promotion.Promote(ty, *iterableInterface->ty);
        CODEC_ASSERT(prTys.size() <= 1);
        return prTys.empty() ? TypeManager::GetInvalidTy() : *prTys.begin();
    }
    return TypeManager::GetInvalidTy();
}
} // namespace

bool TypeChecker::TypeCheckerImpl::ChkWhileExpr(ASTContext& ctx, Ty& target, WhileExpr& we)
{
    Ptr<Ty> unitTy = TypeManager::GetPrimitiveTy(TypeKind::TYPE_UNIT);
    bool isWellTyped = typeManager.IsSubtype(unitTy, &target);
    if (!isWellTyped) {
        DiagMismatchedTypesWithFoundTy(diag, we, target, *unitTy);
    }
    isWellTyped = Ty::IsTyCorrect(SynWhileExpr(ctx, we)) && isWellTyped;
    return isWellTyped;
}

Ptr<Ty> TypeChecker::TypeCheckerImpl::SynWhileExpr(ASTContext& ctx, WhileExpr& we)
{
    bool isWellTyped = CheckCondition(ctx, *we.condExpr, false);
    isWellTyped = Ty::IsTyCorrect(Synthesize(ctx, we.body.get())) && isWellTyped;
    we.ty =
        isWellTyped ? StaticCast<Ty*>(TypeManager::GetPrimitiveTy(TypeKind::TYPE_UNIT)) : TypeManager::GetInvalidTy();
    return we.ty;
}

bool TypeChecker::TypeCheckerImpl::ChkDoWhileExpr(ASTContext& ctx, Ty& target, DoWhileExpr& dwe)
{
    Ptr<Ty> unitTy = TypeManager::GetPrimitiveTy(TypeKind::TYPE_UNIT);
    bool isWellTyped = typeManager.IsSubtype(unitTy, &target);
    if (!isWellTyped) {
        DiagMismatchedTypesWithFoundTy(diag, dwe, target, *unitTy);
    }
    isWellTyped = Ty::IsTyCorrect(SynDoWhileExpr(ctx, dwe)) && isWellTyped;
    return isWellTyped;
}

Ptr<Ty> TypeChecker::TypeCheckerImpl::SynDoWhileExpr(ASTContext& ctx, DoWhileExpr& dwe)
{
    bool isWellTyped = Ty::IsTyCorrect(Synthesize(ctx, dwe.body.get()));
    isWellTyped = CheckCondition(ctx, *dwe.condExpr, false) && isWellTyped;
    dwe.ty =
        isWellTyped ? StaticCast<Ty*>(TypeManager::GetPrimitiveTy(TypeKind::TYPE_UNIT)) : TypeManager::GetInvalidTy();
    return dwe.ty;
}

bool TypeChecker::TypeCheckerImpl::ChkForInExpr(ASTContext& ctx, Ty& target, ForInExpr& fie)
{
    Ptr<Ty> unitTy = TypeManager::GetPrimitiveTy(TypeKind::TYPE_UNIT);
    bool isWellTyped = typeManager.IsSubtype(unitTy, &target);
    if (!isWellTyped) {
        DiagMismatchedTypesWithFoundTy(diag, fie, target, *unitTy);
    }
    isWellTyped = Ty::IsTyCorrect(SynForInExpr(ctx, fie)) && isWellTyped;
    if (!isWellTyped) {
        fie.ty = TypeManager::GetInvalidTy();
    }
    return isWellTyped;
}

Ptr<Ty> TypeChecker::TypeCheckerImpl::SynForInExpr(ASTContext& ctx, ForInExpr& fie)
{
    CODEC_NULLPTR_CHECK(fie.inExpression);
    CODEC_NULLPTR_CHECK(fie.pattern);

    bool isWellTyped = Synthesize(ctx, fie.inExpression.get()) && ReplaceIdealTy(*fie.inExpression);

    // Implemented iterable in stdlib.
    CODEC_NULLPTR_CHECK(fie.inExpression->ty);
    Ptr<Ty> iterableTy = GetIterableTy(typeManager, importManager, promotion, *fie.inExpression->ty);
    Ptr<Ty> inPatternTy = TypeManager::GetInvalidTy();
    if (Ty::IsTyCorrect(iterableTy)) {
        CODEC_ASSERT(!iterableTy->typeArgs.empty());
        inPatternTy = iterableTy->typeArgs[0];
    } else {
        isWellTyped = false;
        if (!CanSkipDiag(*fie.inExpression)) {
            diag.Diagnose(
                *fie.inExpression, DiagKind::sema_expr_in_forin_must_has_iterator, Ty::ToString(fie.inExpression->ty));
        }
    }

    isWellTyped = Check(ctx, inPatternTy, fie.pattern.get()) && isWellTyped;
    if (fie.patternGuard) {
        // PatternGuard's ty should be boolean.
        if (!Check(ctx, TypeManager::GetPrimitiveTy(TypeKind::TYPE_BOOLEAN), fie.patternGuard.get())) {
            isWellTyped = false;
            if (!CanSkipDiag(*fie.patternGuard)) {
                diag.Diagnose(*fie.patternGuard, DiagKind::sema_wrong_forin_guard);
            }
        }
    }

    isWellTyped = Ty::IsTyCorrect(Synthesize(ctx, fie.body.get())) && isWellTyped;
    if (!IsIrrefutablePattern(*fie.pattern)) {
        isWellTyped = false;
        diag.Diagnose(fie, DiagKind::sema_forin_pattern_must_be_irrefutable);
    }

    fie.ty =
        isWellTyped ? StaticCast<Ty*>(TypeManager::GetPrimitiveTy(TypeKind::TYPE_UNIT)) : TypeManager::GetInvalidTy();
    return fie.ty;
}
