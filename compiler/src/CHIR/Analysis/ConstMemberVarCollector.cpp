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
 * This file includes de-virtualization information collector for const member.
 */

#include "Codira/CHIR/Analysis/ConstMemberVarCollector.h"

#include "Codira/CHIR/Utils.h"
#include "Codira/CHIR/Visitor/Visitor.h"
#include "Codira/Utils/Casting.h"

namespace Codira::CHIR {

void ConstMemberVarCollector::CollectConstMemberVarType()
{
    // only collect non-static member since static var is treated as global in CHIR
    auto localCustomType = package->GetCurPkgCustomTypeDef();
    for (auto& def : localCustomType) {
        if (def->IsExtend()) {
            // skip extend
            continue;
        }
        // because parent maybe in other package, it's initialization may not be seen in this package,
        // so only analyse direct class member.
        auto members = def->GetDirectInstanceVars();
        std::unordered_map<size_t, MemberInfo> index2Type;
        size_t indexBase = def->GetAllInstanceVarNum() - def->GetDirectInstanceVarNum();
        for (size_t i = 0; i < members.size(); i++) {
            auto& member = members[i];
            if (!member.IsImmutable()) {
                // skip variable member
                continue;
            }
            auto memberType = member.type->StripAllRefs();
            if (!memberType->IsCustomType()) {
                continue;
            }
            auto customDef = StaticCast<CustomType*>(memberType)->GetCustomTypeDef();
            if (!customDef->IsClassLike() || !customDef->CanBeInherited()) {
                // only analyse virtual member
                continue;
            }
            index2Type.emplace(indexBase + i, MemberInfo(StaticCast<CustomType*>(memberType)));
        }
        JudgeIfOnlyDerivedType(*def, index2Type);
        if (index2Type.empty()) {
            continue;
        }
        std::unordered_map<size_t, Type*> res;
        for (auto& it : index2Type) {
            if (it.second.derivedType == nullptr) {
                continue;
            }
            res.emplace(it.first, it.second.derivedType);
        }
        if (!res.empty()) {
            constMemberMap.emplace(def, res);
        }
    }
}

void ConstMemberVarCollector::JudgeIfOnlyDerivedType(
    const CustomTypeDef& def, std::unordered_map<size_t, MemberInfo>& index2Type)
{
    Parameter* param = nullptr;
    auto preVisit = [this, &param, &index2Type](Expression& expr) {
        if (expr.GetExprKind() != ExprKind::STORE_ELEMENT_REF) {
            return VisitResult::CONTINUE;
        }
        auto stf = StaticCast<StoreElementRef*>(&expr);
        HandleStoreElementRef(stf, param, index2Type);
        if (index2Type.empty()) {
            return VisitResult::STOP;
        }
        return VisitResult::CONTINUE;
    };
    auto methods = def.GetMethods();
    for (auto& method : methods) {
        if (!method->IsConstructor()) {
            continue;
        }
        // only analyse constructor.
        auto func = StaticCast<Func*>(method);
        param = func->GetParam(0);
        Visitor::Visit(*func, preVisit, []([[maybe_unused]] Expression& e) { return VisitResult::CONTINUE; });
    }
}

void ConstMemberVarCollector::HandleStoreElementRef(
    const StoreElementRef* stf, const Value* firstParam, std::unordered_map<size_t, MemberInfo>& index2Type) const
{
    if (stf->GetPath().size() != 1U) {
        // all members are in one path
        return;
    }
    auto index = stf->GetPath()[0];
    if (index2Type.find(index) == index2Type.end()) {
        return;
    }
    auto location = stf->GetLocation();
    if (location != firstParam) {
        // not store to this value, skip.
        return;
    }
    auto sourceValue = GetSourceTargetRecursively(stf->GetValue());
    auto sourceType = sourceValue->GetType()->StripAllRefs();
    auto& info = index2Type[index];
    if (sourceType->IsCustomType() && StaticCast<CustomType*>(sourceType)->GetCustomTypeDef()->CanBeInherited()) {
        // if source type can be inherited, we may get its child type, ending.
        index2Type.erase(index);
        return;
    }
    if (info.derivedType == nullptr) {
        // first meet an initialization to this virtual member.
        info.derivedType = sourceType;
    } else if (info.derivedType != sourceType) {
        // get different member type, ending
        index2Type.erase(index);
    }
}

const Value* ConstMemberVarCollector::GetSourceTargetRecursively(const Value* value)
{
    if (!value->IsLocalVar()) {
        return value;
    }
    auto iter = StaticCast<LocalVar*>(value);
    while (true) {
        auto source = GetCastOriginalTarget(*iter->GetExpr());
        if (source == nullptr) {
            return iter;
        }
        if (source->IsParameter()) {
            return source;
        }
        if (source->IsLocalVar()) {
            iter = StaticCast<LocalVar*>(source);
        } else {
            return source;
        }
    }
}

}  // namespace Codira::CHIR
