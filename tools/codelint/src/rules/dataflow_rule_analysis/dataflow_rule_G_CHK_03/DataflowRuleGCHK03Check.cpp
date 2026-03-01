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

#include "DataflowRuleGCHK03Check.h"

namespace Codira::CodeCheck {
constexpr const char* JSONPATH = "/config/dataflow_rule_G_CHK_03.json";
} // namespace Codira::CodeCheck

using namespace Codira::CHIR;
using namespace Codira::CodeCheck;
using namespace Codira;

static bool IsStructPath(CHIR::Debug* debug)
{
    if (!debug) {
        return false;
    }
    auto value = debug->GetValue();
    if (!value) {
        return false;
    }
    auto ty = value->GetType();
    auto typeName = CommonFunc::PrintType(ty);
    return typeName == "Path" &&
        (ty->GetTypeKind() == CHIR::Type::TYPE_STRUCT || ty->GetTypeKind() == CHIR::Type::TYPE_REFTYPE);
}

// Access Struct-type variables using the Debug node
static void CheckDebug(CHIR::Debug* debug, std::map<const std::string, size_t>& allocateIdxMap, size_t& allocateIdx)
{
    if (IsStructPath(debug)) {
        allocateIdxMap.emplace(debug->GetSrcCodeIdentifier(), allocateIdx++);
    }
}

// Gather all variables of type Struct
static void CollectAllPathsInFunc(
    CHIR::Func* func, std::map<const std::string, size_t>& allocateIdxMap, size_t& allocateIdx)
{
    for (auto bb : func->GetBody()->GetBlocks()) {
        for (auto expr : bb->GetExpressions()) {
            if (expr->GetExprKind() != CHIR::ExprKind::DEBUGEXPR) {
                continue;
            }
            auto debug = StaticCast<CHIR::Debug*>(expr);
            CheckDebug(debug, allocateIdxMap, allocateIdx);
        }
    }
}

// Retrieve the corresponding variable name via the function's arguments
static CHIR::Debug* GetDebugExpr(CHIR::Value* arg)
{
    if (!arg || !arg->IsLocalVar()) {
        return nullptr;
    }
    auto localVar = StaticCast<CHIR::LocalVar*>(arg);
    auto debug = localVar->GetDebugExpr();
    if (debug) {
        return debug;
    }
    auto expr = localVar->GetExpr();
    if (expr->GetExprKind() != CHIR::ExprKind::LOAD) {
        return nullptr;
    }
    auto load = StaticCast<CHIR::Load*>(expr);
    return GetDebugExpr(load->GetLocation());
}

// Verify whether the function conforms to a specific category
static bool IsSpecificFunc(const CHIRFuncInfo& funcInfo, std::string category, nlohmann::json jsonInfo)
{
    auto funcName = funcInfo.funcName;
    auto parentName = funcInfo.parentTy ? CommonFunc::PrintType(funcInfo.parentTy) : "";
    auto pkgName = funcInfo.pkgName;
    if (!jsonInfo.contains(category)) {
        return false;
    }
    for (const auto& item : jsonInfo[category]) {
        if (!item.contains("parent") || !item.contains("name") || !item.contains("lib")) {
            continue;
        }
        std::string parent = item["parent"].get<std::string>();
        std::string name = item["name"].get<std::string>();
        std::string lib = item["lib"].get<std::string>();
        if (funcInfo.funcName == name && parentName == parent && pkgName == lib) {
            return true;
        }
    }
    return false;
}

static CHIR::LocalVar* GetLocalVar(Value* value)
{
    if (!value || !value->IsLocalVar()) {
        return nullptr;
    }
    auto localVar = StaticCast<CHIR::LocalVar*>(value);
    if (localVar->GetDebugExpr()) {
        return localVar;
    }
    auto users = localVar->GetUsers();
    if (users.size() > 0 && users[0]->GetExprKind() == CHIR::ExprKind::STORE) {
        auto store = StaticCast<CHIR::Store*>(users[0]);
        auto loc = store->GetLocation();
        if (!loc || !loc->IsLocalVar()) {
            return nullptr;
        }
        localVar = StaticCast<CHIR::LocalVar*>(loc);
        if (localVar->GetDebugExpr()) {
            return localVar;
        }
    }
    return nullptr;
}

