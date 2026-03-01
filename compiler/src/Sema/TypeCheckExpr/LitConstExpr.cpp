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
Ptr<Ty> GetInnerNumericType(Ty& boxTy, Ty& basicTy)
{
    // This function only used after type was checked by "IsLitBoxableType" for integer and float LitConst.
    // that guaranteed boxTy must be N-dims Option<T> where T is numeric type.
    if (!boxTy.IsEnum() || boxTy.HasInvalidTy()) {
        return boxTy.IsNumeric() ? &boxTy : &basicTy;
    }
    auto enumTy = RawStaticCast<EnumTy*>(&boxTy);
    CODEC_ASSERT(!enumTy->typeArgs.empty());
    return GetInnerNumericType(*enumTy->typeArgs[0], basicTy);
}
} // namespace

bool TypeChecker::TypeCheckerImpl::ChkLitConstExprOfTypeBool(Ty& target, LitConstExpr& lce)
{
    if (target.IsBooleanSubType()) {
        lce.ty = &target;
        return true;
    } else if (typeManager.IsLitBoxableType(TypeManager::GetPrimitiveTy(TypeKind::TYPE_BOOLEAN), &target)) {
        lce.ty = TypeManager::GetPrimitiveTy(TypeKind::TYPE_BOOLEAN);
        return true;
    } else if (target.IsAny() || target.IsCType()) {
        lce.ty = TypeManager::GetPrimitiveTy(TypeKind::TYPE_BOOLEAN);
        return true;
    } else {
        diag.Diagnose(lce, DiagKind::sema_cannot_convert_literal, "a boolean", target.String());
        lce.ty = TypeManager::GetNonNullTy(lce.ty);
        return false;
    }
}

bool TypeChecker::TypeCheckerImpl::ChkLitConstExprOfTypeUnit(Ty& target, LitConstExpr& lce)
{
    if (target.IsAny() || target.IsUnit() || target.IsCType() ||
        typeManager.IsLitBoxableType(TypeManager::GetPrimitiveTy(TypeKind::TYPE_UNIT), &target)) {
        lce.ty = TypeManager::GetPrimitiveTy(TypeKind::TYPE_UNIT);
        return true;
    } else {
        DiagMismatchedTypesWithFoundTy(diag, lce, target.String(), "Unit");
        lce.ty = TypeManager::GetInvalidTy();
        return false;
    }
}

bool TypeChecker::TypeCheckerImpl::ChkLitConstExprOfTypeInteger(Ty& target, LitConstExpr& lce)
{
    TypeKind intSuffixTokenKind = lce.GetNumLitTypeKind();
    TypeKind defaultIntTokenKind =
        intSuffixTokenKind == TypeKind::TYPE_IDEAL_INT ? TypeKind::TYPE_INT64 : intSuffixTokenKind;
    if (target.IsIntegerSubType()) {
        if (intSuffixTokenKind == target.kind || intSuffixTokenKind == TypeKind::TYPE_IDEAL_INT) {
            lce.ty = &target;
            return true;
        } else {
            diag.Diagnose(lce, DiagKind::sema_cannot_convert_literal, lce.stringValue, target.String());
            return false;
        }
    } else if (typeManager.IsLitBoxableType(TypeManager::GetPrimitiveTy(defaultIntTokenKind), &target)) {
        // Check for extendable or option boxable type as int64.
        lce.ty = GetInnerNumericType(target, *TypeManager::GetPrimitiveTy(defaultIntTokenKind));
        return true;
    } else if (target.IsAny() || target.IsCType()) {
        lce.ty = GetInnerNumericType(target, *TypeManager::GetPrimitiveTy(defaultIntTokenKind));
        return true;
    } else if (!target.IsInvalid()) {
        diag.Diagnose(lce, DiagKind::sema_cannot_convert_literal, "an integer", target.String());
        lce.ty = TypeManager::GetInvalidTy();
        return false;
    }
    lce.ty = TypeManager::GetNonNullTy(lce.ty);
    return false;
}

