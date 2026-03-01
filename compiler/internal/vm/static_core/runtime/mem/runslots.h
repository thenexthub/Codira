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
#ifndef PANDA_RUNTIME_MEM_RUNSLOTS_H
#define PANDA_RUNTIME_MEM_RUNSLOTS_H

#include <array>
#include <cstddef>

#include "libarkbase/macros.h"
#include "libarkbase/mem/mem.h"
#include "libarkbase/utils/asan_interface.h"
#include "libarkbase/utils/logger.h"

namespace ark::mem {

static constexpr size_t PAGES_IN_RUNSLOTS = 1;
static constexpr size_t RUNSLOTS_SIZE = PAGES_IN_RUNSLOTS * PANDA_PAGE_SIZE;
static constexpr size_t RUNSLOTS_ALIGNMENT_IN_BYTES = PANDA_PAGE_SIZE;
static constexpr size_t RUNSLOTS_ALIGNMENT = 10 + 2;  // Alignment for shift
static constexpr size_t RUNSLOTS_ALIGNMENT_MASK = (1UL << RUNSLOTS_ALIGNMENT) - 1;
static_assert((1UL << RUNSLOTS_ALIGNMENT) == RUNSLOTS_ALIGNMENT_IN_BYTES);

class RunSlotsLockConfig {
public:
    using CommonLock = os::memory::Mutex;
    using DummyLock = os::memory::DummyLock;
};

/**
 * A class for free slots inside RunSlots object.
 * Each free slot has a link to the next slot in RunSlots.
 * If the link is equal to nullptr, then it is the last free slot.
 */
class FreeSlot {
public:
    FreeSlot *GetNext()
    {
        return nextFree_;
    }
    void SetNext(FreeSlot *next)
    {
        nextFree_ = next;
    }

private:
    FreeSlot *nextFree_ {nullptr};
};
/**
 * The main class for RunSlots.
 * Each RunSlots consumes RUNSLOTS_SIZE bytes of memory.
 * RunSlots is divided into equal size slots which will be used for allocation.
 * The RunSlots header is stored inside the first slot (or slots) of this RunSlots instance.
 */
template <typename LockTypeT = RunSlotsLockConfig::CommonLock>
class RunSlots {
public:
    // Use ATTRIBUTE_NO_SANITIZE_ADDRESS to prevent MT issues with POISON/UNPOISON
    ATTRIBUTE_NO_SANITIZE_ADDRESS
    void Initialize(size_t slotSize, uintptr_t poolPointer, bool initializeLock);

    static constexpr size_t MaxSlotSize()
    {
        return SlotToSize(SlotsSizes::SLOT_MAX_SIZE_BYTES);
    }

    static constexpr size_t MinSlotSize()
    {
        return SlotToSize(SlotsSizes::SLOT_MIN_SIZE_BYTES);
    }

    static constexpr size_t SlotSizesVariants()
    {
        return SlotToSize(SlotsSizes::SLOT_MAX_SIZE_BYTES_POWER_OF_TWO);
    }

    // Use ATTRIBUTE_NO_SANITIZE_ADDRESS to prevent MT issues with POISON/UNPOISON
    ATTRIBUTE_NO_SANITIZE_ADDRESS
    FreeSlot *PopFreeSlot();

    // Use ATTRIBUTE_NO_SANITIZE_ADDRESS to prevent MT issues with POISON/UNPOISON
    ATTRIBUTE_NO_SANITIZE_ADDRESS
    void PushFreeSlot(FreeSlot *memSlot);

    // Use ATTRIBUTE_NO_SANITIZE_ADDRESS to prevent MT issues with POISON/UNPOISON
    ATTRIBUTE_NO_SANITIZE_ADDRESS
    uintptr_t GetPoolPointer()
    {
        ASAN_UNPOISON_MEMORY_REGION(this, GetHeaderSize());
        uintptr_t poolPointer = poolPointer_;
        ASAN_POISON_MEMORY_REGION(this, GetHeaderSize());
        return poolPointer;
    }

    // Use ATTRIBUTE_NO_SANITIZE_ADDRESS to prevent MT issues with POISON/UNPOISON
    ATTRIBUTE_NO_SANITIZE_ADDRESS
    bool IsEmpty()
    {
        ASAN_UNPOISON_MEMORY_REGION(this, GetHeaderSize());
        bool isEmpty = (usedSlots_ == 0);
        ASAN_POISON_MEMORY_REGION(this, GetHeaderSize());
        return isEmpty;
    }

