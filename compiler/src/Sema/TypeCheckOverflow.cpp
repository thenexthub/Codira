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

/**
 * @file
 *
 * This file implements the integer overflow strategy.
 */

#include "TypeCheckerImpl.h"

#include "Codira/AST/Clone.h"
#include "Codira/AST/Create.h"
#include "Codira/AST/Match.h"
#include "Codira/Frontend/CompilerInstance.h"

using namespace Codira;
using namespace AST;

namespace {
bool IsOverflowOp(TokenKind op)
{
    switch (op) {
        case TokenKind::ADD:
        case TokenKind::SUB:
        case TokenKind::MUL:
        case TokenKind::DIV:
        case TokenKind::MOD:
        case TokenKind::EXP:
        case TokenKind::INCR:
        case TokenKind::DECR:
        case TokenKind::ADD_ASSIGN:
        case TokenKind::SUB_ASSIGN:
        case TokenKind::MUL_ASSIGN:
        case TokenKind::DIV_ASSIGN:
        case TokenKind::MOD_ASSIGN:
        case TokenKind::EXP_ASSIGN:
            return true;
        default:
            return false;
    }
}

void SetIncOrDecOverflowExpr(Node& node)
{
    auto ide = As<ASTKind::INC_OR_DEC_EXPR>(&node);
    if (ide == nullptr || !IsOverflowOp(ide->op)) {
        return;
    }
    // Not Integer, no need to set overflow flag.
    if (ide->expr == nullptr || Ty::IsInitialTy(ide->expr->ty) || !ide->expr->ty->IsInteger()) {
        return;
    }
    // Implement overflow in codegen.
    ide->expr->EnableAttr(Attribute::NUMERIC_OVERFLOW);
    ide->EnableAttr(Attribute::NUMERIC_OVERFLOW);
    ide->expr->overflowStrategy = ide->overflowStrategy;
    return;
}

void SetAssignOverflowExpr(Node& node)
{
    auto ae = As<ASTKind::ASSIGN_EXPR>(&node);
    if (ae == nullptr || !IsOverflowOp(ae->op)) {
        return;
    }
    // Not Integer, no need to set overflow flag.
    if (ae->leftValue == nullptr || Ty::IsInitialTy(ae->leftValue->ty) || !ae->leftValue->ty->IsInteger()) {
        return;
    }
    // Implement overflow in codegen.
    ae->EnableAttr(Attribute::NUMERIC_OVERFLOW);
    return;
}

void SetUnaryOverflowExpr(Node& node)
{
    auto ue = As<ASTKind::UNARY_EXPR>(&node);
    if (ue == nullptr || !IsOverflowOp(ue->op)) {
        return;
    }
    // Not Integer, no need to set overflow flag.
    if (ue->expr == nullptr || Ty::IsInitialTy(ue->expr->ty) || !ue->expr->ty->IsInteger()) {
        return;
    }
    // Implement overflow in codegen.
    ue->EnableAttr(Attribute::NUMERIC_OVERFLOW);
    return;
}

void SetBinaryOverflowExpr(Node& node)
{
    auto be = As<ASTKind::BINARY_EXPR>(&node);
    if (be == nullptr || !IsOverflowOp(be->op)) {
        return;
    }
    // Not Integer or not Same no need to set overflow flag.
    if (be->leftExpr == nullptr || Ty::IsInitialTy(be->leftExpr->ty) || !be->leftExpr->ty->IsInteger() ||
        be->rightExpr == nullptr || Ty::IsInitialTy(be->rightExpr->ty) || !be->rightExpr->ty->IsInteger()) {
        return;
    }
    // Implement overflow in codegen.
    be->EnableAttr(Attribute::NUMERIC_OVERFLOW);
    return;
}

void SetOverflowFlag(Node& node)
{
    // Set integer overflow flag.
    if (node.astKind == ASTKind::INC_OR_DEC_EXPR) {
        SetIncOrDecOverflowExpr(node);
        return;
    }
    if (node.astKind == ASTKind::ASSIGN_EXPR) {
        SetAssignOverflowExpr(node);
        return;
    }
    if (Ty::IsInitialTy(node.ty) || !node.ty->IsInteger()) {
        return;
    }
    if (node.astKind == ASTKind::UNARY_EXPR) {
        SetUnaryOverflowExpr(node);
        return;
    }
    if (node.astKind == ASTKind::BINARY_EXPR) {
        SetBinaryOverflowExpr(node);
        return;
    }
    return;
}

void SetOverflowStrategyForPkg(Node& node)
{
    Walker walkerAST(&node, [](Ptr<Node> curNode) -> VisitAction {
        switch (curNode->astKind) {
            case ASTKind::INC_OR_DEC_EXPR:
            case ASTKind::ASSIGN_EXPR:
            case ASTKind::UNARY_EXPR:
            case ASTKind::BINARY_EXPR: {
                SetOverflowFlag(*curNode);
                break;
            }
            default:
                break;
        }
        return VisitAction::WALK_CHILDREN;
    });
    walkerAST.Walk();
}
} // namespace

