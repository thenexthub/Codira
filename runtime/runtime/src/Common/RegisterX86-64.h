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
// x86 regiser map
// | id |     register    | id |    register    | id |    register    | id |    register    | id |    register    |
// |  0 |       rax       |  8 |       r8       | 16 |      rip       | 24 |      xmm7      | 32 |      xmm15     |
// |  1 |       rdx       |  9 |       r9       | 17 |     xmm0       | 25 |      xmm8      |
// |  2 |       rcx       | 10 |      r10       | 18 |     xmm1       | 26 |      xmm9      |
// |  3 |       rbx       | 11 |      r11       | 19 |     xmm2       | 27 |     xmm10      |
// |  4 |       rsi       | 12 |      r12       | 20 |     xmm3       | 28 |     xmm11      |
// |  5 |       rdi       | 13 |      r13       | 21 |     xmm4       | 29 |     xmm12      |
// |  6 |       rbp       | 14 |      r14       | 22 |     xmm5       | 30 |     xmm13      |
// |  7 |       rsp       | 15 |      r15       | 23 |     xmm6       | 31 |     xmm14      |
namespace Register {
enum RegisterId : uint32_t {
    RAX,
    RDX,
    RCX,
    RBX,
    RSI,
    RDI,
    RBP,
    RSP,
    R8,
    R9,
    R10,
    R11,
    R12,
    R13,
    R14,
    R15,
    RIP,
    XMM0,
    XMM1,
    XMM2,
    XMM3,
    XMM4,
    XMM5,
    XMM6,
    XMM7,
    XMM8,
    XMM9,
    XMM10,
    XMM11,
    XMM12,
    XMM13,
    XMM14,
    XMM15,
    REGISTERS_COUNT,
};
static inline const char* GetRegisterName(uint32_t idx)
{
    if (idx >= REGISTERS_COUNT) {
        return "wrong register";
    }
    const char* platformRegister[] = { "rax",   "rdx",   "rcx",   "rbx",   "rsi",   "rdi",  "rbp",  "rsp",  "r8",
                                       "r9",    "r10",   "r11",   "r12",   "r13",   "r14",  "r15",  "rip",  "xmm0",
                                       "xmm1",  "xmm2",  "xmm3",  "xmm4",  "xmm5",  "xmm6", "xmm7", "xmm8", "xmm9",
                                       "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15" };
    return platformRegister[idx];
}
static inline uint32_t ResolveCalleeSaved(uint32_t idx)
{
#ifdef _WIN64
    constexpr uint32_t calleeSavedNum = 17;
    constexpr RegisterId calleeSaved[calleeSavedNum] = { RBX,  RSI,  RDI,   R12,   R13,   R14,   R15,   XMM6, XMM7,
                                                         XMM8, XMM9, XMM10, XMM11, XMM12, XMM13, XMM14, XMM15 };
#else
    constexpr uint32_t calleeSavedNum = 5;
    constexpr RegisterId calleeSaved[calleeSavedNum] = { RBX, R12, R13, R14, R15 };
#endif
    CHECK_DETAIL(idx < calleeSavedNum, "register id: %u is not callee saved register", idx);
    return calleeSaved[idx];
}
} // namespace Register
} // namespace MapleRuntime
#endif // ~MRT_COMMON_REGISTER_H
