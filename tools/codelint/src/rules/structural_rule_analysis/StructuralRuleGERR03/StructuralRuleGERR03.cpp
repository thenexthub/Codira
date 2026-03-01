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

#include "StructuralRuleGERR03.h"
#include "common/CommonFunc.h"

namespace Codira::CodeCheck {
using namespace Codira;
using namespace Codira::AST;
using namespace Meta;

void StructuralRuleGERR03::FindMemberAccess(Ptr<Codira::AST::Node> node)
{
    if (node == nullptr) {
        return;
    }

    Walker walker(node, [this](Ptr<Node> node) -> VisitAction {
        match (*node)([this](const MemberAccess &memberAccess) { CheckMemberAccessAttribute(memberAccess); });
        return VisitAction::WALK_CHILDREN;
    });
    walker.Walk();
}

void StructuralRuleGERR03::CheckMemberAccessAttribute(const Codira::AST::MemberAccess &memberAccess)
{
    if (memberAccess.baseExpr == nullptr || memberAccess.baseExpr->ty == nullptr) {
        return;
    }
    if (memberAccess.field == "getOrThrow" && memberAccess.baseExpr->ty->name == "Option") {
        Diagnose(memberAccess.baseExpr->end, memberAccess.baseExpr->end + 1,
            CodeCheckDiagKind::G_ERR_03_avoid_option_getorthrow);
    }
}

void StructuralRuleGERR03::MatchPattern(ASTContext &ctx, Ptr<Node> node)
{
    (void)ctx;
    FindMemberAccess(node);
}
}
