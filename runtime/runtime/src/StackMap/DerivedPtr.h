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


#ifndef MRT_DERIVED_PTR_ROOT_H
#define MRT_DERIVED_PTR_ROOT_H
#include "StackMap/SlotRoot.h"
#include "StackMap/StackMapTable.h"
#ifdef __aarch64__
#include "StackMap/StackMapAarch64.h"
#elif defined (__arm__)
#include "StackMap/StackMapArm.h"
#else
#include "StackMap/StackMapX86.h"
#endif
namespace MapleRuntime {
class DerivedPtr {
public:
    DerivedPtr() = default;
    DerivedPtr(const DerivedPtrTable& derivePtr, const RegTable& reg, const SlotTable& slot, U32 startIdx)
        : derivePtrTable(derivePtr), regTable(reg), slotTable(slot), derivedPtrIdx(startIdx) {}
    ~DerivedPtr() = default;
    bool VisitDerivedPtr(const DerivedPtrVisitor& visitor, const DerivedPtrDebugVisitor debugVisitor,
                         RegSlotsMap& regSlotsMap, Uptr basePtr, Uptr fp)
    {
        if (LIKELY(derivedPtrIdx == 0)) {
            return false;
        }
        DerivedPtrPair idxPair = derivePtrTable.GetDerivePair(derivedPtrIdx - 1);
        U32 regIdx = idxPair.first;
        U32 slotIdx = idxPair.second;
        if (basePtr != 0) {
            if (regIdx != 0) {
                VisitRegDerivedPtr(visitor, debugVisitor, regSlotsMap, basePtr, regIdx - 1);
            }
            if (slotIdx != 0) {
                VisitSlotDerivedPtr(visitor, debugVisitor, basePtr, fp, slotIdx - 1);
            }
        }
        derivedPtrIdx++;
        return true;
    }
    bool VisitDerivedPtrForStackGrow(const DerivedPtrVisitor& visitor, const DerivedPtrDebugVisitor debugVisitor,
                                     RegSlotsMap& regSlotsMap, Uptr basePtr, Uptr fp)
    {
        if (LIKELY(derivedPtrIdx == 0)) {
            return false;
        }
        DerivedPtrPair idxPair = derivePtrTable.GetDerivePair(derivedPtrIdx - 1);
        U32 regIdx = idxPair.first;
        U32 slotIdx = idxPair.second;
        if (regIdx != 0) {
            VisitRegDerivedPtr(visitor, debugVisitor, regSlotsMap, basePtr, regIdx - 1);
        }
        if (slotIdx != 0) {
            VisitSlotDerivedPtr(visitor, debugVisitor, basePtr, fp, slotIdx - 1);
        }
        derivedPtrIdx++;
        return true;
    }

private:
    inline void VisitRegDerivedPtr(const DerivedPtrVisitor& visitor, const DerivedPtrDebugVisitor debugVisitor,
                                   RegSlotsMap& regSlotsMap, Uptr basePtr, U32 regIdx) const
    {
        RegRoot regRoot(regTable.GetActiveRegBits(regIdx));
        RootVisitor rootVisitor = [&visitor, basePtr](ObjectRef& derivedPtr) {
            visitor(basePtr, reinterpret_cast<Uptr&>(derivedPtr));
        };
        RegDebugVisitor regDebug = nullptr;
        (void)debugVisitor;
#if defined(GCINFO_DEBUG) && GCINFO_DEBUG
        if (debugVisitor != nullptr) {
            regDebug = [&debugVisitor, basePtr](RegisterNum, const BaseObject* derivedPtr) {
                debugVisitor(basePtr, reinterpret_cast<Uptr>(derivedPtr));
            };
        }
#endif
        regRoot.VisitGCRoots(rootVisitor, regDebug, regSlotsMap);
    }
    inline void VisitSlotDerivedPtr(const DerivedPtrVisitor& visitor, const DerivedPtrDebugVisitor debugVisitor,
                                    Uptr basePtr, Uptr fp, U32 slotIdx) const
    {
        SlotRoot slotRoot(slotTable.GetBaseOffset(slotIdx), slotTable.GetSlotBitMap(slotIdx), slotTable.slotFormat);
        RootVisitor rootVisitor = [&visitor, basePtr](ObjectRef& derivedPtr) {
            visitor(basePtr, reinterpret_cast<Uptr&>(derivedPtr.object));
        };
        SlotDebugVisitor slotDebug = nullptr;
        (void)debugVisitor;
#if defined(GCINFO_DEBUG) && GCINFO_DEBUG
        if (debugVisitor != nullptr) {
            slotDebug = [&debugVisitor, basePtr](SlotBias, BaseObject* derivedPtr) {
                debugVisitor(basePtr, reinterpret_cast<Uptr>(derivedPtr));
            };
        }
#endif
        slotRoot.VisitGCRoots(rootVisitor, slotDebug, fp);
    }
    DerivedPtrTable derivePtrTable;
    RegTable regTable;
    SlotTable slotTable;
    U32 derivedPtrIdx = 0;
};
} // namespace MapleRuntime
#endif // ~MRT_DERIVED_PTR_ROOT_H
