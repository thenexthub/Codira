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


#ifndef MRT_MFUNC_DESC_H
#define MRT_MFUNC_DESC_H

#include "Common/Dataref.h"
#include "Common/StackType.h"

namespace MapleRuntime {
class MFuncDesc {
public:
    inline Uptr* GetStackMap() const;
    inline U32 GetCodeSize() const;
    inline Uptr* GetEHTable() const;
    inline CString GetFuncName() const;
    inline CString GetFuncDir() const;
    inline CString GetFuncFilename() const;
    inline int8_t GetStackTraceFormat() const;
    CString GetStringFromDict(U32 offset) const;

    static FuncDescRef GetFuncDesc(FrameAddress *fa);
    static FuncDescRef GetFuncDesc(Uptr startPC);

private:
    DataRefOffset32<Uptr> stackMap;
    U32 codeSize;
    U32 name;
    U32 directory;
    U32 filename;
    U32 dictOffsets;
#ifdef __APPLE__
    DataRefOffset64<Uptr> ehTable;
#else
    DataRefOffset32<Uptr> ehTable;
#endif

    DISABLE_CLASS_IMPLICIT_CONSTRUCTORS(MFuncDesc);
    DISABLE_CLASS_IMPLICIT_DESTRUCTION(MFuncDesc);

    static constexpr U32 STACK_OFFSET_IN_APPLE = 16u;
    static constexpr uint32_t START_PC_OFFSET = 4u;
};
} // namespace MapleRuntime
#endif // MRT_MFUNC_DESC_H
