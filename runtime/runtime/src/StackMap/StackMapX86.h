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


#ifndef MRT_STACKMAP_X86_64_H
#define MRT_STACKMAP_X86_64_H
#include <vector>

#include "Common/RegisterX86-64.h"
#include "StackMap/StackMapTypeDef.h"

namespace MapleRuntime {
class RegRoot {
public:
    RegRoot() = default;
    explicit RegRoot(U32 bits) : regBits(static_cast<RegBits>(bits)) {}
    ~RegRoot() = default;

    RegRoot(const RegRoot& other) : regBits(other.regBits) {}
    RegRoot(RegRoot&& other) : regBits(other.regBits) { other.regBits = 0; }
    RegRoot& operator=(const RegRoot& other)
    {
        if (this == &other) {
            return *this;
        }
        regBits = other.regBits;
        return *this;
    }
    RegRoot& operator=(RegRoot&& other)
    {
        if (this == &other) {
            return *this;
        }
        regBits = other.regBits;
        other.regBits = 0;
        return *this;
    }

    static void RecordStubCalleeSaved(RegSlotsMap& regSlotsMap, Uptr fp)
    {
        constexpr Uptr slotLength = 8;
        Uptr slotAddr = fp - slotLength;
#ifdef _WIN64
        static constexpr RegisterId calleeSavedRegister[] = { R15, R14, R13, R12, RDI, RSI, RBX };
#else
        static constexpr RegisterId calleeSavedRegister[] = { R15, R14, R13, R12, RBX };
#endif
        for (auto reg : calleeSavedRegister) {
            regSlotsMap.Insert(reg, reinterpret_cast<SlotAddress>(slotAddr));
            slotAddr -= slotLength;
        }
    }

#ifdef _WIN64
    static void RecordStubAllRegister(RegSlotsMap& regSlotsMap, Uptr fp)
    {
        constexpr Uptr universalSlotLength = 8;
        constexpr Uptr xmmSlotLength = 16;
        constexpr U32 stubPushNum = 14;
        constexpr RegisterId stubPushOrder[stubPushNum] = { RAX, RBX, RCX, RDX, RDI, RSI, R8,
                                                            R9,  R10, R11, R12, R13, R14, R15 };
        Uptr slotAddr = fp;
        for (U32 i = 0; i < stubPushNum; ++i) {
            slotAddr -= universalSlotLength;
            regSlotsMap.Insert(stubPushOrder[i], reinterpret_cast<SlotAddress>(slotAddr));
        }

        for (U8 i = XMM0; i <= XMM15; ++i) {
            slotAddr -= xmmSlotLength;
            regSlotsMap.Insert(i, reinterpret_cast<SlotAddress>(slotAddr));
        }
    }
#else
    static void RecordStubAllRegister(RegSlotsMap& regSlotsMap, Uptr fp)
    {
        constexpr Uptr universalSlotLength = 8;
        constexpr Uptr xmmSlotLength = 16;
        constexpr Uptr stackDepth = 264; // defined in signalHandlerStub_x86_64.S
        constexpr U32 stubPushNum = 15;
        constexpr RegisterId stubPushOrder[stubPushNum] = { RAX, RBX, RCX, RDX, RDI, RSI, RSP, R8,
                                                            R9,  R10, R11, R12, R13, R14, R15 };
        Uptr slotAddr = fp - universalSlotLength;
        for (U32 i = 0; i < stubPushNum; ++i, slotAddr -= universalSlotLength) {
            regSlotsMap.Insert(stubPushOrder[i], reinterpret_cast<SlotAddress>(slotAddr));
        }

        Uptr sp = slotAddr - stackDepth;
        slotAddr = sp;
        for (U8 i = XMM0; i <= XMM15; ++i, slotAddr += xmmSlotLength) {
            regSlotsMap.Insert(i, reinterpret_cast<SlotAddress>(slotAddr));
        }
    }
#endif

    bool VisitGCRoots(const RootVisitor& visitor, const RegDebugVisitor& debugFunc, RegSlotsMap& regSlotsMap,
                      std::list<Uptr>* rootsList = nullptr) const
    {
        RegBits bits = regBits;
        // Visit universal register roots
        for (RegisterNum i = RAX; i <= R15; ++i, bits >>= 1) {
            if (bits == 0) {
                return true;
            }
            if ((bits & LOWEST_BIT) == 0) {
                continue;
            }
            if (!regSlotsMap.VisitSingleSlotsRoot(visitor, debugFunc, i, rootsList)) {
                return false;
            }
        }
        // Visit xmm register roots.
        for (RegisterNum i = XMM0; i <= XMM15; ++i, bits >>= 1) {
            if (bits == 0) {
                return true;
            }
            if ((bits & LOWEST_BIT) == 0) {
                continue;
            }
            if (!regSlotsMap.VisitDoubleSlotsRoot(visitor, debugFunc, i)) {
                return false;
            }
        }
        return true;
    }

    static void RecordRegs(RegSlotsMap& regSlotsMap, Uptr fp)
    {
        RecordStubAllRegister(regSlotsMap, fp);
    }
private:
    static constexpr RegBits LOWEST_BIT = 0x1;
    RegBits regBits{ 0 };
};
} // namespace MapleRuntime
#endif // MRT_STACKMAP_X86_64_H
