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


#ifndef MRT_LOADER_MANAGER_H
#define MRT_LOADER_MANAGER_H

#include <atomic>
#include <mutex>
#include <vector>

#include "Base/CString.h"
#include "ObjectModel/MClass.h"
#include "Loader/ILoader.h"

namespace MapleRuntime {
class LoaderManager {
public:
    LoaderManager() noexcept;
    ~LoaderManager() noexcept;
    void Init();
    void Fini();
    void AddPreLoadedImageMetaAddr(Uptr address);
    void RemovePreLoadedImageMetaAddr(Uptr address);
    void LoadPreLoadedImages();

    void RegisterLoadFile(Uptr address) const;
    void UnregisterLoadFile(Uptr address) const;
    void LoadFile(Uptr address);
    void UnloadFile(Uptr address);
    void* LoadCODELibrary(const char* libName) const;
    int UnLoadLibrary(const char* libName) const;
    Uptr FindSymbol(const CString libName, const CString symbolName) const;
    bool LibInit(const char* libName) const;

    ILoader* GetLoader() const;
    static LoaderManager* GetInstance();
    bool GetInitStatus() const;
    static bool GetReleaseStatus() { return isReleased; }

    TypeInfo* FindTypeInfoFromLoadedFiles(const char* typeInfoName);
    TypeTemplate* FindTypeTemplateFromLoadedFiles(const char* typeTemplateName);
    void RecordTypeInfo(TypeInfo* ti);
    PackageInfo* GetPackageInfoByName(const char* packageName);
    PackageInfo* GetPackageInfoByPath(const char* path);
    void RemovePackageInfo(const char* path);
    bool FileHasLoaded(const char* path);
    bool FileHasMultiPackage(const char* path);
    void GetSubPackages(PackageInfo* packageInfo, std::vector<PackageInfo*> &subPackages);
    U32 GetNumOfInterface(TypeInfo* ti);
    TypeInfo* GetInterface(TypeInfo* ti, U32 idx);
    bool CheckPackageCompatibility(Uptr fileMetaAddr);
    void PreInitializePackage(Uptr address);

private:
#ifdef __OHOS__
    void RegisterLoadFunc();
#endif
    static bool isReleased;
    ILoader* loader;
    std::atomic<bool> initStatus;
    std::mutex loadedImgsMtx;
    std::vector<Uptr> preLoadedImages;
};
} // namespace MapleRuntime
#endif // MRT_LOADER_MANAGER_H
