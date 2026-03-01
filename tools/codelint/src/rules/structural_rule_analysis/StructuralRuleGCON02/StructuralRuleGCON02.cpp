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

#include "StructuralRuleGCON02.h"

namespace Codira::CodeCheck {
using namespace Codira;
using namespace AST;
using namespace Meta;

std::string StructuralRuleGCON02::GetLockKey(const VarDecl& varDecl)
{
    if (auto refType = DynamicCast<RefType*>(varDecl.type.get().get()); refType) {
        auto ident = refType->ref.identifier;
        if (ident != "ReentrantMutex") {
            return "";
        }
    } else if (auto callExpr = DynamicCast<CallExpr*>(varDecl.initializer.get().get()); callExpr) {
        auto refExpr = DynamicCast<RefExpr*>(callExpr->baseFunc.get().get());
        if (refExpr == nullptr) {
            return "";
        }
        auto ident = refExpr->ref.identifier;
        if (ident != "ReentrantMutex") {
            return "";
        }
    }

    std::string fileName = (varDecl.curFile != nullptr) ? varDecl.curFile->fileName : "";
    auto varName = varDecl.identifier.Val();
    auto declLine = varDecl.identifier.Begin().line;
    std::string key = varName + "_" + fileName + "_" + std::to_string(declLine);

    return key;
}

void StructuralRuleGCON02::GetLockInfo(const VarDecl& varDecl)
{
    auto key = GetLockKey(varDecl);
    if (key.size() == 0) {
        return;
    }

    if (varMap.count(key) != 0) {
        return;
    }

    varMap[key] = varDecl.identifier;

    return;
}

std::string StructuralRuleGCON02::GetMemberAccessKey(const MemberAccess& memberAccess)
{
    if (auto refExpr = DynamicCast<RefExpr*>(memberAccess.baseExpr.get().get()); refExpr) {
        auto varDecl = DynamicCast<VarDecl*>(refExpr->ref.target.get());
        if (varDecl == nullptr) {
            return "";
        }

        return GetLockKey(*varDecl);
    } else if (auto memberAccessBaseExpr = DynamicCast<MemberAccess*>(memberAccess.baseExpr.get().get());
               memberAccessBaseExpr) {
        auto varDecl = DynamicCast<VarDecl*>(memberAccessBaseExpr->target.get());
        if (varDecl == nullptr) {
            return "";
        }

        return GetLockKey(*varDecl);
    }

    return "";
}

std::string StructuralRuleGCON02::GetMemberAccessField(const MemberAccess& memberAccess)
{
    std::string prefixFiled = "";
    if (auto memberAccessBaseExpr = DynamicCast<MemberAccess*>(memberAccess.baseExpr.get().get());
        memberAccessBaseExpr) {
        if (auto refExpr = DynamicCast<RefExpr*>(memberAccessBaseExpr->baseExpr.get().get()); refExpr) {
            prefixFiled = refExpr->ref.identifier;
        }
    }

    return prefixFiled;
}

void StructuralRuleGCON02::TryLockCheck(const MemberAccess& memberAccess)
{
    auto fieldStr = memberAccess.field;
    if (fieldStr != "lock") {
        return;
    }

    auto key = GetMemberAccessKey(memberAccess);
    if (varMap.count(key) == 0) {
        return;
    }

    if (tryLockMap.count(key) != 0) {
        return;
    }

    auto prefix = GetMemberAccessField(memberAccess);
    if (prefix.size() != 0) {
        tryLockMap[key] = std::make_tuple(memberAccess.begin, memberAccess.end, true, prefix + ".");
    } else {
        tryLockMap[key] = std::make_tuple(memberAccess.begin, memberAccess.end, true, "");
    }

    return;
}

void StructuralRuleGCON02::FinallyUnLockCheck(const MemberAccess& memberAccess)
{
    auto fieldStr = memberAccess.field;
    if (fieldStr != "unlock") {
        return;
    }

    auto key = GetMemberAccessKey(memberAccess);
    if (varMap.count(key) == 0) {
        return;
    }

    if (tryLockMap.count(key) == 0) {
        return;
    }

    std::get<TUPLE_PARAM_3RD>(tryLockMap[key]) = false;

    return;
}

void StructuralRuleGCON02::PrintDiagInfo()
{
    std::vector<std::tuple<Position, Position, std::string, std::string>> diagInfo;
    for (auto iter = tryLockMap.begin(); iter != tryLockMap.end(); ++iter) {
        auto lockName = varMap[iter->first];
        if (std::get<TUPLE_PARAM_3RD>(iter->second)) {
            diagInfo.push_back(std::make_tuple(std::get<TUPLE_PARAM_1ST>(iter->second),
                std::get<TUPLE_PARAM_2ND>(iter->second), std::get<TUPLE_PARAM_4TH>(iter->second) + lockName, lockName));
        }
    }

    std::sort(diagInfo.begin(), diagInfo.end());
    (void)diagInfo.erase(std::unique(diagInfo.begin(), diagInfo.end()), diagInfo.end());
    for (auto& diag : diagInfo) {
        Diagnose(std::get<TUPLE_PARAM_1ST>(diag), std::get<TUPLE_PARAM_2ND>(diag), CodeCheckDiagKind::G_CON_02_lock,
            std::get<TUPLE_PARAM_3RD>(diag), std::get<TUPLE_PARAM_4TH>(diag));
    }

    tryLockMap.clear();

    return;
}

void StructuralRuleGCON02::CheckTryExpr(const TryExpr& tryExpr)
{
    std::string fileName = (tryExpr.curFile != nullptr) ? tryExpr.curFile->fileName : "";
    auto tryKey = fileName + "_" + std::to_string(tryExpr.begin.line);
    if (tryBlockKey.count(tryKey) != 0) {
        return;
    }

    Walker walkerTry(tryExpr.tryBlock.get(), [this](Ptr<Node> node) -> VisitAction {
        return match(*node)(
            [this](const VarDecl& varDecl) {
                GetLockInfo(varDecl);
                return VisitAction::WALK_CHILDREN;
            },
            [this](const MemberAccess& memberAccess) {
                TryLockCheck(memberAccess);
                return VisitAction::WALK_CHILDREN;
            },
            []() { return VisitAction::WALK_CHILDREN; });
    });

    walkerTry.Walk();

    Walker walkerFinally(tryExpr.finallyBlock.get(), [this](Ptr<Node> node) -> VisitAction {
        return match(*node)(
            [this](const VarDecl& varDecl) {
                GetLockInfo(varDecl);
                return VisitAction::WALK_CHILDREN;
            },
            [this](const MemberAccess& memberAccess) {
                FinallyUnLockCheck(memberAccess);
                return VisitAction::WALK_CHILDREN;
            },
            []() { return VisitAction::WALK_CHILDREN; });
    });

    walkerFinally.Walk();

    if (tryBlockKey.count(tryKey) == 0) {
        (void)tryBlockKey.insert(tryKey);
    }

    PrintDiagInfo();

    return;
}

void StructuralRuleGCON02::CheckResult(Ptr<Node> node)
{
    if (node == nullptr) {
        return;
    }

    Walker walker(node, [this](Ptr<Node> node) -> VisitAction {
        return match(*node)(
            [this](const VarDecl& varDecl) {
                GetLockInfo(varDecl);
                return VisitAction::WALK_CHILDREN;
            },
            [this](const TryExpr& tryExpr) {
                CheckTryExpr(tryExpr);
                return VisitAction::SKIP_CHILDREN;
            },
            []() { return VisitAction::WALK_CHILDREN; });
    });
    walker.Walk();

    varMap.clear();
    tryLockMap.clear();

    return;
}

void StructuralRuleGCON02::MatchPattern(ASTContext& ctx, Ptr<Node> node)
{
    (void)ctx;
    CheckResult(node);
    return;
}
} // namespace Codira::CodeCheck