bool TypeChecker::TypeCheckerImpl::ChkLitConstExprOfTypeFloat(Ty& targetTy, LitConstExpr& lce)
{
    TypeKind intSuffixTokenKind = lce.GetNumLitTypeKind();
    TypeKind defaultFloat64TokenKind =
        intSuffixTokenKind == TypeKind::TYPE_IDEAL_FLOAT ? TypeKind::TYPE_FLOAT64 : intSuffixTokenKind;
    if (targetTy.IsFloatingSubType()) {
        if (intSuffixTokenKind == targetTy.kind || intSuffixTokenKind == TypeKind::TYPE_IDEAL_FLOAT) {
            lce.ty = &targetTy;
            return true;
        } else {
            diag.Diagnose(lce, DiagKind::sema_cannot_convert_literal, lce.stringValue, targetTy.String());
            return false;
        }
    } else if (typeManager.IsLitBoxableType(TypeManager::GetPrimitiveTy(defaultFloat64TokenKind), &targetTy)) {
        // Check for extendable or option boxable type as float64.
        lce.ty = GetInnerNumericType(targetTy, *TypeManager::GetPrimitiveTy(defaultFloat64TokenKind));
        return true;
    } else if (targetTy.IsAny() || targetTy.IsCType()) {
        lce.ty = GetInnerNumericType(targetTy, *TypeManager::GetPrimitiveTy(defaultFloat64TokenKind));
        return true;
    } else {
        diag.Diagnose(lce, DiagKind::sema_cannot_convert_literal, "a floating-point", targetTy.String());
        lce.ty = TypeManager::GetNonNullTy(lce.ty);
        return false;
    }
}

bool TypeChecker::TypeCheckerImpl::ChkLitConstExprOfTypeChar(Ty& targetTy, LitConstExpr& lce)
{
    if (&targetTy == TypeManager::GetPrimitiveTy(TypeKind::TYPE_RUNE)) {
        lce.ty = &targetTy;
        return true;
    } else if (typeManager.IsLitBoxableType(TypeManager::GetPrimitiveTy(TypeKind::TYPE_RUNE), &targetTy)) {
        lce.ty = TypeManager::GetPrimitiveTy(TypeKind::TYPE_RUNE);
        return true;
    } else if (targetTy.IsAny()) {
        lce.ty = TypeManager::GetPrimitiveTy(TypeKind::TYPE_RUNE);
        return true;
    } else {
        diag.Diagnose(lce, DiagKind::sema_cannot_convert_literal, "a character", targetTy.String());
        lce.ty = TypeManager::GetNonNullTy(lce.ty);
        return false;
    }
}

Ptr<Ty> TypeChecker::TypeCheckerImpl::SynLitConstStringExpr(ASTContext& ctx, LitConstExpr& lce)
{
    // For string literal expr.
    if (!lce.siExpr) {
        lce.ty = Synthesize(ctx, lce.ref.get());
        return lce.ty;
    }
    // For String Interpolation.
    // 1. Get Struct-String and Interface-ToString type.
    auto stringDecl = importManager.GetCoreDecl<InheritableDecl>(STD_LIB_STRING);
    auto toStringInterface = importManager.GetCoreDecl<InheritableDecl>(TOSTRING_NAME);
    if (!stringDecl || !toStringInterface) {
        lce.ty = TypeManager::GetInvalidTy();
        return lce.ty;
    }
    // 2. Check all interpolated expressions.
    auto strExpr = lce.siExpr.get();
    bool isWellTyped = true;
    for (auto& expr : strExpr->strPartExprs) {
        if (expr->astKind != ASTKind::INTERPOLATION_EXPR) {
            isWellTyped = Check(ctx, stringDecl->ty, expr.get()) && isWellTyped;
            continue;
        }
        auto ie = StaticCast<InterpolationExpr*>(expr.get());
        CODEC_NULLPTR_CHECK(ie->block);
        ie->block->ty = Synthesize(ctx, ie->block.get());
        if (!typeManager.IsSubtype(ie->block->ty, toStringInterface->ty)) {
            if (Ty::IsTyCorrect(ie->block->ty)) {
                diag.Diagnose(*ie->block, DiagKind::sema_invalid_string_implementation, ie->block->ty->String());
            }
            isWellTyped = false;
        } else if (ie->block->ty->IsNothing()) {
            // After typechecker, it will desugar as 'expr.toString()', and Nothing type can't access member.
            diag.DiagnoseRefactor(DiagKindRefactor::sema_undeclared_identifier, *ie->block, "toString");
            isWellTyped = false;
        }
        if (!isWellTyped) {
            ie->block->ty = TypeManager::GetInvalidTy();
        }
        ie->ty = ie->block->ty;
    }
    // If not all interpolated expression check passed, directly quit current check.
    if (!isWellTyped) {
        lce.ty = TypeManager::GetInvalidTy();
        return lce.ty;
    }
    lce.siExpr->ty = stringDecl->ty;
    lce.ty = stringDecl->ty;
    return lce.ty;
}

