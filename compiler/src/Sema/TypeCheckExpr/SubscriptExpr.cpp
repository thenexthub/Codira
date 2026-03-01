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

#include "Desugar/DesugarInTypeCheck.h"
#include "DiagSuppressor.h"
#include "Diags.h"
#include "TypeCheckUtil.h"

#include "Codira/AST/Match.h"
#include "Codira/AST/RecoverDesugar.h"

using namespace Codira;
using namespace Sema;
using namespace TypeCheckUtil;

bool TypeChecker::TypeCheckerImpl::ChkSubscriptExpr(ASTContext& ctx, Ptr<Ty> target, SubscriptExpr& se)
{
    if (se.desugarExpr) {
        return typeManager.IsSubtype(se.desugarExpr->ty, target);
    }
    se.ty = TypeManager::GetInvalidTy(); // Set invalid ty at first, will be updated later.
    bool invalid = !se.baseExpr || se.indexExprs.empty();
    if (invalid) {
        return false;
    }
    SetIsNotAlone(*se.baseExpr);
    Ptr<Ty> baseTy = Synthesize(ctx, se.baseExpr.get());
    std::vector<Ptr<Ty>> indexTys{};
    for (auto& expr : se.indexExprs) {
        indexTys.push_back(Synthesize(ctx, expr.get()));
    }
    if (!Ty::IsTyCorrect(baseTy) || !Ty::AreTysCorrect(indexTys)) {
        return false;
    }
    // NOTE: Tuple and VArray type support built-in 'SubscriptExpr', others are all operator overload.
    if (auto tupleTy = DynamicCast<TupleTy*>(baseTy); tupleTy && se.indexExprs.size() == 1) {
        se.isTupleAccess = true;
        return ChkTupleAccess(ctx, target, se, *tupleTy);
    }
    if (auto varrTy = DynamicCast<VArrayTy*>(baseTy); varrTy) {
        return ChkVArrayAccess(ctx, target, se, *varrTy);
    }
    auto ds = DiagSuppressor(diag);
    DesugarOperatorOverloadExpr(ctx, se); // Desugar to callExpr.
    // The type of baseExpr should not be inferred here!
    bool isWellTyped = target == nullptr ? Ty::IsTyCorrect(Synthesize(ctx, se.desugarExpr.get()))
                                         : Check(ctx, target, se.desugarExpr.get());
    if (isWellTyped) {
        ds.ReportDiag();
        se.ty = se.desugarExpr->ty;
        CODEC_ASSERT(se.desugarExpr->astKind == ASTKind::CALL_EXPR);
        auto desugaredCE = StaticAs<ASTKind::CALL_EXPR>(se.desugarExpr.get());
        CODEC_ASSERT(desugaredCE->resolvedFunction != nullptr);
        ReplaceTarget(&se, desugaredCE->resolvedFunction);
        return true;
    }
    auto synTy = Synthesize(ctx, se.desugarExpr.get());
    bool retTyMismatch = Ty::IsTyCorrect(target) && Ty::IsTyCorrect(synTy);
    RecoverToSubscriptExpr(se);
    // Also recover base and index's type.
    typeManager.ReplaceIdealTy(&baseTy);
    se.baseExpr->ty = se.baseExpr->ty ? se.baseExpr->ty : baseTy;
    for (size_t i = 0; i < se.indexExprs.size(); ++i) {
        typeManager.ReplaceIdealTy(&indexTys[i]);
        se.indexExprs[i]->ty = Ty::IsTyCorrect(se.indexExprs[i]->ty) ? se.indexExprs[i]->ty : indexTys[i];
    }
    if (!ds.HasError() && se.ShouldDiagnose(true)) { // Only report subscript diagnoses if no error has beed reported.
        ds.ReportDiag(); // Report warnings.
        if (retTyMismatch) {
            CODEC_NULLPTR_CHECK(target);
            DiagMismatchedTypesWithFoundTy(diag, se, *target, *synTy);
        } else {
            DiagInvalidSubscriptExpr(diag, se, *baseTy, indexTys);
        }
    }
    ds.ReportDiag();
    return false;
}

