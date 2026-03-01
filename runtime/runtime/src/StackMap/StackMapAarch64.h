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


#ifndef MRT_STACKMAP_AARCH64_H
#define MRT_STACKMAP_AARCH64_H
#include <stdint.h>
#include <unordered_map>

#include "StackMap/StackMapTypeDef.h"

namespace MapleRuntime {
class RegRoot {
public:
    RegRoot() = default;
    explicit RegRoot(RegBits bits) : regBits(bits) {}
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
        constexpr Uptr calleeSavedAreaOffset = 8 * 4;
        Uptr slotAddr = fp + calleeSavedAreaOffset;
        static constexpr RegisterId calleeSavedRegiser[] = { X19, X20, X21, X22, X23, X24, X25, X26, X27, X28 };
        for (auto reg : calleeSavedRegiser) {
            regSlotsMap.Insert(reg, reinterpret_cast<SlotAddress>(slotAddr));
            slotAddr += slotLength;
        }
    }

    static void RecordStubAllRegister(RegSlotsMap& regSlotsMap, Uptr fp)
    {
        constexpr Uptr slotLength = 8;
        constexpr Uptr registersAreaOffset = 8 * 2;
        Uptr slotAddr = fp + registersAreaOffset;
        for (RegisterNum i = X0; i <= X28; ++i, slotAddr += slotLength) {
            regSlotsMap.Insert(i, reinterpret_cast<SlotAddress>(slotAddr));
        }
    }

    bool VisitGCRoots(const RootVisitor& visitor, const RegDebugVisitor& debugFunc, RegSlotsMap& regSlotsMap,
                      std::list<Uptr>* rootsList = nullptr) const
    {
        RegBits bits = regBits;
        for (RegisterNum i = 0; bits != 0; ++i, bits >>= 1) {
            if ((bits & LOWEST_BIT) == 0) {
                continue;
            }
            if (!regSlotsMap.VisitSingleSlotsRoot(visitor, debugFunc, i, rootsList)) {
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
#endif // MRT_STACKMAP_AARCH64_H
