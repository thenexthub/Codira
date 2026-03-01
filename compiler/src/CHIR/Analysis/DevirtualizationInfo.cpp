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
 * This file implements the de-virtualization optimization.
 */

#include "Codira/CHIR/Analysis/DevirtualizationInfo.h"

#include "Codira/CHIR/CHIRContext.h"
#include "Codira/CHIR/Type/ExtendDef.h"
#include "Codira/CHIR/Utils.h"
#include "Codira/Utils/Casting.h"
#include "Codira/Modules/ModulesUtils.h"

namespace Codira::CHIR {

void DevirtualizationInfo::CollectInfo()
{
    // Function that collect all global function which has more concrete type than
    // explicit return type in order to infer devirtualization more precise.
    for (auto func : package->GetGlobalFuncs()) {
        CollectReturnTypeMap(*func);
    }

    // Collecting inheritance relationships between type definitions.
    for (const auto customTypeDef : package->GetAllCustomTypeDef()) {
        if (customTypeDef->TestAttr(Attribute::SKIP_ANALYSIS)) {
            continue;
        }
        if (customTypeDef->IsClassLike() && IsClosureConversionEnvClass(*StaticCast<const ClassDef*>(customTypeDef))) {
            continue;
        }

        Type* thisType = customTypeDef->GetType();
        if (customTypeDef->IsExtend()) {
            thisType = StaticCast<ExtendDef*>(customTypeDef)->GetExtendedType();
            if (auto customTy = DynamicCast<CustomType*>(thisType)) {
                thisType = customTy->GetCustomTypeDef()->GetType();
            }
        }

        defsMap[thisType].emplace_back(customTypeDef);
        for (auto parentTy : customTypeDef->GetSuperTypesInCurDef()) {
            auto parentDef = parentTy->GetClassDef();
            if (IsCoreObject(*parentDef)) {
                continue;
            }
            if (CheckCustomTypeInternal(*parentDef)) {
                subtypeMap[parentDef].emplace_back(InheritanceInfo{parentTy, thisType});
 
            }
        }
    }
}

const DevirtualizationInfo::SubTypeMap& DevirtualizationInfo::GetSubtypeMap() const
{
    return subtypeMap;
}

const std::unordered_map<Func*, Type*>& DevirtualizationInfo::GetReturnTypeMap() const
{
    return realRuntimeRetTyMap;
}

const ConstMemberVarCollector::ConstMemberMapType& DevirtualizationInfo::GetConstMemberMap() const
{
    return constMemberTypeMap;
}

static Type* GetRuntimeTypeFromFunc(const Value* retVal, bool isInLambda);

static Type* GetRuntimeTypeFromLambda(const Apply& apply, Type* type)
{
    if (!apply.GetCallee()->IsLocalVar()) {
        return type;
    }
    auto localVar = StaticCast<LocalVar*>(apply.GetCallee());
    if (localVar->GetExpr()->GetExprKind() != ExprKind::LAMBDA) {
        return type;
    }
    auto lambda = StaticCast<Lambda*>(localVar->GetExpr());
    return GetRuntimeTypeFromFunc(lambda->GetReturnValue(), true);
}

static Type* GetRuntimeTypeFromFunc(const Value* retVal, bool isInLambda = false)
{
    if (!retVal) {
        return nullptr;
    }
    auto rtTy = retVal->GetType()->StripAllRefs();
    // Do not process the return type that is not class type.
    if (!rtTy->IsClass()) {
        return nullptr;
    }
    auto users = retVal->GetUsers();
    if (users.size() == 1 && users[0]->GetExprKind() == ExprKind::STORE) {
        auto val = StaticCast<Store*>(users[0])->GetValue();
        if (!val->IsLocalVar()) {
            return nullptr;
        }
        auto expr = StaticCast<LocalVar*>(val)->GetExpr();
        auto srcTy = val->GetType();
        if (expr->GetExprKind() == ExprKind::TYPECAST) {
            auto cast = StaticCast<TypeCast*>(expr);
            srcTy = cast->GetSourceTy();
        } else if (expr->GetExprKind() == ExprKind::APPLY && !isInLambda) {
            auto applyResType = GetRuntimeTypeFromLambda(*StaticCast<Apply*>(expr), srcTy);
            srcTy = applyResType != nullptr ? applyResType : srcTy;
        }
        srcTy = srcTy->StripAllRefs();
        if (srcTy == rtTy) {
            // if type is same, skip
            return nullptr;
        }
        // Ensure that the actual return value type is not a reference.
        return srcTy;
    }
    return nullptr;
}

// Function that collect all global function which has more concrete runtime type
// than explicit return type in order to infer devirtualization more precise.
void DevirtualizationInfo::CollectReturnTypeMap(Func& func)
{
    auto res = GetRuntimeTypeFromFunc(func.GetReturnValue());
    if (res != nullptr) {
        realRuntimeRetTyMap.emplace(&func, res);
    }
}

// after inline, the result type is more accuracy than before, refresh it
void DevirtualizationInfo::FreshRetMap()
{
    for (auto func : package->GetGlobalFuncs()) {
        auto res = GetRuntimeTypeFromFunc(func->GetReturnValue());
        if (res != nullptr) {
            realRuntimeRetTyMap[func] = res;
        }
    }
}

bool DevirtualizationInfo::CheckCustomTypeInternal(const CustomTypeDef& def) const
{
    auto relation = Modules::GetPackageRelation(def.GetPackageName(), package->GetName());
    if (def.TestAttr(Attribute::PUBLIC) || def.TestAttr(Attribute::PROTECTED)) {
        return false;
    }
    if (def.TestAttr(Attribute::PRIVATE)) {
        return true;
    }
    if (def.TestAttr(Attribute::INTERNAL)) {
        if (relation != Modules::PackageRelation::CHILD && relation != Modules::PackageRelation::SAME_PACKAGE) {
            return true;
        }
        if (opts.noSubPkg) {
            return true;
        }
        return false;
    }
    return false;
}

void DevirtualizationInfo::CollectConstMemberVarType()
{
    ConstMemberVarCollector{package, constMemberTypeMap}.CollectConstMemberVarType();
}

}  // namespace Codira::CHIR
