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

#include "StructuralRuleGTYP03.h"

namespace Codira::CodeCheck {
using namespace Codira;
using namespace Codira::AST;
using namespace Meta;

void StructuralRuleGTYP03::AnalyzeBinaryExpr(const Ptr<Expr> expr1, const Ptr<Expr>& expr2)
{
    if (expr1->astKind == ASTKind::MEMBER_ACCESS) {
        auto ma = As<ASTKind::MEMBER_ACCESS>(expr1);
        if (!ma) {
            return;
        }
        if (ma->field == "NaN") {
            Diagnose(expr2->begin, expr2->end, CodeCheckDiagKind::G_TYP_03_use_isNaN_method_float);
        }
    }
}

void StructuralRuleGTYP03::CheckBinaryExpr(const BinaryExpr& binaryExpr)
{
    if (binaryExpr.op != TokenKind::EQUAL && binaryExpr.op != TokenKind::NOTEQ) {
        return;
    }
    if (!binaryExpr.leftExpr || !binaryExpr.rightExpr) {
        return;
    }
    if (!binaryExpr.leftExpr->ty->IsFloating() && !binaryExpr.rightExpr->ty->IsFloating()) {
        return;
    }
    AnalyzeBinaryExpr(binaryExpr.leftExpr, binaryExpr.rightExpr);
    AnalyzeBinaryExpr(binaryExpr.rightExpr, binaryExpr.leftExpr);
}

void StructuralRuleGTYP03::FloatBinaryFinder(Ptr<Codira::AST::Node> node)
{
    if (!node) {
        return;
    }

    Walker walker(node, [this](Ptr<Node> node) -> VisitAction {
        return match(*node)(
            [this](const BinaryExpr& binaryExpr) {
                CheckBinaryExpr(binaryExpr);
                return VisitAction::WALK_CHILDREN;
            },
            [this]() { return VisitAction::WALK_CHILDREN; });
    });
    walker.Walk();
}

void StructuralRuleGTYP03::MatchPattern(ASTContext& ctx, Ptr<Node> node)
{
    (void)ctx;
    FloatBinaryFinder(node);
}
} // namespace Codira::CodeCheck
