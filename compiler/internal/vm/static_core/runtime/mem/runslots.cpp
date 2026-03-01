/**
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "runtime/mem/runslots.h"

#include <cstring>

#include "runtime/include/object_header.h"

namespace ark::mem {

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LOG_RUNSLOTS(level) LOG(level, ALLOC) << "RunSlots: "

template <typename LockTypeT>
void RunSlots<LockTypeT>::Initialize(size_t slotSize, uintptr_t poolPointer, bool initializeLock)
{
    ASAN_UNPOISON_MEMORY_REGION(this, RUNSLOTS_SIZE);
    LOG_RUNSLOTS(DEBUG) << "Initializing RunSlots:";
    ASSERT_PRINT((slotSize >= SlotToSize(SlotsSizes::SLOT_MIN_SIZE_BYTES)), "Size of slot in RunSlots is too small");
    ASSERT_PRINT((slotSize <= SlotToSize(SlotsSizes::SLOT_MAX_SIZE_BYTES)), "Size of slot in RunSlots is too big");
    ASSERT(poolPointer != 0);
    poolPointer_ = poolPointer;
    ASSERT_PRINT(!(ToUintPtr(this) & RUNSLOTS_ALIGNMENT_MASK), "RunSlots object must have alignment");
    slotSize_ = slotSize;
    size_t firstSlotOffset = ComputeFirstSlotOffset(slotSize);
    firstUninitializedSlotOffset_ = firstSlotOffset;
    ASSERT(firstUninitializedSlotOffset_ != 0);
    nextFree_ = nullptr;
    usedSlots_ = 0;
    nextRunslot_ = nullptr;
    prevRunslot_ = nullptr;
    if (initializeLock) {
        new (&lock_) LockTypeT();
    }
    memset_s(bitmap_.data(), BITMAP_ARRAY_SIZE, 0x0, BITMAP_ARRAY_SIZE);
    LOG_RUNSLOTS(DEBUG) << "- Memory started from = 0x" << std::hex << ToUintPtr(this);
    LOG_RUNSLOTS(DEBUG) << "- Pool size = " << RUNSLOTS_SIZE << " bytes";
    LOG_RUNSLOTS(DEBUG) << "- Slots size = " << slotSize_ << " bytes";
    LOG_RUNSLOTS(DEBUG) << "- First free slot = " << std::hex << static_cast<void *>(nextFree_);
    LOG_RUNSLOTS(DEBUG) << "- First uninitialized slot offset = " << std::hex
                        << static_cast<void *>(ToVoidPtr(firstUninitializedSlotOffset_));
    LOG_RUNSLOTS(DEBUG) << "- Pool pointer = " << std::hex << static_cast<void *>(ToVoidPtr(poolPointer_));
    LOG_RUNSLOTS(DEBUG) << "Successful finished RunSlots init";
    ASAN_POISON_MEMORY_REGION(this, RUNSLOTS_SIZE);
}

template <typename LockTypeT>
FreeSlot *RunSlots<LockTypeT>::PopFreeSlot()
{
    ASAN_UNPOISON_MEMORY_REGION(this, GetHeaderSize());
    FreeSlot *freeSlot = nullptr;
    if (nextFree_ == nullptr) {
        void *uninitializedSlot = PopUninitializedSlot();
        if (uninitializedSlot == nullptr) {
            LOG_RUNSLOTS(DEBUG) << "Failed to get free slot - there are no free slots in RunSlots";
            ASAN_POISON_MEMORY_REGION(this, GetHeaderSize());
            return nullptr;
        }
        freeSlot = static_cast<FreeSlot *>(uninitializedSlot);
    } else {
        freeSlot = nextFree_;
        ASAN_UNPOISON_MEMORY_REGION(freeSlot, sizeof(FreeSlot));
        nextFree_ = nextFree_->GetNext();
        ASAN_POISON_MEMORY_REGION(freeSlot, sizeof(FreeSlot));
    }
    MarkAsOccupied(freeSlot);
    usedSlots_++;
    LOG_RUNSLOTS(DEBUG) << "Successfully get free slot " << std::hex << static_cast<void *>(freeSlot)
                        << ". Used slots in this RunSlots = " << std::dec << usedSlots_;
    ASAN_POISON_MEMORY_REGION(this, GetHeaderSize());
    return freeSlot;
}

template <typename LockTypeT>
void RunSlots<LockTypeT>::PushFreeSlot(FreeSlot *memSlot)
{
    ASAN_UNPOISON_MEMORY_REGION(this, GetHeaderSize());
    LOG_RUNSLOTS(DEBUG) << "Free slot in RunSlots at addr " << std::hex << static_cast<void *>(memSlot);
    // We need to poison/unpoison mem_slot here because we could allocate an object with size less than FreeSlot size
    ASAN_UNPOISON_MEMORY_REGION(memSlot, sizeof(FreeSlot));
    memSlot->SetNext(nextFree_);
    ASAN_POISON_MEMORY_REGION(memSlot, sizeof(FreeSlot));
    nextFree_ = memSlot;
    MarkAsFree(memSlot);
    usedSlots_--;
    LOG_RUNSLOTS(DEBUG) << "Used slots in RunSlots = " << usedSlots_;
    ASAN_POISON_MEMORY_REGION(this, GetHeaderSize());
}

template <typename LockTypeT>
size_t RunSlots<LockTypeT>::ComputeFirstSlotOffset(size_t slotSize)
{
    size_t slotsForHeader = (GetHeaderSize() / slotSize);
    if ((GetHeaderSize() % slotSize) > 0) {
        slotsForHeader++;
    }
    return slotsForHeader * slotSize;
}

template <typename LockTypeT>
void *RunSlots<LockTypeT>::PopUninitializedSlot()
{
    if (firstUninitializedSlotOffset_ != 0) {
        ASSERT(RUNSLOTS_SIZE > firstUninitializedSlotOffset_);
        void *uninitializedSlot = ToVoidPtr(ToUintPtr(this) + firstUninitializedSlotOffset_);
        firstUninitializedSlotOffset_ += slotSize_;
        if (firstUninitializedSlotOffset_ >= RUNSLOTS_SIZE) {
            ASSERT(firstUninitializedSlotOffset_ == RUNSLOTS_SIZE);
            firstUninitializedSlotOffset_ = 0;
        }
        return uninitializedSlot;
    }
    return nullptr;
}

template <typename LockTypeT>
void RunSlots<LockTypeT>::MarkAsOccupied(const FreeSlot *slotMem)
{
    uintptr_t bitIndex =
        (ToUintPtr(slotMem) & (RUNSLOTS_SIZE - 1U)) >> SlotToSize(SlotsSizes::SLOT_MIN_SIZE_BYTES_POWER_OF_TWO);
    uintptr_t arrayIndex = bitIndex >> BITS_IN_BYTE_POWER_OF_TWO;
    uintptr_t bitInArrayElement = bitIndex & ((1U << BITS_IN_BYTE_POWER_OF_TWO) - 1U);
    ASSERT(!(bitmap_[arrayIndex] & (1U << bitInArrayElement)));
    bitmap_[arrayIndex] |= 1U << bitInArrayElement;
}

template <typename LockTypeT>
void RunSlots<LockTypeT>::MarkAsFree(const FreeSlot *slotMem)
{
    uintptr_t bitIndex =
        (ToUintPtr(slotMem) & (RUNSLOTS_SIZE - 1U)) >> SlotToSize(SlotsSizes::SLOT_MIN_SIZE_BYTES_POWER_OF_TWO);
    uintptr_t arrayIndex = bitIndex >> BITS_IN_BYTE_POWER_OF_TWO;
    uintptr_t bitInArrayElement = bitIndex & ((1U << BITS_IN_BYTE_POWER_OF_TWO) - 1U);
    ASSERT(bitmap_[arrayIndex] & (1U << bitInArrayElement));
    bitmap_[arrayIndex] ^= 1U << bitInArrayElement;
}

template <typename LockTypeT>
FreeSlot *RunSlots<LockTypeT>::BitMapToSlot(size_t arrayIndex, size_t bit)
{
    return static_cast<FreeSlot *>(
        ToVoidPtr(ToUintPtr(this) + (((arrayIndex << BITS_IN_BYTE_POWER_OF_TWO) + bit)
                                     << SlotToSize(SlotsSizes::SLOT_MIN_SIZE_BYTES_POWER_OF_TWO))));
}

template <typename LockTypeT>
size_t RunSlots<LockTypeT>::RunVerifier::operator()(RunSlots *run)
{
    // 1. should verify whether run's bracket size is the same as recorded in RunSlotsAllocator, but RunSlotsAllocator
    // does not record this
    // 2. should verify thread local run's ownership, but thread local run not implemented yet

    // check alloc'ed size
    auto sizeCheckFunc = [this, &run](const ObjectHeader *obj) {
        auto sizePowerOfTwo = ConvertToPowerOfTwoUnsafe(obj->ObjectSize());
        if ((1U << sizePowerOfTwo) != run->GetSlotsSize()) {
            ++(this->failCnt_);
        }
    };
    run->IterateOverOccupiedSlots(sizeCheckFunc);

    return failCnt_;
}

template <typename LockTypeT>
bool RunSlots<LockTypeT>::IsLive(const ObjectHeader *obj) const
{
    ASAN_UNPOISON_MEMORY_REGION(this, GetHeaderSize());
    uintptr_t memTailByRunslots = ToUintPtr(obj) & (RUNSLOTS_SIZE - 1U);
    if ((memTailByRunslots & (static_cast<uintptr_t>(slotSize_) - 1)) != 0) {
        ASAN_POISON_MEMORY_REGION(this, GetHeaderSize());
        return false;
    }
    uintptr_t bitIndex = memTailByRunslots >> SlotToSize(SlotsSizes::SLOT_MIN_SIZE_BYTES_POWER_OF_TWO);
    uintptr_t arrayIndex = bitIndex >> BITS_IN_BYTE_POWER_OF_TWO;
    uintptr_t bitInArrayElement = bitIndex & ((1U << BITS_IN_BYTE_POWER_OF_TWO) - 1U);
    auto liveWord = bitmap_[arrayIndex] & (1U << bitInArrayElement);
    ASAN_POISON_MEMORY_REGION(this, GetHeaderSize());
    return liveWord != 0;
}

template class RunSlots<RunSlotsLockConfig::CommonLock>;
template class RunSlots<RunSlotsLockConfig::DummyLock>;
}  // namespace ark::mem
