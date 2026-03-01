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

#include "StructuralRuleGCHK02.h"
#include <utility>
#include "common/CommonFunc.h"

namespace Codira::CodeCheck {
using namespace Codira;
using namespace Codira::AST;
using namespace Meta;

constexpr const char* JSONPATH = "/config/structural_rule_G_CHK_02.json";

void StructuralRuleGCHK02::TaintLoggerFinder(Ptr<Codira::AST::Node> node)
{
    if (node == nullptr) {
        return;
    }
    Walker walker(node, [this](Ptr<Node> node) -> VisitAction {
        return match(*node)(
            [this](const CallExpr &callExpr) {
                if (callExpr.baseFunc != nullptr && callExpr.baseFunc->astKind == ASTKind::MEMBER_ACCESS) {
                    auto memberAccess = As<ASTKind::MEMBER_ACCESS>(callExpr.baseFunc.get());
                    LoggerFinderHelper(memberAccess, callExpr);
                    return VisitAction::SKIP_CHILDREN;
                }
                return VisitAction::WALK_CHILDREN;
            },
            []() { return VisitAction::WALK_CHILDREN; });
    });
    walker.Walk();
}

void StructuralRuleGCHK02::LoggerFinderHelper(Ptr<const Codira::AST::MemberAccess> memberAccess,
    const Codira::AST::CallExpr &callExpr)
{
    auto result = IsLogObject(*memberAccess);
    auto resultField = std::any_of(logFunc.begin(), logFunc.end(),
        [&memberAccess](const std::string &func) { return func == memberAccess->field; });
    if (result && resultField) {
        for (auto &arg : callExpr.args) {
            ObjectFilter(arg->expr.get(), memberAccess->begin, memberAccess->end, memberAccess->scopeName);
        }
    }
}

bool StructuralRuleGCHK02::IsLogObject(const Codira::AST::MemberAccess &memberAccess)
{
    if (memberAccess.target == nullptr) {
        return false;
    }
    if (memberAccess.target->astKind != ASTKind::FUNC_DECL) {
        return false;
    }
    auto funcDecl = As<ASTKind::FUNC_DECL>(memberAccess.target);
    if (funcDecl == nullptr || funcDecl->funcBody == nullptr || funcDecl->funcBody->parentClassLike == nullptr) {
        return false;
    }
    if (funcDecl->funcBody->parentClassLike->identifier.GetRawText() == "Logger") {
        return true;
    }
    for (auto &item : funcDecl->funcBody->parentClassLike->inheritedTypes) {
        if (item->ty->name == "Logger") {
            return true;
        }
    }
    return false;
}

void StructuralRuleGCHK02::AddLogInfoToSet(
    const Position start, const Position end, const std::string& scopeName, const std::string refName)
{
    LogInfo logInfo;
    logInfo.name = std::move(refName);
    logInfo.start = start;
    logInfo.end = end;
    logInfo.scopeName = scopeName;
    logInfo.isPurified = false;
    (void)externalDataLogSet.insert(logInfo);
}

void StructuralRuleGCHK02::ObjectFilter(Ptr<Codira::AST::Expr> expr, const Position start, const Position end,
    const std::string& scopeName, const std::string refName)
{
    if (expr == nullptr) {
        return;
    }

    Walker walker(expr, [this, &start, &end, &scopeName, refName](Ptr<Node> node) -> VisitAction {
        return match(*node)(
            [this, start, end, &scopeName, refName](const RefExpr &refExpr) {
                for (auto &taintVar : TaintVarSet) {
                    if (refExpr.ref.identifier == taintVar.name && taintVar.start < start) {
                        auto tmpName = refName.empty() ? refExpr.ref.identifier.Val() : refName;
                        AddLogInfoToSet(start, end, scopeName, tmpName);
                    }
                }
                if (auto pFuncDecl = DynamicCast<AST::FuncDecl*>(refExpr.ref.target.get()); pFuncDecl) {
                    if (ExternalDateCheckerHelper(*refExpr.ref.target)) {
                        auto tmpName = refName.empty() ? refExpr.ref.identifier + "()" : refName;
                        AddLogInfoToSet(start, end, scopeName, tmpName);
                    }
                } else if (auto pVarDecl = DynamicCast<AST::VarDecl*>(refExpr.ref.target.get()); pVarDecl) {
                    ObjectFilter(pVarDecl->initializer.get(), start, end, scopeName,
                        refName.empty() ? refExpr.ref.identifier.Val() : refName);
                }
                return VisitAction::SKIP_CHILDREN;
            },
            [this, start, end, &scopeName, refName](const MemberAccess &memberAccess) {
                if (memberAccess.field == "+") {
                    return VisitAction::WALK_CHILDREN;
                }
                std::string memberAccessName =
                        diagEngine->GetSourceManager().GetContentBetween(memberAccess.begin.fileID, memberAccess.begin,
                                                                         memberAccess.end);
                ObjectFilter(
                    memberAccess.baseExpr.get(), start, end, scopeName, refName.empty() ? memberAccessName : refName);
                auto tmpName = refName.empty() ? memberAccessName : refName;
                if (memberAccess.target == nullptr) {
                    return VisitAction::SKIP_CHILDREN;
                }
                if (auto pFuncDecl = DynamicCast<AST::FuncDecl*>(memberAccess.target.get()); pFuncDecl) {
                    FuncDeclBodyChecker(pFuncDecl, tmpName, start, end, scopeName);
                } else if (auto pVarDecl = DynamicCast<AST::VarDecl*>(memberAccess.target.get()); pVarDecl) {
                    ObjectFilter(pVarDecl->initializer.get(), start, end, scopeName, tmpName);
                }
                return VisitAction::SKIP_CHILDREN;
            },
            []() { return VisitAction::WALK_CHILDREN; });
    });
    walker.Walk();
}

bool StructuralRuleGCHK02::ExternalDateCheckerHelper(Codira::AST::Decl &decl)
{
    if (decl.astKind != ASTKind::FUNC_DECL) {
        return false;
    }
    auto funcDecl = As<ASTKind::FUNC_DECL>(&decl);
    auto parentName = CommonFunc::GetFuncDeclParentTyName(funcDecl);
    return IsTaintVar(funcDecl, parentName, "ExternalDateFunc");
}

bool StructuralRuleGCHK02::IsTaintVar(Ptr<const Codira::AST::FuncDecl> funcDecl, const std::string &parentName,
    const std::string &key)
{
    bool taintVarFlag = false;
    if (!jsonInfo.contains(key)) {
        Errorln(JSONPATH, " read json data failed!");
        return false;
    }

    for (const auto& item: jsonInfo[key]) {
        if (!item.contains("parent") || !item.contains("name") || !item.contains("lib")) {
            continue;
        }
        std::string parent = item["parent"].get<std::string>();
        std::string name = item["name"].get<std::string>();
        std::string lib = item["lib"].get<std::string>();
        if (funcDecl->identifier == name && funcDecl->fullPackageName == lib && parentName == parent) {
            taintVarFlag = true;
        }
    }
    return taintVarFlag;
}

void StructuralRuleGCHK02::TaintVarFinder(Ptr<Codira::AST::Node> node)
{
    if (node == nullptr) {
        return;
    }
    Walker walker(node, [this](Ptr<Node> node) -> VisitAction {
        return match(*node)(
            [this](const CallExpr &callExpr) {
                if (callExpr.args.empty()) {
                    return VisitAction::SKIP_CHILDREN;
                }
                if (callExpr.desugarExpr != nullptr) {
                    TaintVarFinder(callExpr.desugarExpr.get());
                    return VisitAction::SKIP_CHILDREN;
                }
                if (callExpr.baseFunc->astKind != ASTKind::MEMBER_ACCESS) {
                    return VisitAction::SKIP_CHILDREN;
                }
                auto memberAccess = As<ASTKind::MEMBER_ACCESS>(callExpr.baseFunc.get());
                if (memberAccess == nullptr || memberAccess->target == nullptr) {
                    return VisitAction::SKIP_CHILDREN;
                }
                if (memberAccess->target->astKind != ASTKind::FUNC_DECL) {
                    return VisitAction::SKIP_CHILDREN;
                }
                auto funcDecl = As<ASTKind::FUNC_DECL>(memberAccess->target);
                auto parentName = CommonFunc::GetFuncDeclParentTyName(funcDecl);
                if (!IsTaintVar(funcDecl, parentName, "ExternalDateFuncPassToVar")) {
                    return VisitAction::SKIP_CHILDREN;
                }
                for (auto &arg : callExpr.args) {
                    if (arg->expr->astKind != ASTKind::REF_EXPR) {
                        continue;
                    }
                    auto refExpr = As<ASTKind::REF_EXPR>(arg->expr.get());
                    if (refExpr == nullptr) {
                        continue;
                    }
                    TaintVarInfo taintVarInfo;
                    taintVarInfo.name = refExpr->ref.identifier;
                    taintVarInfo.start = refExpr->ref.identifier.Begin();
                    (void)TaintVarSet.insert(taintVarInfo);
                }
                return VisitAction::SKIP_CHILDREN;
            },
            []() { return VisitAction::WALK_CHILDREN; });
    });
    walker.Walk();
}

void StructuralRuleGCHK02::FuncDeclBodyChecker(Ptr<Codira::AST::FuncDecl> pFuncDecl, const std::string &refName,
    const Position start, const Position end, const std::string &scopeName)
{
    if (pFuncDecl->funcBody == nullptr || pFuncDecl->funcBody->body == nullptr) {
        if (ExternalDateCheckerHelper(*pFuncDecl)) {
            AddLogInfoToSet(start, end, scopeName, refName);
        }
        return;
    }
    for (auto &item : pFuncDecl->funcBody->body->body) {
        if (auto pReturnExpr = DynamicCast<AST::ReturnExpr*>(item.get().get()); pReturnExpr) {
            ObjectFilter(pReturnExpr->expr.get(), start, end, scopeName, refName);
        }
    }
}

bool StructuralRuleGCHK02::IsRegex(Ptr<const Codira::AST::CallExpr> callExpr)
{
    if (callExpr == nullptr) {
        return false;
    }
    if (callExpr->baseFunc == nullptr || callExpr->baseFunc->astKind != ASTKind::MEMBER_ACCESS) {
        return false;
    }
    auto memberAccess = As<ASTKind::MEMBER_ACCESS>(callExpr->baseFunc.get());
    if (memberAccess == nullptr) {
        return false;
    }
    if (memberAccess->baseExpr == nullptr || memberAccess->baseExpr->astKind != ASTKind::CALL_EXPR) {
        return false;
    }
    auto pCallExpr = As<ASTKind::CALL_EXPR>(memberAccess->baseExpr.get());
    if (pCallExpr == nullptr) {
        return false;
    }
    if (pCallExpr->baseFunc == nullptr || pCallExpr->baseFunc->astKind != ASTKind::REF_EXPR) {
        return false;
    }
    auto refExpr = As<ASTKind::REF_EXPR>(pCallExpr->baseFunc.get());
    if (refExpr == nullptr) {
        return false;
    }
    if (refExpr->ref.identifier == "Regex") {
        return true;
    }
    return false;
}

void StructuralRuleGCHK02::IsExternalInterface(Ptr<Codira::AST::Expr> expr)
{
    if (expr == nullptr) {
        return;
    }
    Walker walker(expr, [this](Ptr<Node> node) -> VisitAction {
        return match(*node)(
            [this](const RefExpr &refExpr) {
                if (refExpr.ref.target == nullptr) {
                    return VisitAction::SKIP_CHILDREN;
                }
                if (auto pFuncDecl = DynamicCast<AST::FuncDecl*>(refExpr.ref.target.get()); pFuncDecl) {
                    if (ExternalDateCheckerHelper(*refExpr.ref.target)) {
                        isExternalInterfaceFlag = true;
                    }
                    return VisitAction::SKIP_CHILDREN;
                } else if (auto pVarDecl = DynamicCast<AST::VarDecl*>(refExpr.ref.target.get()); pVarDecl) {
                    IsExternalInterface(pVarDecl->initializer.get());
                    return VisitAction::SKIP_CHILDREN;
                }
                return VisitAction::WALK_CHILDREN;
            },
            []() { return VisitAction::WALK_CHILDREN; });
    });
    walker.Walk();
}

void StructuralRuleGCHK02::UpdateLogInfoByAssignExpr(const Codira::AST::AssignExpr &assignExpr)
{
    auto tmpSet = externalDataLogSet;
    for (auto &logInfo : tmpSet) {
        if (auto pRefExpr = DynamicCast<AST::RefExpr*>(assignExpr.leftValue.get().get()); pRefExpr) {
            if (pRefExpr->ref.identifier != logInfo.name) {
                continue;
            }
            IsExternalInterface(assignExpr.rightExpr.get());
        } else if (auto pMemberAccess = DynamicCast<AST::MemberAccess*>(assignExpr.leftValue.get().get());
                   pMemberAccess) {
            auto name = pMemberAccess->baseExpr->ToString() + "." + pMemberAccess->field;
            if (name != logInfo.name) {
                continue;
            }
            IsExternalInterface(assignExpr.rightExpr.get());
        }
        if (isExternalInterfaceFlag && assignExpr.scopeName == logInfo.scopeName &&
            assignExpr.assignPos.fileID == logInfo.start.fileID && assignExpr.assignPos.line < logInfo.start.line) {
            EditLogInfo(logInfo, false);
            isExternalInterfaceFlag = false;
            continue;
        }
        if (!isExternalInterfaceFlag && assignExpr.scopeName == logInfo.scopeName &&
            assignExpr.assignPos.fileID == logInfo.start.fileID && assignExpr.assignPos.line < logInfo.start.line) {
            EditLogInfo(logInfo, true);
        }
    }
}

void StructuralRuleGCHK02::EditLogInfo(const LogInfo &logInfo, bool isPurified)
{
    LogInfo tmp;
    tmp.name = logInfo.name;
    tmp.start = logInfo.start;
    tmp.scopeName = logInfo.scopeName;
    tmp.isPurified = isPurified;
    (void)externalDataLogSet.erase(logInfo);
    (void)externalDataLogSet.insert(tmp);
}

void StructuralRuleGCHK02::UpdateLogInfoByMatchExprHelper(const Codira::AST::MatchExpr &matchExpr,
    Ptr<Codira::AST::Expr> expr, const LogInfo &logInfo)
{
    if (auto pRefExpr = DynamicCast<AST::RefExpr*>(expr.get()); pRefExpr) {
        if (logInfo.name == pRefExpr->ref.identifier && logInfo.start.fileID == matchExpr.leftCurlPos.fileID &&
            logInfo.start.line > matchExpr.leftCurlPos.line && logInfo.start.line < matchExpr.rightCurlPos.line) {
            EditLogInfo(logInfo, true);
        }
    } else if (auto pMemberAccess = DynamicCast<AST::MemberAccess*>(expr.get()); pMemberAccess) {
        if (logInfo.name == pMemberAccess->ToString() && logInfo.start.fileID == matchExpr.leftCurlPos.fileID &&
            logInfo.start.line > matchExpr.leftCurlPos.line && logInfo.start.line < matchExpr.rightCurlPos.line) {
            EditLogInfo(logInfo, true);
        }
    } else if (auto pCallExpr = DynamicCast<AST::CallExpr*>(expr.get()); pCallExpr) {
        UpdateLogInfoByMatchExprHelper(matchExpr, pCallExpr->baseFunc.get(), logInfo);
    }
}

void StructuralRuleGCHK02::UpdateLogInfoByMatchExpr(const Codira::AST::MatchExpr &matchExpr,
    Ptr<const Codira::AST::CallExpr> callExpr)
{
    if (callExpr == nullptr) {
        return;
    }

    auto tmpSet = externalDataLogSet;
    for (auto &item : callExpr->args) {
        for (auto &logInfo : tmpSet) {
            UpdateLogInfoByMatchExprHelper(matchExpr, item->expr.get(), logInfo);
        }
    }
}

void StructuralRuleGCHK02::Purifier(Ptr<Codira::AST::Node> node)
{
    if (node == nullptr) {
        return;
    }

    Walker walker(node, [this](Ptr<Node> node) -> VisitAction {
        return match(*node)(
            [this](const MatchExpr &matchExpr) {
                if (matchExpr.selector == nullptr) {
                    return VisitAction::SKIP_CHILDREN;
                }
                if (matchExpr.selector->astKind != ASTKind::CALL_EXPR) {
                    return VisitAction::SKIP_CHILDREN;
                }
                auto callExpr = As<ASTKind::CALL_EXPR>(matchExpr.selector.get());
                if (IsRegex(callExpr)) {
                    UpdateLogInfoByMatchExpr(matchExpr, callExpr);
                }
                return VisitAction::WALK_CHILDREN;
            },
            [this](const AssignExpr &assignExpr) {
                UpdateLogInfoByAssignExpr(assignExpr);
                return VisitAction::SKIP_CHILDREN;
            },
            []() { return VisitAction::WALK_CHILDREN; });
    });
    walker.Walk();
}

void StructuralRuleGCHK02::MatchPattern(ASTContext &ctx, Ptr<Node> node)
{
    (void)ctx;
    if (CommonFunc::ReadJsonFileToJsonInfo(JSONPATH, ConfigContext::GetInstance(), jsonInfo) == ERR) {
        return;
    }
    TaintVarFinder(node);
    TaintLoggerFinder(node);
    Purifier(node);
    for (const auto &logInfo : externalDataLogSet) {
        if (!logInfo.isPurified) {
            Diagnose(logInfo.start, logInfo.end, CodeCheckDiagKind::G_CHK_02_external_data_record_logs, logInfo.name);
        }
    }
}
} // namespace Codira::CodeCheck