bool TypeChecker::TypeCheckerImpl::ChkLitConstExprOfTypeString(ASTContext& ctx, Ty& target, LitConstExpr& lce)
{
    auto ty = SynLitConstStringExpr(ctx, lce);
    bool isWellTyped = typeManager.IsLitBoxableType(ty, &target) || target.IsAny();
    if (!isWellTyped && !CanSkipDiag(lce)) {
        DiagMismatchedTypesWithFoundTy(
            diag, lce, target.String(), lce.stringKind == StringKind::JSTRING ? "JString" : "Struct-String");
    }
    return isWellTyped;
}

bool TypeChecker::TypeCheckerImpl::ChkLitConstExpr(ASTContext& ctx, Ty& target, LitConstExpr& lce)
{
    switch (lce.kind) {
        case LitConstKind::BOOL:
            return ChkLitConstExprOfTypeBool(target, lce);
        case LitConstKind::UNIT:
            return ChkLitConstExprOfTypeUnit(target, lce);
        case LitConstKind::INTEGER:
        case LitConstKind::RUNE_BYTE:
            return ChkLitConstExprOfTypeInteger(target, lce) && ChkLitConstExprRange(lce);
        case LitConstKind::FLOAT:
            return ChkLitConstExprOfTypeFloat(target, lce) && ChkLitConstExprRange(lce);
        case LitConstKind::RUNE:
            return ChkLitConstExprOfTypeChar(target, lce);
        case LitConstKind::STRING:
        case LitConstKind::JSTRING:
            return ChkLitConstExprOfTypeString(ctx, target, lce);
        default:
            lce.ty = TypeManager::GetInvalidTy();
            return false;
    }
}

Ptr<Ty> TypeChecker::TypeCheckerImpl::SynLitConstExpr(ASTContext& ctx, LitConstExpr& lce)
{
    switch (lce.kind) {
        case LitConstKind::BOOL:
            lce.ty = TypeManager::GetPrimitiveTy(TypeKind::TYPE_BOOLEAN);
            break;
        case LitConstKind::UNIT:
            lce.ty = TypeManager::GetPrimitiveTy(TypeKind::TYPE_UNIT);
            break;
        case LitConstKind::INTEGER:
        case LitConstKind::RUNE_BYTE:
        case LitConstKind::FLOAT:
            if (TypeKind kind = lce.GetNumLitTypeKind(); kind == TypeKind::TYPE_INVALID) {
                lce.ty = TypeManager::GetInvalidTy();
            } else {
                lce.ty = TypeManager::GetPrimitiveTy(kind);
            }
            break;
        case LitConstKind::STRING:
        case LitConstKind::JSTRING:
            lce.ty = SynLitConstStringExpr(ctx, lce);
            break;
        case LitConstKind::RUNE:
            lce.ty = TypeManager::GetPrimitiveTy(TypeKind::TYPE_RUNE);
            break;
        case LitConstKind::NONE:
            lce.ty = TypeManager::GetInvalidTy();
            break;
    }
    ChkLitConstExprRange(lce);
    return lce.ty;
}
