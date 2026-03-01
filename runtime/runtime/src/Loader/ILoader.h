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


#ifndef MRT_ILOADER_H
#define MRT_ILOADER_H
#include <functional>

#include "Base/CString.h"
#include "Common/TypeDef.h"
#include "ObjectModel/ExtensionData.h"
#include "ObjectModel/TypeExt.h"
#include "BinaryFile/BaseFile.h"

namespace MapleRuntime {
class ILoader {
public:
    virtual ~ILoader() = default;
    virtual void Init() = 0;
    virtual void Fini() = 0;
    virtual void RegisterLoadFile(Uptr address) = 0;
    virtual void UnregisterLoadFile(Uptr address) = 0;
    virtual void VisitBaseFile(const std::function<bool(BaseFile*)>& f) const = 0;
    virtual TypeInfo* FindTypeInfoFromLoadedFiles(const char* typeInfoName) = 0;
    virtual TypeTemplate* FindTypeTemplateFromLoadedFiles(const char* typeTemplateName) = 0;
    virtual void RecordTypeInfo(TypeInfo* ti) = 0;
    virtual PackageInfo* GetPackageInfo(const char* packageName) const = 0;
    virtual void RemovePackageInfo(const char* path) = 0;
    virtual PackageInfo* GetPackageInfoByPath(const char* path) = 0;
    virtual bool FileHasLoaded(const char* path) = 0;
    virtual bool FileHasMultiPackage(const char* path) = 0;
    virtual void GetSubPackages(PackageInfo* packageInfo, std::vector<PackageInfo*> &subPackages) = 0;

    virtual bool LibInit(const char*) = 0;
    virtual void* LoadCODELibrary(const char*) = 0;
    virtual int UnloadLibrary(const char*) = 0;
    virtual Uptr FindSymbol(const CString libName, const CString symName) const = 0;
    virtual BaseFile* GetBaseFile(CString fileName) const = 0;
    virtual void VisitExtensionData(
        TypeInfo* ti, const std::function<bool(ExtensionData* ed)>& f, TypeTemplate* tt) const = 0;
    virtual void VisitExtensionData(const std::function<void(BaseFile*)>& f) const = 0;
    virtual bool CheckPackageCompatibility(BaseFile* file) = 0;
    virtual void TryThrowException(Uptr fileMetaAddr) = 0;
    virtual BaseFile* CreateFileRefFromAddr(Uptr address) = 0;
    virtual U32 GetNumOfInterface(TypeInfo* typeInfo) = 0;
    virtual TypeInfo* GetInterface(TypeInfo* typeInfo, U32 idx) = 0;
    virtual TypeExt* GetTypeExt(void* type) = 0;
    virtual void RegisterTypeExt(BaseFile* baseFile) = 0;
#ifdef __OHOS__
    virtual void RegisterLoadFunc(void* loadFunc) = 0;
#endif
    static ILoader* CreateLoader();
};
} // namespace MapleRuntime
#endif // MRT_ILOADER_H
