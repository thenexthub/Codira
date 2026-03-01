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

#include "StructuralRuleGFMT15.h"

namespace Codira::CodeCheck {
using namespace Codira;
using namespace Codira::AST;
using namespace Meta;

void StructuralRuleGFMT15::CheckLitConstExpr(const LitConstExpr& litConstExpr)
{
    if (!litConstExpr.ty || !litConstExpr.ty->IsFloating()) {
        return;
    }
    auto lst = FileUtil::SplitStr(litConstExpr.rawString, '.');
    if (lst.size() == 1 || (lst.size() > 1 && lst[0] == "-")) {
        Diagnose(litConstExpr.begin, litConstExpr.end, CodeCheckDiagKind::G_FMT_15_leading_zero_before_decimal,
            litConstExpr.rawString);
    }
}

void StructuralRuleGFMT15::FindLitConstExpr(Ptr<Codira::AST::Node> node)
{
    if (!node) {
        return;
    }

    Walker walker(node, [this](Ptr<Node> node) -> VisitAction {
        match (*node)([this](const LitConstExpr& litConstExpr) { CheckLitConstExpr(litConstExpr); });
        return VisitAction::WALK_CHILDREN;
    });
    walker.Walk();
}

void StructuralRuleGFMT15::MatchPattern(ASTContext& ctx, Ptr<Node> node)
{
    (void)ctx;
    FindLitConstExpr(node);
}
} // namespace Codira::CodeCheck
