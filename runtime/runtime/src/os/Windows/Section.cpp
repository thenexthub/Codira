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

#include <windows.h>
#include <iostream>
#include <initializer_list>

extern "C" {
// CODEMetadataHeader
unsigned long __CODEMetadataSize;
uintptr_t __CODEMetadataStart;

// CODESDKVersion
unsigned long __CODESDKVersionSize;
uintptr_t __CODESDKVersion;

// CODEMethodInfo
unsigned long __CODEMethodInfoSize;
uintptr_t __CODEMethodInfo;

// CODEGlobalInitFunc
unsigned long __CODEGlobalInitFuncSize;
uintptr_t __CODEGlobalInitFunc;

// CODEStringPoolDict
unsigned long __CODEStringPoolDictSize;
uintptr_t __CODEStringPoolDict;

// CODEStringPool
unsigned long __CODEStringPoolSize;
uintptr_t __CODEStringPool;

// CODEStackMap
unsigned long __CODEStackMapSize;
uintptr_t __CODEStackMap;

// CODEGCTib
unsigned long __CODEGCTibSize;
uintptr_t __CODEGCTib;

// CODEGCRoots
unsigned long __CODEGCRootsSize;
uintptr_t __CODEGCRoots;

// CODETypeTemplate
unsigned long __CODETypeTemplateSize;
uintptr_t __CODETypeTemplate;

// CODETypeInfo
unsigned long __CODETypeInfoSize;
uintptr_t __CODETypeInfo;

// CODETypeInfoFields
unsigned long __CODETypeFieldsSize;
uintptr_t __CODETypeFields;

// CODEMTable
unsigned long __CODEMTableSize;
uintptr_t __CODEMTable;

unsigned long __CODEInnerTypeExtensionsSize;
uintptr_t __CODEInnerTypeExtensions;

unsigned long __CODEOuterTypeExtensionsSize;
uintptr_t __CODEOuterTypeExtensions;

// Static GI
unsigned long __CODEStaticGITableSize;
uintptr_t __CODEStaticGITable;

// CODEGCFlags
unsigned long __CODEGCFlagsSize;
uintptr_t __CODEGCFlags;

// CODEReflectPkgInfo
unsigned long __CODEGCReflectPkgInfoSize;
uintptr_t __CODEReflectPkgInfo;

// CODEReflectGV
unsigned long __CODEReflectGVSize;
uintptr_t __CODEReflectGV;

// CODEReflectGI
unsigned long __CODEReflectGISize;
uintptr_t __CODEReflectGI;

// CODETypeExt
unsigned long __CODETypeExtSize;
uintptr_t __CODETypeExt;

__attribute__((constructor(0))) __declspec(dllexport) void InitData()
{
    HMODULE hModule = NULL;
    if (!GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,
                           reinterpret_cast<LPCSTR>(&InitData),
                           &hModule)) {
        return;
    }
    if (hModule == NULL) {
        return;
    }

    PIMAGE_DOS_HEADER dosHeader = (PIMAGE_DOS_HEADER)hModule;
    if (dosHeader->e_magic != IMAGE_DOS_SIGNATURE) {
        return;
    }

    PIMAGE_NT_HEADERS ntHeaders = (PIMAGE_NT_HEADERS)((BYTE*)hModule + dosHeader->e_lfanew);
    if (ntHeaders->Signature != IMAGE_NT_SIGNATURE) {
        return;
    }

    PIMAGE_SECTION_HEADER sectionHeader = IMAGE_FIRST_SECTION(ntHeaders);

    for (UINT i = 0; i < ntHeaders->FileHeader.NumberOfSections; i++) {
        const char* secName = reinterpret_cast<const char*>(sectionHeader->Name);

        if (strncmp(secName, ".header", sizeof(".header") - 1) == 0) {
            __CODEMetadataStart = reinterpret_cast<uintptr_t>(hModule) +
                                sectionHeader->VirtualAddress;
            __CODEMetadataSize = sectionHeader->Misc.VirtualSize;
        } else if (strncmp(secName, ".codesdkv", sizeof(".codesdkv") - 1) == 0) {
            __CODESDKVersion = reinterpret_cast<uintptr_t>(hModule) +
                             sectionHeader->VirtualAddress;
            __CODESDKVersionSize = sectionHeader->Misc.VirtualSize;
        } else if (strncmp(secName, ".codemthd", sizeof(".codemthd") - 1) == 0) {
            __CODEMethodInfo = reinterpret_cast<uintptr_t>(hModule) +
                             sectionHeader->VirtualAddress;
            __CODEMethodInfoSize = sectionHeader->Misc.VirtualSize;
        } else if (strncmp(secName, ".codeinitF", sizeof(".codeinitF") - 1) == 0) {
            __CODEGlobalInitFunc = reinterpret_cast<uintptr_t>(hModule) +
                             sectionHeader->VirtualAddress;
            __CODEGlobalInitFuncSize = sectionHeader->Misc.VirtualSize;
        } else if (strncmp(secName, ".codespdct", sizeof(".codespdct") - 1) == 0) {
            __CODEStringPoolDict = reinterpret_cast<uintptr_t>(hModule) +
                             sectionHeader->VirtualAddress;
            __CODEStringPoolDictSize = sectionHeader->Misc.VirtualSize;
        } else if (strncmp(secName, ".codesp", sizeof(".codesp") - 1) == 0) {
            __CODEStringPool = reinterpret_cast<uintptr_t>(hModule) +
                             sectionHeader->VirtualAddress;
            __CODEStringPoolSize = sectionHeader->Misc.VirtualSize;
        } else if (strncmp(secName, ".codesm", sizeof(".codesm") - 1) == 0) {
            __CODEStackMap = reinterpret_cast<uintptr_t>(hModule) +
                             sectionHeader->VirtualAddress;
            __CODEStackMapSize = sectionHeader->Misc.VirtualSize;
        } else if (strncmp(secName, ".codegctib", sizeof(".codegctib") - 1) == 0) {
            __CODEGCTib = reinterpret_cast<uintptr_t>(hModule) +
                             sectionHeader->VirtualAddress;
            __CODEGCTibSize = sectionHeader->Misc.VirtualSize;
        } else if (strncmp(secName, ".codegcrts", sizeof(".codegcrts") - 1) == 0) {
            __CODEGCRoots = reinterpret_cast<uintptr_t>(hModule) +
                             sectionHeader->VirtualAddress;
            __CODEGCRootsSize = sectionHeader->Misc.VirtualSize;
        } else if (strncmp(secName, ".codett", sizeof(".codett") - 1) == 0) {
            __CODETypeTemplate = reinterpret_cast<uintptr_t>(hModule) +
                             sectionHeader->VirtualAddress;
            __CODETypeTemplateSize = sectionHeader->Misc.VirtualSize;
        } else if (strncmp(secName, ".codeti", sizeof(".codeti") - 1) == 0) {
            __CODETypeInfo = reinterpret_cast<uintptr_t>(hModule) +
                             sectionHeader->VirtualAddress;
            __CODETypeInfoSize = sectionHeader->Misc.VirtualSize;
        } else if (strncmp(secName, ".codefield", sizeof(".codefield") - 1) == 0) {
            __CODETypeFields = reinterpret_cast<uintptr_t>(hModule) +
                             sectionHeader->VirtualAddress;
            __CODETypeFieldsSize = sectionHeader->Misc.VirtualSize;
        } else if (strncmp(secName, ".codemtlb", sizeof(".codemtlb") - 1) == 0) {
            __CODEMTable = reinterpret_cast<uintptr_t>(hModule) +
                             sectionHeader->VirtualAddress;
            __CODEMTableSize = sectionHeader->Misc.VirtualSize;
        } else if (strncmp(secName, ".codeinty", sizeof(".codeinty") - 1) == 0) {
            __CODEInnerTypeExtensions = reinterpret_cast<uintptr_t>(hModule) +
                             sectionHeader->VirtualAddress;
            __CODEInnerTypeExtensionsSize = sectionHeader->Misc.VirtualSize;
        } else if (strncmp(secName, ".codeouty", sizeof(".codeouty") - 1) == 0) {
            __CODEOuterTypeExtensions = reinterpret_cast<uintptr_t>(hModule) +
                             sectionHeader->VirtualAddress;
            __CODEOuterTypeExtensionsSize = sectionHeader->Misc.VirtualSize;
        } else if (strncmp(secName, ".codesgt", sizeof(".codesgt") - 1) == 0) {
            __CODEStaticGITable = reinterpret_cast<uintptr_t>(hModule) +
                             sectionHeader->VirtualAddress;
            __CODEStaticGITableSize = sectionHeader->Misc.VirtualSize;
        } else if (strncmp(secName, ".codegcflg", sizeof(".codegcflg") - 1) == 0) {
            __CODEGCFlags = reinterpret_cast<uintptr_t>(hModule) +
                             sectionHeader->VirtualAddress;
            __CODEGCFlagsSize = sectionHeader->Misc.VirtualSize;
        } else if (strncmp(secName, ".coderflp", sizeof(".coderflp") - 1) == 0) {
            __CODEReflectPkgInfo = reinterpret_cast<uintptr_t>(hModule) +
                             sectionHeader->VirtualAddress;
            __CODEGCReflectPkgInfoSize = sectionHeader->Misc.VirtualSize;
        } else if (strncmp(secName, ".coderflv", sizeof(".coderflv") - 1) == 0) {
            __CODEReflectGV = reinterpret_cast<uintptr_t>(hModule) +
                             sectionHeader->VirtualAddress;
            __CODEReflectGVSize = sectionHeader->Misc.VirtualSize;
        } else if (strncmp(secName, ".coderflg", sizeof(".coderflg") - 1) == 0) {
            __CODEReflectGI = reinterpret_cast<uintptr_t>(hModule) +
                             sectionHeader->VirtualAddress;
            __CODEReflectGISize = sectionHeader->Misc.VirtualSize;
        } else if (strncmp(secName, ".codetpe", sizeof(".codetpe") - 1) == 0) {
            __CODETypeExt = reinterpret_cast<uintptr_t>(hModule) +
                             sectionHeader->VirtualAddress;
            __CODETypeExtSize = sectionHeader->Misc.VirtualSize;
        }
        ++sectionHeader;
    }

    std::initializer_list<uintptr_t> addrs{
        __CODESDKVersion,     reinterpret_cast<uintptr_t>(__CODESDKVersion + __CODESDKVersionSize),
        __CODEMethodInfo,     reinterpret_cast<uintptr_t>(__CODEMethodInfo + __CODEMethodInfoSize),
        __CODEGlobalInitFunc, reinterpret_cast<uintptr_t>(__CODEGlobalInitFunc + __CODEGlobalInitFuncSize),
        __CODEStringPoolDict, reinterpret_cast<uintptr_t>(__CODEStringPoolDict + __CODEStringPoolDictSize),
        __CODEStringPool,     reinterpret_cast<uintptr_t>(__CODEStringPool + __CODEStringPoolSize),
        __CODEStackMap,       reinterpret_cast<uintptr_t>(__CODEStackMap + __CODEStackMapSize),
        __CODEGCTib,          reinterpret_cast<uintptr_t>(__CODEGCTib + __CODEGCTibSize),
        __CODEGCRoots,        reinterpret_cast<uintptr_t>(__CODEGCRoots + __CODEGCRootsSize),
        __CODETypeTemplate,        reinterpret_cast<uintptr_t>(__CODETypeTemplate + __CODETypeTemplateSize),
        __CODETypeInfo,       reinterpret_cast<uintptr_t>(__CODETypeInfo + __CODETypeInfoSize),
        __CODETypeFields,      reinterpret_cast<uintptr_t>(__CODETypeFields + __CODETypeFieldsSize),
        __CODEMTable,         reinterpret_cast<uintptr_t>(__CODEMTable + __CODEMTableSize),
        __CODEInnerTypeExtensions, reinterpret_cast<uintptr_t>(__CODEInnerTypeExtensions + __CODEInnerTypeExtensionsSize),
        __CODEOuterTypeExtensions, reinterpret_cast<uintptr_t>(__CODEOuterTypeExtensions + __CODEOuterTypeExtensionsSize),
        __CODEStaticGITable,  reinterpret_cast<uintptr_t>(__CODEStaticGITable + __CODEStaticGITableSize),
        __CODEGCFlags,        reinterpret_cast<uintptr_t>(__CODEGCFlags + __CODEGCFlagsSize),
        __CODEReflectPkgInfo, reinterpret_cast<uintptr_t>(__CODEReflectPkgInfo + __CODEGCReflectPkgInfoSize),
        __CODEReflectGV,      reinterpret_cast<uintptr_t>(__CODEReflectGV + __CODEReflectGVSize),
        __CODEReflectGI,      reinterpret_cast<uintptr_t>(__CODEReflectGI + __CODEReflectGISize),
        __CODETypeExt,      reinterpret_cast<uintptr_t>(__CODETypeExt + __CODETypeExtSize),
    };
    uintptr_t start = std::min<uintptr_t>(addrs);
    uintptr_t end = std::max<uintptr_t>(addrs);
    __CODEMetadataSize = end - start;
}
}