bool TypeChecker::TypeCheckerImpl::ChkTupleAccess(ASTContext& ctx, Ptr<Ty> target, SubscriptExpr& se, TupleTy& tupleTy)
{
    if (se.baseExpr == nullptr || se.indexExprs.size() != 1) {
        return false;
    }
    if (se.indexExprs[0]->astKind != ASTKind::LIT_CONST_EXPR) {
        diag.Diagnose(*se.indexExprs[0], DiagKind::sema_builtin_invalid_index, "tuple");
        return false;
    }
    if (!Check(ctx, TypeManager::GetPrimitiveTy(TypeKind::TYPE_INT64), se.indexExprs[0].get())) {
        diag.Diagnose(*se.indexExprs[0], DiagKind::sema_builtin_invalid_index, "tuple");
        return false;
    }
    auto idxExpr = StaticAs<ASTKind::LIT_CONST_EXPR>(se.indexExprs[0].get());
    uint64_t index = idxExpr->constNumValue.asInt.Uint64();
    if (index >= tupleTy.typeArgs.size()) {
        if (se.ShouldDiagnose(true)) {
            diag.Diagnose(*se.indexExprs[0], DiagKind::sema_builtin_index_in_bound, "tuple");
        }
        return false;
    }
    if (target == nullptr) { // Type inferring.
        se.ty = tupleTy.typeArgs[index];
        return true;
    }
    if (auto tl = AST::As<ASTKind::TUPLE_LIT>(se.baseExpr.get()); tl) {
        CODEC_ASSERT(index < tl->children.size());
        // Update tuple ty if baseExpr is a TupleLit whenever check succeed or failed,
        // because the check result will affect child expr of tuple lit which
        // may changing ideal ty to exact ty or changing valid ty to invalid ty.
        if (!Check(ctx, target, tl->children[index].get())) {
            // Reset the ty of tuple lit to allow re-synthesize of the tuple lit.
            tl->ty = TypeManager::GetInvalidTy();
            return false;
        }
        std::vector<Ptr<Ty>> elemTy;
        for (auto& it : tl->children) {
            if (it != nullptr && ReplaceIdealTy(*it)) {
                elemTy.emplace_back(it->ty);
            } else {
                elemTy.emplace_back(TypeManager::GetInvalidTy());
            }
        }
        tl->ty = typeManager.GetTupleTy(elemTy);
        se.ty = elemTy[index];
    } else {
        if (!typeManager.IsSubtype(tupleTy.typeArgs[index], target)) {
            DiagMismatchedTypesWithFoundTy(diag, se, target->String(), tupleTy.typeArgs[index]->String());
            return false;
        }
        se.ty = tupleTy.typeArgs[index];
    }
    return true;
}

bool TypeChecker::TypeCheckerImpl::ChkVArrayAccess(ASTContext& ctx, Ptr<Ty> target, SubscriptExpr& se, VArrayTy& varrTy)
{
    if (se.indexExprs.size() != 1) {
        diag.DiagnoseRefactor(DiagKindRefactor::sema_varray_subscript_num, se);
        return false;
    }
    {
        DiagSuppressor ds(diag); // only for examing missing error report
        auto i64 = TypeManager::GetPrimitiveTy(TypeKind::TYPE_INT64);
        auto idxExpr = se.indexExprs[0].get();
        if (!Check(ctx, i64, idxExpr)) {
            if (!ds.HasError()) {
                DiagMismatchedTypesWithFoundTy(diag, *idxExpr, i64->String(), idxExpr->ty->String());
            }
            ds.ReportDiag();
            return false;
        }
        ds.ReportDiag();
    }
    CODEC_ASSERT(!varrTy.typeArgs.empty() && varrTy.typeArgs[0]);
    if (target == nullptr) { // Type inferring.
        se.ty = varrTy.typeArgs[0];
        return true;
    }
    // check subvalue type.
    if (!typeManager.IsSubtype(varrTy.typeArgs[0], target)) {
        DiagMismatchedTypesWithFoundTy(diag, se, target->String(), varrTy.typeArgs[0]->String());
        return false;
    }
    se.ty = varrTy.typeArgs[0];
    return true;
}

Ptr<Ty> TypeChecker::TypeCheckerImpl::SynSubscriptExpr(ASTContext& ctx, SubscriptExpr& se)
{
    ChkSubscriptExpr(ctx, nullptr, se);
    return se.ty;
}
