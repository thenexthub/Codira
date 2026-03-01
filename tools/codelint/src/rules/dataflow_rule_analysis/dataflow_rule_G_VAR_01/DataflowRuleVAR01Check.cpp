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

#include "DataflowRuleVAR01Check.h"
#include "Codira/AST/Walker.h"
#include "Codira/Basic/Match.h"
#include "Codira/AST/Match.h"
#include "Codira/CHIR/Package.h"
#include "Codira/CHIR/Visitor/Visitor.h"
#include "common/CommonFunc.h"

namespace Codira::CodeCheck {

using namespace Codira;
using namespace Codira::CHIR;

static bool IsNestedUsedInStore(Ptr<CHIR::GetElementRef> getElementRef);
static bool IsUsedInStore(Ptr<CHIR::Load> load)
{
    auto loadUsers = load->GetResult()->GetUsers();
    for (auto loadUser : loadUsers) {
        if (loadUser->GetExprKind() == CHIR::ExprKind::GET_ELEMENT_REF) {
            auto subGetElementRef = StaticCast<CHIR::GetElementRef*>(loadUser);
            if (IsNestedUsedInStore(subGetElementRef)) {
                return true;
            }
        }
    }
    return false;
}

static bool IsPropSetter(CHIR::Value* value)
{
    if (auto func = DynamicCast<FuncBase>(value)) {
        return func->GetFuncKind() == FuncKind::SETTER;
    }
    return false;
}

static bool IsUsedInApply(Ptr<CHIR::Load> load)
{
    auto loadUsers = load->GetResult()->GetUsers();
    for (auto loadUser : loadUsers) {
        if (loadUser->GetExprKind() == CHIR::ExprKind::APPLY) {
            auto apply = StaticCast<CHIR::Apply*>(loadUser);
            auto callee = apply->GetCallee();
            return IsPropSetter(callee);
        }
    }
    return false;
}

static bool IsNestedUsedInStore(Ptr<CHIR::GetElementRef> getElementRef)
{
    auto users = getElementRef->GetResult()->GetUsers();
    for (auto user : users) {
        if (user->GetExprKind() == CHIR::ExprKind::LOAD) {
            auto load = StaticCast<CHIR::Load*>(user);
            if (IsUsedInStore(load)) {
                return true;
            }
        }
    }
    return std::any_of(
        users.begin(), users.end(), [](auto& it) { return it->GetExprKind() == CHIR::ExprKind::STORE; });
}

void DataflowRuleVAR01Check::AddOrEraseElement(Ptr<CHIR::Value> var, GlobalVarAssignCountMap& globalVarAssignCountMap,
    LocalVarAssignCountMap& localVarAssignCountMap)
{
    if (!var) {
        return;
    }
    if (var->IsLocalVar()) {
        auto localVar = StaticCast<CHIR::LocalVar*>(var);
        if (localVarAssignCountMap.count(Ptr(localVar)) > 0) {
            localVarAssignCountMap[Ptr(localVar)] += 1;
        }
    } else if (var->IsGlobalVarInCurPackage()) {
        auto globalVar = VirtualCast<CHIR::GlobalVar*>(var);
        if (globalVarAssignCountMap.count(Ptr(globalVar)) > 0) {
            globalVarAssignCountMap[globalVar] += 1;
        }
    }
}

/*
 * For externally accessible variables, the check needs to be skipped, for example
 * '''
 * package pkg1
 * public var a = 1
 * '''
 */
static bool IsExternalGlobalVar(Ptr<GlobalVar> globalVar)
{
    if (globalVar->TestAttr(Attribute::PUBLIC)) {
        return true;
    }
    auto parentDef = globalVar->GetParentCustomTypeDef();
    if (parentDef && parentDef->TestAttr(Attribute::PUBLIC)) {
        return true;
    }
    return false;
}

void DataflowRuleVAR01Check::CollectMemberVar(Ptr<CHIR::CustomTypeDef> customTypeDef,
    std::vector<Codira::CHIR::MemberVarInfo>& memberVars, ClassMemberVarsMap& memberVarMap)
{
    // Collect all mutable vars in Class/Struct/Enum
    for (uint64_t i = 0; i < memberVars.size(); ++i) {
        if (!memberVars[i].TestAttr(Attribute::READONLY)) {
            if (memberVarMap.count(customTypeDef) == 0) {
                memberVarMap[customTypeDef] = {{i, 0}};
            } else {
                memberVarMap[customTypeDef].insert({i, 0});
            }
        }
    }
}

void DataflowRuleVAR01Check::CollectMemberVar(Ptr<CHIR::ClassDef> classDef, ClassMemberVarsMap& memberVarMap)
{
    const auto& memberVars = classDef->GetAllInstanceVars();
    const auto& localMemberVars = classDef->GetDirectInstanceVars();
    auto isInLocalMambers = [&memberVars, &localMemberVars](size_t index) {
        auto& memberVar = memberVars[index];
        auto it = std::find_if(localMemberVars.begin(), localMemberVars.end(),
            [&memberVar](auto& member) { return member.name == memberVar.name; });
        return it != localMemberVars.end();
    };
    for (uint64_t i = 0; i < memberVars.size(); ++i) {
        if (memberVars[i].TestAttr(Attribute::READONLY) || !isInLocalMambers(i)) {
            continue;
        }
        if (memberVarMap.count(classDef) == 0) {
            memberVarMap[classDef] = {{i, 0}};
        } else {
            memberVarMap[classDef].insert({i, 0});
        }
    }
}

void DataflowRuleVAR01Check::CheckCustomTypeDef(
    Ptr<CHIR::CustomTypeDef> customTypeDef, ClassMemberVarsMap& memberVarMap)
{
    if (customTypeDef->TestAttr(Attribute::PUBLIC)) {
        return;
    }
    bool isGenericInstance = customTypeDef->TestAttr(Attribute::GENERIC_INSTANTIATED);
    bool isGeneric = customTypeDef->TestAttr(Attribute::GENERIC);
    // Check if the mutability of a property is redundant
    for (auto& func : customTypeDef->GetMethods()) {
        if (!IsPropSetter(func)) {
            continue;
        }
        auto users = func->GetUsers();
        if (users.empty()) {
            auto setName = func->GetSrcCodeIdentifier();
            // Remove the '$' at the beginning and the 'set' characters at the end of the name of the setter function
            // obtained by desugar. '1' means the first, '3' and '4' are due to the last three characters.
            auto propName = (setName.front() == '$'
                ? setName.substr(1,
                    (setName.size() > 4 && setName.substr(setName.size() - 3) == "set") ? setName.size() - 4
                        : setName.size() - 1) : setName);
            auto loc = StaticCast<CHIR::Func*>(func)->GetPropLocation();
            auto begPos = Codira::Position(loc.GetFileID(), loc.GetBeginPos().line, loc.GetBeginPos().column);
            auto endPos = Codira::Position(loc.GetFileID(), loc.GetEndPos().line, loc.GetEndPos().column);
            if (!isGeneric && !isGenericInstance) {
                diagEngine->Diagnose(begPos, endPos, CodeCheckDiagKind::G_VAR_01_prefer_immutable_prop, propName);
            }
            if (isGeneric && genericDefSet.count(customTypeDef) == 0) {
                genericDefMap[customTypeDef] = std::make_pair(std::make_pair(begPos, endPos), propName);
            }
            continue;
        }
        if (isGenericInstance) {
            genericDefSet.insert(customTypeDef->GetGenericDecl());
        }
        for (auto& user : users) {
            varHasChanged.insert(user->GetOperand(1));
        }
    }
    if (isGenericInstance) {
        return;
    }
    if (customTypeDef->IsClassLike()) {
        auto classDef = StaticCast<CHIR::ClassDef*>(customTypeDef);
        CollectMemberVar(classDef, memberVarMap);
    } else {
        auto memberVars = customTypeDef->GetAllInstanceVars();
        CollectMemberVar(customTypeDef, memberVars, memberVarMap);
    }
}

void DataflowRuleVAR01Check::CheckAllTypesInPackage(CHIR::Package& package, ClassMemberVarsMap& memberVarMap)
{
    for (auto& classDef : package.GetClasses()) {
        CheckCustomTypeDef(classDef, memberVarMap);
    }
    for (auto& structDef : package.GetStructs()) {
        CheckCustomTypeDef(structDef, memberVarMap);
    }
    for (auto& enumDef : package.GetEnums()) {
        CheckCustomTypeDef(enumDef, memberVarMap);
    }
    for (auto& extendDef : package.GetExtends()) {
        CheckCustomTypeDef(extendDef, memberVarMap);
    }
}

static CHIR::Type* GetBaseTy(CHIR::Type* ty)
{
    return ty->IsRef() ? GetBaseTy(StaticCast<CHIR::RefType*>(ty)->GetBaseType()) : ty;
}

template <typename T> static bool IsCallLocalFunc(T* apply)
{
    auto callee = apply->GetCallee();
    if (callee->IsFuncWithBody()) {
        auto func = VirtualCast<CHIR::Func*>(callee);
        if (func->GetFuncKind() == Codira::CHIR::LAMBDA) {
            return true;
        }
    }
    return false;
}

bool DataflowRuleVAR01Check::CheckVarHelper(const CHIR::Value* var, int& count)
{
    auto users = var->GetUsers();
    if (varHasChanged.count(var) != 0) {
        return false;
    }
    auto hasChangedUser = [](auto& user) {
        if (user->GetExprKind() == CHIR::ExprKind::GET_ELEMENT_REF) {
            auto users = user->GetResult()->GetUsers();
            return std::find_if(users.begin(), users.end(),
                [](auto& subUser) { return subUser->GetExprKind() == CHIR::ExprKind::STORE; }) != users.end();
        }
        return user->GetExprKind() == CHIR::ExprKind::STORE;
    };
    count = std::count_if(users.begin(), users.end(), hasChangedUser);
    return true;
}

void DataflowRuleVAR01Check::CheckGlobalVar(const CHIR::GlobalVar* var)
{
    int count;
    /**
     *  For static member variables of generic classes, these need to be skipped
     *  '''
     *  class A<T> {
     *      static var a = 1
     *  }
     *  '''
     */
    auto isWithStore = CheckVarHelper(var, count);
    if (var->TestAttr(Attribute::STATIC)) {
        auto parentClassDef = var->GetParentCustomTypeDef();
        bool isGeneric = parentClassDef->TestAttr(Attribute::GENERIC);
        bool isGenericInstance = parentClassDef->TestAttr(Attribute::GENERIC_INSTANTIATED);
        if (isGeneric) {
            if (count == 1 && staticVarhasChanged.count(var) == 0) {
                staticVarInGeneric.insert(var);
            }
            return;
        }
        if (isGenericInstance && count > 1) {
            auto staticMemberVars = parentClassDef->GetGenericDecl()->GetStaticMemberVars();
            auto memberVarName = var->GetSrcCodeIdentifier();
            auto varInGeneric = std::find_if(staticMemberVars.begin(), staticMemberVars.end(),
                [&memberVarName](auto& member) { return member->GetSrcCodeIdentifier() == memberVarName; });
            if (varInGeneric != staticMemberVars.end()) {
                auto globalVarInGeneric = StaticCast<const CHIR::GlobalVar*>(*varInGeneric);
                staticVarhasChanged.insert(globalVarInGeneric);
                staticVarInGeneric.erase(globalVarInGeneric);
            }
            return;
        }
    }
    // The global or local variable assigned only once thus can be declared using 'let'
    auto loc = CommonFunc::GetCodePosition(var);
    if (isWithStore && count == 1) {
        diagEngine->Diagnose(
            loc.first, loc.second, CodeCheckDiagKind::G_VAR_01_prefer_immutable_var, var->GetSrcCodeIdentifier());
    }
}

void DataflowRuleVAR01Check::CheckLocalVar(const CHIR::LocalVar* var, LocalVarAssignCountMap& localVarAssignCountMap)
{
    int count;
    if (CheckVarHelper(var, count)) {
        localVarAssignCountMap[const_cast<CHIR::LocalVar*>(var)] = count;
    }
}

void DataflowRuleVAR01Check::CheckMemberVarHelper(
    Codira::CHIR::CustomType* customTy, std::vector<uint64_t>& path, ClassMemberVarsMap& memberVarMap)
{
    if (!customTy) {
        return;
    }
    auto customDef = customTy->GetCustomTypeDef();
    auto pathSize = path.size();
    for (uint64_t i = 0; i < pathSize - 1; i++) {
        uint64_t index = path[i];
        auto member = customDef->GetInstanceVar(index);
        customTy = StaticCast<CHIR::CustomType*>(member.type);
        customDef = customTy->GetCustomTypeDef();
    }
    customDef = customDef->TestAttr(Attribute::GENERIC_INSTANTIATED)
        ? const_cast<CHIR::CustomTypeDef*>(customDef->GetGenericDecl())
        : customDef;
    auto index = path[pathSize - 1];
    if (memberVarMap.find(customDef) != memberVarMap.end() &&
        memberVarMap[customDef].find(index) != memberVarMap[customDef].end()) {
        memberVarMap[customDef][index] += 1;
    }
}

void DataflowRuleVAR01Check::CheckMemberVar(const CHIR::Store* store, ClassMemberVarsMap& memberVarMap)
{
    TRY_GET_EXPR(store->GetLocation(), expr);
    if (expr->GetExprKind() == CHIR::ExprKind::GET_ELEMENT_REF) {
        auto getElementRef = StaticCast<CHIR::GetElementRef*>(expr);
        auto baseTy = GetBaseTy(getElementRef->GetLocation()->GetType());
        auto customTy = DynamicCast<CHIR::CustomType*>(baseTy);
        auto path = getElementRef->GetPath();
        CheckMemberVarHelper(customTy, path, memberVarMap);
    }
}

void DataflowRuleVAR01Check::CheckMemberVar(
    const CHIR::StoreElementRef* storeElementRef, ClassMemberVarsMap& memberVarMap)
{
    auto baseTy = GetBaseTy(storeElementRef->GetLocation()->GetType());
    auto customTy = DynamicCast<CHIR::CustomType*>(baseTy);
    auto path = storeElementRef->GetPath();
    CheckMemberVarHelper(customTy, path, memberVarMap);
}

void DataflowRuleVAR01Check::CheckBasedOnCHIR(CHIR::Package& package)
{
    // Check all properties and member variables
    ClassMemberVarsMap memberVarMap;
    CheckAllTypesInPackage(package, memberVarMap);
    for (auto& genericDef : genericDefMap) {
        diagEngine->Diagnose(genericDef.second.first.first, genericDef.second.first.second,
            CodeCheckDiagKind::G_VAR_01_prefer_immutable_prop, genericDef.second.second);
    }
    for (auto& globalVar : package.GetGlobalVars()) {
        if (globalVar->TestAttr(CHIR::Attribute::IMPORTED)) {
            continue;
        }
        if (!globalVar->TestAttr(Attribute::READONLY) && !IsExternalGlobalVar(globalVar)) {
            CheckGlobalVar(globalVar);
        }
    }
    for (auto& staticVar : staticVarInGeneric) {
        auto loc = CommonFunc::GetCodePosition(staticVar);
        diagEngine->Diagnose(
            loc.first, loc.second, CodeCheckDiagKind::G_VAR_01_prefer_immutable_var, staticVar->GetSrcCodeIdentifier());
    }
    for (auto& func : package.GetGlobalFuncs()) {
        if (CommonFunc::IsGenericInstantated(func) || func->TestAttr(CHIR::Attribute::IMPORTED)) {
            continue;
        }
        CheckBasedOnCHIRFunc(*func->GetBody(), memberVarMap);
    }
    // The member variable has not been modified in any instance, so it can be annotated with 'let'
    PrintDiagnoseInfo(memberVarMap);
}

void DataflowRuleVAR01Check::CheckBasedOnCHIRFunc(CHIR::BlockGroup& body, ClassMemberVarsMap& memberVarMap)
{
    LocalVarAssignCountMap localVarAssignCountMap;
    Visitor::Visit(body, [this, &localVarAssignCountMap, &memberVarMap](CHIR::Expression& expr) {
        if (expr.GetExprKind() == CHIR::ExprKind::LAMBDA) {
            CheckBasedOnCHIRFunc(*StaticCast<const CHIR::Lambda&>(expr).GetBody(), memberVarMap);
        }
        if (expr.GetExprKind() == CHIR::ExprKind::DEBUGEXPR) {
            auto debug = StaticCast<CHIR::Debug*>(&expr);
            auto result = debug->GetResult();
            auto value = debug->GetValue();
            auto pos = CommonFunc::GetCodePosition(value);
            if (!result->TestAttr(Attribute::READONLY) && !value->TestAttr(Attribute::READONLY) &&
                value->IsLocalVar() && !IsInUnSafeBlock(pos.first)) {
                auto localVar = StaticCast<CHIR::LocalVar*>(value);
                CheckLocalVar(localVar, localVarAssignCountMap);
            }
        }
        if (expr.GetExprKind() == CHIR::ExprKind::STORE) {
            auto store = StaticCast<CHIR::Store*>(&expr);
            CheckMemberVar(store, memberVarMap);
        }
        if (expr.GetExprKind() == CHIR::ExprKind::STORE_ELEMENT_REF) {
            auto storeElementRef = StaticCast<CHIR::StoreElementRef*>(&expr);
            CheckMemberVar(storeElementRef, memberVarMap);
        }
        return CHIR::VisitResult::CONTINUE;
    });
    PrintDiagnoseInfo(localVarAssignCountMap);
}

void DataflowRuleVAR01Check::PrintDiagnoseInfo(const LocalVarAssignCountMap& localVarAssignCountMap)
{
    for (const auto& it : localVarAssignCountMap) {
        if (it.second <= 1) {
            auto var = it.first;
            auto pos = CommonFunc::GetCodePosition(var);
            if (CommonFunc::IsStdDerivedMacro(diagEngine, pos.first)) {
                continue;
            }
            diagEngine->Diagnose(
                pos.first, pos.second, CodeCheckDiagKind::G_VAR_01_prefer_immutable_var, var->GetSrcCodeIdentifier());
        }
    }
}

void DataflowRuleVAR01Check::PrintDiagnoseInfo(const ClassMemberVarsMap& memberVarMap)
{
    for (const auto& it : memberVarMap) {
        for (auto i : it.second) {
            if (i.second <= 1) {
                auto memberVar = it.first->GetInstanceVar(i.first);
                auto loc = memberVar.loc;
                auto begPos = Codira::Position(loc.GetFileID(), loc.GetBeginPos().line, loc.GetBeginPos().column);
                auto endPos = Codira::Position(loc.GetFileID(), loc.GetEndPos().line, loc.GetEndPos().column);
                if (CommonFunc::IsStdDerivedMacro(diagEngine, begPos)) {
                    continue;
                }
                diagEngine->Diagnose(
                    begPos, endPos, CodeCheckDiagKind::G_VAR_01_prefer_immutable_member, memberVar.name);
            }
        }
    }
}

void DataflowRuleVAR01Check::FindAllUnSafeBlock(AST::Package& package)
{
    auto preVisit = [this](Ptr<AST::Node> node) -> AST::VisitAction {
        if (node->astKind == AST::ASTKind::BLOCK && node->TestAttr(AST::Attribute::UNSAFE)) {
            unsafeBlock.insert(std::make_pair(node->begin, node->end));
            return AST::VisitAction::SKIP_CHILDREN;
        }
        return AST::VisitAction::WALK_CHILDREN;
    };
    AST::Walker walker(&package, preVisit);
    walker.Walk();
}

bool DataflowRuleVAR01Check::IsInUnSafeBlock(Codira::Position pos)
{
    for (auto blockRange : unsafeBlock) {
        if (pos.fileID != blockRange.first.fileID) {
            continue;
        }
        auto begin = blockRange.first;
        auto end = blockRange.second;
        auto inUnSafeBlock = (pos.line > begin.line && pos.line < end.line) ||
            (pos.line == begin.line && pos.column > begin.column) || (pos.line == end.line && pos.column < end.column);
        if (inUnSafeBlock) {
            return true;
        }
    }
    return false;
}

void DataflowRuleVAR01Check::DoAnalysis(CODELintCompilerInstance* instance)
{
    auto packageList = instance->GetSourcePackages();
    for (auto package : packageList) {
        FindAllUnSafeBlock(*package);
    }

    // Traverse and inspect CHIR packages
    analysisWrapper = const_cast<AnalysisWrapper*>(&instance->GetConstAnalysisWrapper());
    for (auto chirPkg : instance->GetAllCHIRPackages()) {
        CheckBasedOnCHIR(*chirPkg);
    }
}

} // namespace Codira::CodeCheck
