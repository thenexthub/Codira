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


#ifndef MRT_CODEFILELOADER_H
#define MRT_CODEFILELOADER_H
#include <functional>
#include <list>
#include <mutex>
#include <unordered_set>

#include "Base/HashUtils.h"
#include "Base/Types.h"
#include "ILoader.h"
#include "os/Loader.h"
#include "CodeSemanticVersion/CodeSemanticVersion.h"
#include "RuntimeConfig.h"
namespace MapleRuntime {
class CODEFileLoader : public ILoader {
public:
    CODEFileLoader()
    {
        binLoadApi.binLoad = &Os::Loader::LoadBinaryFile;
        binLoadApi.binUnload = &Os::Loader::UnloadBinaryFile;
        binLoadApi.getBinaryInfoFromAddress = &Os::Loader::GetBinaryInfoFromAddress;
        binLoadApi.findSymbol = &Os::Loader::FindSymbol;
    }
    ~CODEFileLoader() override
    {
        binLoadApi.binLoad = nullptr;
        binLoadApi.binUnload = nullptr;
        binLoadApi.getBinaryInfoFromAddress = nullptr;
        binLoadApi.findSymbol = nullptr;
    }

    void Init() override {}
    void Fini() override;

    void RegisterLoadFile(Uptr address) override;
    void UnregisterLoadFile(Uptr address) override;
    void AddLoadedFiles(BaseFile* baseFile);
    void AddPackageInfos(BaseFile* baseFile);
    void RemoveLoadedFiles(BaseFile* baseFile);
    void ClearLoadedFiles();
    void VisitBaseFile(const std::function<bool(BaseFile*)>& f) const override;
    bool LibInit(const char* libName) override;
    void* LoadCODELibrary(const char* libName) override;
    int UnloadLibrary(const char* libName) override;
    Uptr FindSymbol(const CString libName, const CString symName) const override;
    bool DoInitImage(BaseFile* baseFile) const;
    BaseFile* GetBaseFile(CString fileName) const override;
    auto GetBinaryInfoFromAddressFunc() const { return binLoadApi.getBinaryInfoFromAddress; }
    TypeInfo* FindTypeInfoFromLoadedFiles(const char* className) override;
    TypeTemplate* FindTypeTemplateFromLoadedFiles(const char* className) override;
    void RecordTypeInfo(TypeInfo* ti) override;
    PackageInfo* GetPackageInfo(const char* packageName) const override;
    PackageInfo* GetPackageInfoByPath(const char* path) override;
    void RemovePackageInfo(const char* path) override;
    bool FileHasLoaded(const char* path) override;
    bool FileHasMultiPackage(const char* path) override;
    void GetSubPackages(PackageInfo* packageInfo, std::vector<PackageInfo*> &subPackages) override;
    void VisitExtensionData(
        TypeInfo* ti, const std::function<bool(ExtensionData* ed)>& f, TypeTemplate* tt) const override;
    void VisitExtensionData(const std::function<void(BaseFile*)>& f) const override;
    bool CheckPackageCompatibility(BaseFile* file) override;
    void TryThrowException(Uptr fileMetaAddr) override;
    BaseFile* GetBaseFileByMetaAddr(Uptr fileMetaAddr);
    BaseFile* CreateFileRefFromAddr(Uptr address) override;
    U32 GetNumOfInterface(TypeInfo* typeInfo) override;
    TypeInfo* GetInterface(TypeInfo* typeInfo, U32 idx) override;
    TypeExt* GetTypeExt(void* type) override;
    void RegisterTypeExt(BaseFile* baseFile) override;
#ifdef __OHOS__
    void RegisterLoadFunc(void* loadFunc) override;
#endif

protected:
    struct BinLoadApi binLoadApi;

private:
    void ParseEnumCtor(TypeInfo* ti);
    void RegisterTypeInfoCreatedByFE(BaseFile* baseFile);
    void RegisterOuterTypeExtensions(BaseFile* baseFile);
    int InitCODEFile(const char* libName);
    struct LibNameToHandler {
        CString baseName;
        void* handler;
    };
    std::mutex libCodesoHandlersMutex;
    std::list<LibNameToHandler> codeLibHandlers;
    std::list<BaseFile*> loadedFiles;
    std::unordered_map<const char*, PackageInfo*, HashString, EqualString> packageInfos;
    std::unordered_map<const char*, TypeInfo*, HashString, EqualString> typeInfoCache;
    std::unordered_map<const char*, TypeTemplate*, HashString, EqualString> typeTemplateCache;
    std::unordered_map<PackageInfo*, std::vector<PackageInfo*>> subPackageMap;
    // fileName: packages
    std::unordered_map<const char*, std::vector<PackageInfo*>, HashString, EqualString> filePackageMap;
    // typeTemplate : extensionData1
    //              : ExtensionData2
    //              : ...
    std::unordered_map<BaseFile*, std::unordered_multimap<TypeTemplate*, ExtensionData*>> extensionDatas;
    std::unordered_map<void*, TypeExt*> typeExts;
    // These mutexes are used for lazy initialization.
    std::unordered_set<U32> lazyInitStaticGIs;
    std::mutex lazyStaticGIMutex;
    std::vector<TypeInfo*> staticGIs;
    CodeSemanticVersion compatibility;
    bool lastIsFinished = true;
};
} // namespace MapleRuntime
#endif // MRT_CODE_FILE_LOADER_H
