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


#ifndef MRT_STACK_METADATA_H
#define MRT_STACK_METADATA_H

#include "Base/Macros.h"
#include "Base/Types.h"
#include "Common/TypeDef.h"
#include "MangleNameHelper.h"
#include "ObjectModel/MFuncdesc.h"

namespace MapleRuntime {
// The function of this class is to encapsulate the parsing interface of
// all backstack metadata. All printing information can be obtained directly
// through this class when printing the call stack.
class StackMetadataHelper {
public:
    explicit StackMetadataHelper(const uint32_t* ip, const uint32_t* startPC, uint64_t* funcDesc)
        : funcPC(ip), funcStartAddress(reinterpret_cast<uintptr_t>(startPC)), funcDesc(funcDesc)
    {
        mangleNameHelper = new (std::nothrow)
            MangleNameHelper(reinterpret_cast<FuncDescRef>(funcDesc)->GetFuncName(),
                             StackTraceFormatFlag(reinterpret_cast<FuncDescRef>(funcDesc)->GetStackTraceFormat()));
    }

    explicit StackMetadataHelper(const FrameInfo& frameInfo)
        : funcPC(frameInfo.mFrame.GetIP()), funcStartAddress(reinterpret_cast<uintptr_t>(frameInfo.GetFuncStartPC()))
    {
#ifdef __APPLE__
        FuncDescRef tmpFuncDesc = MFuncDesc::GetFuncDesc(frameInfo.mFrame.GetFA());
#else
        FuncDescRef tmpFuncDesc = MFuncDesc::GetFuncDesc(reinterpret_cast<Uptr>(frameInfo.GetFuncStartPC()));
#endif
        mangleNameHelper = new (std::nothrow)
            MangleNameHelper(tmpFuncDesc->GetFuncName(), StackTraceFormatFlag(tmpFuncDesc->GetStackTraceFormat()));
        CHECK_DETAIL(mangleNameHelper != nullptr, "new mangleNameHelper failed when create StackMetadataHelper.");
        funcDesc = reinterpret_cast<uint64_t*>(tmpFuncDesc);
    }

    StackMetadataHelper() = delete;

    ~StackMetadataHelper()
    {
        if (mangleNameHelper != nullptr) {
            delete mangleNameHelper;
            mangleNameHelper = nullptr;
        }
    };

    // Get file level line number.
    uint32_t GetLineNumber() const;

    // Get file path information.
    CString GetFilePath() const { return reinterpret_cast<FuncDescRef>(funcDesc)->GetFuncDir(); }

    // Get file name information.
    CString GetFileName() const { return reinterpret_cast<FuncDescRef>(funcDesc)->GetFuncFilename(); }

    // Get file path and file name.
    CString GetFilePathAndName() const
    {
        CString filePath = GetFilePath();
        CString fileName = GetFileName();
#ifdef _WIN64
        CString slash = "\\";
#else
        CString slash = "/";
#endif
        return filePath.IsEmpty() ? fileName : filePath + slash + fileName;
    }

    MangleNameHelper* GetMangleNameHelper() const { return mangleNameHelper; }

    // Return Mangle Infomation.
    CString GetMangleName() const { return mangleNameHelper->GetMangleName(); }

    // Demangle function name.
    CString GetDemangleName() const { return mangleNameHelper->GetDemangleName(); }

    // Demangle function name and get method name from it.
    CString GetDemangleMethodName() const { return mangleNameHelper->GetMethodName(); }

    // Demangle function name and get class name from it.
    CString GetDemangleClassName() const { return mangleNameHelper->GetClassName(); }

    // used for exception backtrace.
    bool IsNeedFiltExceptionCreationLayer() const { return mangleNameHelper->IsNeedFilt(); }

private:
    // function pc address.
    const uint32_t* funcPC;

    // Demangle information helper.
    MangleNameHelper* mangleNameHelper = nullptr;

    // function start address
    uintptr_t funcStartAddress = 0;

    DISABLE_CLASS_COPY_AND_ASSIGN(StackMetadataHelper);

    uint64_t* funcDesc;
};
} // namespace MapleRuntime
#endif // MRT_STACK_METADATA_H
