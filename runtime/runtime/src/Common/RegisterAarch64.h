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
// aarch64 register map
// | id |    register   | id |   register   | id |   register   | id |   register   |
// |  0 |       x0      |  8 |      x8      | 16 |     x16      | 24 |     x24      |
// |  1 |       x1      |  9 |      x9      | 17 |     x17      | 25 |     x25      |
// |  2 |       x2      | 10 |     x10      | 18 |     x18      | 26 |     x26      |
// |  3 |       x3      | 11 |     x11      | 19 |     x19      | 27 |     x27      |
// |  4 |       x4      | 12 |     x12      | 20 |     x20      | 28 |     x28      |
// |  5 |       x5      | 13 |     x13      | 21 |     x21      | 29 |     x29      |
// |  6 |       x6      | 14 |     x14      | 22 |     x22      | 30 |     x30      |
// |  7 |       x7      | 15 |     x15      | 23 |     x23      | 31 |     x31      |
// |--------------------------------------------------------------------------------|
// | 32 |       d0      | 40 |      d8      | 48 |     d16      | 56 |     d24      |
// | 33 |       d1      | 41 |      d9      | 49 |     d17      | 57 |     d25      |
// | 34 |       d2      | 42 |     d10      | 50 |     d18      | 58 |     d26      |
// | 35 |       d3      | 43 |     d11      | 51 |     d19      | 59 |     d27      |
// | 36 |       d4      | 44 |     d12      | 52 |     d20      | 60 |     d28      |
// | 37 |       d5      | 45 |     d13      | 53 |     d21      | 61 |     d29      |
// | 38 |       d6      | 46 |     d14      | 54 |     d22      | 62 |     d30      |
// | 39 |       d7      | 47 |     d15      | 55 |     d23      | 63 |     d31      |
}
namespace Register {
enum RegisterId : uint32_t {
    X0,
    X1,
    X2,
    X3,
    X4,
    X5,
    X6,
    X7,
    X8,
    X9,
    X10,
    X11,
    X12,
    X13,
    X14,
    X15,
    X16,
    X17,
    X18,
    X19,
    X20,
    X21,
    X22,
    X23,
    X24,
    X25,
    X26,
    X27,
    X28,
    X29,
    X30,
    X31,
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
    const char* platformRegister[] = { "x0",  "x1",  "x2",  "x3",  "x4",  "x5",  "x6",  "x7",  "x8",  "x9",  "x10",
                                       "x11", "x12", "x13", "x14", "x15", "x16", "x17", "x18", "x19", "x20", "x21",
                                       "x22", "x23", "x24", "x25", "x26", "x27", "x28", "x29", "x30", "x31", "d0",
                                       "d1",  "d2",  "d3",  "d4",  "d5",  "d6",  "d7",  "d8",  "d9",  "d10", "d11",
                                       "d12", "d13", "d14", "d15", "d16", "d17", "d18", "d19", "d20", "d21", "d22",
                                       "d23", "d24", "d25", "d26", "d27", "d28", "d29", "d30", "d31" };
    return platformRegister[idx];
}
static inline uint32_t ResolveCalleeSaved(uint32_t idx)
{
    constexpr uint32_t calleeSavedNum = 20;
    constexpr RegisterId calleeSaved[calleeSavedNum] = { X19, X20, X21, X22, X23, X24, X25, X26, X27, X28,
                                                         X29, X30, D8,  D9,  D10, D11, D12, D13, D14, D15 };
    if (idx >= calleeSavedNum) {
        LOG(RTLOG_FATAL, "register id: %u is not callee saved register", idx);
    }
    return calleeSaved[idx];
}
} // namespace Register
#endif // ~MRT_COMMON_REGISTER_H