template <> const std::optional<unsigned> Analysis<CHK03VerifyDomain>::blockLimit = std::nullopt;
template <> const AnalysisKind GenKillDomain<CHK03VerifyDomain>::mustOrMaybe = AnalysisKind::MAYBE;
CHK03VerifyAnalysis::CHK03VerifyAnalysis(Func* func, nlohmann::json jsonInfo)
    : GenKillAnalysis(func), jsonInfo(jsonInfo)
{
    size_t allocateIdx = 0;
    CollectAllPathsInFunc(func, allocateIdxMap, allocateIdx);
    domainSize = allocateIdx;
}

void CHK03VerifyAnalysis::InitializeFuncEntryState(CHK03VerifyDomain& state)
{
    state.kind = ReachableKind::REACHABLE;
    state.GenAll();
}

void CHK03VerifyAnalysis::PropagateExpressionEffect(CHK03VerifyDomain& state, const Expression* expression)
{
    auto kind = expression->GetExprKind();
    if (kind == CHIR::ExprKind::APPLY) {
        auto apply = Codira::StaticCast<Codira::CHIR::Apply*>(expression);
        auto funcInfo = CommonFunc::GetCHIRFuncInfo(apply->GetCallee());
        if (!IsSpecificFunc(funcInfo, "VerifyFunc", jsonInfo)) {
            return;
        }
        for (auto arg : apply->GetArgs()) {
            auto debug = GetDebugExpr(arg);
            if (!debug || !IsStructPath(debug)) {
                continue;
            }
            state.Kill(allocateIdxMap[debug->GetSrcCodeIdentifier()]);
        }
    }
    if (kind == CHIR::ExprKind::STORE) {
        auto store = Codira::StaticCast<Codira::CHIR::Store*>(expression);
        auto debugOfValue = GetDebugExpr(store->GetValue());
        auto debugOfLoc = GetDebugExpr(store->GetLocation());
        if (!debugOfValue || !debugOfLoc) {
            return;
        }
        if (state.IsTrueAt(allocateIdxMap[debugOfValue->GetSrcCodeIdentifier()])) {
            state.Gen(allocateIdxMap[debugOfLoc->GetSrcCodeIdentifier()]);
        } else {
            state.Kill(allocateIdxMap[debugOfLoc->GetSrcCodeIdentifier()]);
        }
    }
    if (kind == CHIR::ExprKind::LOAD) {
        auto load = Codira::StaticCast<Codira::CHIR::Load*>(expression);
        auto debugOfLoc = GetDebugExpr(load->GetLocation());
        auto debugOfLoad = load->GetResult()->GetDebugExpr();
        if (!debugOfLoc || !debugOfLoad) {
            return;
        }
        if (state.IsTrueAt(allocateIdxMap[debugOfLoc->GetSrcCodeIdentifier()])) {
            state.Gen(allocateIdxMap[debugOfLoad->GetSrcCodeIdentifier()]);
        } else {
            state.Kill(allocateIdxMap[debugOfLoad->GetSrcCodeIdentifier()]);
        }
    }
}

std::optional<Block*> CHK03VerifyAnalysis::PropagateTerminatorEffect(
    CHK03VerifyDomain& state, const Terminator* terminator)
{
    return std::nullopt;
}

CHK03VerifyDomain CHK03VerifyAnalysis::Bottom()
{
    return CHK03VerifyDomain(domainSize, &allocateIdxMap);
}

template <> const std::optional<unsigned> Analysis<CHK03CanonicalizeDomain>::blockLimit = std::nullopt;
template <> const AnalysisKind GenKillDomain<CHK03CanonicalizeDomain>::mustOrMaybe = AnalysisKind::MAYBE;

CHK03CanonicalizeAnalysis::CHK03CanonicalizeAnalysis(Func* func, nlohmann::json jsonInfo)
    : GenKillAnalysis(func), jsonInfo(jsonInfo)
{
    size_t allocateIdx = 0;
    CollectAllPathsInFunc(func, allocateIdxMap, allocateIdx);
    domainSize = allocateIdx;
}

void CHK03CanonicalizeAnalysis::InitializeFuncEntryState(CHK03CanonicalizeDomain& state)
{
    state.kind = ReachableKind::REACHABLE;
    state.GenAll();
}

