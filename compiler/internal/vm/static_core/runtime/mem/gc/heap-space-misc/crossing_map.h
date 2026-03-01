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
#ifndef PANDA_MEM_GC_CROSSING_MAP_H
#define PANDA_MEM_GC_CROSSING_MAP_H

#include <array>
#include <limits>

#include "libarkbase/mem/mem.h"
#include "libarkbase/utils/logger.h"
#include "runtime/include/mem/allocator.h"
#include "runtime/mem/runslots.h"

namespace ark::mem {

static constexpr uint64_t PANDA_CROSSING_MAP_COVERAGE = PANDA_MAX_HEAP_SIZE;
// If enabled - we will manage elements, which cross map borders.
// since now we dirty card by object header, so disable cross-border
static constexpr bool PANDA_CROSSING_MAP_MANAGE_CROSSED_BORDER = false;
static constexpr size_t PANDA_CROSSING_MAP_GRANULARITY = PANDA_PAGE_SIZE;
// NOTE(aemelenko): Now, we can't change the granularity parameter here
// because IteratingOverObjectsInRange is depended on it
static_assert(PANDA_CROSSING_MAP_GRANULARITY == PANDA_PAGE_SIZE);

//  CrossingMap structure is a double linked array:
//
//  Each static array has a link to dynamic map
//  which will be dynamically allocated/deallocated via internal allocator:
//   |-------|-------|-------|-------|-------|-------|
//   |       |       |       |       |       |       |
//   |-------|-------|-------|-------|-------|-------|
//       |       |       |       |       |       |
//       |       |       |       |       |       |
//    nullptr    |    nullptr nullptr nullptr nullptr
//               |
//               |
//               |
//               |
//      |-----|-----|-----|
//      |     |     |     |
//      |-----|-----|-----|
//          dynamic map
//

// Each page from PANDA_CROSSING_MAP_COVERAGE heap space has its element in crossing map.
// This element (or map) can be used to get the first object address, which starts inside this page (if it exists)
// or an object address, which crosses the borders of this page.
class CrossingMap {
public:
    /**
     * @brief Create a new instance of a Crossing map.
     * @param internal_allocator - pointer to the internal allocator.
     * @param start_addr - first bit of the memory which must be covered by the Crossing Map.
     * @param size - size of the memory which must be covered by the Crossing Map.
     */
    explicit CrossingMap(InternalAllocatorPtr internalAllocator, uintptr_t startAddr, size_t size);
    ~CrossingMap();
    NO_COPY_SEMANTIC(CrossingMap);
    NO_MOVE_SEMANTIC(CrossingMap);

    void Initialize();
    void Destroy();

    /**
     * @brief Add object to the Crossing map.
     * @param obj_addr - pointer to the object (object header).
     * @param obj_size - size of the object.
     */
    void AddObject(const void *objAddr, size_t objSize);

    /**
     * @brief Remove object from the Crossing map.
     *  Crossing map doesn't know about existed objects (it knows only about first).
     *  Therefore, during removing we need to send next and previous object parameters.
     * @param obj_addr - pointer to the removing object (object header).
     * @param obj_size - size of the removing object.
     * @param next_obj_addr - pointer to the next object (object header). It can be nullptr.
     * @param prev_obj_addr - pointer to the previous object (object header). It can be nullptr.
     * @param prev_obj_size - size of the previous object.
     *        It is used check if previous object crosses the borders of the current map.
     */
    void RemoveObject(const void *objAddr, size_t objSize, const void *nextObjAddr = nullptr,
                      const void *prevObjAddr = nullptr, size_t prevObjSize = 0);

    /**
     * @brief Find and return the first object, which starts in an interval inclusively
     * or an object, which crosses the interval border.
     * It is essential to check the previous object of the returned object to make sure that
     * we find the first object, which crosses the border of this interval.
     * @param start_addr - pointer to the first byte of the interval.
     * @param end_addr - pointer to the last byte of the interval.
     * @return Returns the first object which starts inside an interval,
     *  or an object which crosses a border of this interval
     *  or nullptr
     */
    void *FindFirstObject(const void *startAddr, const void *endAddr);

    /**
     * @brief Initialize a Crossing map for the corresponding memory ranges.
     * @param start_addr - pointer to the first byte of the interval.
     * @param size - size of the interval.
     */
    void InitializeCrossingMapForMemory(const void *startAddr, size_t size);

