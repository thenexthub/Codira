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

#ifndef CODIRA_CHIR_VTABLE_CREATOR_H
#define CODIRA_CHIR_VTABLE_CREATOR_H

#include <vector>

#include "Codira/CHIR/UserDefinedType.h"
#include "Codira/CHIR/Type/CustomTypeDef.h"

namespace Codira::CHIR {
class VTableGenerator {
public:
    explicit VTableGenerator(CHIRBuilder& builder);
    /**
    * @brief generate vtable for CustomTypeDef
    *
    * @param customTypeDef generate and set this def's vtable
    */
    void GenerateVTable(CustomTypeDef& customTypeDef);

private:
    void MergeVtable(ClassType& instParentTy, VTableType& vtable);
    void CollectCurDefMethodsMayBeInVtable(const CustomTypeDef& def, std::vector<FuncBase*>& publicFuncs);
    std::vector<FuncBase*> GetAllMethods(const CustomTypeDef& def);
    std::vector<FuncBase*> GetAllMethods(const Type& ty);
    VirtualFuncInfo CreateVirtualFuncInfo(const AbstractMethodInfo& method,
        Type& originalParentType, const std::unordered_map<const GenericType*, Type*>& replaceTable);
    VirtualFuncInfo CreateVirtualFuncInfo(
        FuncBase& method, Type& originalParentType, const std::unordered_map<const GenericType*, Type*>& replaceTable);
    bool UpdateVtable(VirtualFuncInfo& curFuncInfo, VTableType& vtable);
    bool IsSigTypeMatched(const VirtualFuncInfo& curFuncInfo, const VirtualFuncInfo& funcInfoInVtable);
    bool VirtualFuncShouldAddToVTableInItsOwnParent(ClassType& ownParent, ClassType& alreadyIn);
    void UpdateAbstractMethodInVtable(VTableType& vtable);
    void UpdateAbstractMethodWithImplementedMethod(
        VTableType& vtable, const ClassType& curParentTy, VirtualFuncInfo& abstractFuncInfo);
    std::unordered_map<std::string, VirtualFuncInfo> CollectAllPublicAndProtectedMethods(const CustomTypeDef& curDef);
    std::unordered_map<const GenericType*, Type*> GetInstMapFromDefIncludeParents(
        const CustomTypeDef& def, const Type& curType);
    std::vector<FuncBase*> CollectMethodsIncludeParentsMayBeInVtable(const CustomTypeDef& curDef);
    void CollectMethodsFromAncestorInterfaceMayBeInVTable(
        const CustomTypeDef& curDef, std::vector<FuncBase*>& methods);

private:
    CHIRBuilder& builder;
};
}

#endif