void CHK03CanonicalizeAnalysis::PropagateExpressionEffect(CHK03CanonicalizeDomain& state, const Expression* expression)
{
    auto kind = expression->GetExprKind();
    if (kind == CHIR::ExprKind::APPLY) {
        auto apply = Codira::StaticCast<Codira::CHIR::Apply*>(expression);
        auto funcInfo = CommonFunc::GetCHIRFuncInfo(apply->GetCallee());
        if (IsSpecificFunc(funcInfo, "CanonicalizeFunc", jsonInfo)) {
            auto result = apply->GetResult();
            auto localVar = GetLocalVar(result);
            if (!localVar) {
                return;
            }
            auto debug = localVar->GetDebugExpr();
            if (debug) {
                state.Kill(allocateIdxMap[debug->GetSrcCodeIdentifier()]);
            }
        }
    }
    if (kind == CHIR::ExprKind::STORE) {
        auto store = Codira::StaticCast<Codira::CHIR::Store*>(expression);
        auto debugOfValue = GetDebugExpr(store->GetValue());
        auto debugOfLoc = GetDebugExpr(store->GetLocation());
        if (!debugOfValue || !debugOfLoc) {
            return;
        }
        if (state.IsTrueAt(allocateIdxMap[debugOfValue->GetSrcCodeIdentifier()])) {
            state.Gen(allocateIdxMap[debugOfLoc->GetSrcCodeIdentifier()]);
        } else {
            state.Kill(allocateIdxMap[debugOfLoc->GetSrcCodeIdentifier()]);
        }
    }
    if (kind == CHIR::ExprKind::LOAD) {
        auto load = Codira::StaticCast<Codira::CHIR::Load*>(expression);
        auto debugOfLoc = GetDebugExpr(load->GetLocation());
        auto debugOfLoad = load->GetResult()->GetDebugExpr();
        if (!debugOfLoc || !debugOfLoad) {
            return;
        }
        if (state.IsTrueAt(allocateIdxMap[debugOfLoc->GetSrcCodeIdentifier()])) {
            state.Gen(allocateIdxMap[debugOfLoad->GetSrcCodeIdentifier()]);
        } else {
            state.Kill(allocateIdxMap[debugOfLoad->GetSrcCodeIdentifier()]);
        }
    }
}

std::optional<Block*> CHK03CanonicalizeAnalysis::PropagateTerminatorEffect(
    CHK03CanonicalizeDomain& state, const Terminator* terminator)
{
    return std::nullopt;
}

CHK03CanonicalizeDomain CHK03CanonicalizeAnalysis::Bottom()
{
    return CHK03CanonicalizeDomain(domainSize, &allocateIdxMap);
}

void DataflowRuleGCHK03Check::IsPathCanonicalized(CHIR::Func* func)
{
    auto analysis = std::make_unique<CHK03CanonicalizeAnalysis>(func, jsonInfo);
    auto engine = Engine<CHK03CanonicalizeDomain>(func, std::move(analysis));
    auto result = engine.IterateToFixpoint();
    if (!result) {
        return;
    }
    const auto actionBeforeVisitExpr = [this](const CHK03CanonicalizeDomain& state, Expression* expr, size_t) {
        if (expr->GetExprKind() == CHIR::ExprKind::APPLY) {
            auto apply = Codira::StaticCast<Codira::CHIR::Apply*>(expr);
            auto func = Codira::DynamicCast<Codira::CHIR::Func*>(apply->GetCallee());
            if (!func) {
                return;
            }
            auto funcInfo = CommonFunc::GetCHIRFuncInfo(func);
            if (!IsSpecificFunc(funcInfo, "VerifyFunc", jsonInfo)) {
                return;
            }
            for (auto arg : apply->GetArgs()) {
                auto debug = GetDebugExpr(arg);
                if (!debug) {
                    continue;
                }
                auto name = debug->GetSrcCodeIdentifier();
                auto stateMap = *state.GetMap();
                if (stateMap.count(name) > 0 && state.IsTrueAt(stateMap[name])) {
                    auto pos = CommonFunc::GetCodePosition(apply->GetResult());
                    diagEngine->Diagnose(pos.first, pos.second, CodeCheckDiagKind::G_CHK_03_without_canonicalize,
                        debug->GetSrcCodeIdentifier());
                }
            }
        }
    };
    const auto actionAfterVisitExpr = [](const CHK03CanonicalizeDomain&, Expression*, size_t) {};
    const auto actionOnTerminator = [](const CHK03CanonicalizeDomain&, const Terminator*, std::optional<Block*>) {};
    result->VisitWith(actionBeforeVisitExpr, actionAfterVisitExpr, actionOnTerminator);
}

