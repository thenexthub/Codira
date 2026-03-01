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

#include "StructuralRuleGOPR01.h"

using namespace Codira;
using namespace Codira::AST;
using namespace Meta;
using namespace CodeCheck;

static const std::unordered_set<Codira::TokenKind> operatorSet = {TokenKind::ADD, TokenKind::SUB, TokenKind::MUT,
    TokenKind::DIV, TokenKind::LT, TokenKind::GT, TokenKind::LE, TokenKind::GE, TokenKind::NOTEQ, TokenKind::EQUAL};

void StructuralRuleGOPR01::CheckExtendDecl(Codira::AST::ExtendDecl& extendDecl)
{
    if (!extendDecl.ty->IsPrimitive()) {
        return;
    }
    for (auto& member : extendDecl.members) {
        if (member->astKind == ASTKind::FUNC_DECL) {
            auto funcDecl = static_cast<AST::FuncDecl*>(member.get().get());
            if (funcDecl->TestAttr(Attribute::OPERATOR)) {
                Diagnose(funcDecl->begin, funcDecl->end,
                    CodeCheckDiagKind::G_OPR_01_avoid_operator_overloading_that_violates_conventions_01,
                    funcDecl->identifier.Val());
            }
        }
    }
}

void StructuralRuleGOPR01::CheckFuncDecl(Codira::AST::FuncDecl& funcDecl)
{
    if (funcDecl.TestAttr(Attribute::OPERATOR)) {
        needDiag = true;
        if (operatorSet.count(funcDecl.op) > 0) {
            Walker walker(&funcDecl, [&funcDecl, this](Ptr<Node> node) -> VisitAction {
                return match(*node)(
                    [&funcDecl, this](BinaryExpr& binaryExpr) {
                        if (binaryExpr.op == funcDecl.op) {
                            needDiag = false;
                        }
                        return VisitAction::WALK_CHILDREN;
                    },
                    [this]() { return VisitAction::WALK_CHILDREN; });
            });
            walker.Walk();
        }
        if (needDiag) {
            Diagnose(funcDecl.begin, funcDecl.end,
                CodeCheckDiagKind::G_OPR_01_avoid_operator_overloading_that_violates_conventions_02,
                funcDecl.identifier.Val());
        }
    }
}
void StructuralRuleGOPR01::FindExtendDecl(Ptr<Node> node)
{
    if (node == nullptr) {
        return;
    }

    Walker walker(node, [this](Ptr<Node> node) -> VisitAction {
        return match(*node)(
            [this](ExtendDecl& extendDecl) {
                CheckExtendDecl(extendDecl);
                return VisitAction::WALK_CHILDREN;
            },
            [this](FuncDecl& funcDecl) {
                CheckFuncDecl(funcDecl);
                return VisitAction::WALK_CHILDREN;
            },
            []() { return VisitAction::WALK_CHILDREN; });
    });
    walker.Walk();
}

void StructuralRuleGOPR01::MatchPattern(ASTContext& ctx, Ptr<Node> node)
{
    (void)ctx;
    FindExtendDecl(node);
}
