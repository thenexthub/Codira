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

#ifndef CODIRA_CHIR_ANALYSIS_CONST_MEMBER_VAR_COLLECTOR_H
#define CODIRA_CHIR_ANALYSIS_CONST_MEMBER_VAR_COLLECTOR_H

#include <unordered_map>

#include "Codira/CHIR/Package.h"
#include "Codira/CHIR/Expression/Expression.h"

namespace Codira::CHIR {
class ConstMemberVarCollector {
public:
    using ConstMemberMapType = std::unordered_map<const CustomTypeDef*, std::unordered_map<size_t, Type*>>;

    explicit ConstMemberVarCollector(const Package* package,
        ConstMemberMapType& constMemberMap) : package(package), constMemberMap(constMemberMap)
    {
    }

    /// mark member info containing its original type and its derived class
    struct MemberInfo {
        MemberInfo() = default;
        explicit MemberInfo(Type* orig) : oriType(orig)
        {
        }
        Type* oriType = nullptr;
        Type* derivedType = nullptr;
    };

    /// collect memher which can be using for devirtualization pass.
    void CollectConstMemberVarType();

    /// judge if a member is declared as base type, and only init as one devrived type.
    void JudgeIfOnlyDerivedType(const CustomTypeDef& def, std::unordered_map<size_t, MemberInfo>& index2Type);

    /// handle StoreElementRef expression in CHIR IR.
    void HandleStoreElementRef(
        const StoreElementRef* stf, const Value* firstParam, std::unordered_map<size_t, MemberInfo>& index2Type) const;

    /// get source for a location.
    static const Value* GetSourceTargetRecursively(const Value* value);

private:
    const Package* package = nullptr;
    ConstMemberMapType& constMemberMap;
};

}  // namespace Codira::CHIR

#endif
