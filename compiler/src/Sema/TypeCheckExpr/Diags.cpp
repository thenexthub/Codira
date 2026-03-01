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

#include "Diags.h"

#include "TypeCheckUtil.h"
#include "Codira/Basic/DiagnosticEngine.h"

using namespace Codira;
using namespace AST;

namespace Codira::Sema {
void DiagInvalidMultipleAssignExpr(
    DiagnosticEngine& diag, const Node& leftNode, const Expr& rightExpr, const std::string& because)
{
    auto builder = diag.DiagnoseRefactor(DiagKindRefactor::sema_mismatched_types_multiple_assign, rightExpr);
    builder.AddMainHintArguments(rightExpr.ty->String());
    builder.AddHint(leftNode, because);
}

void DiagInvalidBinaryExpr(DiagnosticEngine& diag, const BinaryExpr& be)
{
    CODEC_NULLPTR_CHECK(be.leftExpr);
    CODEC_NULLPTR_CHECK(be.rightExpr);
    CODEC_ASSERT(Ty::IsTyCorrect(be.leftExpr->ty));
    CODEC_ASSERT(Ty::IsTyCorrect(be.rightExpr->ty));
    const std::string& opStr = TOKENS[static_cast<int>(be.op)];
    auto range = be.operatorPos.IsZero() ? MakeRange(be.begin, be.end) : MakeRange(be.operatorPos, opStr);
    auto builder = diag.DiagnoseRefactor(DiagKindRefactor::sema_invalid_binary_expr, be, range, opStr,
        be.leftExpr->ty->String(), be.rightExpr->ty->String());
    if (be.leftExpr->ty->IsFunc() || be.leftExpr->ty->IsCFunc() || be.leftExpr->ty->IsTuple()) {
        // func and tuple type cannot be extended
        return;
    }
    if (TypeCheckUtil::IsOverloadableOperator(be.op)) {
        std::string note("you may want to implement 'operator func " + opStr + "(right: " + be.rightExpr->ty->String() +
            ")' for type '" + be.leftExpr->ty->String() + "'");
        if (be.op == TokenKind::EXP) {
            if (be.leftExpr->ty->kind == TypeKind::TYPE_INT64) {
                note += ", or to provide a right operand of type 'UInt64'";
            } else if (be.leftExpr->ty->kind == TypeKind::TYPE_FLOAT64) {
                note += ", or to provide a right operand of type 'Int64' or 'Float64'";
            } else if (be.rightExpr->ty->kind == TypeKind::TYPE_INT64) {
                note += ", or to provide a left operand of type 'Float64'";
            } else if (be.rightExpr->ty->kind == TypeKind::TYPE_FLOAT64) {
                note += ", or to provide a left operand of type 'Float64'";
            } else if (be.rightExpr->ty->kind == TypeKind::TYPE_UINT64) {
                note += ", or to provide a left operand of type 'Int64'";
            }
        }
        builder.AddNote(note);
    }
}

void DiagInvalidUnaryExpr(DiagnosticEngine& diag, const UnaryExpr& ue)
{
    if (!ue.ShouldDiagnose()) {
        return;
    }
    CODEC_NULLPTR_CHECK(ue.expr);
    CODEC_ASSERT(Ty::IsTyCorrect(ue.expr->ty));
    const std::string& opStr = TOKENS[static_cast<int>(ue.op)];
    auto builder = diag.DiagnoseRefactor(DiagKindRefactor::sema_invalid_unary_expr, ue, opStr, ue.expr->ty->String());
    if (ue.expr->ty->IsExtendable()) {
        builder.AddNote(
            "you may want to implement 'operator func " + opStr + "()' for type '" + ue.expr->ty->String() + "'");
    }
}

void DiagInvalidUnaryExprWithTarget(DiagnosticEngine& diag, const UnaryExpr& ue, Ty& target)
{
    if (!ue.ShouldDiagnose()) {
        return;
    }
    CODEC_NULLPTR_CHECK(ue.expr);
    CODEC_ASSERT(Ty::IsTyCorrect(ue.expr->ty));
    CODEC_ASSERT(Ty::IsTyCorrect(&target));
    const std::string& opStr = TOKENS[static_cast<int>(ue.op)];
    auto builder = diag.DiagnoseRefactor(
        DiagKindRefactor::sema_invalid_unary_expr_with_target, ue, opStr, ue.expr->ty->String(), target.String());
}

void DiagInvalidSubscriptExpr(
    DiagnosticEngine& diag, const SubscriptExpr& se, const Ty& baseTy, const std::vector<Ptr<Ty>>& indexTys)
{
    if (!se.ShouldDiagnose()) {
        return;
    }
    CODEC_ASSERT(!indexTys.empty());
    CODEC_ASSERT(Ty::IsTyCorrect(&baseTy));
    CODEC_ASSERT(Ty::AreTysCorrect(indexTys));
    std::string indexPrefix = "type" + (indexTys.size() > 1 ? std::string("s ") : " ");
    std::string indexStr = indexPrefix + "'" + Ty::GetTypesToStr(indexTys, "', ") + "'";
    auto builder = diag.DiagnoseRefactor(DiagKindRefactor::sema_invalid_subscript_expr, se, baseTy.String(), indexStr);
    if (baseTy.IsExtendable()) {
        std::string indexParam;
        for (size_t i = 0; i < indexTys.size(); ++i) {
            indexParam += "index" + std::to_string(i) + ": " + indexTys[i]->String();
            if (i != indexTys.size() - 1) {
                indexParam += ", ";
            }
        }
        builder.AddNote(
            "you may want to implement 'operator func [](" + indexParam + ")' for type '" + baseTy.String() + "'");
    }
}

void DiagUnableToInferExpr(DiagnosticEngine& diag, const Expr& expr)
{
    (void)diag.DiagnoseRefactor(DiagKindRefactor::sema_unable_to_infer_expr, expr);
}
} // namespace Codira::Sema
