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


#ifndef MRT_PACKAGEINFO_H
#define MRT_PACKAGEINFO_H

#include "Common/TypeDef.h"
#include "Common/Dataref.h"
#include "RefField.h"
#include "MethodInfo.h"
#include "FieldInfo.h"
#include "Base/Globals.h"

namespace MapleRuntime {
class TypeInfo;
class TypeTemplate;
extern const size_t TYPEINFO_PTR_SIZE;

// PackageInfo includes classes, global functions, global variable.
// global function layout likes MethodInfo.
// global variable layout likes StaticFieldInfo.
class PackageInfo {
public:
    bool IsVaild() { return isVaild == 1; }
    const char* GetPackageName() const { return packageName.GetDataRef(); }
    const char* GetModuleName() const { return moduleName.GetDataRef(); }
    const char* GetVersion() const { return version.GetDataRef(); }
    TypeInfo* GetTypeInfo(const char* name);
    TypeTemplate* GetTypeTemplate(const char* name);

    U32 GetNumOfTypeInfos() const { return typeInfoCnt; }
    U32 GetNumOfTypeTemplates() const { return typeTemplateCnt; }
    U32 GetNumOfGlobalMethodInfos() const { return globalMethodCnt; }
    U32 GetNumOfGlobalFieldInfos() const { return globalVariableCnt; }

    TypeInfo* GetTypeInfo(U32 index);
    MethodInfo* GetGlobalMethodInfo(U32 index);
    StaticFieldInfo* GetGlobalFieldInfo(U32 index);

    PackageInfo* GetRelatedPackageInfo() { return relatedPackageInfo; }

    size_t GetPackageSize()
    {
        U32 packageAlign = 16;
        size_t packageSize = sizeof(PackageInfo);
        packageSize += typeInfoCnt * TYPEINFO_PTR_SIZE;
        packageSize += typeTemplateCnt * sizeof(TypeTemplate*);
        packageSize += globalMethodCnt * sizeof(DataRefOffset64<MethodInfo>);
        packageSize += globalVariableCnt * sizeof(DataRefOffset64<InstanceFieldInfo>);
        return AlignUp<size_t>(packageSize, packageAlign);
    }
private:
    Uptr GetBaseAddr() { return reinterpret_cast<Uptr>(base); }
    PackageInfo* relatedPackageInfo;
    U32 typeInfoCnt;
    U32 typeTemplateCnt;
    U32 globalMethodCnt;
    U32 globalVariableCnt;
    U8 isVaild;
    U8 __attribute__((unused)) slot[7];
    DataRefOffset64<char> moduleName;
    DataRefOffset64<char> packageName;
    DataRefOffset64<char> version;
    void* __attribute__((unused)) slot0;
    void* __attribute__((unused)) slot1;
    Uptr base[0];
};
} // namespace MapleRuntime
#endif // MRT_PACKAGEINFO_H
