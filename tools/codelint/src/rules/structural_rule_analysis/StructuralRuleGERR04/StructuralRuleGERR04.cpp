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

#include "StructuralRuleGERR04.h"
#include "common/CommonFunc.h"

namespace Codira::CodeCheck {
using namespace Codira;
using namespace Codira::AST;
using namespace Meta;

void StructuralRuleGERR04::CheckNode(Ptr<Codira::AST::Node> node, bool inChildScope)
{
    if (!node) {
        return;
    }

    auto preVisit = [this](Ptr<Node> node) -> VisitAction {
        return match(*node)(
            [this](const JumpExpr& jumpExpr) {
                if (jumpExpr.isBreak) {
                    Diagnose(jumpExpr.begin, jumpExpr.end, CodeCheckDiagKind::G_ERR_04_avoid_break_finally);
                } else {
                    Diagnose(jumpExpr.begin, jumpExpr.end, CodeCheckDiagKind::G_ERR_04_avoid_continue_finally);
                }
                return VisitAction::SKIP_CHILDREN;
            },
            [this](const ReturnExpr& returnExpr) {
                Diagnose(returnExpr.begin, returnExpr.end, CodeCheckDiagKind::G_ERR_04_avoid_return_finally);
                return VisitAction::SKIP_CHILDREN;
            },
            [this](const ThrowExpr& throwExpr) {
                Diagnose(throwExpr.begin, throwExpr.end, CodeCheckDiagKind::G_ERR_04_avoid_throw_finally);
                return VisitAction::SKIP_CHILDREN;
            },
            [this](const WhileExpr&) { return VisitAction::SKIP_CHILDREN; },
            [this](const ForInExpr&) { return VisitAction::SKIP_CHILDREN; },
            [this](const DoWhileExpr&) { return VisitAction::SKIP_CHILDREN; },
            [this](const FuncDecl&) { return VisitAction::SKIP_CHILDREN; },
            [this](const LambdaExpr&) { return VisitAction::SKIP_CHILDREN; },
            [this](const TryExpr&) { return VisitAction::SKIP_CHILDREN; }, []() { return VisitAction::WALK_CHILDREN; });
    };
    Walker walker(node, preVisit);
    walker.Walk();
}

void StructuralRuleGERR04::FindFinallyBlock(Ptr<Codira::AST::Node> node)
{
    if (!node) {
        return;
    }

    auto preVisit = [this](Ptr<Node> node) -> VisitAction {
        match (*node)([this](const TryExpr& tryExpr) {
            auto& finallyBlock = tryExpr.finallyBlock;
            if (!finallyBlock) {
                return;
            }
            for (auto& subNode : finallyBlock->body) {
                CheckNode(subNode);
            }
        });
        return VisitAction::WALK_CHILDREN;
    };
    Walker walker(node, preVisit);
    walker.Walk();
}

void StructuralRuleGERR04::MatchPattern(ASTContext& ctx, Ptr<Node> node)
{
    (void)ctx;
    FindFinallyBlock(node);
}
} // namespace Codira::CodeCheck
