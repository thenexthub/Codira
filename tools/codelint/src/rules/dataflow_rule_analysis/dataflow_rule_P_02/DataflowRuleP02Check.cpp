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

#include "DataflowRuleP02Check.h"
#include "Codira/AST/Match.h"
#include "Codira/CHIR/Package.h"
#include "Codira/CHIR/Visitor/Visitor.h"
#include "common/CommonFunc.h"

namespace Codira::CodeCheck {

using namespace Codira::CHIR;

// Check whether the store node corresponds to the return value
static bool IsReturnStore(const CHIR::Store *store)
{
    auto localVar = DynamicCast<CHIR::LocalVar>(store->GetLocation());
    if (localVar && localVar->GetExpr() && localVar->GetExpr()->GetExprKind() == CHIR::ExprKind::ALLOCATE) {
        auto allocate = StaticCast<CHIR::Allocate *>(localVar->GetExpr());
        if (allocate->GetResult()->IsRetValue()) {
            return true;
        }
    }
    return false;
}

static bool IsMtxLoad(const CHIR::Load *load)
{
    auto location = load->GetLocation();
    if (location && location->GetType() && CommonFunc::PrintType(location->GetType()) == "ReentrantMutex") {
        return true;
    }
    return false;
}

static bool IsNotLetGlobalVar(const CHIR::Load *load)
{
    auto location = load->GetLocation();
    return !location->TestAttr(Attribute::READONLY);
}

void DataflowRuleP02Check::AddElementToSet(const CHIR::Value *value)
{
    if (!value->IsLocalVar()) {
        varStoreInSpawn.insert(value);
        return;
    }
    auto localVar = StaticCast<CHIR::LocalVar *>(value);
    auto expr = localVar->GetExpr();
    if (expr->GetExprKind() == CHIR::ExprKind::GET_ELEMENT_REF) {
        auto getElementRef = StaticCast<CHIR::GetElementRef *>(expr);
        auto memberInChain = GetMemberVarInChain<CHIR::GetElementRef>(getElementRef);
        varInChainStoreInSpawn.insert(memberInChain);
    }
    if (expr->GetExprKind() == CHIR::ExprKind::STORE_ELEMENT_REF) {
        auto storeElementRef = StaticCast<CHIR::StoreElementRef *>(expr);
        auto memberInChain = GetMemberVarInChain<CHIR::StoreElementRef>(storeElementRef);
        varInChainStoreInSpawn.insert(memberInChain);
    }
    if (expr->GetExprKind() == CHIR::ExprKind::FIELD) {
        auto field = StaticCast<CHIR::Field*>(expr);
        auto memberInChain = GetMemberVarInChain(expr);
        varInChainStoreInSpawn.insert(memberInChain);
    }
    if (expr->GetExprKind() == CHIR::ExprKind::STORE_ELEMENT_REF) {
        auto storeElementRef = StaticCast<CHIR::StoreElementRef *>(expr);
        auto memberInChain = GetMemberVarInChain<CHIR::StoreElementRef>(storeElementRef);
        varInChainStoreInSpawn.insert(memberInChain);
    }
}


template <typename T>
static void AddElementToMapWithMutexHelper(std::map<T, std::map<CHIR::Value*, std::set<Codira::Position>>>& varMap,
    T& value, CHIR::Value* mutex, Codira::Position& pos)
{
    if (varMap.count(value) == 0) {
        varMap[value] = {{mutex, {pos}}};
    } else if (varMap[value].count(mutex) == 0) {
        varMap[value].insert({mutex, {pos}});
    } else {
        varMap[value][mutex].insert(pos);
    }
}

template <typename T>
static void AddElementToMapWithoutMutexHelper(
    std::map<T, std::set<Codira::Position>>& varMap, T& value, Codira::Position& pos)
{
    if (varMap.count(value) == 0) {
        varMap[value] = {pos};
    } else if (varMap[value].count(pos) == 0) {
        varMap[value].insert(pos);
    }
}

void DataflowRuleP02Check::AddElementToMap(
    const CHIR::Value *value, Codira::Position pos, bool withLock)
{
    if (withLock) {
        auto mutex = mutexLock;
        if (!value->IsLocalVar()) {
            AddElementToMapWithMutexHelper<const CHIR::Value *>(varWithDiffLock, value, mutex, pos);
            return;
        }
        auto localVar = StaticCast<CHIR::LocalVar *>(value);
        auto expr = localVar->GetExpr();
        if (expr->GetExprKind() == CHIR::ExprKind::GET_ELEMENT_REF) {
            auto getElementRef = StaticCast<CHIR::GetElementRef *>(expr);
            auto memberInChain = GetMemberVarInChain<CHIR::GetElementRef>(getElementRef);
            AddElementToMapWithMutexHelper<MemberVarInChain>(varInChainWithDiffLock, memberInChain, mutex, pos);
            return;
        }
        if (expr->GetExprKind() == CHIR::ExprKind::STORE_ELEMENT_REF) {
            auto storeElementRef = StaticCast<CHIR::StoreElementRef *>(expr);
            auto memberInChain = GetMemberVarInChain<CHIR::StoreElementRef>(storeElementRef);
            AddElementToMapWithMutexHelper<MemberVarInChain>(varInChainWithDiffLock, memberInChain, mutex, pos);
        }
        if (expr->GetExprKind() == CHIR::ExprKind::FIELD) {
            auto field = StaticCast<CHIR::Field *>(expr);
            auto memberInChain = GetMemberVarInChain(expr);
            AddElementToMapWithMutexHelper<MemberVarInChain>(varInChainWithDiffLock, memberInChain, mutex, pos);
        }
    } else {
        if (!value->IsLocalVar()) {
            AddElementToMapWithoutMutexHelper<const CHIR::Value *>(varWithoutLock, value, pos);
            return;
        }
        auto localVar = StaticCast<CHIR::LocalVar *>(value);
        auto expr = localVar->GetExpr();
        if (expr->GetExprKind() == CHIR::ExprKind::GET_ELEMENT_REF) {
            auto getElementRef = StaticCast<CHIR::GetElementRef *>(expr);
            auto memberInChain = GetMemberVarInChain<CHIR::GetElementRef>(getElementRef);
            AddElementToMapWithoutMutexHelper<MemberVarInChain>(varInChainWithoutLock, memberInChain, pos);
            return;
        }
        if (expr->GetExprKind() == CHIR::ExprKind::STORE_ELEMENT_REF) {
            auto storeElementRef = StaticCast<CHIR::StoreElementRef *>(expr);
            auto memberInChain = GetMemberVarInChain<CHIR::StoreElementRef>(storeElementRef);
            AddElementToMapWithoutMutexHelper<MemberVarInChain>(varInChainWithoutLock, memberInChain, pos);
        }
        if (expr->GetExprKind() == CHIR::ExprKind::FIELD) {
            auto field = StaticCast<CHIR::Field *>(expr);
            auto memberInChain = GetMemberVarInChain(expr);
            AddElementToMapWithoutMutexHelper<MemberVarInChain>(varInChainWithoutLock, memberInChain, pos);
        }
    }
}

static CHIR::Value *GetClosureMutex(const CHIR::Invoke *invoke)
{
    auto localVar = StaticCast<CHIR::LocalVar *>(invoke->GetObject());
    if (localVar->GetDebugExpr()) {
        return localVar;
    }
    return localVar->GetExpr()->GetOperand(0);
}

static CHIR::Type *GetBaseTy(CHIR::Type *ty)
{
    return ty->IsRef() ? GetBaseTy(StaticCast<CHIR::RefType *>(ty)->GetBaseType()) : ty;
}

static bool IsClosureBase(const CHIR::Value *base)
{
    auto baseTy = GetBaseTy(base->GetType());
    if (baseTy->GetTypeKind() == Type::TypeKind::TYPE_CLASS) {
        auto classType = StaticCast<CHIR::ClassType *>(baseTy);
        auto parentDef = classType->GetClassDef();
        if (parentDef->GetIdentifier().find("$Auto_Env") != std::string::npos) {
            return true;
        }
    }
    return false;
}

static CHIR::Type *GetPathInChain(const CHIR::Type *ty, size_t index, std::string &str)
{
    auto baseTy = ty;
    while (baseTy->IsRef()) {
        baseTy = StaticCast<CHIR::RefType *>(baseTy)->GetBaseType();
    }
    if (!baseTy) {
        return nullptr;
    }
    if (baseTy->IsClass()) {
        auto classTy = StaticCast<CHIR::ClassType *>(baseTy);
        auto classDef = classTy->GetClassDef();
        auto memberVar = classDef->GetInstanceVar(index);
        str += ("." + memberVar.name);
        return memberVar.type;
    } else if (baseTy->IsStruct()) {
        auto structTy = StaticCast<CHIR::StructType *>(baseTy);
        auto structDef = structTy->GetStructDef();
        auto memberVar = structDef->GetInstanceVar(index);
        str += ("." + memberVar.name);
        return memberVar.type;
    }
    return nullptr;
}

static std::string GetPathStr(const CHIR::Value *base, const std::vector<uint64_t> &path)
{
    std::string pathStr = base->GetSrcCodeIdentifier();
    auto baseTy = base->GetType();
    for (auto i : path) {
        baseTy = GetPathInChain(baseTy, i, pathStr);
    }
    return pathStr;
}

static std::string GetVarName(CHIR::Value *value)
{
    if (value->IsLocalVar()) {
        auto localVar = StaticCast<CHIR::LocalVar *>(value);
        auto debugInfo = localVar->GetDebugExpr();
        if (debugInfo) {
            return debugInfo->GetSrcCodeIdentifier();
        }
    }
    return value->GetSrcCodeIdentifier();
}

template <typename T>
DataflowRuleP02Check::MemberVarInChain DataflowRuleP02Check::GetMemberVarInChain(const T* getElementRef)
{
    auto base = getElementRef->GetLocation();
    auto path = getElementRef->GetPath();
    if (!base->IsLocalVar()) {
        std::string str = base->GetSrcCodeIdentifier();
        auto baseTy = base->GetType();
        for (auto i : path) {
            baseTy = GetPathInChain(baseTy, i, str);
        }
        return MemberVarInChain(base, str);
    }
    auto localVar = StaticCast<CHIR::LocalVar *>(base);
    auto debugForLocalVar = localVar->GetDebugExpr();
    if (debugForLocalVar) {
        auto str = debugForLocalVar->GetSrcCodeIdentifier();
        auto baseTy = localVar->GetType();
        for (auto i : path) {
            baseTy = GetPathInChain(baseTy, i, str);
        }
        return MemberVarInChain(localVar, str);
    }
    auto expr = localVar->GetExpr();
    if (expr->GetExprKind() == CHIR::ExprKind::LOAD) {
        auto load = StaticCast<CHIR::Load *>(expr);
        auto value = load->GetLocation();
        auto baseTy = value->GetType();
        if (!value->IsLocalVar()) {
            std::string str = value->GetSrcCodeIdentifier();
            for (auto i : path) {
                baseTy = GetPathInChain(baseTy, i, str);
            }
            return MemberVarInChain(base, str);
        }
        auto newLocalVar = StaticCast<CHIR::LocalVar *>(value);
        auto newexpr = newLocalVar->GetExpr();
        if (newexpr->GetExprKind() == CHIR::ExprKind::GET_ELEMENT_REF) {
            auto newGetElementRef = StaticCast<CHIR::GetElementRef *>(newexpr);
            auto memberVarInChain = GetMemberVarInChain<CHIR::GetElementRef>(newGetElementRef);
            auto &str = memberVarInChain.path;
            for (auto i : path) {
                baseTy = GetPathInChain(baseTy, i, str);
            }
            return memberVarInChain;
        }
    }
    return MemberVarInChain(nullptr, "");
}

DataflowRuleP02Check::MemberVarInChain DataflowRuleP02Check::GetMemberVarInChain(
    CHIR::Expression *expr)
{
    if (expr->GetExprKind() == CHIR::ExprKind::GET_ELEMENT_REF) {
        auto getElementRef = StaticCast<CHIR::GetElementRef *>(expr);
        return GetMemberVarInChain<CHIR::GetElementRef>(getElementRef);
    }
    if (expr->GetExprKind() == CHIR::ExprKind::FIELD) {
        auto field = StaticCast<CHIR::Field *>(expr);
        auto base = field->GetBase();
        if (base->IsLocalVar()) {
            auto localVar = StaticCast<CHIR::LocalVar *>(base);
            auto memberVarInChain = GetMemberVarInChain(localVar->GetExpr());
            memberVarInChain.path += GetPathStr(localVar, field->GetPath());
            return memberVarInChain;
        } else {
            auto pathStr = GetPathStr(base, field->GetPath());
            return MemberVarInChain(base, pathStr);
        }
    }
    if (expr->GetExprKind() == CHIR::ExprKind::LOAD) {
        auto load = StaticCast<CHIR::Load *>(expr);
        auto base = load->GetLocation();
        if (base->IsLocalVar()) {
            auto localVar = StaticCast<CHIR::LocalVar *>(base);
            auto memberVarInChain = GetMemberVarInChain(localVar->GetExpr());
            return memberVarInChain;
        } else {
            auto pathStr = "";
            return MemberVarInChain(base, pathStr);
        }
    }
    return MemberVarInChain(expr->GetResult(), "");
}

static bool IsGlobalOrMemberVarInChain(const CHIR::Value *value)
{
    if (!value->IsLocalVar()) {
        return true;
    }
    auto localVar = StaticCast<CHIR::LocalVar *>(value);
    auto expr = localVar->GetExpr();
    if (expr->GetExprKind() == CHIR::ExprKind::GET_ELEMENT_REF) {
        return true;
    }
    return false;
}

void DataflowRuleP02Check::CheckNonTerminal(
    const CHIR::Expression *expr, bool &isLock)
{
    switch (expr->GetExprKind()) {
        case CHIR::ExprKind::STORE: {
            auto store = StaticCast<CHIR::Store *>(expr);
            auto value = store->GetLocation();
            AddElementToSet(value);
            if (!IsReturnStore(store)) {
                AddElementToMap(value, CommonFunc::GetCodePosition(store).first, isLock);
            }
            return;
        }
        case CHIR::ExprKind::STORE_ELEMENT_REF: {
            auto storeElementRef = StaticCast<CHIR::StoreElementRef *>(expr);
            auto value = storeElementRef->GetResult();
            AddElementToSet(value);
            AddElementToMap(value, CommonFunc::GetCodePosition(storeElementRef).first, isLock);
            return;
        }
        case CHIR::ExprKind::LOAD: {
            auto load = StaticCast<CHIR::Load *>(expr);
            auto users = load->GetResult()->GetUsers();
            /**
             * Skip the 'Load' Node In the Member Chain
             * class A {
             *  var a = 1
             *  func foo() {
             *      println(a) // %1: Load(this)-> %2: GetElementRef(%1, 1)
             *  }
             * }
             */
            if (users.size() == 1 && users[0]->GetExprKind() == CHIR::ExprKind::GET_ELEMENT_REF) {
                return;
            }
            auto value = load->GetLocation();
            // Temporary modification because CHIR didn't assign an position to load.
            auto pos = CommonFunc::GetCodePosition(load).first != Codira::Position(0, 0, 0)
                ? CommonFunc::GetCodePosition(load)
                : CommonFunc::GetCodePosition(value);
            if (IsGlobalOrMemberVarInChain(value) && !IsMtxLoad(load) && IsNotLetGlobalVar(load)) {
                AddElementToMap(value, pos.first, isLock);
            }
            return;
        }
        case CHIR::ExprKind::LAMBDA: {
            CheckSpawnClosure(StaticCast<const CHIR::Lambda*>(expr));
            return;
        }
        case CHIR::ExprKind::INVOKE: {
            auto invoke = StaticCast<CHIR::Invoke *>(expr);
            if (invoke->GetMethodName() == "lock" &&
                CommonFunc::PrintType(invoke->GetMethodType()) == "(Lock)->Unit") {
                auto closureMutex = GetClosureMutex(invoke);
                if (closureMutex->IsLocalVar()) {
                    auto localVar = StaticCast<CHIR::LocalVar *>(closureMutex);
                    mutexLock = GetMemberVarInChain(localVar->GetExpr()).base;
                } else {
                    mutexLock = closureMutex;
                }
                isLock = true;
                return;
            }
            if (invoke->GetMethodName() == "unlock" &&
                CommonFunc::PrintType(invoke->GetMethodType()) == "(Lock)->Unit") {
                isLock = false;
                mutexLock = nullptr;
                return;
            }
            return;
        }
        default:
            return;
    }
}

void DataflowRuleP02Check::CheckSpawnClosure(const CHIR::Lambda *spawnClosure)
{
    std::function<void(CHIR::Block *, bool, std::unordered_set<Block *>)> visitBlock =
        [this, &visitBlock](CHIR::Block *block, bool isLock, std::unordered_set<Block *> visited) {
            if (visited.find(block) != visited.end()) {
                return;
            }
            visited.emplace(block);
            for (auto expr : block->GetExpressions()) {
                if (!expr->IsTerminator()) {
                    CheckNonTerminal(expr, isLock);
                } else {
                    auto term = StaticCast<CHIR::Terminator *>(expr);
                    for (auto block : term->GetSuccessors()) {
                        visitBlock(block, isLock, visited);
                    }
                }
            }
        };
    std::unordered_set<Block *> visited;
    visitBlock(spawnClosure->GetEntryBlock(), false, visited);
}

template <typename T, typename = decltype(std::declval<T>().GetCallee())>
static CHIR::Func *GetSpawnClosure(const T *apply, AstFuncInfo spawenFunc)
{
    if (CommonFunc::FindCHIRFunction(apply->GetCallee(), spawenFunc)) {
        auto arg = StaticCast<CHIR::LocalVar *>(apply->GetArgs()[1]);
        return DynamicCast<CHIR::Func *>(arg->GetExpr()->GetOperands()[0]);
    }
    return nullptr;
}

template <typename T> void DataflowRuleP02Check::CheckApplyOrInVoke(const CHIR::Expression &expr)
{
    auto apply = StaticCast<T *>(&expr);
    auto spawnFunc = AstFuncInfo("init", "Future", {"Future", NOT_CARE}, ANY_TYPE, "std.core");
    if (CommonFunc::FindCHIRFunction(apply->GetCallee(), spawnFunc)) {
        auto lambdaVar = StaticCast<CHIR::LocalVar *>(apply->GetArgs()[1]);
        auto lambdaExpr = lambdaVar->GetExpr();
        if (lambdaExpr->GetExprKind() == CHIR::ExprKind::TYPECAST) {
            auto cast = StaticCast<const CHIR::TypeCast*>(lambdaExpr);
            CODEC_ASSERT(cast->GetSourceValue()->IsLocalVar());
            lambdaExpr = StaticCast<const CHIR::LocalVar*>(cast->GetSourceValue())->GetExpr();
        }
        CODEC_ASSERT(lambdaExpr->GetExprKind() == CHIR::ExprKind::LAMBDA);
        CheckSpawnClosure(StaticCast<const CHIR::Lambda*>(lambdaExpr));
    }
}

void DataflowRuleP02Check::CheckBasedOnCHIRFunc(CHIR::BlockGroup& body)
{
    Visitor::Visit(body, [this](CHIR::Expression &expr) {
        auto exprKind = expr.GetExprKind();
        if (exprKind == CHIR::ExprKind::LAMBDA) {
            CheckBasedOnCHIRFunc(*StaticCast<const CHIR::Lambda&>(expr).GetBody());
        }
        if (exprKind == CHIR::ExprKind::APPLY) {
            CheckApplyOrInVoke<CHIR::Apply>(expr);
        } else if (exprKind == CHIR::ExprKind::APPLY_WITH_EXCEPTION) {
            CheckApplyOrInVoke<CHIR::ApplyWithException>(expr);
        }
        return CHIR::VisitResult::CONTINUE;
    });
}

void DataflowRuleP02Check::CheckBasedOnCHIR(CHIR::Package &package)
{
    auto funcs = package.GetGlobalFuncs();
    for (auto &func : funcs) {
        if (func->TestAttr(CHIR::Attribute::IMPORTED)) {
            continue;
        }
        CheckBasedOnCHIRFunc(*func->GetBody());
    }
    PrintVarWithoutMutex();
    PrintVarInChainWithoutMutex();
    PrintVarWithDiffMutex();
    PrintVarInChainWithDiffMutex();
}

void DataflowRuleP02Check::PrintVarWithoutMutex()
{
    for (auto it : varWithoutLock) {
        if (varStoreInSpawn.count(it.first) == 0) {
            continue;
        }
        auto var = it.first;
        auto name = var->GetSrcCodeIdentifier();
        if (var->IsGlobalVarInCurPackage() && var->TestAttr(Attribute::STATIC)) {
            name = VirtualCast<CHIR::GlobalVar *>(var)->
                GetParentCustomTypeDef()->GetSrcCodeIdentifier() + "." + name;
        }
        auto begpos = *it.second.begin();
        std::set<size_t> lineSets;
        for (auto pos : it.second) {
            lineSets.insert(pos.line);
        }
        std::string strLine;
        for (auto line : lineSets) {
            strLine += std::to_string(line);
            strLine += ",";
        }
        diagEngine->Diagnose(begpos, CodeCheckDiagKind::P_02_no_mutex, name, strLine);
    }
}

void DataflowRuleP02Check::PrintVarInChainWithoutMutex()
{
    for (auto it : varInChainWithoutLock) {
        if (varInChainStoreInSpawn.count(it.first) == 0) {
            continue;
        }
        auto name = it.first.path;
        auto begpos = *it.second.begin();
        std::set<size_t> lineSets;
        for (auto pos : it.second) {
            lineSets.insert(pos.line);
        }
        std::string strLine;
        for (auto line : lineSets) {
            strLine += std::to_string(line);
            strLine += ",";
        }
        diagEngine->Diagnose(begpos, CodeCheckDiagKind::P_02_no_mutex, name, strLine);
    }
}

void DataflowRuleP02Check::PrintVarWithDiffMutex()
{
    for (auto it : varWithDiffLock) {
        if (it.second.size() == 1) {
            continue;
        }
        std::string tips{};
        auto varName = it.first->GetSrcCodeIdentifier();
        for (auto mutexInfo : it.second) {
            for (auto pos : mutexInfo.second) {
                auto line = CommonFunc::GetCodePosition(mutexInfo.first).first.line;
                if (!line) {
                    return;
                }
                tips += "'" + varName + "'" + " used in line " + std::to_string(pos.line) +
                    " is protected by mutex in line " + std::to_string(line) + ", ";
            }
        }
        auto loc = CommonFunc::GetCodePosition(it.first);
        diagEngine->Diagnose(loc.first, loc.second, CodeCheckDiagKind::P_02_diff_mutex, tips);
    }
}

CommonFunc::PositionPair DataflowRuleP02Check::FindPositionOfMemberVar(const MemberVarInChain& memberVarInChain)
{
    auto memberChainName = memberVarInChain.path;
    // 5 is the length of string "this.", 0 means the begin of the string
    if (memberChainName.size() < 5 || memberChainName.substr(0, 5) != "this.") {
        return CommonFunc::GetCodePosition(memberVarInChain.base);
    }
    auto baseType = memberVarInChain.base->GetType();
    std::vector<Codira::CHIR::MemberVarInfo> members;
    if (baseType->IsRef()) {
        auto type = StaticCast<CHIR::RefType*>(baseType)->GetBaseType();
        auto classTy = DynamicCast<CHIR::ClassType*>(type);
        if (!classTy) {
            return CommonFunc::GetCodePosition(memberVarInChain.base);
        }
        auto classDef = classTy->GetClassDef();
        members = classDef->GetDirectInstanceVars();
    } else {
        auto structTy = DynamicCast<CHIR::StructType*>(baseType);
        if (!structTy) {
            return CommonFunc::GetCodePosition(memberVarInChain.base);
        }
        auto structDef = structTy->GetStructDef();
        members = structDef->GetAllInstanceVars();
    }
    size_t startPos = 5;
    size_t endPos = memberChainName.find('.', startPos);
    std::string memberVarName = endPos == std::string::npos ? memberChainName.substr(startPos)
                                                            : memberChainName.substr(startPos, endPos - startPos);
    auto it = std::find_if(
        members.begin(), members.end(), [&memberVarName](auto& member) { return member.name == memberVarName; });
    if (it == members.end()) {
        return CommonFunc::GetCodePosition(memberVarInChain.base);
    } else {
        auto loc = (*it).loc;
        auto begPos = Codira::Position(loc.GetFileID(), loc.GetBeginPos().line, loc.GetBeginPos().column);
        auto endPos = Codira::Position(loc.GetFileID(), loc.GetEndPos().line, loc.GetEndPos().column);
        return std::make_pair(begPos, endPos);
    }
}

void DataflowRuleP02Check::PrintVarInChainWithDiffMutex()
{
    for (auto it : varInChainWithDiffLock) {
        if (it.second.size() == 1) {
            continue;
        }
        std::string tips{};
        auto fullName = it.first.path;
        // 5 is the length of string "this."
        auto varName = fullName.substr(0, 5) == "this." ? fullName.substr(5) : fullName;
        for (auto mutexInfo : it.second) {
            for (auto pos : mutexInfo.second) {
                auto line = CommonFunc::GetCodePosition(mutexInfo.first).first.line;
                if (!line) {
                    return;
                }
                tips += "'" + varName + "'" + " used in line " + std::to_string(pos.line) +
                    " is protected by mutex in line " + std::to_string(line) + ", ";
            }
        }
        auto pos = FindPositionOfMemberVar(it.first);
        diagEngine->Diagnose(pos.first, pos.second, CodeCheckDiagKind::P_02_diff_mutex, tips);
    }
}


} // namespace Codira::CodeCheck
