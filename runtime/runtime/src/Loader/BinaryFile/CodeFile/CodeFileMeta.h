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


#ifndef MRT_CODEFILE_META_H
#define MRT_CODEFILE_META_H
#include "ObjectModel/ExtensionData.h"
#include "ObjectModel/TypeExt.h"
namespace MapleRuntime {
enum CFileTable {
    // RW Section
    FUNC_DESC_TABLE,
    GLOBAL_INIT_FUNC_TABLE,
    STRING_POOL_DICT_TABLE,
    STRING_POOL_TABLE,
    STACK_MAP_TABLE, // StackMap Table
    GC_TIB_TABLE,
    GC_ROOT_TABLE,
    TYPE_TEMPLATE_TABLE,
    TYPE_INFO_TABLE,
    TYPE_FIELDS_TABLE,
    EXTENSION_DATA_TABLE,
    INNER_TYPE_EXTENSIONS_TABLE,
    OUTER_TYPE_EXTENSIONS_TABLE,
    STATIC_GI_TABLE,
    GC_FLAGS_TABLE,
    PACKINFO_TABLE,
    REFLECT_GV_TABLE,
    REFLECT_GI_TABLE,
    TYPE_EXT_TABLE,
    C_FILE_MAX
};

#pragma pack(4)
#ifdef __APPLE__
using TableDesc = struct {
    U64 *tableAddr;
    U32 *tableSize;
};
#else
using TableDesc = struct {
#if defined(_WIN64)
    U64 *tableAddr;
    U32 *tableSize;
#else
    U32 tableOffset;
    U32 tableSize;
#endif
};
#endif
#pragma pack()

#ifdef __APPLE__
using CODEFileHeader = struct {
    U32 magic;
    U32 version;
    U64 checkSum;
    U32 *cJFileSize;
    U64 *cJSDKVersionPtr;
    TableDesc tables[C_FILE_MAX];
};
#elif defined(_WIN64)
using CODEFileHeader = struct {
    U32 magic;
    U32 version;
    U64 checkSum;
    U32 *cJFileSize;
    U64 *cJSDKVersionPtr;
    TableDesc tables[C_FILE_MAX];
};
#else
using CODEFileHeader = struct {
    U32 magic;
    U32 version;
    U32 checkSum;
    U32 cJFileSize;
    U32 cJSDKVersionOffset;
    TableDesc tables[C_FILE_MAX];
};
#endif

using CODEStackMapTable = struct {
    U32 stackMapTotalSize;
    void* stackMapBasePtr;
};

using CODETypeInfoTable = struct {
    U32 typeInfoTotalSize;
    TypeInfo* typeInfoBasePtr;
};

using CODETypeExtTable = struct {
    U32 typeExtTotalSize;
    TypeExt* typeExtBasePtr;
};

using CODEFuncDescTable = struct {
    U32 funcDescTotalSize;
    FuncDescRef funcDescBasePtr;
};

using CODEGlobalInitFuncTable = struct {
    U32 globalInitFuncTotalSize;
    Uptr* globalInitFuncBasePtr;
};

using CODEGcTibTable = struct {
    U32 gcTibTotalSize;
    void* gcTibBasePtr;
};

using CODEExtensionDataTable = struct {
    U32 extensionDataTotalSize;
    ExtensionData* extensionDataBasePtr;
};

using CODEInnerTypeExtensionTable = struct {
    U32 innerTyExtensionTotalSize;
    void *innerTyExtensionBasePtr;
};

using CODEOuterTypeExtensionTable = struct {
    U32 outerTyExtensionTotalSize;
    void *outerTyExtensionBasePtr;
};

using CODEStaticGITable = struct {
    U32 staticGITotalSize;
    void* staticGIBasePtr;
};

using CODEGCFlagsTable = struct {
    U8 withSafepoint;
    U8 withBarrier;
    U8 hasStackPointerMap;
};

using CODEPackageInfoTable = struct {
    U32 packageInfoTotalSize;
    Uptr packageInfoBasePtr;
};

using CODEFileMeta = struct CODEFileMetadata {
    CODEStackMapTable stackMapTbl;
    CODETypeInfoTable typeInfoTbl;
    CODEFuncDescTable funcDescTbl;
    CODEGlobalInitFuncTable globalInitFuncTbl;
    CODEGcTibTable gcTibTbl;
    CODEExtensionDataTable extensionDataTbl;
    CODEInnerTypeExtensionTable innerTyExtensionTbl;
    CODEOuterTypeExtensionTable outerTyExtensionTbl;
    CODEStaticGITable staticGITbl;
    CODEGCFlagsTable gcFlagsTbl;
    CODEPackageInfoTable packageInfoTbl;
    Uptr gcRootsAddr;
    U32 gcRootSize;
    CODETypeExtTable typeExtTbl;
};
} // namespace MapleRuntime
#endif // MRT_CODEFILE_META_H
