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

#include "StructuralRuleGOPR02.h"

using namespace Codira;
using namespace Codira::AST;
using namespace Meta;
using namespace CodeCheck;

/**
 * Avoid defining `()` operator overloading functions in enumeration types.
 * wrong eg:
 * enum E {
 *     Y | X | X(Int64)
 *     operator func ()(a: Int64) {
 *         a
 *     }
 * }
 */
void StructuralRuleGOPR02::CheckParenthesesOpertorInEnum(EnumDecl& enumDecl)
{
    for (auto& decl : enumDecl.GetMemberDecls()) {
        if (decl->astKind == ASTKind::FUNC_DECL) {
            if (decl->TestAttr(Attribute::OPERATOR) && decl->identifier == "()") {
                Diagnose(decl->begin, decl->end,
                    CodeCheckDiagKind::G_OPR_02_enum_parentheses_overload_information, decl->identifier.Val());
            }
        }
    }
}

void StructuralRuleGOPR02::CheckParenthesesOpertorInExtend(ExtendDecl& extendDecl)
{
    if (!extendDecl.ty) {
        return;
    }
    if (extendDecl.ty->kind != TypeKind::TYPE_ENUM) {
        return;
    }
    for (auto& decl : extendDecl.GetMemberDecls()) {
        if (decl->astKind == ASTKind::FUNC_DECL) {
            if (decl->TestAttr(Attribute::OPERATOR) && decl->identifier == "()") {
                Diagnose(decl->begin, decl->end,
                    CodeCheckDiagKind::G_OPR_02_enum_parentheses_overload_information, decl->identifier.Val());
            }
        }
    }
}

void StructuralRuleGOPR02::FindEnum(Ptr<Node> node)
{
    if (node == nullptr) {
        return;
    }

    Walker walker(node, [this](Ptr<Node> node) -> VisitAction {
        return match(*node)(
            [this](EnumDecl& enumDecl) {
                CheckParenthesesOpertorInEnum(enumDecl);
                return VisitAction::WALK_CHILDREN;
            },
            [this](ExtendDecl& extendDecl) {
                CheckParenthesesOpertorInExtend(extendDecl);
                return VisitAction::WALK_CHILDREN;
            },
            []() { return VisitAction::WALK_CHILDREN; });
    });
    walker.Walk();
}

void StructuralRuleGOPR02::MatchPattern(ASTContext& ctx, Ptr<Node> node)
{
    (void)ctx;
    FindEnum(node);
}
