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

#include "StructuralRuleGEXP06.h"

namespace Codira::CodeCheck {
using namespace Codira;
using namespace Codira::AST;
using namespace Meta;

void StructuralRuleGEXP06::FindBinaryExpr(Ptr<Codira::AST::Node> node)
{
    if (node == nullptr) {
        return;
    }

    Walker walker(node, [this](Ptr<Node> node) -> VisitAction {
        match (*node)([this](const BinaryExpr &binaryExpr) { CheckBinaryExpr(binaryExpr); });
        return VisitAction::WALK_CHILDREN;
    });
    walker.Walk();
}

void StructuralRuleGEXP06::CheckBinaryExpr(const Codira::AST::BinaryExpr &binaryExpr)
{
    if (binaryExpr.op != TokenKind::EQUAL && binaryExpr.op != TokenKind::NOTEQ) {
        return;
    }
    if (binaryExpr.leftExpr == nullptr || binaryExpr.rightExpr == nullptr) {
        return;
    }
    if (binaryExpr.leftExpr->ty == nullptr || binaryExpr.rightExpr->ty == nullptr) {
        return;
    }
    if (binaryExpr.leftExpr->ty->IsBoolean() && binaryExpr.rightExpr->ty->IsBoolean()) {
        if (binaryExpr.leftExpr->astKind == AST::ASTKind::LIT_CONST_EXPR ||
            binaryExpr.rightExpr->astKind == AST::ASTKind::LIT_CONST_EXPR) {
            // 2 is the length of a binary operator.
            Diagnose(binaryExpr.operatorPos, binaryExpr.operatorPos + 2,
                CodeCheckDiagKind::G_EXP_06_avoid_redundant_op_in_bool_type_comparisons);
        }
    }
}

void StructuralRuleGEXP06::MatchPattern(ASTContext &ctx, Ptr<Node> node)
{
    (void)ctx;
    FindBinaryExpr(node);
}
}
