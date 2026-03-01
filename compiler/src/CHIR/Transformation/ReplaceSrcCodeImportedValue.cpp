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

#include "Codira/CHIR/Analysis/Utils.h"
#include "Codira/CHIR/CHIR.h"
#include "Codira/CHIR/Expression/Terminator.h"
#include "Codira/CHIR/Package.h"
#include "Codira/CHIR/Visitor/Visitor.h"
#include "Codira/CHIR/CHIRCasting.h"
#include "Codira/Utils/CheckUtils.h"
#include "Codira/Utils/ProfileRecorder.h"

namespace Codira::CHIR {
namespace {
void ReplaceCustomTypeDefVtable(CustomTypeDef& def, const std::unordered_map<Value*, Value*>& symbol)
{
    auto vtable = def.GetVTable();
    for (auto& vtableIt : vtable) {
        for (size_t i = 0; i < vtableIt.second.size(); ++i) {
            auto res = symbol.find(vtableIt.second[i].instance);
            if (res != symbol.end()) {
                vtableIt.second[i].instance = VirtualCast<FuncBase*>(res->second);
            }
        }
    }
    def.SetVTable(vtable);
}

void ReplaceCustomTypeDefAndExtendVtable(CustomTypeDef& def, const std::unordered_map<Value*, Value*>& symbol)
{
    ReplaceCustomTypeDefVtable(def, symbol);
    for (auto exDef : def.GetExtends()) {
        ReplaceCustomTypeDefVtable(*exDef, symbol);
    }
}

void ReplaceParentAndSubClassVtable(CustomTypeDef& def, const std::unordered_map<Value*, Value*>& symbol,
    const std::unordered_map<ClassDef*, std::unordered_set<CustomTypeDef*>>& subClasses)
{
    // replace self vtable
    ReplaceCustomTypeDefAndExtendVtable(def, symbol);

    if (!def.IsClassLike()) {
        return;
    }
    auto& classDef = StaticCast<ClassDef&>(def);
    auto it = subClasses.find(&classDef);
    if (it == subClasses.end()) {
        return;
    }
    // replace sub class vtable
    for (auto subClass : it->second) {
        ReplaceCustomTypeDefAndExtendVtable(*subClass, symbol);
    }
}

void ReplaceMethodAndStaticVar(
    const std::unordered_map<CustomTypeDef*, std::unordered_map<Value*, Value*>>& replaceTable,
    const std::unordered_map<ClassDef*, std::unordered_set<CustomTypeDef*>>& subClasses)
{
    for (auto& it : replaceTable) {
        auto methods = it.first->GetMethods();
        for (size_t i = 0; i < methods.size(); ++i) {
            auto res = it.second.find(methods[i]);
            if (res != it.second.end()) {
                methods[i] = VirtualCast<FuncBase*>(res->second);
            }
        }
        it.first->SetMethods(methods);
        ReplaceParentAndSubClassVtable(*it.first, it.second, subClasses);

        auto staticVars = it.first->GetStaticMemberVars();
        for (size_t i = 0; i < staticVars.size(); ++i) {
            auto res = it.second.find(staticVars[i]);
            if (res != it.second.end()) {
                staticVars[i] = VirtualCast<GlobalVarBase*>(res->second);
            }
        }
        it.first->SetStaticMemberVars(staticVars);

        for (auto& it2 : it.second) {
            if (auto func = DynamicCast<Func*>(it2.first)) {
                func->DestroySelf();
            } else {
                VirtualCast<GlobalVarBase*>(it2.first)->DestroySelf();
            }
        }
    }
}

bool IsEmptyInitFunc(Func& func)
{
    if (func.GetFuncKind() != CHIR::FuncKind::GLOBALVAR_INIT) {
        return false;
    }
    bool isEmpty = true;
    auto preVisit = [&isEmpty](Expression& e) {
        if (e.GetExprKind() != CHIR::ExprKind::EXIT && e.GetExprKind() != CHIR::ExprKind::RAISE_EXCEPTION) {
            isEmpty = false;
        }
        return VisitResult::CONTINUE;
    };
    Visitor::Visit(func, preVisit);
    return isEmpty;
}

std::unordered_map<ClassDef*, std::unordered_set<CustomTypeDef*>> CollectSubClasses(
    const Package& pkg, CHIRBuilder& builder)
{
    //                 parent     sub
    std::unordered_map<ClassDef*, std::unordered_set<CustomTypeDef*>> subClasses;
    for (auto def : pkg.GetAllCustomTypeDef()) {
        for (auto parentType : def->GetSuperTypesRecusively(builder)) {
            subClasses[parentType->GetClassDef()].emplace(def);
        }
    }
    return subClasses;
}

void CreateSrcImportedFuncSymbol(
    CHIRBuilder& builder, Func& fn, std::unordered_map<Func*, ImportedFunc*>& srcCodeImportedFuncMap)
{
    auto genericParamTy = fn.GetGenericTypeParams();
    auto pkgName = fn.GetPackageName();
    auto funcTy = fn.GetType();
    auto mangledName = fn.GetIdentifierWithoutPrefix();
    auto srcCodeName = fn.GetSrcCodeIdentifier();
    auto rawMangledName = fn.GetRawMangledName();
    auto importedFunc = builder.CreateImportedVarOrFunc<ImportedFunc>(
        funcTy, mangledName, srcCodeName, rawMangledName, pkgName, genericParamTy);
    importedFunc->AppendAttributeInfo(fn.GetAttributeInfo());
    importedFunc->SetFuncKind(fn.GetFuncKind());
    if (auto hostFunc = fn.GetParamDftValHostFunc()) {
        auto it = srcCodeImportedFuncMap.find(StaticCast<Func*>(hostFunc));
        CODEC_ASSERT(it != srcCodeImportedFuncMap.end());
        importedFunc->SetParamDftValHostFunc(*it->second);
    }
    importedFunc->SetFastNative(fn.IsFastNative());
    importedFunc->Set<LinkTypeInfo>(Linkage::EXTERNAL);
    srcCodeImportedFuncMap.emplace(&fn, importedFunc);
}

void CreateSrcImportedVarSymbol(
    CHIRBuilder& builder, Value& gv, std::unordered_map<GlobalVar*, ImportedVar*>& srcCodeImportedVarMap)
{
    auto globalVar = VirtualCast<GlobalVar*>(&gv);
    auto mangledName = globalVar->GetIdentifierWithoutPrefix();
    auto srcCodeName = globalVar->GetSrcCodeIdentifier();
    auto rawMangledName = globalVar->GetRawMangledName();
    auto packageName = globalVar->GetPackageName();
    auto ty = globalVar->GetType();
    auto importedVar =
        builder.CreateImportedVarOrFunc<ImportedVar>(ty, mangledName, srcCodeName, rawMangledName, packageName);
    importedVar->AppendAttributeInfo(globalVar->GetAttributeInfo());
    importedVar->Set<LinkTypeInfo>(gv.Get<LinkTypeInfo>());
    srcCodeImportedVarMap.emplace(globalVar, importedVar);
}

void CreateSrcImpotedValueSymbol(const std::unordered_set<Func*>& srcCodeImportedFuncs,
    const std::unordered_set<GlobalVar*>& srcCodeImportedVars, CHIRBuilder& builder,
    std::unordered_map<Func*, ImportedFunc*>& srcCodeImportedFuncMap,
    std::unordered_map<GlobalVar*, ImportedVar*>& srcCodeImportedVarMap)
{
    for (auto func : builder.GetCurPackage()->GetGlobalFuncs()) {
        CODEC_NULLPTR_CHECK(func);
        if (srcCodeImportedFuncs.find(func) != srcCodeImportedFuncs.end()) {
            CreateSrcImportedFuncSymbol(builder, *func, srcCodeImportedFuncMap);
        }
    }
    for (auto gv : builder.GetCurPackage()->GetGlobalVars()) {
        CODEC_NULLPTR_CHECK(gv);
        if (srcCodeImportedVars.find(gv) != srcCodeImportedVars.end()) {
            CreateSrcImportedVarSymbol(builder, *gv, srcCodeImportedVarMap);
        }
    }
}
}

void ToCHIR::ReplaceSrcCodeImportedValueWithSymbol()
{
    std::unordered_set<Func*> toBeRemovedFuncs;
    std::unordered_set<GlobalVar*> toBeRemovedVars;
    std::unordered_map<Func*, ImportedFunc*> srcCodeImportedFuncMap;
    std::unordered_map<GlobalVar*, ImportedVar*> srcCodeImportedVarMap;
    CreateSrcImpotedValueSymbol(
        srcCodeImportedFuncs, srcCodeImportedVars, builder, srcCodeImportedFuncMap, srcCodeImportedVarMap);
    for (auto lambda : uselessLambda) {
        for (auto user : lambda->GetUsers()) {
            user->RemoveSelfFromBlock();
        }
        lambda->DestroySelf();
        toBeRemovedFuncs.emplace(lambda);
    }

    for (auto def : uselessClasses) {
        for (auto func : def->GetMethods()) {
            for (auto user : func->GetUsers()) {
                user->RemoveSelfFromBlock();
            }
            auto funcWithBody = StaticCast<Func*>(func);
            funcWithBody->DestroySelf();
            toBeRemovedFuncs.emplace(funcWithBody);
        }
    }
    std::vector<ClassDef*> newClasses;
    auto classes = chirPkg->GetClasses();
    for (auto def : classes) {
        if (uselessClasses.find(def) == uselessClasses.end()) {
            newClasses.emplace_back(def);
        }
    }
    chirPkg->SetClasses(std::move(newClasses));

    std::unordered_map<CustomTypeDef*, std::unordered_map<Value*, Value*>> replaceTable;
    for (auto& it : srcCodeImportedFuncMap) {
        auto funcWithBody = it.first;
        auto importedSymbol = it.second;
        // Attributes may be added in the chir phase. For example, 'final' is added when a virtual table is created. In
        // this case, you need to append the attributes again.
        importedSymbol->AppendAttributeInfo(funcWithBody->GetAttributeInfo());
        for (auto user : funcWithBody->GetUsers()) {
            user->ReplaceOperand(funcWithBody, importedSymbol);
        }
        if (auto parentDef = funcWithBody->GetParentCustomTypeDef()) {
            replaceTable[parentDef][funcWithBody] = importedSymbol;
        }
        toBeRemovedFuncs.emplace(funcWithBody);
        auto implicitIt = implicitFuncs.find(funcWithBody->GetIdentifierWithoutPrefix());
        if (implicitIt != implicitFuncs.end()) {
            implicitIt->second = importedSymbol;
        }
    }
    for (auto& it : srcCodeImportedVarMap) {
        auto varWithInit = it.first;
        auto importedSymbol = it.second;
        // Attributes may be added in the chir phase. For example, 'final' is added when a virtual table is created. In
        // this case, you need to append the attributes again.
        importedSymbol->AppendAttributeInfo(varWithInit->GetAttributeInfo());
        if (auto initFunc = varWithInit->GetInitFunc()) {
            for (auto user : initFunc->GetUsers()) {
                user->RemoveSelfFromBlock();
            }
            initFunc->DestroySelf();
            toBeRemovedFuncs.emplace(initFunc);
        }
        for (auto user : varWithInit->GetUsers()) {
            user->ReplaceOperand(varWithInit, importedSymbol);
        }
        if (auto parentDef = varWithInit->GetParentCustomTypeDef()) {
            replaceTable[parentDef][varWithInit] = importedSymbol;
        }
        toBeRemovedVars.emplace(varWithInit);
    }
    auto subClasses = CollectSubClasses(*chirPkg, builder);
    ReplaceMethodAndStaticVar(replaceTable, subClasses);

    std::vector<Func*> globalFuncs;
    for (auto func : chirPkg->GetGlobalFuncs()) {
        if (toBeRemovedFuncs.find(func) != toBeRemovedFuncs.end()) {
            continue;
        } else if (IsEmptyInitFunc(*func)) {
            for (auto user : func->GetUsers()) {
                user->RemoveSelfFromBlock();
            }
            func->DestroySelf();
            continue;
        }
        globalFuncs.emplace_back(func);
    }
    chirPkg->SetGlobalFuncs(globalFuncs);

    std::vector<GlobalVar*> globalVars;
    for (auto var : chirPkg->GetGlobalVars()) {
        if (toBeRemovedVars.find(var) == toBeRemovedVars.end()) {
            globalVars.emplace_back(var);
        }
    }
    chirPkg->SetGlobalVars(std::move(globalVars));
}
}  // namespace Codira::CHIR
