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


#ifndef MRT_CODEFILE_H
#define MRT_CODEFILE_H
#include "Base/Types.h"
#include "BaseFile.h"
#include "CodeFileMeta.h"

namespace MapleRuntime {
class CODEFile : public BaseFile {
public:
    explicit CODEFile(CString path, Uptr addr) : BaseFile(path), cJFileMetaBegin(addr){};
    ~CODEFile() override = default;

    void RegisterFile() override;
    void UnregisterFile() override;

    void LoadCODEFileMeta();
    Uptr GetFileMetaAddr() const override;
    const CODEFileMeta& GetCODEFileMeta() const;
    bool IsAddrInCODEFile(Uptr addr) const override;
    Uptr GetPackageInfoBase() override;
    U32 GetPackageInfoTotalSize() override;
    Uptr GetExtensionDataBase() override;
    U32 GetExtensionDataSize() override;
    Uptr GetInnerTypeExtensionsBase() override;
    U32 GetInnerTypeExtensionsSize() override;
    Uptr GetOuterTypeExtensionsBase() override;
    U32 GetOuterTypeExtensionsSize() override;
    Uptr GetStaticGIBase() override;
    U32 GetStaticGISize() override;
    Uptr GetTypeInfoBase() override;
    U32 GetTypeInfoTotalSize() override;
    Uptr GetTypeExtBase() override;
    U32 GetTypeExtTotalSize() override;
    CString GetSDKVersion() const override;

    void GetGlobalInitFunc(std::vector<Uptr> &globalInitFuncs) const override;
#if defined(_WIN64)
    void LoadWinCODEFileMeta();
#elif defined(__APPLE__)
    void LoadMacCODEFileMeta();
#else
    void LoadLinuxCODEFileMeta();
#endif

private:
    Uptr* GetGlobalInitFuncPtr(const CODEFileMeta& cFileMeta) const;
    U32 GetGlobalInitFuncSize(const CODEFileMeta& cFileMeta) const;
    CODEFileMeta cJFileMeta;
    Uptr cJFileMetaBegin;
    Uptr cJFileMetaEnd;
};
using CODEFileRef = CODEFile*;
} // namespace MapleRuntime
#endif // MRT_CODEFILE_H
