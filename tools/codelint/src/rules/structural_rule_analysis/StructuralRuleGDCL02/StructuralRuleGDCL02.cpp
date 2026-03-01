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

#include "StructuralRuleGDCL02.h"
#include "Codira/Basic/Match.h"

namespace Codira::CodeCheck {
using namespace Codira;
using namespace Codira::AST;
using namespace Meta;

static bool IsPublic(std::set<Codira::AST::Modifier>& modifiers)
{
    auto item = std::find_if(
        modifiers.begin(), modifiers.end(), [](auto& modifier) { return modifier.modifier == TokenKind::PUBLIC; });
    return item != modifiers.end();
}

static bool IsOperator(std::set<Codira::AST::Modifier>& modifiers)
{
    auto item = std::find_if(
        modifiers.begin(), modifiers.end(), [](auto& modifier) { return modifier.modifier == TokenKind::OPERATOR; });
    return item != modifiers.end();
}

void StructuralRuleGDCL02::CheckReturnValType(Ptr<Node> node)
{
    if (!node) {
        return;
    }
    auto preVisit = [this](Ptr<Node> node) -> VisitAction {
        if (node->astKind == ASTKind::FUNC_DECL) {
            auto funcDecl = StaticCast<FuncDecl*>(node);
            if (funcDecl->IsFinalizer() || funcDecl->TestAttr(AST::Attribute::CONSTRUCTOR)) {
                return VisitAction::WALK_CHILDREN;
            }
            if (IsOperator(funcDecl->modifiers)) {
                return VisitAction::WALK_CHILDREN;
            }
            if (IsPublic(funcDecl->modifiers) && funcDecl->funcBody && !funcDecl->funcBody->retType) {
                Diagnose(funcDecl->identifier.Begin(), funcDecl->identifier.End(),
                    CodeCheckDiagKind::G_DCL_02_public_function_type, funcDecl->identifier.GetRawText());
            }
            return VisitAction::WALK_CHILDREN;
        }
        if (node->astKind == ASTKind::VAR_DECL) {
            auto varDecl = StaticCast<VarDecl*>(node);
            if (IsPublic(varDecl->modifiers) && !varDecl->type) {
                Diagnose(varDecl->identifier.Begin(), varDecl->identifier.End(),
                    CodeCheckDiagKind::G_DCL_02_public_variable_type, varDecl->identifier.GetRawText());
            }
            return VisitAction::SKIP_CHILDREN;
        }
        return VisitAction::WALK_CHILDREN;
    };
    Walker walker(node, preVisit);
    walker.Walk();
}

void StructuralRuleGDCL02::MatchPattern(ASTContext& ctx, Ptr<Node> node)
{
    (void)ctx;
    CheckReturnValType(node);
}
} // namespace Codira::CodeCheck
