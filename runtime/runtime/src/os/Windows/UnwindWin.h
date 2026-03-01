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


#ifndef MRT_UNWIND_WIN_H
#define MRT_UNWIND_WIN_H

#include "Base/CString.h"
#include "Base/Types.h"
#include "Common/StackType.h"
#include "WinModuleManager.h"

namespace MapleRuntime {

enum UnwindOpCodes : uint32_t {
    PUSH_NON_VOL = 0,
    ALLOC_LARGE,
    ALLOC_SMALL,
    SET_FP_REG,
    SAVE_NON_VOL,
    SAVE_NON_VOL_FAR,
    EPILOG,
    SPARE_CODE,
    SAVE_XMM128,
    SAVE_XMM128_FAR,
    PUSH_MACH_FRAME
};

union UnwindCode {
    struct {
        uint8_t codeOffset;
        uint8_t unwindOpAndOpInfo;
    } u;
    uint16_t frameOffset;

    uint8_t GetUnwindOp() const
    {
        return u.unwindOpAndOpInfo & 0x0f; // 0f: 4bits for unwindOp
    }
    uint8_t GetOpInfo() const
    {
        return (u.unwindOpAndOpInfo >> 4) & 0x0f; // 4: 4bits for opInfo
    }
};

struct UnwindInfo {
    uint8_t versionAndFlags;
    uint8_t prologSize;
    uint8_t codesCount;
    uint8_t frameRegisterAndOffset;
    UnwindCode unwindCodes[1];

    uint8_t GetVersion() const
    {
        return versionAndFlags & 0x07; // 07: 5bits for flags
    }
    uint8_t GetFlags() const
    {
        return (versionAndFlags >> 3) & 0x1f; // 3: 3bits for version, 1f: 5bits for flags
    }
    uint8_t GetFrameRegister() const
    {
        return frameRegisterAndOffset & 0x0f; // 0f: 4bits for frameRegister
    }
    uint8_t GetFrameOffset() const
    {
        return (frameRegisterAndOffset >> 4) & 0x0f; // 4: 4bits for frameOffset
    }
};

__attribute__((__noinline__)) extern "C" void GetContextWin64(uint64_t* rip, uint64_t* rsp);
FrameInfo GetCurFrameInfo(WinModuleManager& winModuleManager, Uptr pc, Uptr sp);
FrameInfo GetCallerFrameInfo(WinModuleManager& winModuleManager, const MachineFrame& curFrame,
                             UnwindContextStatus& status);
uintptr_t GetCallerRsp(WinModuleManager& winModuleManager, const MachineFrame& curFrame);

} // namespace MapleRuntime
#endif