// Set overflow strategy after typechecked.
void TypeChecker::SetOverflowStrategy(const std::vector<Ptr<AST::Package>>& pkgs) const
{
    // Update overflow strategy for desugared decls.
    impl->SetIntegerOverflowStrategy();
    // Check integer overflow strategy.
    for (auto& pkg : pkgs) {
        SetOverflowStrategyForPkg(*pkg);
    }
}

namespace {
void SetOverflowStrategy(Node& node, const OverflowStrategy overflowStrategy, const OverflowStrategy optionStrategy)
{
    auto setOverflowStrategyInFuncBody = [optionStrategy](Node& curNode) -> void {
        if (curNode.astKind == ASTKind::FUNC_DECL) {
            auto& fd = StaticCast<FuncDecl>(curNode);
            if (fd.funcBody) {
                SetOverflowStrategy(*fd.funcBody, fd.overflowStrategy, optionStrategy);
            }
        } else if (curNode.astKind == ASTKind::LAMBDA_EXPR) {
            auto& le = StaticCast<LambdaExpr>(curNode);
            if (le.funcBody) {
                SetOverflowStrategy(*le.funcBody, le.overflowStrategy, optionStrategy);
            }
        }
    };
    auto preVisit = [overflowStrategy, optionStrategy, setOverflowStrategyInFuncBody](
                        Ptr<Node> curNode) -> VisitAction {
        switch (curNode->astKind) {
            case ASTKind::FUNC_DECL:
            case ASTKind::LAMBDA_EXPR: {
                if (curNode->TestAttr(Attribute::NUMERIC_OVERFLOW)) {
                    setOverflowStrategyInFuncBody(*curNode);
                    return VisitAction::SKIP_CHILDREN;
                }
                break;
            }
            case ASTKind::INC_OR_DEC_EXPR:
            case ASTKind::ASSIGN_EXPR:
            case ASTKind::UNARY_EXPR:
            case ASTKind::BINARY_EXPR:
            case ASTKind::TYPE_CONV_EXPR: {
                auto expr = RawStaticCast<Expr*>(curNode);
                if (overflowStrategy == OverflowStrategy::NA) {
                    expr->overflowStrategy = optionStrategy;
                } else {
                    expr->overflowStrategy = overflowStrategy;
                }
#ifdef CODIRA_CODEGEN_CODENATIVE_BACKEND
                // mark call to operator func with overflowStrategy, used in split operator
                if (expr->desugarExpr) {
                    expr->desugarExpr->overflowStrategy = expr->overflowStrategy;
                }
#endif
                break;
            }
            default:
                break;
        }
        return VisitAction::WALK_CHILDREN;
    };
    Walker walkerAST(&node, preVisit);
    walkerAST.Walk();
}
} // namespace

// Set integer overflow strategy before sema typechecking.
void TypeChecker::TypeCheckerImpl::SetIntegerOverflowStrategy() const
{
    CODEC_NULLPTR_CHECK(ci);
    if (ci->invocation.globalOptions.overflowStrategy == OverflowStrategy::NA) {
        return;
    }
    // Choose integer overflow strategy.
    for (auto& pkg : ci->GetSourcePackages()) {
        ::SetOverflowStrategy(*pkg, OverflowStrategy::NA, ci->invocation.globalOptions.overflowStrategy);
    }
}
