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


#include "CodeFile.h"

#include "Base/Types.h"
#include "CodeFileMeta.h"
#include "Common/TypeDef.h"
#include "Heap/Heap.h"
#include "Utils/Demangler.h"
namespace MapleRuntime {
void CODEFile::RegisterFile() { LoadCODEFileMeta(); }

void CODEFile::UnregisterFile()
{
    // unregist gcroot
    Heap::GetHeap().UnregisterStaticRoots(cJFileMeta.gcRootsAddr, cJFileMeta.gcRootSize);
}

#if defined(_WIN64)
void CODEFile::LoadWinCODEFileMeta()
{
    Uptr begin = GetFileMetaAddr();
    CODEFileHeader* header = reinterpret_cast<CODEFileHeader*>(begin);
    // Init TablePtrs
    cJFileMetaEnd = cJFileMetaBegin + *header->cJFileSize;
    cJFileMeta.stackMapTbl.stackMapBasePtr = reinterpret_cast<void*>(*header->tables[STACK_MAP_TABLE].tableAddr);
    cJFileMeta.typeInfoTbl.typeInfoBasePtr = reinterpret_cast<TypeInfo*>(*header->tables[TYPE_INFO_TABLE].tableAddr);
    cJFileMeta.typeInfoTbl.typeInfoTotalSize = *header->tables[TYPE_INFO_TABLE].tableSize;
    cJFileMeta.funcDescTbl.funcDescBasePtr = reinterpret_cast<FuncDescRef>(*header->tables[FUNC_DESC_TABLE].tableAddr);
    cJFileMeta.globalInitFuncTbl.globalInitFuncTotalSize = *header->tables[GLOBAL_INIT_FUNC_TABLE].tableSize;
    cJFileMeta.globalInitFuncTbl.globalInitFuncBasePtr =
        reinterpret_cast<Uptr*>(*header->tables[GLOBAL_INIT_FUNC_TABLE].tableAddr);
    cJFileMeta.gcTibTbl.gcTibBasePtr = reinterpret_cast<void*>(*header->tables[GC_TIB_TABLE].tableAddr);
    cJFileMeta.extensionDataTbl.extensionDataTotalSize = *header->tables[EXTENSION_DATA_TABLE].tableSize;
    cJFileMeta.extensionDataTbl.extensionDataBasePtr =
        reinterpret_cast<ExtensionData*>(*header->tables[EXTENSION_DATA_TABLE].tableAddr);
    cJFileMeta.innerTyExtensionTbl.innerTyExtensionTotalSize = *header->tables[INNER_TYPE_EXTENSIONS_TABLE].tableSize;
    cJFileMeta.innerTyExtensionTbl.innerTyExtensionBasePtr =
        reinterpret_cast<void*>(*header->tables[INNER_TYPE_EXTENSIONS_TABLE].tableAddr);
    cJFileMeta.outerTyExtensionTbl.outerTyExtensionTotalSize =
        *header->tables[OUTER_TYPE_EXTENSIONS_TABLE].tableSize;
    cJFileMeta.outerTyExtensionTbl.outerTyExtensionBasePtr =
        reinterpret_cast<void*>(*header->tables[OUTER_TYPE_EXTENSIONS_TABLE].tableAddr);
    cJFileMeta.staticGITbl.staticGITotalSize = *header->tables[STATIC_GI_TABLE].tableSize;
    cJFileMeta.staticGITbl.staticGIBasePtr =
        reinterpret_cast<void*>(*header->tables[STATIC_GI_TABLE].tableAddr);
    cJFileMeta.gcFlagsTbl.withSafepoint =
        reinterpret_cast<CODEGCFlagsTable*>(*header->tables[GC_FLAGS_TABLE].tableAddr)->withSafepoint;
    cJFileMeta.gcFlagsTbl.withBarrier =
        reinterpret_cast<CODEGCFlagsTable*>(*header->tables[GC_FLAGS_TABLE].tableAddr)->withBarrier;
    cJFileMeta.gcFlagsTbl.hasStackPointerMap =
        reinterpret_cast<CODEGCFlagsTable*>(*header->tables[GC_FLAGS_TABLE].tableAddr)->hasStackPointerMap;
    cJFileMeta.gcRootsAddr = *header->tables[GC_ROOT_TABLE].tableAddr;
    cJFileMeta.gcRootSize = *header->tables[GC_ROOT_TABLE].tableSize / sizeof(U64);
    cJFileMeta.packageInfoTbl.packageInfoBasePtr = *header->tables[PACKINFO_TABLE].tableAddr;
    cJFileMeta.packageInfoTbl.packageInfoTotalSize = *header->tables[PACKINFO_TABLE].tableSize;
    cJFileMeta.typeExtTbl.typeExtBasePtr = reinterpret_cast<TypeExt*>(header->tables[TYPE_EXT_TABLE].tableAddr);
    cJFileMeta.typeExtTbl.typeExtTotalSize = *header->tables[TYPE_EXT_TABLE].tableSize;
    Heap::GetHeap().RegisterStaticRoots(cJFileMeta.gcRootsAddr, cJFileMeta.gcRootSize);
}
#elif defined(__APPLE__)
void CODEFile::LoadMacCODEFileMeta()
{
    Uptr begin = GetFileMetaAddr();
    CODEFileHeader* header = reinterpret_cast<CODEFileHeader*>(begin);
    // Init TablePtrs
    cJFileMetaEnd = cJFileMetaBegin + *header->cJFileSize;
    cJFileMeta.stackMapTbl.stackMapBasePtr = reinterpret_cast<void*>(*header->tables[STACK_MAP_TABLE].tableAddr);
    cJFileMeta.typeInfoTbl.typeInfoBasePtr = reinterpret_cast<TypeInfo*>(*header->tables[TYPE_INFO_TABLE].tableAddr);
    cJFileMeta.typeInfoTbl.typeInfoTotalSize = *header->tables[TYPE_INFO_TABLE].tableSize;
    cJFileMeta.funcDescTbl.funcDescBasePtr = reinterpret_cast<FuncDescRef>(*header->tables[FUNC_DESC_TABLE].tableAddr);
    cJFileMeta.globalInitFuncTbl.globalInitFuncTotalSize = *header->tables[GLOBAL_INIT_FUNC_TABLE].tableSize;
    cJFileMeta.globalInitFuncTbl.globalInitFuncBasePtr =
        reinterpret_cast<Uptr*>(*header->tables[GLOBAL_INIT_FUNC_TABLE].tableAddr);
    cJFileMeta.gcTibTbl.gcTibBasePtr = reinterpret_cast<void*>(*header->tables[GC_TIB_TABLE].tableAddr);
    cJFileMeta.extensionDataTbl.extensionDataTotalSize = *header->tables[EXTENSION_DATA_TABLE].tableSize;
    cJFileMeta.extensionDataTbl.extensionDataBasePtr =
        reinterpret_cast<ExtensionData*>(*header->tables[EXTENSION_DATA_TABLE].tableAddr);
    cJFileMeta.innerTyExtensionTbl.innerTyExtensionTotalSize = *header->tables[INNER_TYPE_EXTENSIONS_TABLE].tableSize;
    cJFileMeta.innerTyExtensionTbl.innerTyExtensionBasePtr =
        reinterpret_cast<void*>(*header->tables[INNER_TYPE_EXTENSIONS_TABLE].tableAddr);
    cJFileMeta.outerTyExtensionTbl.outerTyExtensionTotalSize =
        *header->tables[OUTER_TYPE_EXTENSIONS_TABLE].tableSize;
    cJFileMeta.outerTyExtensionTbl.outerTyExtensionBasePtr =
        reinterpret_cast<void*>(*header->tables[OUTER_TYPE_EXTENSIONS_TABLE].tableAddr);
    cJFileMeta.staticGITbl.staticGITotalSize = *header->tables[STATIC_GI_TABLE].tableSize;
    cJFileMeta.staticGITbl.staticGIBasePtr =
        reinterpret_cast<void*>(*header->tables[STATIC_GI_TABLE].tableAddr);
    cJFileMeta.gcFlagsTbl.withSafepoint =
        reinterpret_cast<CODEGCFlagsTable*>(*header->tables[GC_FLAGS_TABLE].tableAddr)->withSafepoint;
    cJFileMeta.gcFlagsTbl.withBarrier =
        reinterpret_cast<CODEGCFlagsTable*>(*header->tables[GC_FLAGS_TABLE].tableAddr)->withBarrier;
    cJFileMeta.gcFlagsTbl.hasStackPointerMap =
        reinterpret_cast<CODEGCFlagsTable*>(*header->tables[GC_FLAGS_TABLE].tableAddr)->hasStackPointerMap;
    cJFileMeta.gcRootsAddr = *header->tables[GC_ROOT_TABLE].tableAddr;
    cJFileMeta.gcRootSize = *header->tables[GC_ROOT_TABLE].tableSize / sizeof(U64);
    cJFileMeta.packageInfoTbl.packageInfoBasePtr = *header->tables[PACKINFO_TABLE].tableAddr;
    cJFileMeta.packageInfoTbl.packageInfoTotalSize = *header->tables[PACKINFO_TABLE].tableSize;
    cJFileMeta.typeExtTbl.typeExtBasePtr = reinterpret_cast<TypeExt*>(*header->tables[TYPE_EXT_TABLE].tableAddr);
    cJFileMeta.typeExtTbl.typeExtTotalSize = *header->tables[TYPE_EXT_TABLE].tableSize;
    Heap::GetHeap().RegisterStaticRoots(cJFileMeta.gcRootsAddr, cJFileMeta.gcRootSize);
}
#else
void CODEFile::LoadLinuxCODEFileMeta()
{
    Uptr begin = GetFileMetaAddr();
    CODEFileHeader* header = reinterpret_cast<CODEFileHeader*>(begin);

    // Init TablePtrs
    cJFileMetaEnd = cJFileMetaBegin + header->cJFileSize;
    cJFileMeta.stackMapTbl.stackMapBasePtr =
        reinterpret_cast<void*>(begin + header->tables[STACK_MAP_TABLE].tableOffset);
    cJFileMeta.typeInfoTbl.typeInfoBasePtr =
        reinterpret_cast<TypeInfo*>(begin + header->tables[TYPE_INFO_TABLE].tableOffset);
    cJFileMeta.typeInfoTbl.typeInfoTotalSize = header->tables[TYPE_INFO_TABLE].tableSize;
    cJFileMeta.funcDescTbl.funcDescBasePtr =
        reinterpret_cast<FuncDescRef>(begin + header->tables[FUNC_DESC_TABLE].tableOffset);
    cJFileMeta.globalInitFuncTbl.globalInitFuncTotalSize = header->tables[GLOBAL_INIT_FUNC_TABLE].tableSize;
    cJFileMeta.globalInitFuncTbl.globalInitFuncBasePtr =
        reinterpret_cast<Uptr*>(begin + header->tables[GLOBAL_INIT_FUNC_TABLE].tableOffset);
    cJFileMeta.gcTibTbl.gcTibBasePtr = reinterpret_cast<void*>(begin + header->tables[GC_TIB_TABLE].tableOffset);
    cJFileMeta.extensionDataTbl.extensionDataTotalSize = header->tables[EXTENSION_DATA_TABLE].tableSize;
    cJFileMeta.extensionDataTbl.extensionDataBasePtr =
        reinterpret_cast<ExtensionData*>(begin + header->tables[EXTENSION_DATA_TABLE].tableOffset);
    cJFileMeta.innerTyExtensionTbl.innerTyExtensionTotalSize = header->tables[INNER_TYPE_EXTENSIONS_TABLE].tableSize;
    cJFileMeta.innerTyExtensionTbl.innerTyExtensionBasePtr =
        reinterpret_cast<void*>(begin + header->tables[INNER_TYPE_EXTENSIONS_TABLE].tableOffset);
    cJFileMeta.outerTyExtensionTbl.outerTyExtensionTotalSize =
        header->tables[OUTER_TYPE_EXTENSIONS_TABLE].tableSize;
    cJFileMeta.outerTyExtensionTbl.outerTyExtensionBasePtr =
        reinterpret_cast<void*>(begin + header->tables[OUTER_TYPE_EXTENSIONS_TABLE].tableOffset);
    cJFileMeta.staticGITbl.staticGITotalSize = header->tables[STATIC_GI_TABLE].tableSize;
    cJFileMeta.staticGITbl.staticGIBasePtr =
        reinterpret_cast<void*>(begin + header->tables[STATIC_GI_TABLE].tableOffset);
    cJFileMeta.gcFlagsTbl.withSafepoint =
        reinterpret_cast<CODEGCFlagsTable*>(begin + header->tables[GC_FLAGS_TABLE].tableOffset)->withSafepoint;
    cJFileMeta.gcFlagsTbl.withBarrier =
        reinterpret_cast<CODEGCFlagsTable*>(begin + header->tables[GC_FLAGS_TABLE].tableOffset)->withBarrier;
    cJFileMeta.gcFlagsTbl.hasStackPointerMap =
        reinterpret_cast<CODEGCFlagsTable*>(begin + header->tables[GC_FLAGS_TABLE].tableOffset)->hasStackPointerMap;
    cJFileMeta.gcRootsAddr = begin + header->tables[GC_ROOT_TABLE].tableOffset;
#ifdef __arm__
    cJFileMeta.gcRootSize = header->tables[GC_ROOT_TABLE].tableSize / sizeof(U32);
#else
    cJFileMeta.gcRootSize = header->tables[GC_ROOT_TABLE].tableSize / sizeof(U64);
#endif
    cJFileMeta.packageInfoTbl.packageInfoBasePtr = begin + header->tables[PACKINFO_TABLE].tableOffset;
    cJFileMeta.packageInfoTbl.packageInfoTotalSize = header->tables[PACKINFO_TABLE].tableSize;
    cJFileMeta.typeExtTbl.typeExtBasePtr =
        reinterpret_cast<TypeExt*>(begin + header->tables[TYPE_EXT_TABLE].tableOffset);
    cJFileMeta.typeExtTbl.typeExtTotalSize = header->tables[TYPE_EXT_TABLE].tableSize;
    Heap::GetHeap().RegisterStaticRoots(cJFileMeta.gcRootsAddr, cJFileMeta.gcRootSize);
}
#endif

void CODEFile::LoadCODEFileMeta()
{
#if defined(_WIN64)
    LoadWinCODEFileMeta();
#elif defined(__APPLE__)
    LoadMacCODEFileMeta();
#else
    LoadLinuxCODEFileMeta();
#endif
    if (Heap::GetHeap().IsGCEnabled()) {
        if (cJFileMeta.gcFlagsTbl.withSafepoint != 1 || cJFileMeta.gcFlagsTbl.withBarrier != 1) {
            LOG(RTLOG_FATAL, "no safepoint or barrier defined in file %s \n", GetBaseName().Str());
        }
    }
    if (CodiraRuntime::stackGrowConfig == StackGrowConfig::UNDEF) {
        if (cJFileMeta.gcFlagsTbl.hasStackPointerMap == 0) {
            CodiraRuntime::stackGrowConfig = StackGrowConfig::STACK_GROW_OFF;
        } else {
            CodiraRuntime::stackGrowConfig = StackGrowConfig::STACK_GROW_ON;
        }
    } else {
        if ((cJFileMeta.gcFlagsTbl.hasStackPointerMap == 0 &&
                CodiraRuntime::stackGrowConfig != StackGrowConfig::STACK_GROW_OFF) ||
            (cJFileMeta.gcFlagsTbl.hasStackPointerMap == 1 &&
                CodiraRuntime::stackGrowConfig != StackGrowConfig::STACK_GROW_ON)) {
                LOG(RTLOG_FATAL, "The stackmap config are inconsistent. Check whether stack Grow is enabled.\n");
            }
    }
}

Uptr CODEFile::GetFileMetaAddr() const { return cJFileMetaBegin; }

const CODEFileMeta& CODEFile::GetCODEFileMeta() const { return cJFileMeta; }

bool CODEFile::IsAddrInCODEFile(Uptr addr) const { return cJFileMetaBegin <= addr && addr < cJFileMetaEnd; }

void CODEFile::GetGlobalInitFunc(std::vector<Uptr> &globalInitFuncs) const
{
    CString globalInitFuncName = "_CGP";
    const CODEFileMeta& cFileMeta = GetCODEFileMeta();
    Uptr* globalInitPtr = cFileMeta.globalInitFuncTbl.globalInitFuncBasePtr;
    size_t sectionSize = cFileMeta.globalInitFuncTbl.globalInitFuncTotalSize;
#ifdef __APPLE__
    size_t offset = 0;
    while (offset < sectionSize) {
        uint8_t* funcPtrField = reinterpret_cast<uint8_t*>(globalInitPtr) + offset;
        offset += sizeof(void*);
        char* funcNamePtr = reinterpret_cast<char*>(globalInitPtr) + offset;
        offset += strlen(funcNamePtr) + 1; // 1: '\0\' takes one charactor length.
        offset = AlignUp(offset, static_cast<size_t>(8)); // 8: apple xcode needs aligment.
        CString funcName(funcNamePtr);
        if (funcName.Find(globalInitFuncName.Str()) != -1) {
            Uptr globalInitFunc = *reinterpret_cast<Uptr*>(funcPtrField);
            globalInitFuncs.emplace_back(globalInitFunc);
        }
    }
#else
    U32 cnt = sectionSize / sizeof(Uptr);
    for (int i = 0; i < cnt; ++i) {
        auto func = reinterpret_cast<Uptr>(*(globalInitPtr + i));
        CHECK_DETAIL(func != 0, "global init func could not be null");
        globalInitFuncs.emplace_back(func);
    }
#endif
}

Uptr CODEFile::GetPackageInfoBase()
{
    return cJFileMeta.packageInfoTbl.packageInfoBasePtr;
}

U32 CODEFile::GetPackageInfoTotalSize()
{
    return cJFileMeta.packageInfoTbl.packageInfoTotalSize;
}

Uptr CODEFile::GetExtensionDataBase()
{
    return reinterpret_cast<Uptr>(cJFileMeta.extensionDataTbl.extensionDataBasePtr);
}

U32 CODEFile::GetExtensionDataSize()
{
    return cJFileMeta.extensionDataTbl.extensionDataTotalSize;
}

Uptr CODEFile::GetInnerTypeExtensionsBase()
{
    return reinterpret_cast<Uptr>(cJFileMeta.innerTyExtensionTbl.innerTyExtensionBasePtr);
}

U32 CODEFile::GetInnerTypeExtensionsSize() { return cJFileMeta.innerTyExtensionTbl.innerTyExtensionTotalSize; }

Uptr CODEFile::GetOuterTypeExtensionsBase()
{
    return reinterpret_cast<Uptr>(cJFileMeta.outerTyExtensionTbl.outerTyExtensionBasePtr);
}

U32 CODEFile::GetOuterTypeExtensionsSize() { return cJFileMeta.outerTyExtensionTbl.outerTyExtensionTotalSize; }

Uptr CODEFile::GetStaticGIBase()
{
    return reinterpret_cast<Uptr>(cJFileMeta.staticGITbl.staticGIBasePtr);
}

U32 CODEFile::GetStaticGISize()
{
    return cJFileMeta.staticGITbl.staticGITotalSize;
}

Uptr CODEFile::GetTypeInfoBase()
{
    return reinterpret_cast<Uptr>(cJFileMeta.typeInfoTbl.typeInfoBasePtr);
}

U32 CODEFile::GetTypeInfoTotalSize()
{
    return cJFileMeta.typeInfoTbl.typeInfoTotalSize;
}

Uptr CODEFile::GetTypeExtBase()
{
    return reinterpret_cast<Uptr>(cJFileMeta.typeExtTbl.typeExtBasePtr);
}

U32 CODEFile::GetTypeExtTotalSize()
{
    return cJFileMeta.typeExtTbl.typeExtTotalSize;
}

CString CODEFile::GetSDKVersion() const
{
    Uptr begin = GetFileMetaAddr();
    CODEFileHeader* header = reinterpret_cast<CODEFileHeader*>(begin);
#if defined(__APPLE__)
    return CString(reinterpret_cast<char*>(*reinterpret_cast<U64*>(*header->cJSDKVersionPtr)));
#elif defined(_WIN64)
    return CString(reinterpret_cast<char*>(*reinterpret_cast<U64*>(*header->cJSDKVersionPtr)));
#else
    return CString(reinterpret_cast<char*>(*reinterpret_cast<Uptr*>(begin + header->cJSDKVersionOffset)));
#endif
}
} // namespace MapleRuntime