    // Use ATTRIBUTE_NO_SANITIZE_ADDRESS to prevent MT issues with POISON/UNPOISON
    ATTRIBUTE_NO_SANITIZE_ADDRESS
    bool IsFull()
    {
        ASAN_UNPOISON_MEMORY_REGION(this, GetHeaderSize());
        bool isFull = (nextFree_ == nullptr) && (firstUninitializedSlotOffset_ == 0);
        ASAN_POISON_MEMORY_REGION(this, GetHeaderSize());
        return isFull;
    }

    // Use ATTRIBUTE_NO_SANITIZE_ADDRESS to prevent MT issues with POISON/UNPOISON
    ATTRIBUTE_NO_SANITIZE_ADDRESS
    void SetNextRunSlots(RunSlots *runslots)
    {
        ASAN_UNPOISON_MEMORY_REGION(this, GetHeaderSize());
        nextRunslot_ = runslots;
        ASAN_POISON_MEMORY_REGION(this, GetHeaderSize());
    }

    // Use ATTRIBUTE_NO_SANITIZE_ADDRESS to prevent MT issues with POISON/UNPOISON
    ATTRIBUTE_NO_SANITIZE_ADDRESS
    RunSlots *GetNextRunSlots()
    {
        ASAN_UNPOISON_MEMORY_REGION(this, GetHeaderSize());
        RunSlots *next = nextRunslot_;
        ASAN_POISON_MEMORY_REGION(this, GetHeaderSize());
        return next;
    }

    // Use ATTRIBUTE_NO_SANITIZE_ADDRESS to prevent MT issues with POISON/UNPOISON
    ATTRIBUTE_NO_SANITIZE_ADDRESS
    void SetPrevRunSlots(RunSlots *runslots)
    {
        ASAN_UNPOISON_MEMORY_REGION(this, GetHeaderSize());
        prevRunslot_ = runslots;
        ASAN_POISON_MEMORY_REGION(this, GetHeaderSize());
    }

    // Use ATTRIBUTE_NO_SANITIZE_ADDRESS to prevent MT issues with POISON/UNPOISON
    ATTRIBUTE_NO_SANITIZE_ADDRESS
    RunSlots *GetPrevRunSlots()
    {
        ASAN_UNPOISON_MEMORY_REGION(this, GetHeaderSize());
        RunSlots *prev = prevRunslot_;
        ASAN_POISON_MEMORY_REGION(this, GetHeaderSize());
        return prev;
    }

    // Use ATTRIBUTE_NO_SANITIZE_ADDRESS to prevent MT issues with POISON/UNPOISON
    ATTRIBUTE_NO_SANITIZE_ADDRESS
    size_t GetSlotsSize()
    {
        ASAN_UNPOISON_MEMORY_REGION(this, GetHeaderSize());
        size_t size = slotSize_;
        ASAN_POISON_MEMORY_REGION(this, GetHeaderSize());
        return size;
    }

    static constexpr size_t ConvertToPowerOfTwoUnsafe(size_t size)
    {
        size_t i = SlotToSize(SlotsSizes::SLOT_MIN_SIZE_BYTES_POWER_OF_TWO);
        size_t val = SlotToSize(SlotsSizes::SLOT_MIN_SIZE_BYTES);
        while (size > val) {
            i++;
            val = val << 1UL;
        }
        return i;
    }

    // Use ATTRIBUTE_NO_SANITIZE_ADDRESS to prevent MT issues with POISON/UNPOISON
    template <typename ObjectVisitor>
    ATTRIBUTE_NO_SANITIZE_ADDRESS void IterateOverOccupiedSlots(const ObjectVisitor &objectVisitor)
    {
        ASAN_UNPOISON_MEMORY_REGION(this, GetHeaderSize());
        // NOTE(aemelenko): We can increase execution speed of this loops and do not count BitMapToSlot each time
        for (size_t arrayIndex = 0; arrayIndex < BITMAP_ARRAY_SIZE; arrayIndex++) {
            uint8_t byte = bitmap_[arrayIndex];
            if (byte == 0x0) {
                continue;
            }
            for (size_t bit = 0; bit < (1U << BITS_IN_BYTE_POWER_OF_TWO); bit++) {
                if (byte & 0x1U) {
                    objectVisitor(static_cast<ObjectHeader *>(static_cast<void *>(BitMapToSlot(arrayIndex, bit))));
                }
                byte = byte >> 1U;
            }
            // We must unpoison again, because we can poison header somewhere inside a visitor
            ASAN_UNPOISON_MEMORY_REGION(this, GetHeaderSize());
        }
        ASAN_POISON_MEMORY_REGION(this, GetHeaderSize());
    }

    /// @brief Check integraty of ROS allocator, return failure count.
    size_t VerifyRun()
    {
        return RunVerifier()(this);
    }

    // Use ATTRIBUTE_NO_SANITIZE_ADDRESS to prevent MT issues with POISON/UNPOISON
    ATTRIBUTE_NO_SANITIZE_ADDRESS
    bool IsLive(const ObjectHeader *obj) const;

