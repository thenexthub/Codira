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
#include "LocalTypeArgumentSynthesis.h"
#include "TypeCheckUtil.h"

using namespace Codira;
using namespace Sema;
using namespace TypeCheckUtil;

bool TypeChecker::TypeCheckerImpl::CheckRangeElements(ASTContext& ctx, Ptr<Ty> elemTy, const RangeExpr& re)
{
    bool isWellTyped = !re.startExpr || Check(ctx, elemTy, re.startExpr.get());
    isWellTyped = (!re.stopExpr || Check(ctx, elemTy, re.stopExpr.get())) && isWellTyped;
    // When target type of rangeExpr is not given, the elemTy is type of start or stop.
    bool inconsistent = !ctx.HasTargetTy(&re) && re.startExpr && re.stopExpr && !isWellTyped;
    if (inconsistent) {
        if (!CanSkipDiag(*re.startExpr) && !CanSkipDiag(*re.stopExpr)) {
            diag.Diagnose(re, DiagKind::sema_inconsistency_range_elemType);
        }
    }
    if (isWellTyped && !CheckGenericDeclInstantiation(re.decl, std::vector<Ptr<Ty>>{elemTy}, re)) {
        isWellTyped = false;
    }
    if (re.stepExpr) {
        if (!Check(ctx, TypeManager::GetPrimitiveTy(TypeKind::TYPE_INT64), re.stepExpr.get())) {
            if (!CanSkipDiag(*re.stepExpr)) {
                diag.Diagnose(re, DiagKind::sema_range_step_not_int64);
            }
            isWellTyped = false;
        } else if (re.stepExpr->isConst) {
            // Step expr cannot be zero.
            if (re.stepExpr->constNumValue.asInt.Uint64() == 0) {
                diag.Diagnose(*re.stepExpr, DiagKind::sema_step_non_zero_range);
            }
        }
    }
    return isWellTyped;
}

bool TypeChecker::TypeCheckerImpl::ChkRangeExpr(ASTContext& ctx, Ty& target, RangeExpr& re)
{
    re.ty = TypeManager::GetInvalidTy(); // Set type invalid first, will be updated if check passed.
    re.decl = importManager.GetCoreDecl<StructDecl>("Range");
    if (!re.decl) {
        diag.Diagnose(re, DiagKind::sema_no_core_object);
        return false;
    }
    if (!Ty::IsTyCorrect(&target)) {
        return false;
    }

    auto synAndCheckRangeTy = [this, &ctx, &re, &target]() {
        re.ty = SynRangeExpr(ctx, re);
        bool isWellTyped = typeManager.IsSubtype(re.ty, &target);
        if (!isWellTyped && Ty::IsTyCorrect(re.ty)) {
            DiagMismatchedTypes(diag, re, target);
        }
        return isWellTyped;
    };

    Ptr<Ty> targetTy = TypeCheckUtil::UnboxOptionType(&target);
    bool isWellTyped = Ty::IsTyCorrect(targetTy) && (targetTy->IsStruct() || targetTy->IsInterface());
    if (!isWellTyped) {
        (void)synAndCheckRangeTy();
        re.ty = TypeManager::GetInvalidTy();
        return false;
    }

    auto rangeTy = targetTy;
    if (targetTy->IsInterface()) {
        auto candiTys = promotion.Downgrade(*re.decl->ty, target);
        if (candiTys.empty()) {
            // If range type cannot be inferred from target type, syn range expr.
            if (synAndCheckRangeTy()) {
                return true;
            }
            DiagUnableToInferExpr(diag, re);
            return false;
        }
        rangeTy = *candiTys.begin();
    } else if (!targetTy->IsRange() || rangeTy->typeArgs.size() != 1) {
        return synAndCheckRangeTy();
    }
    if (!CheckRangeElements(ctx, rangeTy->typeArgs[0], re)) {
        return false;
    }
    re.ty = rangeTy;
    isWellTyped = typeManager.IsSubtype(re.ty, &target);
    if (!isWellTyped) {
        DiagMismatchedTypes(diag, re, target);
    }
    return isWellTyped;
}

Ptr<Ty> TypeChecker::TypeCheckerImpl::SynRangeExpr(ASTContext& ctx, RangeExpr& re)
{
    re.ty = TypeManager::GetInvalidTy(); // Set type invalid first, will be updated if check passed.
    re.decl = importManager.GetCoreDecl<StructDecl>("Range");
    if (!re.decl) {
        return re.ty;
    }
    Ptr<Ty> elemTy = SynRangeExprInferElemTy(re, ctx);

    if (Ty::IsTyCorrect(elemTy) && elemTy->IsIdeal() && elemTy->IsInteger()) {
        elemTy = TypeManager::GetPrimitiveTy(TypeKind::TYPE_INT64);
    }

    if (!CheckRangeElements(ctx, elemTy, re)) {
        return re.ty;
    }
    re.ty = typeManager.GetStructTy(*re.decl, {elemTy});
    return re.ty;
}

Ptr<Ty> TypeChecker::TypeCheckerImpl::SynRangeExprInferElemTy(const RangeExpr& re, ASTContext& ctx)
{
    // If type of startExpr is Nothing or startExpr is litconst when only one of start/stop is litconst,
    // using type of stopExpr or default Int64 type.
    Ptr<Ty> startTy = re.startExpr ? Synthesize(ctx, re.startExpr.get()) : TypeManager::GetInvalidTy();
    bool useStartLit = !Is<LitConstExpr>(re.startExpr.get()) || Is<LitConstExpr>(re.stopExpr.get());
    if (Ty::IsTyCorrect(startTy) && !startTy->IsNothing() && useStartLit) {
        return startTy;
    }
    Ptr<Ty> stopTy = re.stopExpr ? Synthesize(ctx, re.stopExpr.get()) : TypeManager::GetInvalidTy();
    if (Ty::IsTyCorrect(stopTy) && !stopTy->IsNothing()) {
        return stopTy;
    } else if (Ty::IsTyCorrect(startTy)) {
        return startTy;
    }
    // Element type is Int64 by default.
    return TypeManager::GetPrimitiveTy(TypeKind::TYPE_INT64);
}
