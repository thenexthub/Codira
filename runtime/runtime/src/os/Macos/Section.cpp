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


#include <algorithm>
#include <cstdint>
#include <dlfcn.h>
#include <initializer_list>
#include <mach-o/dyld.h>
#include <mach-o/getsect.h>
#include <cassert>

extern "C" {
// CODEMetadataHeader
unsigned long _CODEMetadataSize;
uintptr_t _CODEMetadataStart;

// CODESDKVersion
unsigned long _CODESDKVersionSize;
uintptr_t _CODESDKVersion;

// CODEMethodInfo
unsigned long _CODEMethodInfoSize;
uintptr_t _CODEMethodInfo;

// CODEGlobalInitFunc
unsigned long _CODEGlobalInitFuncSize;
uintptr_t _CODEGlobalInitFunc;

// CODEStringPoolDict
unsigned long _CODEStringPoolDictSize;
uintptr_t _CODEStringPoolDict;

// CODEStringPool
unsigned long _CODEStringPoolSize;
uintptr_t _CODEStringPool;

// CODEStackMap
unsigned long _CODEStackMapSize;
uintptr_t _CODEStackMap;

// CODEGCTib
unsigned long _CODEGCTibSize;
uintptr_t _CODEGCTib;

// CODEGCRoots
unsigned long _CODEGCRootsSize;
uintptr_t _CODEGCRoots;

// CODETypeTemplate
unsigned long _CODETypeTemplateSize;
uintptr_t _CODETypeTemplate;

// CODETypeInfo
unsigned long _CODETypeInfoSize;
uintptr_t _CODETypeInfo;

// CODETypeInfoFields
unsigned long _CODETypeFieldsSize;
uintptr_t _CODETypeFields;

// CODEMTable
unsigned long _CODEMTableSize;
uintptr_t _CODEMTable;

unsigned long _CODEInnerTypeExtensionsSize;
uintptr_t _CODEInnerTypeExtensions;

unsigned long _CODEOuterTypeExtensionsSize;
uintptr_t _CODEOuterTypeExtensions;

// Static GI
unsigned long _CODEStaticGITableSize;
uintptr_t _CODEStaticGITable;

// CODEGCFlags
unsigned long _CODEGCFlagsSize;
uintptr_t _CODEGCFlags;

// CODEReflectPkgInfo
unsigned long _CODEGCReflectPkgInfoSize;
uintptr_t _CODEReflectPkgInfo;

// CODEReflectGV
unsigned long _CODEReflectGVSize;
uintptr_t _CODEReflectGV;

// CODEReflectGI
unsigned long _CODEReflectGISize;
uintptr_t _CODEReflectGI;

// CODETypeExt
unsigned long _CODETypeExtSize;
uintptr_t _CODETypeExt;

int _CODEMetaDataSize;

unsigned long* g_runtimeStaticStart;
unsigned long* g_runtimeStaticEnd;

static void InitSectionInformation(const struct mach_header_64* mhp)
{
    _CODEMetadataStart =
        reinterpret_cast<uintptr_t>(getsectiondata(mhp, "__CODEMETAHEADER", "__codemetaheader", &_CODEMetadataSize));
    _CODESDKVersion =
        reinterpret_cast<uintptr_t>(getsectiondata(mhp, "__CODE_METADATA", "__codesdkversion", &_CODESDKVersionSize));
    _CODEMethodInfo =
        reinterpret_cast<uintptr_t>(getsectiondata(mhp, "__CODE_METADATA", "__codemethodinfo", &_CODEMethodInfoSize));
    _CODEGlobalInitFunc =
        reinterpret_cast<uintptr_t>(getsectiondata(mhp, "__CODE_METADATA", "__codeglobalFunc", &_CODEGlobalInitFuncSize));
    _CODEStringPoolDict =
        reinterpret_cast<uintptr_t>(getsectiondata(mhp, "__CODE_METADATA", "__codestrpooldict", &_CODEStringPoolDictSize));
    _CODEStringPool =
        reinterpret_cast<uintptr_t>(getsectiondata(mhp, "__CODE_METADATA", "__codestrpool", &_CODEStringPoolSize));
    _CODEStackMap = reinterpret_cast<uintptr_t>(getsectiondata(mhp, "__CODE_METADATA", "__codestackmap", &_CODEStackMapSize));
    _CODEGCTib = reinterpret_cast<uintptr_t>(getsectiondata(mhp, "__CODE_METADATA", "__codegctib", &_CODEGCTibSize));
    _CODEGCRoots = reinterpret_cast<uintptr_t>(getsectiondata(mhp, "__CODE_METADATA", "__codegcroots", &_CODEGCRootsSize));
    _CODETypeTemplate =
        reinterpret_cast<uintptr_t>(getsectiondata(mhp, "__CODE_METADATA", "__codetemplate", &_CODETypeTemplateSize));
    _CODETypeInfo = reinterpret_cast<uintptr_t>(getsectiondata(mhp, "__CODE_METADATA", "__codetypeinfo", &_CODETypeInfoSize));
    _CODETypeFields =
        reinterpret_cast<uintptr_t>(getsectiondata(mhp, "__CODE_METADATA", "__code_fields", &_CODETypeFieldsSize));
    _CODEMTable = reinterpret_cast<uintptr_t>(getsectiondata(mhp, "__CODE_METADATA", "__codemtable", &_CODEMTableSize));
    _CODEInnerTypeExtensions = reinterpret_cast<uintptr_t>(
        getsectiondata(mhp, "__CODE_METADATA", "__codeinnerty_eds", &_CODEInnerTypeExtensionsSize));
    _CODEOuterTypeExtensions = reinterpret_cast<uintptr_t>(
        getsectiondata(mhp, "__CODE_METADATA", "__codeouterty_eds", &_CODEOuterTypeExtensionsSize));
    _CODEStaticGITable =
        reinterpret_cast<uintptr_t>(getsectiondata(mhp, "__CODE_METADATA", "__codestatic_gi", &_CODEStaticGITableSize));
    _CODEGCFlags = reinterpret_cast<uintptr_t>(getsectiondata(mhp, "__CODE_METADATA", "__codegcflags", &_CODEGCFlagsSize));
    _CODEReflectPkgInfo =
        reinterpret_cast<uintptr_t>(getsectiondata(mhp, "__CODE_METADATA", "__coderef_pkginfo", &_CODEGCReflectPkgInfoSize));
    _CODEReflectGV = reinterpret_cast<uintptr_t>(getsectiondata(mhp, "__CODE_METADATA", "__coderef_gv", &_CODEReflectGVSize));
    _CODEReflectGI = reinterpret_cast<uintptr_t>(getsectiondata(mhp, "__CODE_METADATA", "__coderef_gi", &_CODEReflectGISize));
    _CODETypeExt = reinterpret_cast<uintptr_t>(getsectiondata(mhp, "__CODE_METADATA", "__codetype_ext", &_CODETypeExtSize));

    unsigned long _CODERuntimeTextSize;
    unsigned long _CODERuntimeText =
        reinterpret_cast<unsigned long>(getsectiondata(mhp, "__TEXT", "__codert_text", &_CODERuntimeTextSize));
    g_runtimeStaticStart = reinterpret_cast<unsigned long*>(_CODERuntimeText);
    g_runtimeStaticEnd = reinterpret_cast<unsigned long*>(_CODERuntimeText + _CODERuntimeTextSize);
}

__attribute__((constructor)) void InitData()
{
    // 1. Obtain the name of the current shared library based on the specified symbol.
    Dl_info info;
    const void* addr = reinterpret_cast<const void*>(&InitSectionInformation);
    dladdr(addr, &info);
    const char* dylib_name = info.dli_fname;

    // 2. Traverse all shared libraries and find the index of the current shared library.
    int count = _dyld_image_count();
    int index = -1;
    for (int i = 0; i < count; ++i) {
        const char* name = _dyld_get_image_name(i);
        if (strcmp(name, dylib_name) == 0) {
            index = i;
            break;
        }
    }

    // 3. Init section information use index
    assert(index != -1);
    const struct mach_header_64* mhp = reinterpret_cast<const struct mach_header_64*>(_dyld_get_image_header(index));
    InitSectionInformation(mhp);

    // 4. And last, init _CODEMetaDataSize
    std::initializer_list<uintptr_t> addrs{
        _CODESDKVersion,     reinterpret_cast<uintptr_t>(_CODESDKVersion + _CODESDKVersionSize),
        _CODEMethodInfo,     reinterpret_cast<uintptr_t>(_CODEMethodInfo + _CODEMethodInfoSize),
        _CODEGlobalInitFunc, reinterpret_cast<uintptr_t>(_CODEGlobalInitFunc + _CODEGlobalInitFuncSize),
        _CODEStringPoolDict, reinterpret_cast<uintptr_t>(_CODEStringPoolDict + _CODEStringPoolDictSize),
        _CODEStringPool,     reinterpret_cast<uintptr_t>(_CODEStringPool + _CODEStringPoolSize),
        _CODEStackMap,       reinterpret_cast<uintptr_t>(_CODEStackMap + _CODEStackMapSize),
        _CODEGCTib,          reinterpret_cast<uintptr_t>(_CODEGCTib + _CODEGCTibSize),
        _CODEGCRoots,        reinterpret_cast<uintptr_t>(_CODEGCRoots + _CODEGCRootsSize),
        _CODETypeTemplate,        reinterpret_cast<uintptr_t>(_CODETypeTemplate + _CODETypeTemplateSize),
        _CODETypeInfo,       reinterpret_cast<uintptr_t>(_CODETypeInfo + _CODETypeInfoSize),
        _CODETypeFields,      reinterpret_cast<uintptr_t>(_CODETypeFields + _CODETypeFieldsSize),
        _CODEMTable,         reinterpret_cast<uintptr_t>(_CODEMTable + _CODEMTableSize),
        _CODEInnerTypeExtensions, reinterpret_cast<uintptr_t>(_CODEInnerTypeExtensions + _CODEInnerTypeExtensionsSize),
        _CODEOuterTypeExtensions, reinterpret_cast<uintptr_t>(_CODEOuterTypeExtensions + _CODEOuterTypeExtensionsSize),
        _CODEStaticGITable,  reinterpret_cast<uintptr_t>(_CODEStaticGITable + _CODEStaticGITableSize),
        _CODEGCFlags,        reinterpret_cast<uintptr_t>(_CODEGCFlags + _CODEGCFlagsSize),
        _CODEReflectPkgInfo, reinterpret_cast<uintptr_t>(_CODEReflectPkgInfo + _CODEGCReflectPkgInfoSize),
        _CODEReflectGV,      reinterpret_cast<uintptr_t>(_CODEReflectGV + _CODEReflectGVSize),
        _CODEReflectGI,      reinterpret_cast<uintptr_t>(_CODEReflectGI + _CODEReflectGISize),
        _CODETypeExt,      reinterpret_cast<uintptr_t>(_CODETypeExt + _CODETypeExtSize),
    };
    uintptr_t start = std::min<uintptr_t>(addrs);
    uintptr_t end = std::max<uintptr_t>(addrs);
    _CODEMetaDataSize = end - start;
};
}
