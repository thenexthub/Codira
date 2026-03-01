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


#include "PackageInfo.h" // module internal header
#include "MClass.inline.h"

namespace MapleRuntime {
TypeInfo* PackageInfo::GetTypeInfo(const char* name)
{
    PackageInfo* self = this;
    while (self != nullptr) {
        for (U32 idx = 0; idx < self->GetNumOfTypeInfos(); ++idx) {
            Uptr baseAddr = self->GetBaseAddr();
            TypeInfo** base = reinterpret_cast<TypeInfo**>(baseAddr);
            TypeInfo* ti = *(base + idx);
            if (strcmp(name, ti->GetName()) == 0) {
                return ti;
            }
        }
        self = self->GetRelatedPackageInfo();
    }
    return nullptr;
}

TypeTemplate* PackageInfo::GetTypeTemplate(const char* name)
{
    PackageInfo* self = this;
    while (self != nullptr) {
        Uptr baseAddr = self->GetBaseAddr();
        baseAddr += self->GetNumOfTypeInfos() * TYPEINFO_PTR_SIZE;
        TypeTemplate** base = reinterpret_cast<TypeTemplate**>(baseAddr);
        for (U32 idx = 0; idx < self->GetNumOfTypeTemplates(); ++idx) {
            TypeTemplate* tt = *(base + idx);
            if (tt == nullptr) { continue; }
            if (strcmp(name, tt->GetName()) == 0) {
                return tt;
            }
        }
        self = self->GetRelatedPackageInfo();
    }
    return nullptr;
}

TypeInfo* PackageInfo::GetTypeInfo(U32 index)
{
    Uptr baseAddr = GetBaseAddr();
    TypeInfo** ti = reinterpret_cast<TypeInfo**>(baseAddr);
    return *(ti + index);
}

MethodInfo* PackageInfo::GetGlobalMethodInfo(U32 index)
{
    Uptr baseAddr = GetBaseAddr();
    baseAddr += GetNumOfTypeInfos() * TYPEINFO_PTR_SIZE;
    baseAddr += GetNumOfTypeTemplates() * sizeof(TypeTemplate*);
    baseAddr += index * sizeof(DataRefOffset64<MethodInfo>);
    return reinterpret_cast<DataRefOffset64<MethodInfo>*>(baseAddr)->GetDataRef();
}

StaticFieldInfo* PackageInfo::GetGlobalFieldInfo(U32 index)
{
    Uptr baseAddr = GetBaseAddr();
    baseAddr += GetNumOfTypeInfos() * TYPEINFO_PTR_SIZE;
    baseAddr += GetNumOfTypeTemplates() * sizeof(TypeTemplate*);
    baseAddr += GetNumOfGlobalMethodInfos() * sizeof(DataRefOffset64<MethodInfo>);
    baseAddr += index * sizeof(DataRefOffset64<StaticFieldInfo>);
    return reinterpret_cast<DataRefOffset64<StaticFieldInfo>*>(baseAddr)->GetDataRef();
}
} // namespace MapleRuntime
