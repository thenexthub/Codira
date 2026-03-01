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
 * This file includes de-virtualization type analysis.
 */

#ifndef CODIRA_CHIR_ANALYSIS_DEVIRTUALIZATION_INFO_H
#define CODIRA_CHIR_ANALYSIS_DEVIRTUALIZATION_INFO_H

#include <unordered_map>

#include "Codira/CHIR/Analysis/ConstMemberVarCollector.h"
#include "Codira/CHIR/Package.h"
#include "Codira/CHIR/Type/ClassDef.h"
#include "Codira/CHIR/Type/Type.h"
#include "Codira/CHIR/Value.h"
#include "Codira/Option/Option.h"

namespace Codira::CHIR {

/**
 * @brief type kind for devirtualization pass.
 */
enum class DevirtualTyKind : uint8_t {
    SUBTYPE_OF, // Means a type who is the sub-class or sub-interface of another type.
    EXACTLY,    // Means a type exactly.
};

/**
 * @brief collect info for devirtualization pass, such as return map, subtype map.
 */
class DevirtualizationInfo {
public:
    DevirtualizationInfo() = delete;

    /// constructor of info collector for devirtualization pass.
    explicit DevirtualizationInfo(const Package* package, const GlobalOptions& opts)
        : package(package), opts(opts)
    {
    }

    /**
     * @brief main method to collect devirtualization info.
     */
    void CollectInfo();

    /**
     * @brief re-collect ret map after other optimization pass.
     */
    void FreshRetMap();

    /**
     * @brief check custom type is internal.
     * @param def custom type to check.
     * @return flag whether is internal custom type.
     */
    bool CheckCustomTypeInternal(const CustomTypeDef& def) const;

    /**
     * @brief collect const members to devirt.
     */
    void CollectConstMemberVarType();

    /**
     * @brief subType map inheritance info
     */
    struct InheritanceInfo {
        ClassType* parentType;
        Type* subType;
    };

    /**
     * @brief subtype map from class definition to inheritance info list.
     */
    using SubTypeMap = std::unordered_map<ClassDef*, std::vector<InheritanceInfo>>;

    /// return subtype map.
    const SubTypeMap& GetSubtypeMap() const;

    /// return const member type map.
    const ConstMemberVarCollector::ConstMemberMapType& GetConstMemberMap() const;

    /// return real runtime return type map.
    const std::unordered_map<Func*, Type*>& GetReturnTypeMap() const;

    /// map from type to its custom type definition.
    std::unordered_map<const Type*, std::vector<CustomTypeDef*>> defsMap;

private:
    void CollectReturnTypeMap(Func& func);

    SubTypeMap subtypeMap;

    std::unordered_map<Func*, Type*> realRuntimeRetTyMap;

    ConstMemberVarCollector::ConstMemberMapType constMemberTypeMap;

    const Package* package;

    const GlobalOptions opts;
};
} // namespace Codira::CHIR
#endif // CODIRA_DEVIRTUALIZATION_INFO_COLLECT_H