    /**
     * @brief Remove a Crossing map for the corresponding memory ranges.
     * @param start_addr - pointer to the first byte of the interval.
     * @param size - size of the interval.
     */
    void RemoveCrossingMapForMemory(const void *startAddr, size_t size);

private:
    static constexpr bool CROSSING_MAP_MANAGE_CROSSED_BORDER = PANDA_CROSSING_MAP_MANAGE_CROSSED_BORDER;
    static constexpr size_t CROSSING_MAP_GRANULARITY = PANDA_CROSSING_MAP_GRANULARITY;
    // How much memory we manage via one element of the static array.
    static constexpr size_t CROSSING_MAP_STATIC_ARRAY_GRANULARITY = PANDA_POOL_ALIGNMENT_IN_BYTES;
    static constexpr Alignment CROSSING_MAP_OBJ_ALIGNMENT = DEFAULT_ALIGNMENT;
    using CrossingMapType = uint16_t;

    static_assert(CROSSING_MAP_STATIC_ARRAY_GRANULARITY % CROSSING_MAP_GRANULARITY == 0);
    static constexpr size_t CROSSING_MAP_COUNT_IN_STATIC_ARRAY_ELEMENT =
        CROSSING_MAP_STATIC_ARRAY_GRANULARITY / CROSSING_MAP_GRANULARITY;

    // Each element of the crossing map consists of two fields:
    //
    // |.... Offset ....|.... Status ....|
    //
    // According Status bits, we can use the offset value in such a way:
    //
    // - if Status == Uninitialized, there is no element in this Page at all.
    //
    // - if Status == Initialized, the Offset value is an offset in words of the first element on this Page.
    //   We can start our range iteration from this element.
    //
    // - if Status == Crossed border, the Offset value is an offset in crossing map array to a page where the object,
    //   which crossed the Page border, is stored.
    //
    // - if Status == Initialized and Crossed border, the Offset value is an offset in words of the first element on
    //   this Page, and also we know what there is an object, which crossed the Page border.
    class CrossingMapElement {
    public:
        enum STATE {
            // This element of crossing map hasn't been initialized yet.
            STATE_UNINITIALIZED,
            // There are no objects which start in this page,
            // but there is an object which starts before this page and crosses page border.
            STATE_CROSSED_BORDER,
            // We have some object that starts inside this page,
            // but there are no objects which crosses page border.
            STATE_INITIALIZED,
            // We have some object that starts inside this page,
            // and there is an object which crosses page border.
            STATE_INITIALIZED_AND_CROSSED_BORDERS,
        };

        static constexpr size_t GetMaxOffsetValue()
        {
            return MAX_OFFSET_VALUE;
        }

        STATE GetState()
        {
            switch (value_ & STATUS_MASK) {
                case STATUS_UNINITIALIZED:
                    return (value_ | 0U) == 0U ? STATE_UNINITIALIZED : STATE_CROSSED_BORDER;
                case STATUS_INITIALIZED:
                    return STATE_INITIALIZED;
                case STATUS_INITIALIZED_AND_CROSSED_BORDERS:
                    return STATE_INITIALIZED_AND_CROSSED_BORDERS;
                default:
                    LOG(FATAL, ALLOC) << "CrossingMapElement: Undefined map state";
                    return STATE_UNINITIALIZED;
            }
        }

        CrossingMapType GetOffset()
        {
            return value_ >> STATUS_SIZE;
        }

        void SetUninitialized()
        {
            value_ = STATUS_UNINITIALIZED;
        }

        void SetInitialized(CrossingMapType offset)
        {
            ASSERT(offset <= MAX_OFFSET_VALUE);
            value_ = (offset << STATUS_SIZE);
            value_ |= STATUS_INITIALIZED;
        }

        void SetInitializedAndCrossedBorder(CrossingMapType offset)
        {
            ASSERT(offset <= MAX_OFFSET_VALUE);
            value_ = (offset << STATUS_SIZE);
            value_ |= STATUS_INITIALIZED_AND_CROSSED_BORDERS;
        }

        void SetCrossedBorder(CrossingMapType offset)
        {
            ASSERT(offset <= MAX_OFFSET_VALUE);
            value_ = (offset << STATUS_SIZE);
            value_ |= STATUS_CROSSED_BORDER;
        }

    private:
        enum MapRepresentation : CrossingMapType {
            STATUS_UNINITIALIZED = 0U,
            STATUS_CROSSED_BORDER = 0U,
            STATUS_INITIALIZED = 1U,
            STATUS_INITIALIZED_AND_CROSSED_BORDERS = 2U,
            STATUS_SIZE = 2U,
            STATUS_MASK = (1U << STATUS_SIZE) - 1U,
        };

