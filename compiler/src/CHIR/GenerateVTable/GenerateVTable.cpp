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

#include "Codira/CHIR/GenerateVTable/GenerateVTable.h"

#include "Codira/CHIR/CHIRCasting.h"
#include "Codira/CHIR/GenerateVTable/UpdateOperatorVTable.h"
#include "Codira/CHIR/GenerateVTable/VTableGenerator.h"
#include "Codira/CHIR/GenerateVTable/WrapMutFunc.h"
#include "Codira/CHIR/GenerateVTable/WrapVirtualFunc.h"
#include "Codira/CHIR/Utils.h"
#include "Codira/CHIR/Visitor/Visitor.h"
#include "Codira/Mangle/CHIRManglingUtils.h"
#include "Codira/Utils/ProfileRecorder.h"

using namespace Codira;
using namespace Codira::CHIR;

namespace {
bool CalleeIsMutFuncFromParent(Type* thisType, FuncBase* callee, const Func& topLevelFunc)
{
    // thisType must be Struct
    if (thisType == nullptr || !thisType->StripAllRefs()->IsStruct()) {
        return false;
    }
    // callee must be mut func
    if (callee == nullptr || !callee->TestAttr(Attribute::MUT)) {
        return false;
    }
    // callee's parent must be from interface
    if (!callee->GetParentCustomTypeOrExtendedType()->IsClass()) {
        return false;
    }
    // current Apply is not in wrapper func
    return topLevelFunc.Get<WrappedRawMethod>() == nullptr;
}
} // namespace

GenerateVTable::GenerateVTable(Package& pkg, CHIRBuilder& b, const Codira::GlobalOptions& opts)
    : package(pkg), builder(b), opts(opts)
{
}

void GenerateVTable::CreateVTable()
{
    Utils::ProfileRecorder recorder("GenerateVTable", "CreateVTable");
    auto vtableGenerator = VTableGenerator(builder);
    for (auto customDef : package.GetAllCustomTypeDef()) {
        if (customDef->TestAttr(Attribute::SKIP_ANALYSIS)) {
            continue;
        }
        vtableGenerator.GenerateVTable(*customDef);
    }
}

void GenerateVTable::UpdateOperatorVirFunc()
{
    UpdateOperatorVTable(package, builder).Update();
}

void GenerateVTable::CreateVirtualFuncWrapper(const IncreKind& kind, const CompilationCache& increCachedInfo,
    VirtualWrapperDepMap& curVirtFuncWrapDep, VirtualWrapperDepMap& delVirtFuncWrapForIncr)
{
    Utils::ProfileRecorder recorder("GenerateVTable", "CreateVirtualFuncWrapper");
    bool targetIsWin = opts.target.os == Triple::OSType::WINDOWS;
    IncreKind tempKind = opts.enIncrementalCompilation ? kind : IncreKind::INVALID;
    auto wrapper = WrapVirtualFunc(builder, increCachedInfo, tempKind, targetIsWin);
    for (auto customDef : package.GetAllCustomTypeDef()) {
        if (customDef->TestAttr(Attribute::SKIP_ANALYSIS)) {
            continue;
        }
        wrapper.CheckAndWrap(*customDef);
    }
    curVirtFuncWrapDep = wrapper.GetCurVirtFuncWrapDep();
    delVirtFuncWrapForIncr = wrapper.GetDelVirtFuncWrapForIncr();
}

void GenerateVTable::CreateMutFuncWrapper()
{
    Utils::ProfileRecorder recorder("GenerateVTable", "CreateMutFuncWrapper");
    auto wrapper = WrapMutFunc(builder);
    for (auto customDef : package.GetAllCustomTypeDef()) {
        if (customDef->TestAttr(Attribute::SKIP_ANALYSIS)) {
            continue;
        }
        wrapper.Run(*customDef);
    }
    mutFuncWrappers = wrapper.GetWrappers();
}

FuncBase* GenerateVTable::GetMutFuncWrapper(const Type& thisType, const std::vector<Value*>& args,
    const std::vector<Type*>& instTypeArgs, Type& retType, const FuncBase& callee)
{
    std::vector<Type*> paramTypes;
    for (auto arg : args) {
        paramTypes.emplace_back(arg->GetType());
    }
    auto funcCallType = FuncCallType {
        .funcName = callee.GetSrcCodeIdentifier(),
        .funcType = builder.GetType<FuncType>(paramTypes, &retType),
        .genericTypeArgs = instTypeArgs
    };
    auto vtableRes = GetFuncIndexInVTable(
        *thisType.StripAllRefs(), funcCallType, callee.TestAttr(Attribute::STATIC), builder);
    CODEC_ASSERT(vtableRes.size() == 1);
    auto wrapperName = CHIRMangling::GenerateVirtualFuncMangleName(
        &callee, *vtableRes[0].originalDef, vtableRes[0].halfInstSrcParentType, false);
    auto it = mutFuncWrappers.find(wrapperName);
    CODEC_ASSERT(it != mutFuncWrappers.end());
    return it->second;
}

void GenerateVTable::UpdateFuncCall()
{
    Utils::ProfileRecorder recorder("GenerateVTable", "UpdateFuncCall");
    std::vector<Apply*> applys;
    std::vector<ApplyWithException*> applyEs;
    std::function<VisitResult(Expression&)> preVisit = [this, &preVisit, &applys, &applyEs](Expression& e) {
        if (auto lambda = DynamicCast<Lambda*>(&e)) {
            Visitor::Visit(*lambda->GetBody(), preVisit);
        } else if (auto dyExpr = DynamicCast<DynamicDispatch*>(&e)) {
            e.Set<VirMethodOffset>(dyExpr->GetVirtualMethodOffset(&builder));
        } else if (auto dyExprE = DynamicCast<DynamicDispatchWithException*>(&e)) {
            e.Set<VirMethodOffset>(dyExprE->GetVirtualMethodOffset(&builder));
        } else if (auto apply = DynamicCast<Apply*>(&e)) {
            auto callee = DynamicCast<FuncBase*>(apply->GetCallee());
            if (CalleeIsMutFuncFromParent(apply->GetThisType(), callee, *e.GetTopLevelFunc())) {
                applys.emplace_back(apply);
            }
        } else if (auto applyE = DynamicCast<ApplyWithException*>(&e)) {
            auto callee = DynamicCast<FuncBase*>(applyE->GetCallee());
            if (CalleeIsMutFuncFromParent(applyE->GetThisType(), callee, *e.GetTopLevelFunc())) {
                applyEs.emplace_back(applyE);
            }
        }
        return VisitResult::CONTINUE;
    };
    for (auto func : package.GetGlobalFuncs()) {
        Visitor::Visit(*func, preVisit);
    }
    for (auto apply : applys) {
        auto callee = VirtualCast<FuncBase*>(apply->GetCallee());
        auto wrapperFunc = GetMutFuncWrapper(*apply->GetThisType(), apply->GetArgs(),
            apply->GetInstantiatedTypeArgs(), *apply->GetResult()->GetType(), *callee);
        apply->ReplaceOperand(callee, wrapperFunc);
    }
    for (auto apply : applyEs) {
        auto callee = VirtualCast<FuncBase*>(apply->GetCallee());
        auto wrapperFunc = GetMutFuncWrapper(*apply->GetThisType(), apply->GetArgs(),
            apply->GetInstantiatedTypeArgs(), *apply->GetResult()->GetType(), *callee);
        apply->ReplaceOperand(callee, wrapperFunc);
    }
}