    // Use ATTRIBUTE_NO_SANITIZE_ADDRESS to prevent MT issues with POISON/UNPOISON
    ATTRIBUTE_NO_SANITIZE_ADDRESS
    LockTypeT *GetLock()
    {
        ASAN_UNPOISON_MEMORY_REGION(this, GetHeaderSize());
        LockTypeT *lock = &lock_;
        ASAN_POISON_MEMORY_REGION(this, GetHeaderSize());
        return lock;
    }

private:
    enum SlotsSizes : size_t {
        SLOT_8_BYTES_POWER_OF_TWO = 3,
        SLOT_16_BYTES_POWER_OF_TWO = 4,
        SLOT_32_BYTES_POWER_OF_TWO = 5,
        SLOT_64_BYTES_POWER_OF_TWO = 6,
        SLOT_128_BYTES_POWER_OF_TWO = 7,
        SLOT_256_BYTES_POWER_OF_TWO = 8,
        SLOT_MAX_SIZE_BYTES_POWER_OF_TWO = SLOT_256_BYTES_POWER_OF_TWO,
        SLOT_MIN_SIZE_BYTES_POWER_OF_TWO = SLOT_8_BYTES_POWER_OF_TWO,
        SLOT_MAX_SIZE_BYTES = 1UL << SLOT_MAX_SIZE_BYTES_POWER_OF_TWO,
        SLOT_MIN_SIZE_BYTES = 1UL << SLOT_MIN_SIZE_BYTES_POWER_OF_TWO,
    };

    static constexpr size_t SlotToSize(SlotsSizes val)
    {
        return static_cast<size_t>(val);
    }

    static constexpr size_t BITS_IN_BYTE_POWER_OF_TWO = 3U;
    static constexpr size_t BITMAP_ARRAY_SIZE =
        (RUNSLOTS_SIZE >> (SlotsSizes::SLOT_MIN_SIZE_BYTES_POWER_OF_TWO)) >> BITS_IN_BYTE_POWER_OF_TWO;

    static size_t GetHeaderSize()
    {
        return sizeof(RunSlots);
    }

    size_t ComputeFirstSlotOffset(size_t slotSize);

    void SetupSlots();

    // Use ATTRIBUTE_NO_SANITIZE_ADDRESS to prevent MT issues with POISON/UNPOISON
    ATTRIBUTE_NO_SANITIZE_ADDRESS
    void MarkAsOccupied(const FreeSlot *slotMem);

    // Use ATTRIBUTE_NO_SANITIZE_ADDRESS to prevent MT issues with POISON/UNPOISON
    ATTRIBUTE_NO_SANITIZE_ADDRESS
    void MarkAsFree(const FreeSlot *slotMem);

    FreeSlot *BitMapToSlot(size_t arrayIndex, size_t bit);

    class RunVerifier {
    public:
        RunVerifier() = default;
        ~RunVerifier() = default;
        NO_COPY_SEMANTIC(RunVerifier);
        NO_MOVE_SEMANTIC(RunVerifier);

        size_t operator()(RunSlots *run);

    private:
        size_t failCnt_ {0};
    };

    // Use ATTRIBUTE_NO_SANITIZE_ADDRESS to prevent MT issues with POISON/UNPOISON
    ATTRIBUTE_NO_SANITIZE_ADDRESS
    void *PopUninitializedSlot();

    static_assert((RUNSLOTS_SIZE / SlotsSizes::SLOT_MIN_SIZE_BYTES) <=
                  std::numeric_limits<uint16_t>::max());                                     // used_slots_
    static_assert(SlotsSizes::SLOT_MAX_SIZE_BYTES <= std::numeric_limits<uint16_t>::max());  // slot_size_
    static_assert(RUNSLOTS_SIZE <= std::numeric_limits<uint16_t>::max());  // first_uninitialized_slot_offset_
    uint16_t usedSlots_ {0};
    uint16_t slotSize_ {0};
    uint16_t firstUninitializedSlotOffset_ {0};  // If equal to zero - we don't have uninitialized slots
    uintptr_t poolPointer_ {0};
    FreeSlot *nextFree_ {nullptr};
    RunSlots *nextRunslot_ {nullptr};
    RunSlots *prevRunslot_ {nullptr};
    LockTypeT lock_;

    // Bitmap for identifying live objects in this RunSlot
    std::array<uint8_t, BITMAP_ARRAY_SIZE> bitmap_ {};
};

static_assert(RunSlots<>::MinSlotSize() >= sizeof(uintptr_t));

}  // namespace ark::mem

#endif  // PANDA_RUNTIME_MEM_RUNSLOTS_H
