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


#ifndef MRT_STACKMAP_TYPE_DEF_H
#define MRT_STACKMAP_TYPE_DEF_H
#include <functional>
#include <list>
#include <unordered_map>
#include <vector>

#include "Common/BaseObject.h"
#ifdef __aarch64__
#include "Common/RegisterAarch64.h"
#elif defined(__arm__)
#include "Common/RegisterArm.h"
#else
#include "Common/RegisterX86-64.h"
#endif
namespace MapleRuntime {
using SlotAddress = ObjectRef*;
using SlotBias = I32;
using PCOff = U32;
using LineNum = U32;
using SlotBits = U32;
using BitsMapSize = U32;
using SlotDebugVisitor = std::function<void(SlotBias, BaseObject*)>;
using RegisterNum = uint32_t;
using RegDebugVisitor = std::function<void(RegisterNum, const BaseObject*)>;
constexpr uintptr_t METHOD_DESC_OFFSET = 4;
using RegBits = uint32_t;
using PrologueBits = uint16_t;
using PrologueBias = int16_t;
using namespace Register;
using BasePtrType = Uptr;
using DerivedPtrType = Uptr;
constexpr U32 PURE_COMPRESSED_STACKMAP = 0;
// derivedptr visitor parameters: basePtr, the reference of derivedptr
using DerivedPtrVisitor = std::function<void(BasePtrType, DerivedPtrType&)>;
using DerivedPtrDebugVisitor = std::function<void(BasePtrType, DerivedPtrType)>;
struct RegSlotsMap {
    SlotAddress addrMap[REGISTERS_COUNT]{ nullptr };
    bool isRecorded[REGISTERS_COUNT]{ false };
    bool HasReg(RegisterNum reg) const { return isRecorded[reg]; }
    inline void Insert(RegisterNum reg, SlotAddress slot)
    {
        isRecorded[reg] = true;
        addrMap[reg] = slot;
    }

    inline void Erase(RegisterNum reg) { isRecorded[reg] = false; }

    bool VisitSingleSlotsRoot(const RootVisitor& visitor, const RegDebugVisitor& debugFunc, RegisterNum reg,
                              std::list<Uptr>* rootsList = nullptr)
    {
        if (!HasReg(reg)) {
            LOG(RTLOG_ERROR, "register %s is not recorded", GetRegisterName(reg));
            return false;
        }
        if (rootsList != nullptr) {
            rootsList->push_back(reinterpret_cast<Uptr>(addrMap[reg]->object));
        }
        if (debugFunc != nullptr) {
            debugFunc(reg, addrMap[reg]->object);
        }
        visitor(*addrMap[reg]);
        Erase(reg);
        return true;
    }

    bool VisitDoubleSlotsRoot(const RootVisitor& visitor, const RegDebugVisitor& debugFunc, RegisterNum reg)
    {
        if (!HasReg(reg)) {
            LOG(RTLOG_ERROR, "register %s is not recorded", GetRegisterName(reg));
            return false;
        }
        SlotAddress slot = addrMap[reg];
        if (debugFunc != nullptr) {
            debugFunc(reg, slot->object);
            debugFunc(reg, (slot + 1)->object);
        }
        visitor(*slot);
        visitor(*(slot + 1));
        Erase(reg);
        return true;
    }
};
} // namespace MapleRuntime
#endif // ~MRT_STACKMAP_TYPE_DEF_H
