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

#include "StructuralRuleGEXP07.h"

namespace Codira::CodeCheck {
using namespace Codira;
using namespace Codira::AST;
using namespace Meta;

static std::set<TokenKind> COMPARE_OP{
    TokenKind::EQUAL, TokenKind::NOTEQ, TokenKind::GE, TokenKind::GT, TokenKind::LE, TokenKind::LT};

void StructuralRuleGEXP07::VarDeclDiagnose(
    Ptr<AST::Decl> decl, Position& start, Position& end, Ptr<AST::VarDecl> rightVarDecl)
{
    auto leftVarDecl = As<ASTKind::VAR_DECL>(decl);
    if (nullptr == leftVarDecl) {
        return;
    }
    if ((leftVarDecl->isConst && !rightVarDecl->isConst) || (!leftVarDecl->isVar && rightVarDecl->isVar)) {
        Diagnose(start, end, CodeCheckDiagKind::G_EXP_07_constants_on_the_right, leftVarDecl->identifier.Val());
    }
}

void StructuralRuleGEXP07::ReferenceExprAnalysis(Ptr<AST::Expr> expr, Ptr<AST::VarDecl> rightVarDecl)
{
    if (expr->astKind == ASTKind::REF_EXPR) {
        auto refExpr = As<ASTKind::REF_EXPR>(expr);
        if (!refExpr || refExpr->ref.target == nullptr || refExpr->ref.target->astKind != ASTKind::VAR_DECL) {
            return;
        }
        VarDeclDiagnose(refExpr->ref.target, refExpr->begin, refExpr->end, rightVarDecl);
    } else if (expr->astKind == ASTKind::MEMBER_ACCESS) {
        auto ma = As<ASTKind::MEMBER_ACCESS>(expr);
        if (!ma || ma->target == nullptr || ma->target->astKind != ASTKind::VAR_DECL) {
            return;
        }
        VarDeclDiagnose(ma->target, ma->begin, ma->end, rightVarDecl);
    } else if (expr->astKind == ASTKind::LIT_CONST_EXPR && !rightVarDecl->isConst) {
        auto leftExpr = As<ASTKind::LIT_CONST_EXPR>(expr);
        if (!leftExpr) {
            return;
        }
        Diagnose(
            leftExpr->begin, leftExpr->end, CodeCheckDiagKind::G_EXP_07_constants_on_the_right, leftExpr->rawString);
    }
}

void StructuralRuleGEXP07::BinaryDesugar(const BinaryExpr& binaryExpr)
{
    if (binaryExpr.desugarExpr->astKind != ASTKind::CALL_EXPR) {
        return;
    }
    auto ce = As<ASTKind::CALL_EXPR>(binaryExpr.desugarExpr);
    if (!ce || ce->args.size() != 1) {
        return;
    }
    if (ce->args[0]->expr->astKind != ASTKind::REF_EXPR) {
        return;
    }
    auto ref = As<ASTKind::REF_EXPR>(ce->args[0]->expr);
    if (ref == nullptr || ref->ref.target == nullptr || ref->ref.target->astKind != ASTKind::VAR_DECL) {
        return;
    }
    auto vd = As<ASTKind::VAR_DECL>(ref->ref.target);
    if (!vd || ce->baseFunc == nullptr || ce->baseFunc->astKind != ASTKind::MEMBER_ACCESS) {
        return;
    }
    auto ma = As<ASTKind::MEMBER_ACCESS>(ce->baseFunc);
    if (!ma || !ma->baseExpr) {
        return;
    }
    ReferenceExprAnalysis(ma->baseExpr, vd);
}

void StructuralRuleGEXP07::ExprCoverToVarDecl(Ptr<Expr> expr, Ptr<VarDecl> &varDecl)
{
    auto ref = As<ASTKind::REF_EXPR>(expr);
    auto ma = As<ASTKind::MEMBER_ACCESS>(expr);
    if (ref && ref->ref.target &&
        (ref->ref.target->astKind == ASTKind::VAR_DECL || ref->ref.target->astKind == ASTKind::PROP_DECL)) {
        varDecl = As<ASTKind::VAR_DECL>(ref->ref.target);
    } else if (ma && ma->target &&
        (ma->target->astKind == ASTKind::VAR_DECL || ma->target->astKind == ASTKind::PROP_DECL)) {
        varDecl = As<ASTKind::VAR_DECL>(ma->target);
    }
}

void StructuralRuleGEXP07::BinaryNotDesugar(const BinaryExpr& binaryExpr)
{
    if (binaryExpr.leftExpr == nullptr || binaryExpr.rightExpr == nullptr) {
        return;
    }
    if (binaryExpr.leftExpr->ty == nullptr || binaryExpr.rightExpr->ty == nullptr) {
        return;
    }
    if ((!binaryExpr.leftExpr->IsReferenceExpr() && binaryExpr.leftExpr->astKind != ASTKind::LIT_CONST_EXPR) ||
        !binaryExpr.rightExpr->IsReferenceExpr()) {
        return;
    }
    Ptr<VarDecl> rightVarDecl;
    ExprCoverToVarDecl(binaryExpr.rightExpr.get(), rightVarDecl);
    if (!rightVarDecl) {
        return;
    }
    if (binaryExpr.leftExpr->IsReferenceExpr()) {
        ReferenceExprAnalysis(binaryExpr.leftExpr, rightVarDecl);
    } else if (binaryExpr.leftExpr->astKind == ASTKind::LIT_CONST_EXPR && !rightVarDecl->isConst) {
        auto leftExpr = As<ASTKind::LIT_CONST_EXPR>(binaryExpr.leftExpr.get());
        if (!leftExpr) {
            return;
        }
        Diagnose(
            leftExpr->begin, leftExpr->end, CodeCheckDiagKind::G_EXP_07_constants_on_the_right, leftExpr->rawString);
    }
}

void StructuralRuleGEXP07::CheckBinaryExpr(const BinaryExpr& binaryExpr)
{
    if (COMPARE_OP.find(binaryExpr.op) == COMPARE_OP.end()) {
        return;
    }
    if (binaryExpr.desugarExpr) {
        BinaryDesugar(binaryExpr);
    } else {
        BinaryNotDesugar(binaryExpr);
    }
}

bool StructuralRuleGEXP07::ExcludeIntervalHelper(const BinaryExpr& binaryExpr)
{
    auto lb = As<ASTKind::BINARY_EXPR>(binaryExpr.leftExpr.get());
    auto rb = As<ASTKind::BINARY_EXPR>(binaryExpr.rightExpr.get());
    if (lb == nullptr || rb == nullptr) {
        return false;
    }
    if (!lb->rightExpr || !rb->leftExpr || !lb->rightExpr->IsReferenceExpr() || !rb->leftExpr->IsReferenceExpr()) {
        return false;
    }

    Ptr<VarDecl> lv = nullptr;
    ExprCoverToVarDecl(lb->rightExpr, lv);

    Ptr<VarDecl> rv = nullptr;
    ExprCoverToVarDecl(rb->leftExpr, rv);

    return (rv && lv) && ExcludeIntervalCoverToVarDecl(lv, rv, lb, rb);
}

bool StructuralRuleGEXP07::ExcludeIntervalCoverToVarDecl(
    VarDecl* lv, VarDecl* rv, Ptr<BinaryExpr> lb, Ptr<BinaryExpr> rb)
{
    if (lv != rv) {
        return false;
    }
    if (((lb->op == TokenKind::LT || lb->op == TokenKind::LE) &&
            (rb->op == TokenKind::LT || rb->op == TokenKind::LE)) ||
        ((lb->op == TokenKind::GT || lb->op == TokenKind::GE) &&
            (rb->op == TokenKind::GT || rb->op == TokenKind::GE))) {
        return true;
    }
    return false;
}

bool StructuralRuleGEXP07::ExcludeInterval(const BinaryExpr& binaryExpr)
{
    if (binaryExpr.op != TokenKind::AND) {
        return false;
    }
    if (binaryExpr.leftExpr == nullptr || binaryExpr.rightExpr == nullptr) {
        return false;
    }
    if (binaryExpr.leftExpr->astKind != ASTKind::BINARY_EXPR || binaryExpr.rightExpr->astKind != ASTKind::BINARY_EXPR) {
        return false;
    }
    return ExcludeIntervalHelper(binaryExpr);
}

bool StructuralRuleGEXP07::IsTuple(const BinaryExpr& binaryExpr)
{
    return binaryExpr.leftExpr && binaryExpr.rightExpr &&
        (binaryExpr.leftExpr->astKind == ASTKind::TUPLE_LIT || binaryExpr.rightExpr->astKind == ASTKind::TUPLE_LIT);
}

void StructuralRuleGEXP07::FindBinaryExpr(Node* node)
{
    if (node == nullptr) {
        return;
    }

    Walker walker(node, [this](Node* node) -> VisitAction {
        return match(*node)(
            [this](const BinaryExpr& binaryExpr) {
                if (IsTuple(binaryExpr)) {
                    return VisitAction::SKIP_CHILDREN;
                }
                if (!binaryExpr.desugarExpr && ExcludeInterval(binaryExpr)) {
                    return VisitAction::SKIP_CHILDREN;
                }
                CheckBinaryExpr(binaryExpr);
                return VisitAction::WALK_CHILDREN;
            },
            [this]() { return VisitAction::WALK_CHILDREN; });
    });
    walker.Walk();
}

void StructuralRuleGEXP07::MatchPattern(ASTContext& ctx, Ptr<Node> node)
{
    (void)ctx;
    FindBinaryExpr(node);
}
}
