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

#include "StructuralRuleGEXP04.h"

namespace Codira::CodeCheck {
using namespace Codira;
using namespace Codira::AST;
using namespace Meta;

static bool CheckRefExpr(Ptr<AST::RefExpr> refExpr)
{
    auto target = refExpr->ref.target;
    if (target->astKind == ASTKind::VAR_DECL) {
        auto varDecl = StaticCast<AST::VarDecl*>(target);
        return varDecl->IsStaticOrGlobal();
    }
    return false;
}

static bool CheckLeft(Ptr<AST::Expr> expr);

static bool CheckMemberAccess(Ptr<AST::MemberAccess> memberAccess)
{
    auto baseExpr = memberAccess->baseExpr.get();
    return CheckLeft(baseExpr);
}

static bool CheckLeft(Ptr<AST::Expr> expr)
{
    if (!expr) {
        return false;
    }
    if (expr->astKind == ASTKind::REF_EXPR) {
        auto refExpr = StaticCast<AST::RefExpr*>(expr);
        return CheckRefExpr(refExpr);
    }
    if (expr->astKind == ASTKind::MEMBER_ACCESS) {
        auto memberAccess = StaticCast<AST::MemberAccess*>(expr);
        return CheckMemberAccess(memberAccess);
    }
    return false;
}

bool StructuralRuleGEXP04::HasSideEffectInFunc(Ptr<Codira::AST::FuncDecl> funcDecl)
{
    if (!funcDecl->funcBody || !funcDecl->funcBody->body) {
        return false;
    }
    auto& body = funcDecl->funcBody->body->body;
    for (auto& decl : body) {
        if (decl->astKind != ASTKind::ASSIGN_EXPR) {
            break;
        }
        auto assignExpr = StaticCast<Codira::AST::AssignExpr*>(decl.get().get());
        auto left = assignExpr->leftValue.get().get();
        if (CheckLeft(left)) {
            funcHasCheckedMap[funcDecl] = true;
            return true;
        }
    }
    funcHasCheckedMap[funcDecl] = false;
    return false;
}

bool StructuralRuleGEXP04::CheckSideEffectInExpr(Ptr<AST::Expr> expr)
{
    if (!expr) {
        return false;
    }
    if (expr->astKind == ASTKind::CALL_EXPR) {
        auto call = StaticCast<AST::CallExpr*>(expr);
        if (!call->baseFunc || call->baseFunc->astKind != ASTKind::REF_EXPR) {
            return false;
        }
        auto refExpr = StaticCast<AST::RefExpr*>(call->baseFunc.get());
        return CheckSideEffectInExpr(refExpr);
    }
    if (expr->astKind == ASTKind::REF_EXPR) {
        auto refExpr = StaticCast<AST::RefExpr*>(expr);
        auto target = refExpr->ref.target;
        if (!target || target->astKind != ASTKind::FUNC_DECL) {
            return false;
        }
        auto funcDecl = StaticCast<AST::FuncDecl*>(target);
        if (funcHasCheckedMap.count(funcDecl) == 0) {
            if (HasSideEffectInFunc(funcDecl)) {
                funcWithSideEffect = funcDecl;
                return true;
            }
        } else {
            if (funcHasCheckedMap[funcDecl]) {
                funcWithSideEffect = funcDecl;
                return true;
            }
        }
    }
    if (expr->astKind == ASTKind::MEMBER_ACCESS) {
        auto memberAccess = StaticCast<AST::MemberAccess*>(expr);
        auto baseExpr = memberAccess->baseExpr.get().get();
        return CheckSideEffectInExpr(baseExpr);
    }
    return false;
}

static bool IsSubsequence(const std::vector<std::string>& a, const std::vector<std::string>& b)
{
    auto m = a.size();
    auto n = b.size();
    size_t j = 0; // Pointer for a

    // Traverse b and check elements with a
    for (size_t i = 0; i < n && j < m; ++i) {
        if (a[j] == b[i]) {
            ++j;
        }
    }

    // If j equals the length of a, all elements of a were found in b
    return j == m;
}

static bool CheckParamsSequence(Ptr<AST::CallExpr> callExpr)
{
    std::vector<std::string> params{};
    std::vector<std::string> args{};
    for (auto& arg : callExpr->args) {
        if (!arg) {
            continue;
        }
        args.emplace_back(arg->name.Val());
    }
    auto baseFunc = callExpr->baseFunc.get();
    if (!baseFunc || baseFunc->astKind != ASTKind::REF_EXPR) {
        return false;
    }
    auto refExpr = StaticCast<AST::RefExpr*>(callExpr->baseFunc.get());
    auto target = refExpr->ref.target;
    if (!target || target->astKind != ASTKind::FUNC_DECL) {
        return false;
    }
    auto funcDecl = StaticCast<AST::FuncDecl*>(target);
    auto paramList = funcDecl->funcBody->paramLists[0].get();
    for (auto& param : paramList->params) {
        params.emplace_back(param->identifier.Val());
    }
    return !IsSubsequence(params, args);
}

void StructuralRuleGEXP04::CheckSideEffect(Ptr<Codira::AST::Node> node)
{
    auto preVisit = [this](Ptr<Node> node) {
        if (node->astKind == ASTKind::CALL_EXPR) {
            auto callExpr = StaticCast<AST::CallExpr*>(node);
            bool isNotSameOrder = CheckParamsSequence(callExpr);
            bool isNothingInArgs = false;
            for (auto& arg : callExpr->args) {
                if (!arg) {
                    continue;
                }
                auto funcArg = StaticCast<AST::FuncArg*>(arg.get());
                if (funcArg->ty->IsNothing()) {
                    isNothingInArgs = true;
                }
                if (!isNothingInArgs && !isNotSameOrder) {
                    continue;
                }
                // CASE 2: When a function is called, the parameter evaluation order is from left to right in the order
                // of definition, not in the order of actual parameters when called.
                // CASE 3: If a parameter of a function call is of type Nothing, the parameters following that parameter
                // will not be evaluated, and the function call itself will not be executed.
                auto expr = funcArg->expr.get();
                if (CheckSideEffectInExpr(expr)) {
                    Diagnose(expr->begin, expr->end,
                        CodeCheckDiagKind::G_EXP_04_side_effect_depend_on_order, funcWithSideEffect->identifier.Val());
                }
            }
        }
        // CASE 1: The compound assignment expression a op= b cannot simply be regarded as a combination of the
        // assignment expression and other binary operators.
        if (node->astKind == ASTKind::ASSIGN_EXPR) {
            auto assignExpr = StaticCast<Codira::AST::AssignExpr*>(node);
            auto left = assignExpr->leftValue.get().get();
            if (CheckSideEffectInExpr(left)) {
                Diagnose(left->begin, left->end, CodeCheckDiagKind::G_EXP_04_side_effect_depend_on_order,
                    funcWithSideEffect->identifier.Val());
            }
        }
        return VisitAction::WALK_CHILDREN;
    };
    Walker walker(node, preVisit);
    walker.Walk();
}

void StructuralRuleGEXP04::MatchPattern(ASTContext& ctx, Ptr<Node> node)
{
    (void)ctx;
    CheckSideEffect(node);
}
} // namespace Codira::CodeCheck
