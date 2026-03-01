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

#include "StructuralRuleGCHK04.h"
#include <regex>
#include "common/CommonFunc.h"

namespace Codira::CodeCheck {
using namespace Codira;
using namespace Codira::AST;
using namespace Meta;
using namespace Codira::CodeCheck::TaintData;

const std::string JSONPATH = "/config/structural_rule_G_CHK_04.json";

void StructuralRuleGCHK04::MemberAccessTargetCheck(Ptr<Codira::AST::MemberAccess> memberAccess, const Position start,
    const Position end, const std::string refName, std::string& tmpStr)
{
    if (auto pVarDecl = DynamicCast<AST::VarDecl*>(memberAccess->target.get());
        pVarDecl && pVarDecl->initializer != nullptr) {
        tmpStr += GetLitConstExprHelper(pVarDecl->initializer.get(), start, end, pVarDecl->identifier);
    } else if (auto pFuncDecl = DynamicCast<AST::FuncDecl*>(memberAccess->target.get()); pFuncDecl) {
        for (auto& param : taintSource) {
            if (pFuncDecl->identifier == param.first.funcName && pFuncDecl->fullPackageName == param.first.pkgName) {
                Diagnose(start, end, CodeCheckDiagKind::G_CHK_04_untrusted_regular_expressions_external_data,
                    refName.empty() ? pFuncDecl->identifier.Val() : refName);
            }
        }
    }
}

void StructuralRuleGCHK04::GetLitConstExprHelperRefExpr(
    Ptr<Codira::AST::Expr> expr, const Position start, const Position end, std::string refName, std::string& str)
{
    auto refExpr = As<ASTKind::REF_EXPR>(expr);
    if (refExpr->ref.target == nullptr) {
        return;
    }
    if (refExpr->ref.target->astKind == AST::ASTKind::FUNC_PARAM) {
        Diagnose(start, end, CodeCheckDiagKind::G_CHK_04_untrusted_regular_expressions_dynamical_regex,
            refName.empty() ? refExpr->ref.identifier.Val() : refName);
        return;
    }
    if (auto pVarDecl = DynamicCast<AST::VarDecl*>(refExpr->ref.target.get()); pVarDecl) {
        if (pVarDecl->initializer != nullptr) {
            str += GetLitConstExprHelper(
                pVarDecl->initializer.get(), start, end, refName.empty() ? refExpr->ref.identifier.Val() : refName);
        }
    } else if (auto pFuncDecl = DynamicCast<AST::FuncDecl*>(refExpr->ref.target.get()); pFuncDecl) {
        refName = refName.empty() ? refExpr->ref.identifier.Val() : refName;
        auto parentTy = CommonFunc::GetFuncDeclParentTyName(pFuncDecl);
        for (auto& param : taintSource) {
            if (pFuncDecl->identifier == param.first.funcName && pFuncDecl->fullPackageName == param.first.pkgName &&
                parentTy == param.first.parentTy) {
                Diagnose(start, end, CodeCheckDiagKind::G_CHK_04_untrusted_regular_expressions_external_data, refName);
            }
        }
    }
}

std::string StructuralRuleGCHK04::GetLitConstExprHelper(
    Ptr<Codira::AST::Expr> expr, const Position start, const Position end, const std::string& refName)
{
    std::string tmpStr;
    switch (expr->astKind) {
        case ASTKind::LIT_CONST_EXPR: {
            auto litConstExpr = As<ASTKind::LIT_CONST_EXPR>(expr);
            tmpStr += litConstExpr->stringValue;
            break;
        }
        case ASTKind::BINARY_EXPR: {
            auto binaryExpr = As<ASTKind::BINARY_EXPR>(expr);
            if (binaryExpr->desugarExpr != nullptr) {
                tmpStr += GetLitConstExprHelper(binaryExpr->desugarExpr.get(), start, end, refName);
            }
            if (binaryExpr->leftExpr.get() != nullptr && binaryExpr->rightExpr.get() != nullptr) {
                (void)GetLitConstExprHelper(binaryExpr->leftExpr.get(), start, end, refName);
                (void)GetLitConstExprHelper(binaryExpr->rightExpr.get(), start, end, refName);
            }
            break;
        }
        case ASTKind::CALL_EXPR: {
            auto callExpr = As<ASTKind::CALL_EXPR>(expr);
            if (callExpr->desugarExpr && callExpr->desugarExpr.get()->astKind == ASTKind::CALL_EXPR) {
                callExpr = As<ASTKind::CALL_EXPR>(callExpr->desugarExpr.get());
            }
            tmpStr += GetLitConstExprHelper(callExpr->baseFunc.get(), start, end, refName);
            for (auto& arg : callExpr->args) {
                tmpStr += GetLitConstExprHelper(arg->expr.get(), start, end, refName);
            }
            break;
        }
        case ASTKind::SUBSCRIPT_EXPR: {
            auto subscriptExpr = As<ASTKind::SUBSCRIPT_EXPR>(expr);
            if (subscriptExpr->desugarExpr != nullptr) {
                tmpStr += GetLitConstExprHelper(subscriptExpr->desugarExpr.get(), start, end, refName);
                break;
            }
            break;
        }
        case ASTKind::MEMBER_ACCESS: {
            auto memberAccess = As<ASTKind::MEMBER_ACCESS>(expr);
            tmpStr += GetLitConstExprHelper(memberAccess->baseExpr.get(), start, end, refName);
            MemberAccessTargetCheck(memberAccess, start, end, refName, tmpStr);
            break;
        }
        case ASTKind::REF_EXPR:
            GetLitConstExprHelperRefExpr(expr, start, end, refName, tmpStr);
            break;
        default:
            break;
    }
    return tmpStr;
}

void StructuralRuleGCHK04::RegexChecker(const OwnedPtr<AST::FuncArg>& arg)
{
    std::string content(GetLitConstExprHelper(arg->expr.get(), arg->expr->begin, arg->expr->end));
    if (!jsonInfo.contains("CheckType")) {
        Errorln(JSONPATH, " read json data failed!");
        return;
    }

    for (const auto& item : jsonInfo["CheckType"]) {
        std::smatch m;
        auto start = content.cbegin();
        auto end = content.cend();

        if (!item.contains("regex") || !item.contains("name")) {
            Errorln(JSONPATH, " read json data failed!");
            return;
        }
        std::string regex = item["regex"].get<std::string>();
        std::string name = item["name"].get<std::string>();
        for (; std::regex_search(start, end, m, std::regex(regex)); start = m.suffix().first) {
            if (name == "redundantGroup") {
                Diagnose(arg->expr->begin, arg->expr->end,
                    CodeCheckDiagKind::G_CHK_04_untrusted_regular_expressions_redundantGroup, m.str());
            } else if (name == "itselfRepeated") {
                Diagnose(arg->expr->begin, arg->expr->end,
                    CodeCheckDiagKind::G_CHK_04_untrusted_regular_expressions_itselfRepeated, m.str());
            } else {
                Diagnose(arg->expr->begin, arg->expr->end,
                    CodeCheckDiagKind::G_CHK_04_untrusted_regular_expressions_subexpressionsOverlap, m.str());
            }
        }
    }
}

void StructuralRuleGCHK04::GetLitConstExpr(const Codira::AST::CallExpr& callExpr)
{
    for (auto& arg : callExpr.args) {
        RegexChecker(arg);
    }
}

void StructuralRuleGCHK04::RegexFinder(Ptr<Codira::AST::Node> node)
{
    if (node == nullptr) {
        return;
    }

    Walker walker(node, [this](Ptr<Node> node) -> VisitAction {
        return match(*node)(
            [this](const CallExpr& callExpr) {
                if (auto pRefExpr = DynamicCast<AST::RefExpr*>(callExpr.baseFunc.get().get()); pRefExpr) {
                    if (pRefExpr->ref.target == nullptr) {
                        return VisitAction::WALK_CHILDREN;
                    }
                    if (pRefExpr->ref.identifier == "Regex" && pRefExpr->ref.target->fullPackageName == "std.regex") {
                        GetLitConstExpr(callExpr);
                    }
                }
                return VisitAction::WALK_CHILDREN;
            },
            []() { return VisitAction::WALK_CHILDREN; });
    });
    walker.Walk();
}

void StructuralRuleGCHK04::MatchPattern(ASTContext&, Ptr<Node> node)
{
    if (CommonFunc::ReadJsonFileToJsonInfo(JSONPATH, ConfigContext::GetInstance(), jsonInfo) == ERR) {
        return;
    }
    RegexFinder(node);
}
} // namespace Codira::CodeCheck
