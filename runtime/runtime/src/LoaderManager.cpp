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

#ifdef __OHOS__
#include <dlfcn.h>
#endif

#include "LoaderManager.h"
#include "Loader/ILoader.h"
namespace MapleRuntime {
bool LoaderManager::isReleased;
LoaderManager* LoaderManager::GetInstance()
{
    static LoaderManager loaderManager;
    return &loaderManager;
}

LoaderManager::LoaderManager() noexcept
{
    isReleased = false;
    loader = ILoader::CreateLoader();
    loader->Init();
    initStatus.store(false, std::memory_order_relaxed);
}

LoaderManager::~LoaderManager() noexcept
{
    delete loader;
    loader = nullptr;
    isReleased = true;
}

ILoader* LoaderManager::GetLoader() const { return loader; }

TypeInfo* LoaderManager::FindTypeInfoFromLoadedFiles(const char* typeInfoName)
{
    return loader->FindTypeInfoFromLoadedFiles(typeInfoName);
}

TypeTemplate* LoaderManager::FindTypeTemplateFromLoadedFiles(const char* typeTemplateName)
{
    return loader->FindTypeTemplateFromLoadedFiles(typeTemplateName);
}

void LoaderManager::RecordTypeInfo(TypeInfo* ti)
{
    return loader->RecordTypeInfo(ti);
}

PackageInfo* LoaderManager::GetPackageInfoByName(const char* packageName)
{
    return loader->GetPackageInfo(packageName);
}

PackageInfo* LoaderManager::GetPackageInfoByPath(const char* path)
{
    return loader->GetPackageInfoByPath(path);
}

void LoaderManager::RemovePackageInfo(const char* path)
{
    return loader->RemovePackageInfo(path);
}

bool LoaderManager::FileHasLoaded(const char* path)
{
    return loader->FileHasLoaded(path);
}

bool LoaderManager::FileHasMultiPackage(const char* path)
{
    return loader->FileHasMultiPackage(path);
}

void LoaderManager::GetSubPackages(PackageInfo* packageInfo, std::vector<PackageInfo*> &subPackages)
{
    loader->GetSubPackages(packageInfo, subPackages);
}

U32 LoaderManager::GetNumOfInterface(TypeInfo* ti)
{
    return loader->GetNumOfInterface(ti);
}

TypeInfo* LoaderManager::GetInterface(TypeInfo* ti, U32 idx)
{
    return loader->GetInterface(ti, idx);
}

bool LoaderManager::GetInitStatus() const { return initStatus.load(std::memory_order_acquire); }

void LoaderManager::LoadFile(Uptr address)
{
    std::lock_guard<std::mutex> lck(loadedImgsMtx);
    CheckPackageCompatibility(address);
    // MRT_LibraryOnLoad can be invoked before runtime init
    if (GetInitStatus()) {
        RegisterLoadFile(address);
    } else {
        AddPreLoadedImageMetaAddr(address);
    }
}

void LoaderManager::UnloadFile(Uptr address)
{
    std::lock_guard<std::mutex> lck(loadedImgsMtx);
    // MRT_LibraryUnLoad can be invoked before runtime init
    if (GetInitStatus()) {
        UnregisterLoadFile(address);
    } else {
        RemovePreLoadedImageMetaAddr(address);
    }
}

void LoaderManager::Init()
{
    LoadPreLoadedImages();
#ifdef __OHOS__
    RegisterLoadFunc();
#endif
    initStatus.store(true, std::memory_order_relaxed);
}

void LoaderManager::Fini()
{
    loader->Fini();
    initStatus.store(false, std::memory_order_relaxed);
}

void* LoaderManager::LoadCODELibrary(const char* libName) const { return loader->LoadCODELibrary(libName); }

int LoaderManager::UnLoadLibrary(const char* libName) const { return loader->UnloadLibrary(libName); }

Uptr LoaderManager::FindSymbol(const CString libName, const CString symbolName) const
{
    return loader->FindSymbol(libName, symbolName);
}

bool LoaderManager::LibInit(const char* libName) const { return loader->LibInit(libName); }

void LoaderManager::LoadPreLoadedImages()
{
    std::lock_guard<std::mutex> lck(loadedImgsMtx);
    for (auto it = preLoadedImages.rbegin(); it != preLoadedImages.rend(); it++) {
        RegisterLoadFile(*it);
    }
    preLoadedImages.clear();
}

void LoaderManager::RegisterLoadFile(Uptr address) const { loader->RegisterLoadFile(address); }

void LoaderManager::UnregisterLoadFile(Uptr address) const { loader->UnregisterLoadFile(address); }

void LoaderManager::AddPreLoadedImageMetaAddr(Uptr address) { preLoadedImages.push_back(address); }

void LoaderManager::RemovePreLoadedImageMetaAddr(Uptr address)
{
    for (auto it = preLoadedImages.begin(); it != preLoadedImages.end();) {
        if (*it == address) {
            it = preLoadedImages.erase(it);
        } else {
            ++it;
        }
    }
}

bool LoaderManager::CheckPackageCompatibility(Uptr address)
{
    return loader->CheckPackageCompatibility(loader->CreateFileRefFromAddr(address));
};

#ifdef __OHOS__
// Due to the namespace isolation mechanism of ohos, the runtime has no
// permission to directly open the dynamic library on the application side.
// The runtime opens the dynamic library on the application side by using
// the dynamic loading interface on the default namespace.
void LoaderManager::RegisterLoadFunc()
{
    struct CODEEnvMethods {
        void (*initCODEAppNS)(void* path) = nullptr;
        void (*initCODESDKNS)(void* path) = nullptr;
        void (*initCODESysNS)(void* path) = nullptr;
        void (*initCODEChipSDKNS)(void* path) = nullptr;
        bool (*startRuntime)() = nullptr;
        bool (*startUIScheduler)() = nullptr;
        void* (*loadCODEModule)(const char* dllName) = nullptr;
        void* (*loadLibrary)(uint32_t kind, const char* dllName) = nullptr;
        void* (*getSymbol)(void* handle, const char* symbol) = nullptr;
        void* (*loadCODELibrary)(const char* dllName) = nullptr;
        bool (*startDebugger)() = nullptr;
        void (*registerCODEUncaughtExceptionHandler)(void* uncaughtExceptionInfo) = nullptr;
        void (*setSanitizerKindRuntimeVersion)(void* kind) = nullptr;
        bool (*checkLoadCODELibrary)() = nullptr;
        void (*registerArkVMInRuntime)(unsigned long long arkVM) = nullptr;
        void (*registerStackInfoCallbacks)(void* uFunc) = nullptr;
        void (*setAppVersion)(void* version) = nullptr;
        void (*dumpHeapSnapshot) (int fd) = nullptr;
        void (*forceFullGC) () = nullptr;
    };
    Dl_namespace dlns;
    dlns_get(nullptr, &dlns);
    if (strcmp(dlns.name, "code_rom_sdk") != 0) {
        return;
    }
    using GenEnvFunc = CODEEnvMethods*(*)();
#ifdef __arm__
    const char* codeEnvFile = "/system/lib/platformsdk/libcode_environment.z.so";
#else
    const char* codeEnvFile = "/system/lib64/platformsdk/libcode_environment.z.so";
#endif
    // "CODEEnvMethods* CODEEnvironment::CreateEnvMethods()" mangled in c++
    const char* createEnvFuncMangledName = "_ZN4OHOS13CODEEnvironment16CreateEnvMethodsEv";
    void* handle = dlopen(codeEnvFile, RTLD_NOW);
    if (handle == nullptr) {
        LOG(RTLOG_ERROR, "LoaderManager::RegisterLoadFunc: dlopen %s fail\n", codeEnvFile);
        return;
    }

    void* getEnvFunc = dlsym(handle, createEnvFuncMangledName);
    if (getEnvFunc == nullptr) {
        LOG(RTLOG_ERROR, "LoaderManager::RegisterLoadFunc: dlsym func `CODEEnvironment::CreateEnvMethods()` fail\n");
        dlclose(handle);
        return;
    }

    CODEEnvMethods* envFuncs = ((GenEnvFunc)getEnvFunc)();
    if (envFuncs == nullptr) {
        LOG(RTLOG_ERROR, "LoaderManager::RegisterLoadFunc: envFuncs is nullptr\n");
        dlclose(handle);
        return;
    }
    void* loadCODELibraryFunc = (void*)envFuncs->loadCODELibrary;
    if (loadCODELibraryFunc == nullptr) {
        LOG(RTLOG_ERROR, "LoaderManager::RegisterLoadFunc: get loadCODELibraryFunc fail\n");
        dlclose(handle);
        return;
    }
    loader->RegisterLoadFunc(loadCODELibraryFunc);
    dlclose(handle);
}
#endif

void LoaderManager::PreInitializePackage(Uptr address) { loader->TryThrowException(address); };
} // namespace MapleRuntime
