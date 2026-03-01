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

#include "runtime/mem/tlab.h"

#include "libarkbase/utils/logger.h"
#include "runtime/mem/object_helpers.h"

namespace ark::mem {

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LOG_TLAB_ALLOCATOR(level) LOG(level, ALLOC) << "TLAB: "

TLAB::TLAB(void *address, size_t size)
{
    prevTlab_ = nullptr;
    nextTlab_ = nullptr;
    Fill(address, size);
    LOG_TLAB_ALLOCATOR(DEBUG) << "Construct a new TLAB at addr " << std::hex << address << " with size " << std::dec
                              << size;
}

void TLAB::Fill(void *address, size_t size)
{
    ASSERT(ToUintPtr(address) == AlignUp(ToUintPtr(address), DEFAULT_ALIGNMENT_IN_BYTES));
    memoryStartAddr_ = address;
    memoryEndAddr_ = ToVoidPtr(ToUintPtr(address) + size);
    curFreePosition_ = address;
    zeroed_ = false;
    ASAN_POISON_MEMORY_REGION(memoryStartAddr_, GetSize());
    LOG_TLAB_ALLOCATOR(DEBUG) << "Fill a TLAB with buffer at addr " << std::hex << address << " with size " << std::dec
                              << size;
}

TLAB::~TLAB()
{
    LOG_TLAB_ALLOCATOR(DEBUG) << "Destroy a TLAB at addr " << std::hex << memoryStartAddr_ << " with size " << std::dec
                              << GetSize();
}

void TLAB::Destroy()
{
    LOG_TLAB_ALLOCATOR(DEBUG) << "Destroy the TLAB at addr " << std::hex << this;
    ASAN_UNPOISON_MEMORY_REGION(memoryStartAddr_, GetSize());
}

void *TLAB::Alloc(size_t size)
{
    void *ret = nullptr;
    size_t freeSize = GetFreeSize();
    size_t requestedSize = GetAlignedObjectSize(size);
    if (requestedSize <= freeSize) {
        ASSERT(ToUintPtr(curFreePosition_) == AlignUp(ToUintPtr(curFreePosition_), DEFAULT_ALIGNMENT_IN_BYTES));
        ret = curFreePosition_;
        ASAN_UNPOISON_MEMORY_REGION(ret, size);
        curFreePosition_ = ToVoidPtr(ToUintPtr(curFreePosition_) + requestedSize);
    }
    LOG_TLAB_ALLOCATOR(DEBUG) << "Alloc size = " << size << " at addr = " << ret;
    return ret;
}

void TLAB::IterateOverObjects(const std::function<void(ObjectHeader *objectHeader)> &objectVisitor)
{
    LOG_TLAB_ALLOCATOR(DEBUG) << "IterateOverObjects started";
    auto *curPtr = memoryStartAddr_;
    void *endPtr = curFreePosition_;
    while (curPtr < endPtr) {
        auto objectHeader = static_cast<ObjectHeader *>(curPtr);
        size_t objectSize = GetObjectSize(curPtr);
        objectVisitor(objectHeader);
        curPtr = ToVoidPtr(AlignUp(ToUintPtr(curPtr) + objectSize, DEFAULT_ALIGNMENT_IN_BYTES));
    }
    LOG_TLAB_ALLOCATOR(DEBUG) << "IterateOverObjects finished";
}

void TLAB::IterateOverObjectsInRange(const std::function<void(ObjectHeader *objectHeader)> &memVisitor,
                                     const MemRange &memRange)
{
    LOG_TLAB_ALLOCATOR(DEBUG) << "IterateOverObjectsInRange started";
    if (GetOccupiedSize() == 0 || !GetMemRangeForOccupiedMemory().IsIntersect(memRange)) {
        return;
    }
    void *currentPtr = memoryStartAddr_;
    void *endPtr = ToVoidPtr(std::min(ToUintPtr(curFreePosition_), memRange.GetEndAddress() + 1));
    void *startIteratePos = ToVoidPtr(std::max(ToUintPtr(currentPtr), memRange.GetStartAddress()));
    while (currentPtr < startIteratePos) {
        size_t objectSize = GetObjectSize(static_cast<ObjectHeader *>(currentPtr));
        currentPtr = ToVoidPtr(AlignUp(ToUintPtr(currentPtr) + objectSize, DEFAULT_ALIGNMENT_IN_BYTES));
    }
    while (currentPtr < endPtr) {
        auto objectHeader = static_cast<ObjectHeader *>(currentPtr);
        size_t objectSize = GetObjectSize(currentPtr);
        memVisitor(objectHeader);
        currentPtr = ToVoidPtr(AlignUp(ToUintPtr(currentPtr) + objectSize, DEFAULT_ALIGNMENT_IN_BYTES));
    }
    LOG_TLAB_ALLOCATOR(DEBUG) << "IterateOverObjects finished";
}

bool TLAB::ContainObject(const ObjectHeader *obj)
{
    return (ToUintPtr(curFreePosition_) > ToUintPtr(obj)) && (ToUintPtr(memoryStartAddr_) <= ToUintPtr(obj));
}

bool TLAB::IsLive(const ObjectHeader *obj)
{
    ASSERT(ContainObject(obj));
    return ContainObject(obj);
}

#undef LOG_TLAB_ALLOCATOR

}  // namespace ark::mem