        static_assert(std::is_unsigned<CrossingMapType>::value);
        static constexpr size_t MAX_OFFSET_VALUE = std::numeric_limits<CrossingMapType>::max() >> STATUS_SIZE;

        // We must be sure that we can use such type for all possible obj offsets inside one page
        static_assert((CROSSING_MAP_GRANULARITY >> CROSSING_MAP_OBJ_ALIGNMENT) <= MAX_OFFSET_VALUE);

        CrossingMapType value_ {STATUS_UNINITIALIZED};
    };
    // It is a pointer to an array of Crossing Map elements.
    using StaticArrayPtr = CrossingMapElement **;

    size_t GetMapNumFromAddr(const void *addr)
    {
        ASSERT(ToUintPtr(addr) >= startAddr_);
        size_t mapNum = (ToUintPtr(addr) - startAddr_) / CROSSING_MAP_GRANULARITY;
        ASSERT(mapNum < mapElementsCount_);
        return mapNum;
    }

    void *GetAddrFromOffset(size_t mapNum, size_t offset)
    {
        ASSERT(mapNum < mapElementsCount_);
        return ToVoidPtr(startAddr_ + mapNum * CROSSING_MAP_GRANULARITY + (offset << CROSSING_MAP_OBJ_ALIGNMENT));
    }

    size_t GetOffsetFromAddr(const void *addr)
    {
        ASSERT(ToUintPtr(addr) >= startAddr_);
        size_t offset = (ToUintPtr(addr) - startAddr_) % CROSSING_MAP_GRANULARITY;
        ASSERT((offset & (GetAlignmentInBytes(CROSSING_MAP_OBJ_ALIGNMENT) - 1U)) == 0U);
        return offset >> CROSSING_MAP_OBJ_ALIGNMENT;
    }

    void UpdateCrossedBorderOnAdding(size_t firstCrossedBorderMap, size_t lastCrossedBorderMap);
    void UpdateCrossedBorderOnRemoving(size_t crossedBorderMap);
    void *FindObjInMap(size_t mapNum);

    CrossingMapElement *GetMapElement(size_t mapNum)
    {
        ASSERT(mapNum < mapElementsCount_);
        size_t staticArrayNum = mapNum / CROSSING_MAP_COUNT_IN_STATIC_ARRAY_ELEMENT;
        size_t relativeMapNum = mapNum % CROSSING_MAP_COUNT_IN_STATIC_ARRAY_ELEMENT;
        ASSERT(GetStaticArrayElement(staticArrayNum) != nullptr);
        return static_cast<CrossingMapElement *>(ToVoidPtr(
            (ToUintPtr(GetStaticArrayElement(staticArrayNum)) + relativeMapNum * sizeof(CrossingMapElement))));
    }

    CrossingMapElement *GetStaticArrayElement(size_t staticArrayNum)
    {
        ASSERT(staticArrayNum < staticArrayElementsCount_);
        auto elementPointer =
            static_cast<StaticArrayPtr>(ToVoidPtr((ToUintPtr(staticArray_) + staticArrayNum * sizeof(StaticArrayPtr))));
        return *elementPointer;
    }

    void SetStaticArrayElement(size_t staticArrayNum, CrossingMapElement *value)
    {
        ASSERT(staticArrayNum < staticArrayElementsCount_);
        void *element = ToVoidPtr(ToUintPtr(staticArray_) + staticArrayNum * sizeof(StaticArrayPtr));
        *static_cast<StaticArrayPtr>(element) = value;
    }

    size_t GetStaticArrayNumFromAddr(const void *addr)
    {
        ASSERT(ToUintPtr(addr) >= startAddr_);
        size_t staticArrayNum = (ToUintPtr(addr) - startAddr_) / CROSSING_MAP_STATIC_ARRAY_GRANULARITY;
        ASSERT(staticArrayNum < staticArrayElementsCount_);
        return staticArrayNum;
    }

    StaticArrayPtr staticArray_ {nullptr};
    uintptr_t startAddr_ {0};
    size_t mapElementsCount_ {0};
    size_t staticArrayElementsCount_ {0};
    InternalAllocatorPtr internalAllocator_ {nullptr};
    friend class CrossingMapTest;
};

}  // namespace ark::mem

#endif  // PANDA_MEM_GC_CROSSING_MAP_H
