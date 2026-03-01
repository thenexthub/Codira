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


#ifndef MRT_COMPRESSED_STACKMAP_H
#define MRT_COMPRESSED_STACKMAP_H
#include "Common/Dataref.h"
#include "Common/StackType.h"
#include "Common/TypeDef.h"
#include "ObjectModel/MFuncdesc.inline.h"
#include "StackMap/DerivedPtr.h"
#include "StackMap/SlotRoot.h"
#include "StackMap/StackMapTable.h"
#include "StackMap/StackSizeVarInt.h"
#ifdef __aarch64__
#include "StackMap/StackMapAarch64.h"
#elif defined (__arm__)
#include "StackMap/StackMapArm.h"
#else
#include "StackMap/StackMapX86.h"
#endif
namespace MapleRuntime {
class CompressedStackMapEntry {
public:
    CompressedStackMapEntry(const IdxSet& idx, const RegTable& reg, const SlotTable& slot, const LineNumTable& lineNum,
                            const DerivedPtrTable derived, bool valid)
        : idxSet(idx), regTable(reg), slotTable(slot),
          lineNumTable(lineNum), derivedPtrTable(derived), isValid(valid) {}
    explicit CompressedStackMapEntry(bool valid) : isValid(valid) {}

    ~CompressedStackMapEntry() = default;

    bool IsValid() const { return isValid; }
    SlotRoot BuildSlotRoot() const
    {
        U32 idx = idxSet.slotIdx;
        if (idx == 0) {
            return SlotRoot();
        }
        return SlotRoot(slotTable.GetBaseOffset(idx - 1), slotTable.GetSlotBitMap(idx - 1), slotTable.slotFormat);
    }

    RegRoot BuildRegRoot() const
    {
        U32 idx = idxSet.regIdx;
        if (idx == 0) {
            return RegRoot();
        }
        return RegRoot(regTable.GetActiveRegBits(idx - 1));
    }

    DerivedPtr BuildDerivedPtrRoot() const
    {
        U32 idx = idxSet.derivedPtrIdx;
        if (idx == 0) {
            return DerivedPtr();
        }
        return DerivedPtr(derivedPtrTable, regTable, slotTable, idx);
    }

    LineNum BuildLineNum() const
    {
        U32 idx = idxSet.lineNumIdx;
        if (idx == 0) {
            return 0;
        }
        return lineNumTable.GetLineNumber(idx - 1);
    }

    SlotRoot BuildStackSlotRoot() const
    {
        U32 idx = idxSet.stackSlotIdx;
        if (idx == 0) {
            return SlotRoot();
        }
        return SlotRoot(slotTable.GetBaseOffset(idx - 1), slotTable.GetSlotBitMap(idx - 1), slotTable.slotFormat);
    }

    RegRoot BuildStackRegRoot() const
    {
        U32 idx = idxSet.stackRegIdx;
        if (idx == 0) {
            return RegRoot();
        }
        return RegRoot(regTable.GetActiveRegBits(idx - 1));
    }

private:
    IdxSet idxSet;
    RegTable regTable;
    SlotTable slotTable;
    LineNumTable lineNumTable;
    DerivedPtrTable derivedPtrTable;
    bool isValid = false;
};
class CompressedStackMapHead {
public:
    CompressedStackMapHead(U8* ptr, U32 bitPos, const PrologueVisitor& visitor, U32 format)
        : prologue(ptr, bitPos, visitor), slotFormat(format) {}
    CompressedStackMapHead(const BitsManager& prologueManager, const PrologueVisitor& visitor, U32 format)
        : prologue(prologueManager, visitor), slotFormat(format) {}
    ~CompressedStackMapHead() = default;
    static CompressedStackMapHead GetStackMapHead(Uptr addr, const PrologueVisitor& visitor,
                                                  uint64_t* funcDesc = nullptr)
    {
        U8 *stackmapStart = nullptr;
        if (funcDesc)
            stackmapStart = reinterpret_cast<U8*>(reinterpret_cast<FuncDescRef>(funcDesc)->GetStackMap());
        else {
#if defined(__APPLE__)
            FuncDescRef desc = MFuncDesc::GetFuncDesc(reinterpret_cast<FrameAddress*>(addr));
#else
            FuncDescRef desc = MFuncDesc::GetFuncDesc(addr);
#endif
            stackmapStart = reinterpret_cast<U8*>(desc->GetStackMap());
        }
        StacksizeVarInt stacksizeVarInt(stackmapStart, 0);
        StacksizeVarInt compressedFormatVarInt(stacksizeVarInt.GetNextTable());
        U32 format = compressedFormatVarInt.GetStacksize();
        return CompressedStackMapHead(compressedFormatVarInt.GetNextTable(), visitor, format);
    }
    static void DestroyStackMapHead(CompressedStackMapHead*& stackMapHead) noexcept
    {
        if (stackMapHead != nullptr) {
            delete stackMapHead;
            stackMapHead = nullptr;
        }
    }

    CompressedStackMapEntry GetStackMapEntry(Uptr startPC, Uptr framePC) const
    {
        StackMapTable stackMapTable(prologue.GetNextTable());
        auto idxSet = stackMapTable.GetIdxSet(startPC, framePC);
        if (idxSet.slotIdx == 0 && idxSet.regIdx == 0 && idxSet.lineNumIdx == 0 &&
                idxSet.stackRegIdx == 0 && idxSet.stackSlotIdx == 0) {
            return CompressedStackMapEntry(false);
        }
        RegTable regTable(stackMapTable.GetNextTable());
        SlotTable slotTable(regTable.GetNextTable(), slotFormat);
        LineNumTable lineTable(slotTable.GetNextTable());
        DerivedPtrTable derivedTable(lineTable.GetNextTable(), stackMapTable.GetRegBitsLen(),
                                     stackMapTable.GetSlotBitsLen());
        return CompressedStackMapEntry(idxSet, regTable, slotTable, lineTable, derivedTable, true);
    }

private:
    PrologueVarInt prologue;
    U32 slotFormat;
};
using StackMapEntry = CompressedStackMapEntry;
using StackMapHead = CompressedStackMapHead;
} // namespace MapleRuntime
#endif // ~MRT_COMPRESSED_STACKMAP_H
