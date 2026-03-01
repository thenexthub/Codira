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

#include "StructuralRuleGERR01.h"
#include "common/CommonFunc.h"

namespace Codira::CodeCheck {
using namespace Codira;
using namespace AST;

void StructuralRuleGERR01::AnalyzeCatchBlock(std::vector<OwnedPtr<Codira::AST::Block>>& blocks)
{
    for (auto& block : blocks) {
        if (block->body.empty() || block->body[0]->TestAttr(Attribute::COMPILER_ADD)) {
            Diagnose(block->leftCurlPos, block->rightCurlPos, CodeCheckDiagKind::G_ERR_01_exceptions_process_01);
        }
    }
}

bool StructuralRuleGERR01::CommentsIncludeExceptionInfo(
    const std::vector<AST::CommentGroup>& comments, const std::string& exception)
{
    for (auto& comment : comments) {
        for (auto& c : comment.cms) {
            if (c.info.Value().find(exception) != std::string::npos) {
                return true;
            }
        }
    }
    return false;
}

void StructuralRuleGERR01::AnalyzeThrowExpr(
    const Ptr<AST::ThrowExpr>& throwExpr, const std::vector<CommentGroup>& comments)
{
    if (throwExpr->expr->astKind == ASTKind::CALL_EXPR) {
        auto pCallExpr = As<ASTKind::CALL_EXPR>(throwExpr->expr);
        if (!pCallExpr->baseFunc) {
            return;
        }
        auto exception = pCallExpr->baseFunc->ToString();
        if (!CommentsIncludeExceptionInfo(comments, exception)) {
            Diagnose(throwExpr->begin, throwExpr->end, CodeCheckDiagKind::G_ERR_01_exceptions_process_02);
        }
    }
}

void StructuralRuleGERR01::AnalyzeFunctionDecl(Ptr<AST::Node> node, const std::vector<CommentGroup>& comments)
{
    if (!node) {
        return;
    }

    Walker walker(node, [this, &comments](Ptr<Node> node) -> VisitAction {
        if (node->astKind == ASTKind::TRY_EXPR) {
            auto pTryExpr = As<ASTKind::TRY_EXPR>(node.get());
            AnalyzeCatchBlock(pTryExpr->catchBlocks);
        }
        if (node->astKind == ASTKind::THROW_EXPR) {
            auto pThrowExpr = As<ASTKind::THROW_EXPR>(node.get());
            AnalyzeThrowExpr(pThrowExpr, comments);
        }
        if (node->astKind == ASTKind::FUNC_DECL) {
            return VisitAction::SKIP_CHILDREN;
        }
        return VisitAction::WALK_CHILDREN;
    });
    walker.Walk();
}

void StructuralRuleGERR01::CheckFuncDeclHelper(
    const Ptr<AST::FuncDecl>& funcDecl, const std::vector<CommentGroup>& comments)
{
    if (!funcDecl->funcBody || !funcDecl->funcBody->body) {
        return;
    }

    for (auto& item : funcDecl->funcBody->body->body) {
        AnalyzeFunctionDecl(item.get(), comments);
    }
}

void StructuralRuleGERR01::CheckFuncDecl(Ptr<Codira::AST::Node>& node)
{
    if (!node) {
        return;
    }
    Walker walker(node, [this](Ptr<Node> node) -> VisitAction {
        if (node->astKind == ASTKind::MAIN_DECL) {
            auto pMainDecl = As<ASTKind::MAIN_DECL>(node.get());
            if (!pMainDecl->desugarDecl) {
                return VisitAction::SKIP_CHILDREN;
            }
            auto comments = pMainDecl->desugarDecl->comments.leadingComments;
            CheckFuncDeclHelper(pMainDecl->desugarDecl.get(), comments);
            return VisitAction::SKIP_CHILDREN;
        }
        if (node->astKind == ASTKind::FUNC_DECL) {
            auto pFuncDecl = As<ASTKind::FUNC_DECL>(node.get());
            auto comments = pFuncDecl->comments.leadingComments;
            CheckFuncDeclHelper(pFuncDecl, comments);
        }
        return VisitAction::WALK_CHILDREN;
    });
    walker.Walk();
}

void StructuralRuleGERR01::MatchPattern(ASTContext& ctx, Ptr<Node> node)
{
    (void)ctx;
    CheckFuncDecl(node);
}
} // namespace Codira::CodeCheck
