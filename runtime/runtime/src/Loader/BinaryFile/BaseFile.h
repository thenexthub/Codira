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


#ifndef MRT_BASE_FILE_H
#define MRT_BASE_FILE_H

#include "Base/CString.h"
#include "Base/Types.h"
#include "os/Path.h"
namespace MapleRuntime {
enum class FileType {
    C_FILE, // CODIRA File
    UNKNOWN
};

class BaseFile {
public:
    explicit BaseFile(const CString nameStr) : realPath(nameStr)
    {
        baseName = Os::Path::GetBaseName(realPath.Str());
    }
    static BaseFile* CreateCODEFile(FileType type, CString filePath, Uptr cfileMetaAddr);
    virtual ~BaseFile() = default;
    virtual void RegisterFile() = 0;
    virtual void UnregisterFile() = 0;

    virtual bool IsAddrInCODEFile(Uptr addr) const = 0;
    virtual Uptr GetPackageInfoBase() = 0;
    virtual U32 GetPackageInfoTotalSize() = 0;
    virtual void GetGlobalInitFunc(std::vector<Uptr> &globalFuncs) const = 0;
    virtual Uptr GetFileMetaAddr() const = 0;
    virtual Uptr GetExtensionDataBase() = 0;
    virtual U32 GetExtensionDataSize() = 0;
    virtual Uptr GetInnerTypeExtensionsBase() = 0;
    virtual U32 GetInnerTypeExtensionsSize() = 0;
    virtual Uptr GetOuterTypeExtensionsBase() = 0;
    virtual U32 GetOuterTypeExtensionsSize() = 0;
    virtual Uptr GetStaticGIBase() = 0;
    virtual U32 GetStaticGISize() = 0;
    virtual Uptr GetTypeInfoBase() = 0;
    virtual U32 GetTypeInfoTotalSize() = 0;
    virtual Uptr GetTypeExtBase() = 0;
    virtual U32 GetTypeExtTotalSize() = 0;
    virtual CString GetSDKVersion() const = 0;

    virtual const CString& GetRealPath() const;
    const CString& GetBaseName() const;
    void SetFileCompatibility(bool isComp) { isCompatible = isComp; }
    bool IsCompatible() const { return isCompatible; }
private:
    CString realPath; // file real path
    CString baseName;
    bool isCompatible { false };
};
} // namespace MapleRuntime
#endif
