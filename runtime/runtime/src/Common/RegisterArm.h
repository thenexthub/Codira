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


#ifndef MRT_COMMON_REGISTER_H
#define MRT_COMMON_REGISTER_H
#include "Base/Log.h"
namespace MapleRuntime {
}
namespace Register {
enum RegisterId : uint32_t {
    R0,
    R1,
    R2,
    R3,
    R4,
    R5,
    R6,
    R7,
    R8,
    R9,
    R10,
    R11,
    R12,
    R13,
    R14,
    R15,
    D0,
    D1,
    D2,
    D3,
    D4,
    D5,
    D6,
    D7,
    D8,
    D9,
    D10,
    D11,
    D12,
    D13,
    D14,
    D15,
    D16,
    D17,
    D18,
    D19,
    D20,
    D21,
    D22,
    D23,
    D24,
    D25,
    D26,
    D27,
    D28,
    D29,
    D30,
    D31,
    REGISTERS_COUNT,
};
static inline const char* GetRegisterName(uint32_t idx)
{
    if (idx >= REGISTERS_COUNT) {
        return "wrong register";
    }
    const char* platformRegister[] = { "r0",  "r1",  "r2",  "r3",  "r4",  "r5",  "r6",  "r7",  "r8",  "r9",  "r10",
                                       "r11", "r12", "r13", "r14", "r15",
                                       "d0", "d1",  "d2",  "d3",  "d4",  "d5",  "d6",  "d7",  "d8",  "d9",  "d10",
                                       "d11", "d12", "d13", "d14", "d15", "d16", "d17", "d18", "d19", "d20", "d21",
                                       "d22", "d23", "d24", "d25", "d26", "d27", "d28", "d29", "d30", "d31" };
    return platformRegister[idx];
}
static inline uint32_t ResolveCalleeSaved(uint32_t idx)
{
    constexpr uint32_t calleeSavedNum = 17;
    constexpr RegisterId calleeSaved[calleeSavedNum] = { R4, R5, R6, R7, R8, R9, R10, R11, R14,
                                                         D8, D9, D10, D11, D12, D13, D14, D15 };
    if (idx >= calleeSavedNum) {
        LOG(RTLOG_FATAL, "register id: %u is not callee saved register", idx);
    }
    return calleeSaved[idx];
}
} // namespace Register
#endif // ~MRT_COMMON_REGISTER_H
