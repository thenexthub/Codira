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


#ifndef MRT_MFUNC_DESC_INLINE_H
#define MRT_MFUNC_DESC_INLINE_H

#include "Common/StackType.h"
#include "MFuncdesc.h"

namespace MapleRuntime {
inline Uptr* MFuncDesc::GetStackMap() const { return stackMap.GetDataRef(); }

inline U32 MFuncDesc::GetCodeSize() const { return codeSize; }

inline Uptr* MFuncDesc::GetEHTable() const
{
#ifdef __APPLE__
    return reinterpret_cast<Uptr*>(ehTable.refOffset);
#else
    return ehTable.GetDataRef();
#endif
}

inline CString MFuncDesc::GetFuncName() const { return GetStringFromDict(name); }

inline CString MFuncDesc::GetFuncDir() const { return GetStringFromDict(directory); }

inline CString MFuncDesc::GetFuncFilename() const { return GetStringFromDict(filename); }

inline int8_t MFuncDesc::GetStackTraceFormat() const
{
    Uptr base = reinterpret_cast<Uptr>(this);
    // 1: stack trace format flag size, 1 bytes
    return *(reinterpret_cast<const int8_t*>(dictOffsets + base - 1));
}

inline FuncDescRef MFuncDesc::GetFuncDesc(FrameAddress* fa)
{
    return reinterpret_cast<FuncDescRef>(
        *reinterpret_cast<U64*>(reinterpret_cast<uintptr_t>(fa) - STACK_OFFSET_IN_APPLE));
}

inline FuncDescRef MFuncDesc::GetFuncDesc(Uptr startPC)
{
    DataRefOffset32<MFuncDesc>* offset =
        reinterpret_cast<DataRefOffset32<MFuncDesc>*>(startPC - START_PC_OFFSET);
    return offset->GetDataRef();
}
} // namespace MapleRuntime
#endif // MRT_MFUNC_DESC_INLINE_H
