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

#include "StructuralRuleGEXP05.h"

using namespace Codira;
using namespace Codira::AST;
using namespace Meta;
using namespace CodeCheck;

static std::unordered_map<Codira::TokenKind, std::string> unaryOpToStringMap = {
    {TokenKind::NOT, "!"}, {TokenKind::SUB, "-"}};

void StructuralRuleGEXP05::CheckParenExpr(const Codira::AST::ParenExpr& parenExpr)
{
    // Check if the parentheses are redundant.
    // For example, '(!a)', '(-a)'.
    if (parenExpr.expr && parenExpr.expr->astKind == ASTKind::UNARY_EXPR) {
        auto op = RawStaticCast<AST::UnaryExpr*>(parenExpr.expr.get())->op;
        Diagnose(parenExpr.begin, parenExpr.end,
            CodeCheckDiagKind::G_EXP_05_use_parentheses_to_express_operations_order_01,
            unaryOpToStringMap.count(op) > 0 ? unaryOpToStringMap[op] : "");
    }
    // For example, '((a))'
    if (parenExpr.expr && parenExpr.expr->astKind == ASTKind::PAREN_EXPR) {
        Diagnose(parenExpr.begin, parenExpr.end,
            CodeCheckDiagKind::G_EXP_05_use_parentheses_to_express_operations_order_03, "");
    }
}

void StructuralRuleGEXP05::CheckSubBinaryExpr(AST::Expr* subExpr, const Codira::TokenKind& op)
{
    // Check if the parentheses are redundant
    // for example, '(a + b) + c'
    if (subExpr->astKind == ASTKind::PAREN_EXPR) {
        auto parenExpr = RawStaticCast<AST::ParenExpr*>(subExpr);
        if (parenExpr->expr && parenExpr->expr->astKind == ASTKind::BINARY_EXPR) {
            auto subBinaryExpr = RawStaticCast<AST::BinaryExpr*>(parenExpr->expr.get());
            if (subBinaryExpr->op == op) {
                Diagnose(parenExpr->begin, parenExpr->end,
                    CodeCheckDiagKind::G_EXP_05_use_parentheses_to_express_operations_order_02, "");
            }
        }
        return;
    }
    // Check if parentheses are needed
    // For example, 'a << b < c'
    if (subExpr->astKind == ASTKind::BINARY_EXPR) {
        auto subBinaryExpr = RawStaticCast<AST::BinaryExpr*>(subExpr);
        if (ConfusOperMap.count(op) > 0 && ConfusOperMap[op].count(subBinaryExpr->op)) {
            Diagnose(subBinaryExpr->begin, subBinaryExpr->end,
                CodeCheckDiagKind::G_EXP_05_use_parentheses_to_express_operations_order_04, "");
        }
    }
}

void StructuralRuleGEXP05::CheckBinaryExpr(const Codira::AST::BinaryExpr& binaryExpr)
{
    if (binaryExpr.leftExpr) {
        CheckSubBinaryExpr(binaryExpr.leftExpr.get(), binaryExpr.op);
    }
    if (binaryExpr.rightExpr) {
        CheckSubBinaryExpr(binaryExpr.rightExpr.get(), binaryExpr.op);
    }
}

void StructuralRuleGEXP05::FindParenExpr(Node* node)
{
    if (node == nullptr) {
        return;
    }

    Walker walker(node, [this](Node* node) -> VisitAction {
        return match(*node)(
            [this](const ParenExpr& parenExpr) {
                CheckParenExpr(parenExpr);
                return VisitAction::WALK_CHILDREN;
            },
            [this](const BinaryExpr& binaryExpr) {
                CheckBinaryExpr(binaryExpr);
                return VisitAction::WALK_CHILDREN;
            },
            [this](const TupleLit& tupleLit) {
                for (auto& child : tupleLit.children) {
                    if (child->astKind == ASTKind::PAREN_EXPR) {
                        Diagnose(child->begin, child->end,
                            CodeCheckDiagKind::G_EXP_05_use_parentheses_to_express_operations_order_03, "");
                    }
                }
                return VisitAction::WALK_CHILDREN;
            },
            [this](const ArrayLit& arrayLit) {
                for (auto& child : arrayLit.children) {
                    if (child->astKind == ASTKind::PAREN_EXPR) {
                        Diagnose(child->begin, child->end,
                            CodeCheckDiagKind::G_EXP_05_use_parentheses_to_express_operations_order_03, "");
                    }
                }
                return VisitAction::WALK_CHILDREN;
            },
            []() { return VisitAction::WALK_CHILDREN; });
    });
    
    walker.Walk();
}

void StructuralRuleGEXP05::MatchPattern(ASTContext& ctx, Ptr<Node> node)
{
    (void)ctx;
    FindParenExpr(node);
}
