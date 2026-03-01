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

#include "StructuralRuleFFIC7.h"

namespace Codira::CodeCheck {
using namespace Codira;
using namespace AST;
using namespace Meta;

static Ptr<Ty> GetType(Ptr<Ty> ty)
{
    if (!ty || ty->typeArgs.empty()) {
        return nullptr;
    }
    return ty->typeArgs[0];
}

void StructuralRuleFFIC7::CheckPointerExpr(Ptr<AST::Node> node)
{
    auto preVisit = [this](Ptr<Node> node) -> VisitAction {
        if (node->astKind != ASTKind::POINTER_EXPR) {
            return VisitAction::WALK_CHILDREN;
        }
        auto pointerExpr = StaticAs<ASTKind::POINTER_EXPR>(node);
        auto toTy = GetType(pointerExpr->ty);
        if (!pointerExpr->arg || !pointerExpr->arg->ty) {
            return VisitAction::WALK_CHILDREN;
        }
        auto fromTy = GetType(pointerExpr->arg->ty);
        if (!toTy || !fromTy) {
            return VisitAction::WALK_CHILDREN;
        }
        if (toTy->kind != fromTy->kind) {
            Diagnose(pointerExpr->begin, pointerExpr->end, CodeCheckDiagKind::FFI_C_7_avoid_truncation_error, "");
        }
        return VisitAction::WALK_CHILDREN;
    };
    Walker walker(node, preVisit);
    walker.Walk();
}

void StructuralRuleFFIC7::MatchPattern(ASTContext& ctx, Ptr<Node> node)
{
    (void)ctx;
    CheckPointerExpr(node);
}
} // namespace Codira::CodeCheck