void DataflowRuleGCHK03Check::IsPathVerified(CHIR::Func* func)
{
    auto analysis = std::make_unique<CHK03VerifyAnalysis>(func, jsonInfo);
    auto engine = Engine<CHK03VerifyDomain>(func, std::move(analysis));
    auto result = engine.IterateToFixpoint();
    if (!result) {
        return;
    }
    const auto actionBeforeVisitExpr = [this](const CHK03VerifyDomain& state, Expression* expr, size_t) {
        if (expr->GetExprKind() == CHIR::ExprKind::APPLY) {
            auto apply = Codira::StaticCast<Codira::CHIR::Apply*>(expr);
            auto func = Codira::DynamicCast<Codira::CHIR::Func*>(apply->GetCallee());
            if (!func) {
                return;
            }
            auto funcInfo = CommonFunc::GetCHIRFuncInfo(func);
            if (IsSpecificFunc(funcInfo, "CanonicalizeFunc", jsonInfo) ||
                IsSpecificFunc(funcInfo, "VerifyFunc", jsonInfo)) {
                return;
            }
            for (auto arg : apply->GetArgs()) {
                auto debug = GetDebugExpr(arg);
                if (!debug) {
                    continue;
                }
                auto name = debug->GetSrcCodeIdentifier();
                auto stateMap = *state.GetMap();
                if (stateMap.count(name) > 0 && state.IsTrueAt(stateMap[name])) {
                    auto pos = CommonFunc::GetCodePosition(apply->GetResult());
                    diagEngine->Diagnose(pos.first, pos.second, CodeCheckDiagKind::G_CHK_03_without_verify,
                        debug->GetSrcCodeIdentifier());
                }
            }
        }
    };
    const auto actionAfterVisitExpr = [](const CHK03VerifyDomain&, Expression*, size_t) {};
    const auto actionOnTerminator = [](const CHK03VerifyDomain&, const Terminator*, std::optional<Block*>) {};
    result->VisitWith(actionBeforeVisitExpr, actionAfterVisitExpr, actionOnTerminator);
}

void DataflowRuleGCHK03Check::CheckGlobalFunc(CHIR::Func* func)
{
    for (auto bb : func->GetBody()->GetBlocks()) {
        for (auto expr : bb->GetExpressions()) {
            if (expr->GetExprKind() != CHIR::ExprKind::APPLY) {
                continue;
            }
            auto apply = StaticCast<CHIR::Apply*>(expr);
            auto funcInfo = CommonFunc::GetCHIRFuncInfo(apply->GetCallee());
            if (!IsSpecificFunc(funcInfo, "ExternalDateFunc", jsonInfo)) {
                continue;
            }
            auto localVar = GetLocalVar(apply->GetResult());
            if (!localVar) {
                continue;
            }
            for (auto user : localVar->GetUsers()) {
                if (user->GetExprKind() != CHIR::ExprKind::APPLY) {
                    continue;
                }
                auto apply = StaticCast<CHIR::Apply*>(user);
                auto callee = apply->GetCallee();
                if (!callee->IsFuncWithBody()) {
                    continue;
                }
                auto func = DynamicCast<CHIR::Func*>(callee);
                if (!func) {
                    continue;
                }
                IsPathCanonicalized(func);
                IsPathVerified(func);
            }
        }
    }
}

void DataflowRuleGCHK03Check::CheckBasedOnCHIR(CHIR::Package& package)
{
    if (Codira::CodeCheck::CommonFunc::ReadJsonFileToJsonInfo(
        JSONPATH, ConfigContext::GetInstance(), jsonInfo) == ERR) {
        return;
    }
    for (auto func : package.GetGlobalFuncs()) {
        // skip global var init func which is compilerAdd
        // Extract the first 5 characters to determine if it is a class automatically generated by the closure
        if (func->GetIdentifier().compare(0, 5, "@_CGV") == 0) {
            continue;
        }
        // Skip generic instantiation functions
        if (func->TestAttr(Attribute::GENERIC_INSTANTIATED) || func->TestAttr(CHIR::Attribute::IMPORTED)) {
            continue;
        }
        // Skip generic instantiation member functions
        if (func->GetOuterDeclaredOrExtendedDef() &&
            func->GetOuterDeclaredOrExtendedDef()->TestAttr(Attribute::GENERIC_INSTANTIATED)) {
            continue;
        }
        CheckGlobalFunc(func);
    }
}
