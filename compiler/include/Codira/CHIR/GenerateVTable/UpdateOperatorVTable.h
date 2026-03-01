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

#ifndef CODIRA_CHIR_UPDATE_OPERATOR_VTABLE_H
#define CODIRA_CHIR_UPDATE_OPERATOR_VTABLE_H

#include <vector>

#include "Codira/CHIR/CHIRBuilder.h"
#include "Codira/CHIR/Package.h"
#include "Codira/CHIR/Type/ClassDef.h"
#include "Codira/CHIR/UserDefinedType.h"
#include "Codira/Utils/ConstantsUtils.h"

namespace Codira::CHIR {
/// Integer operators that can be affected by overflow strategy, given a specific set of argument types, are called
/// overflow operator. After collecting extend and vtable info, split overflow operator in vtable into three
/// versions if that operator func (of an interface) can be extended by integer types.
class UpdateOperatorVTable {
public:
    UpdateOperatorVTable(const Package& package, CHIRBuilder& builder);
    /**
    * @brief update vtable
    */
    void Update();

private:
    using OverflowOpIndex = size_t;
    struct RewriteVtableInfo {
        std::set<OverflowOpIndex> ov;
    };
    
    void CollectOverflowOperators();
    void CollectOverflowOperatorsOnInterface(ClassDef& def);
    void AddRewriteInfo(ClassDef& def, size_t index);
    void RewriteVtable();
    void RewriteOneVtableEntry(ClassType& infType, CustomTypeDef& user, const VirtualFuncInfo& funcInfo, size_t index);
    Func* GenerateBuiltinOverflowOperatorFunc(
        const std::string& name, OverflowStrategy ovf, const ExtendDef& user, bool isBinary);
    void RewriteVtableEntryRec(const ClassDef& inf, CustomTypeDef& user, const RewriteVtableInfo& info);
    void CollectVTableUsers();

private:
    const Package& package;
    CHIRBuilder& builder;

    // order ClassDef* by mangled name to keep binary equality
    struct RewriteInfoOrdering {
        bool operator()(ClassDef* one, ClassDef* another) const;
    };
    std::map<ClassDef*, RewriteVtableInfo, RewriteInfoOrdering> interRewriteInfo;
    std::unordered_map<std::string, Func*> cache;
    //             parent vtable, sub vtables
    std::unordered_map<ClassDef*, std::vector<CustomTypeDef*>> vtableUsers;
};
}

#endif
