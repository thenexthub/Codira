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


#include "CodeFileLoader.h"

#include "ExceptionManager.inline.h"
#include "ObjectManager.inline.h"
#include "TypeInfoManager.h"
namespace MapleRuntime {

void CODEFileLoader::Fini()
{
    ClearLoadedFiles();
}

void CODEFileLoader::RegisterLoadFile(Uptr fileMetaAddr)
{
    ScopedEntryTrace trace("CODERT_RegisterLoadFile");
    BaseFile* file = GetBaseFileByMetaAddr(fileMetaAddr);
    if (file == nullptr) {
        return;
    }
    file->RegisterFile();
#ifndef __arm__
    AddPackageInfos(file);
#endif
    RegisterTypeExt(file);
    RegisterTypeInfoCreatedByFE(file);
    RegisterOuterTypeExtensions(file);
}

BaseFile* CODEFileLoader::GetBaseFileByMetaAddr(Uptr fileMetaAddr)
{
    BaseFile* file = nullptr;
    VisitBaseFile([&file, &fileMetaAddr](BaseFile* cJfile) {
        if (cJfile->GetFileMetaAddr() == fileMetaAddr) {
            file = cJfile;
            return true;
        } else {
            return false;
        }
    });
    return file;
}

void CODEFileLoader::UnregisterLoadFile(Uptr fileMetaAddr)
{
    BaseFile* file = GetBaseFileByMetaAddr(fileMetaAddr);
    if (file != nullptr) {
        RemoveLoadedFiles(file);
    }
}
void CODEFileLoader::AddLoadedFiles(BaseFile* baseFile) { loadedFiles.push_back(baseFile); }

BaseFile* CODEFileLoader::CreateFileRefFromAddr(Uptr fileMetaAddr)
{
    auto getBinaryInfoFromAddressFunc = GetBinaryInfoFromAddressFunc();
    CHECK(getBinaryInfoFromAddressFunc != nullptr);
    Os::Loader::BinaryInfo binInfo;
    int isGetBinInfoSuccess = getBinaryInfoFromAddressFunc(reinterpret_cast<void*>(fileMetaAddr), &binInfo);
    if (isGetBinInfoSuccess == 0) {
        isGetBinInfoSuccess = Os::Loader::GetBinaryInfoFromAddress(reinterpret_cast<void*>(fileMetaAddr), &binInfo);
    }
    CHECK(isGetBinInfoSuccess != 0);
    BaseFile* file = BaseFile::CreateCODEFile(FileType::C_FILE, CString(binInfo.filePathName), fileMetaAddr);
    if (file == nullptr) {
        return nullptr;
    }
    return file;
}

void CODEFileLoader::AddPackageInfos(BaseFile* baseFile)
{
    Uptr packageInfoBase = baseFile->GetPackageInfoBase();
    U32 pkgTotalSize = baseFile->GetPackageInfoTotalSize();
    while (pkgTotalSize > 0) {
        PackageInfo* packageInfo = reinterpret_cast<PackageInfo*>(packageInfoBase);
        const char* pkgName = packageInfo->GetPackageName();
        auto pkgIt = packageInfos.find(pkgName);
        if (pkgIt == packageInfos.end()) {
            packageInfos.insert({ pkgName, packageInfo });
            // record the relation between file and the packageInfo,
            // identify whether multiple packages exist in a file.
            auto fileIt = filePackageMap.find(baseFile->GetBaseName().Str());
            if (fileIt == filePackageMap.end()) {
                std::vector<PackageInfo*> pkgs { packageInfo };
                filePackageMap.insert({ baseFile->GetBaseName().Str(), pkgs });
            } else {
                fileIt->second.push_back(packageInfo);
            }
        }

        size_t packageInfoSize = packageInfo->GetPackageSize();
        if (pkgTotalSize >= packageInfoSize) {
            pkgTotalSize -= packageInfoSize;
        } else {
            break;
        }
        packageInfoBase += packageInfoSize;
    }
}

bool CODEFileLoader::FileHasLoaded(const char* path)
{
    CString baseName = Os::Path::GetBaseName(path);
    auto fileIt = filePackageMap.find(baseName.Str());
    if (fileIt != filePackageMap.end()) {
        return true;
    }
    return false;
}

bool CODEFileLoader::FileHasMultiPackage(const char* path)
{
    CString baseName = Os::Path::GetBaseName(path);
    auto fileIt = filePackageMap.find(baseName.Str());
    if (fileIt != filePackageMap.end() && fileIt->second.size() > 1) {
        return true;
    }
    return false;
}

void CODEFileLoader::GetSubPackages(PackageInfo* packageInfo, std::vector<PackageInfo*> &subPackages)
{
    CString prefix = CString(packageInfo->GetPackageName()) + ".";
    for (auto &pkgInfoPair : packageInfos) {
        PackageInfo* pkgInfo = pkgInfoPair.second;
        if (CString(pkgInfo->GetPackageName()).StartWith(prefix)) {
            subPackages.emplace_back(pkgInfo);
        }
    }
}

// Traverse outer extension data grouped by BaseFile
void CODEFileLoader::VisitExtensionData(
    TypeInfo* ti, const std::function<bool(ExtensionData* ed)>& f, TypeTemplate* tt) const
{
    ti->TryInitMTable();
    auto mtDesc = ti->GetMTableDesc();
    std::lock_guard<std::recursive_mutex> lock(mtDesc->mTableMutex);
    if (mtDesc->waitedExtensionDatas.empty()) {
        return;
    }
    size_t cnt = 0;
    for (auto baseFile : mtDesc->waitedExtensionDatas) {
        ++cnt;
        auto it1 = extensionDatas.find(baseFile);
        if (it1 == extensionDatas.end()) {
            continue;
        }
        auto& extensions = it1->second;
        if (extensions.find(tt) == extensions.end()) {
            continue;
        }
        bool found = false;
        auto range = extensions.equal_range(tt);
        for (auto it2 = range.first; it2 != range.second; ++it2) {
            auto res = f(it2->second);
            found |= res;
        }
        if (found) {
            break;
        }
    }
    if (!lastIsFinished && cnt == mtDesc->waitedExtensionDatas.size()) {
        auto last = mtDesc->waitedExtensionDatas.back();
        mtDesc->waitedExtensionDatas.clear();
        mtDesc->waitedExtensionDatas.emplace_back(last);
    } else {
        auto beginIt = mtDesc->waitedExtensionDatas.begin();
        mtDesc->waitedExtensionDatas.erase(beginIt, beginIt + cnt);
    }
}

void CODEFileLoader::VisitExtensionData(const std::function<void(BaseFile*)>& f) const
{
    CHECK(loadedFiles.size() >= extensionDatas.size());
    for (auto baseFile : loadedFiles) {
        f(baseFile);
    }
}

void CODEFileLoader::ParseEnumCtor(TypeInfo* ti)
{
#ifdef __arm__
    return;
#endif
    if (ti->IsGenericTypeInfo()) {
        return TypeInfoManager::GetTypeInfoManager().ParseEnumInfo(
            ti->GetSourceGeneric(), ti->GetTypeArgNum(), ti->GetTypeArgs(), ti);
    }
    EnumInfo* ei = ti->GetEnumInfo();
    if (ei == nullptr || ei->GetNumOfEnumCtor() == 0 || ei->IsParsed()) {
        return;
    }
    U32 enumCtorNum = ei->GetNumOfEnumCtor();
    for (U32 idx = 0; idx < enumCtorNum; ++idx) {
        EnumCtorInfo* enumCtorInfo = ei->GetEnumCtor(idx);
        void* fn = reinterpret_cast<void*>(enumCtorInfo->GetCtorFn());
        if (fn == nullptr) {
            continue;
        }
        TypeInfo* enumTi = reinterpret_cast<TypeInfo*>(
            TypeTemplate::ExecuteGenericFunc(fn, ti->GetTypeArgNum(), ti->GetTypeArgs()));
        enumCtorInfo->SetTypeInfo(enumTi);
    }
    ei->SetParsed();
}

void CODEFileLoader::RegisterTypeExt(BaseFile* baseFile)
{
    Uptr typeExtBase = baseFile->GetTypeExtBase();
    Uptr typeExtEnd = typeExtBase + baseFile->GetTypeExtTotalSize();
    while (typeExtBase < typeExtEnd) {
        TypeExt* typeExt = reinterpret_cast<TypeExt*>(typeExtBase);
        constexpr uint32_t typeExtAlign = 16u;
        uint32_t sizeAlign = MRT_ALIGN(typeExt->size, typeExtAlign);
        typeExtBase += sizeAlign;
        typeExts.emplace(reinterpret_cast<void*>(typeExt->ti), typeExt);
    }
}

void CODEFileLoader::RegisterTypeInfoCreatedByFE(BaseFile* baseFile)
{
    Uptr typeInfoBase = baseFile->GetTypeInfoBase();
    Uptr typeInfoEnd = typeInfoBase + baseFile->GetTypeInfoTotalSize();
    while (typeInfoBase < typeInfoEnd) {
        TypeInfo* ti = reinterpret_cast<TypeInfo*>(typeInfoBase);
        constexpr uint32_t typeInfoAlign = 16u;
        constexpr uint32_t sizeAlign = MRT_ALIGN(sizeof(TypeInfo), typeInfoAlign);
        typeInfoBase += sizeAlign;
        auto tt = ti->GetSourceGeneric();
        if (tt != nullptr) {
            ti->SetvExtensionDataStart(tt->GetvExtensionDataStart());
        }
        TypeInfoManager::GetTypeInfoManager().AddTypeInfo(ti);
        if (ti->IsEnum() || ti->IsTempEnum()) {
            ParseEnumCtor(ti);
        }
    }
    TypeInfoManager::GetTypeInfoManager().InitAnyAndObjectType();

    Uptr staticGIBase = baseFile->GetStaticGIBase();
    Uptr staticGIEnd = staticGIBase + baseFile->GetStaticGISize();
    staticGIs.clear();
    while (staticGIBase < staticGIEnd) {
        I32 offset = *reinterpret_cast<I32*>(staticGIBase);
#if defined(__APPLE__)
        TypeInfo* ti = reinterpret_cast<TypeInfo*>(staticGIBase - offset);
#else
        TypeInfo* ti = reinterpret_cast<TypeInfo*>(staticGIBase + offset);
#endif
        staticGIBase += sizeof(I32);
        staticGIs.push_back(ti);
        if (ti->IsEnum() || ti->IsTempEnum()) {
            continue;
        } else if (ti->IsGenericTypeInfo() && ti->ReflectInfoIsNull() && !ti->GetSourceGeneric()->ReflectInfoIsNull()) {
            TypeInfoManager::GetTypeInfoManager().FillReflectInfo(ti->GetSourceGeneric(), ti);
        }
    }
}

void CODEFileLoader::RegisterOuterTypeExtensions(BaseFile* baseFile)
{
    lastIsFinished = false;
    for (auto mtDesc : TypeInfoManager::GetTypeInfoManager().mTableList) {
        mtDesc.second->waitedExtensionDatas.emplace_back(baseFile);
    }
    Uptr extensionDataRefBase = baseFile->GetOuterTypeExtensionsBase();
    Uptr extensionDataRefEnd = extensionDataRefBase + baseFile->GetOuterTypeExtensionsSize();
    while (extensionDataRefBase < extensionDataRefEnd) {
        I32 offset = *reinterpret_cast<I32*>(extensionDataRefBase);
#ifdef __APPLE__
        ExtensionData* extensionData = reinterpret_cast<ExtensionData*>(extensionDataRefBase - offset);
#else
        ExtensionData* extensionData = reinterpret_cast<ExtensionData*>(extensionDataRefBase + offset);
#endif
        extensionDataRefBase += sizeof(I32);
        // for the extension of which target is a TypeInfo, since it cannot be used
        // in subsequent processes, we add MTable for it in advance so that it won't
        // need to be collected in `extensionDatas`.
        if (extensionData->TargetIsTypeInfo()) {
            TypeInfo* itf = extensionData->GetInterfaceTypeInfo();
            TypeInfoManager::GetTypeInfoManager().AddTypeInfo(itf);
            TypeInfo* ti = reinterpret_cast<TypeInfo*>(extensionData->GetTargetType());
            TypeInfoManager::GetTypeInfoManager().AddTypeInfo(ti);
            ti->AddMTable(itf, extensionData);
            continue;
        }
        TypeTemplate* tt = reinterpret_cast<TypeTemplate*>(extensionData->GetTargetType());
        extensionDatas[baseFile].emplace(tt, extensionData);
    }
    lastIsFinished = true;
}

PackageInfo* CODEFileLoader::GetPackageInfoByPath(const char* path)
{
    CString baseName = Os::Path::GetBaseName(path);
    auto fileIt = filePackageMap.find(baseName.Str());
    if (fileIt == filePackageMap.end()) {
        return nullptr;
    }
    return fileIt->second[0];
}

void CODEFileLoader::RemovePackageInfo(const char* path)
{
    CString baseName = Os::Path::GetBaseName(path);
    auto fileIt = filePackageMap.find(baseName.Str());
    if (fileIt != filePackageMap.end()) {
        for (auto pkgInfo : fileIt->second) {
            packageInfos.erase(pkgInfo->GetPackageName());
        }
        filePackageMap.erase(baseName.Str());
    }
}

PackageInfo* CODEFileLoader::GetPackageInfo(const char* pkgName) const
{
    PackageInfo* pkgInfo = nullptr;
    auto it = packageInfos.find(pkgName);
    if (it != packageInfos.end()) {
        pkgInfo = it->second;
        if (!pkgInfo->IsVaild()) {
            return nullptr;
        }
        return pkgInfo;
    }
    return nullptr;
}

void CODEFileLoader::RemoveLoadedFiles(BaseFile* baseFile)
{
    loadedFiles.remove(baseFile);
    baseFile->UnregisterFile();
    delete baseFile;
}

void CODEFileLoader::VisitBaseFile(const std::function<bool(BaseFile*)>& f) const
{
    for (auto file : loadedFiles) {
        if (f(file)) {
            return;
        }
    }
}

TypeInfo* CODEFileLoader::FindTypeInfoFromLoadedFiles(const char* typeInfoName)
{
    auto it = typeInfoCache.find(typeInfoName);
    if (it != typeInfoCache.end()) {
        return it->second;
    }
    CString pkgName;
    CString typeInfoNameStr = CString(typeInfoName);
    int idx = typeInfoNameStr.Find(':');
    if (idx < 0) {
        pkgName = "std.core";
    } else {
        pkgName = typeInfoNameStr.SubStr(0, idx);
    }
    auto pkgIt = packageInfos.find(pkgName.Str());
    if (pkgIt != packageInfos.end()) {
        PackageInfo* pkgInfo = pkgIt->second;
        TypeInfo* ti = pkgInfo->GetTypeInfo(typeInfoName);
        if (ti == nullptr) {
            return nullptr;
        }
        typeInfoCache.insert({ ti->GetName(), ti });
        return ti;
    }
    return nullptr;
}

TypeTemplate* CODEFileLoader::FindTypeTemplateFromLoadedFiles(const char* typeTemplateName)
{
    auto it = typeTemplateCache.find(typeTemplateName);
    if (it != typeTemplateCache.end()) {
        return it->second;
    }
    CString pkgName;
    CString typeTemplateNameStr = CString(typeTemplateName);
    int idx = typeTemplateNameStr.Find(':');
    if (idx < 0) {
        pkgName = "std.core";
    } else {
        pkgName = typeTemplateNameStr.SubStr(0, idx);
    }
    auto pkgIt = packageInfos.find(pkgName.Str());
    if (pkgIt != packageInfos.end()) {
        PackageInfo* pkgInfo = pkgIt->second;
        TypeTemplate* tt = pkgInfo->GetTypeTemplate(typeTemplateName);
        if (tt == nullptr) {
            return nullptr;
        }
        typeTemplateCache.insert({ tt->GetName(), tt });
        return tt;
    }
    return nullptr;
}

void CODEFileLoader::RecordTypeInfo(TypeInfo* ti)
{
    typeInfoCache.insert({ ti->GetName(), ti });
}

void CODEFileLoader::ClearLoadedFiles()
{
    VisitBaseFile([](BaseFile* baseFile) {
        baseFile->UnregisterFile();
        delete baseFile;
        return false;
    });
    loadedFiles.clear();
}

bool CODEFileLoader::LibInit(const char* libName)
{
    BaseFile* baseFile = GetBaseFile(libName);
    if (baseFile == nullptr) {
        return false;
    }
    return DoInitImage(baseFile);
}

#ifdef __OHOS__
void CODEFileLoader::RegisterLoadFunc(void* loadFunc)
{
    binLoadApi.binLoad = (void*(*)(const char*))(loadFunc);
}
#endif

void* CODEFileLoader::LoadCODELibrary(const char* libName)
{
    void* handler = binLoadApi.binLoad(libName);
    if (handler != nullptr) {
        std::lock_guard<std::mutex> lock(libCodesoHandlersMutex);
        CString baseName = Os::Path::GetBaseName(libName);
        auto handlerIt =
            std::find_if(codeLibHandlers.begin(), codeLibHandlers.end(), [&baseName](const LibNameToHandler& info) {
                return baseName == Os::Path::GetBaseName(info.baseName.Str());
            });
        if (handlerIt == codeLibHandlers.end()) {
            codeLibHandlers.push_back({ baseName, handler });
        }
    }
    return handler;
}

int CODEFileLoader::UnloadLibrary(const char* libName)
{
    if (libName == nullptr) {
        return -1;
    }
    CString baseName = Os::Path::GetBaseName(libName);
    std::lock_guard<std::mutex> lock(libCodesoHandlersMutex);
    auto handlerIt =
        std::find_if(codeLibHandlers.begin(), codeLibHandlers.end(), [&baseName](const LibNameToHandler& info) {
            return baseName == Os::Path::GetBaseName(info.baseName.Str());
        });
    if (handlerIt == codeLibHandlers.end()) {
        return -1;
    }

    int ret = binLoadApi.binUnload(handlerIt->handler);
    if (ret == 0) {
        codeLibHandlers.erase(handlerIt);
    }
    return ret;
}

Uptr CODEFileLoader::FindSymbol(const CString libName, const CString symName) const
{
    CString baseName = Os::Path::GetBaseName(libName.Str());
    auto handlerIt =
        std::find_if(codeLibHandlers.begin(), codeLibHandlers.end(), [&baseName](const LibNameToHandler& info) {
            return baseName == Os::Path::GetBaseName(info.baseName.Str());
        });
    if (handlerIt == codeLibHandlers.end()) {
        return 0;
    }
    return reinterpret_cast<Uptr>(binLoadApi.findSymbol(handlerIt->handler, symName.Str()));
}

bool CODEFileLoader::DoInitImage(BaseFile* baseFile) const
{
    ScopedEntryTrace trace((CString("CODERT_INIT_LIBRARY_") + baseFile->GetBaseName()).Str());
    std::vector<Uptr> funcs;
    baseFile->GetGlobalInitFunc(funcs);
    for (Uptr func : funcs) {
        if (reinterpret_cast<void*>(func) != nullptr) {
            using FuncType = void (*)();
            FuncType initAddr = reinterpret_cast<FuncType>(func);
#if defined(__OHOS__)
            InitCODELibraryStub(reinterpret_cast<void*>(initAddr));
#else
            Mutator* mutator = ThreadLocal::GetMutator();
            if (mutator != nullptr) {
                mutator->SetManagedContext(true);
            }
            uintptr_t threadData = MapleRuntime::MRT_GetThreadLocalData();
            ExecuteCodiraStub(0, 0, 0, reinterpret_cast<void*>(initAddr), reinterpret_cast<void*>(threadData), 0);
            if (mutator != nullptr) {
                mutator->SetManagedContext(false);
            }
            if (ExceptionManager::HasPendingException()) {
                ExceptionRef ex = ExceptionManager::GetPendingException();
                LOG(RTLOG_ERROR, "Init Image fail! exception occurrence when init image, exception:%s ",
                    ex->GetTypeInfo()->GetName());
                ExceptionManager::ClearPendingException();
                return false;
            }
#endif
        }
    }
    return true;
}

BaseFile* CODEFileLoader::GetBaseFile(CString fileName) const
{
    BaseFile* baseFile = nullptr;
    CString baseName = Os::Path::GetBaseName(fileName.Str());
    VisitBaseFile([&baseName, &baseFile](BaseFile* file) {
        if (file->GetBaseName() == baseName) {
            baseFile = file;
            return true;
        } else {
            return false;
        }
    });
    return baseFile;
}

bool CODEFileLoader::CheckPackageCompatibility(BaseFile* file)
{
    if (file == nullptr) {
        return false;
    }
#ifdef __arm__
    bool isCompatible = true;
#else
    CString packageName = file->GetRealPath();
    CString packageVersion = file->GetSDKVersion();
    bool isCompatible = compatibility.CheckPackageCompatibility(packageName, packageVersion);
#endif
    file->SetFileCompatibility(isCompatible);
    AddLoadedFiles(file);
    return isCompatible;
}

void CODEFileLoader::TryThrowException(Uptr fileMetaAddr)
{
    BaseFile* file = GetBaseFileByMetaAddr(fileMetaAddr);
    if (file == nullptr || file->IsCompatible()) {
        return;
    }
    CString packageName = file->GetRealPath();
    CString packageVersion = file->GetSDKVersion();
    CString msg = "executable cangjie file ";
    msg.Append(packageName);
    msg.Append(CString::FormatString(" version %s is not compatible with deployed cangjie runtime version %s",
        packageVersion.Str(), compatibility.GetRuntimeSDKVersion()));
#ifndef DISABLE_VERSION_CHECK
    ExceptionManager::IncompatiblePackageExpection(msg);
    RemoveLoadedFiles(file);
#else
    LOG(RTLOG_WARNING, "%s", msg.Str());
#endif
}

U32 CODEFileLoader::GetNumOfInterface(TypeInfo* ti)
{
    std::vector<TypeInfo*> itfs;
    ti->GetInterfaces(itfs);
    return itfs.size();
}

TypeInfo* CODEFileLoader::GetInterface(TypeInfo* ti, U32 idx)
{
    std::vector<TypeInfo*> itfs;
    ti->GetInterfaces(itfs);
    if (idx >= itfs.size()) {
        return nullptr;
    }
    return itfs[idx];
}

TypeExt* CODEFileLoader::GetTypeExt(void* type)
{
    auto it = typeExts.find(type);
    return it == typeExts.end() ? nullptr : it->second;
}
} // namespace MapleRuntime
