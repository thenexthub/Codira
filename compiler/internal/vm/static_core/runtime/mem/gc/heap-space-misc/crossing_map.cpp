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

#include "runtime/mem/gc/heap-space-misc/crossing_map.h"

#include <cstring>

namespace ark::mem {

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LOG_CROSSING_MAP(level) LOG(level, GC) << "CrossingMap: "

CrossingMap::CrossingMap(InternalAllocatorPtr internalAllocator, uintptr_t startAddr, size_t size)
    : startAddr_(startAddr), internalAllocator_(internalAllocator)
{
    ASSERT(size % CROSSING_MAP_GRANULARITY == 0);
    mapElementsCount_ = size / CROSSING_MAP_GRANULARITY;
    staticArrayElementsCount_ =
        AlignUp(size, CROSSING_MAP_STATIC_ARRAY_GRANULARITY) / CROSSING_MAP_STATIC_ARRAY_GRANULARITY;
    ASSERT((startAddr & (PANDA_PAGE_SIZE - 1)) == 0U);
    LOG_CROSSING_MAP(DEBUG) << "Create CrossingMap with start_addr 0x" << std::hex << startAddr_;
}

CrossingMap::~CrossingMap()
{
    ASSERT(staticArray_ == nullptr);
}

void CrossingMap::Initialize()
{
    if (staticArray_ != nullptr) {
        LOG_CROSSING_MAP(FATAL) << "Try to initialize already initialized CrossingMap";
    }
    size_t staticArraySizeInBytes = staticArrayElementsCount_ * sizeof(StaticArrayPtr);
    staticArray_ = static_cast<StaticArrayPtr>(internalAllocator_->Alloc(staticArraySizeInBytes));
    ASSERT(staticArray_ != nullptr);
    for (size_t i = 0; i < staticArrayElementsCount_; i++) {
        SetStaticArrayElement(i, nullptr);
    }
}

void CrossingMap::Destroy()
{
    for (size_t i = 0; i < staticArrayElementsCount_; i++) {
        void *element = GetStaticArrayElement(i);
        if (element != nullptr) {
            internalAllocator_->Free(element);
        }
    }
    ASSERT(staticArray_ != nullptr);
    internalAllocator_->Free(staticArray_);
    staticArray_ = nullptr;
}

void CrossingMap::AddObject(const void *objAddr, size_t objSize)
{
    LOG_CROSSING_MAP(DEBUG) << "Try to AddObject with addr " << std::hex << objAddr << " and size " << std::dec
                            << objSize;
    size_t firstMapNum = GetMapNumFromAddr(objAddr);
    size_t objOffset = GetOffsetFromAddr(objAddr);
    CrossingMapElement::STATE state = GetMapElement(firstMapNum)->GetState();
    switch (state) {
        case CrossingMapElement::STATE::STATE_UNINITIALIZED:
            LOG_CROSSING_MAP(DEBUG) << "AddObject - state of the map num " << firstMapNum
                                    << " wasn't INITIALIZED. Initialize it with offset " << objOffset;
            GetMapElement(firstMapNum)->SetInitialized(objOffset);
            break;
        case CrossingMapElement::STATE::STATE_CROSSED_BORDER:
            LOG_CROSSING_MAP(DEBUG) << "AddObject - state of the map num " << firstMapNum
                                    << " was CROSSED BORDER. Initialize it with offset " << objOffset;
            GetMapElement(firstMapNum)->SetInitializedAndCrossedBorder(objOffset);
            break;
        case CrossingMapElement::STATE::STATE_INITIALIZED_AND_CROSSED_BORDERS:
            if (GetMapElement(firstMapNum)->GetOffset() > objOffset) {
                LOG_CROSSING_MAP(DEBUG) << "AddObject - state of the map num " << firstMapNum
                                        << " is INITIALIZED and CROSSED BORDERS, but this object is the first in it."
                                        << " Initialize it with new offset " << objOffset;
                GetMapElement(firstMapNum)->SetInitializedAndCrossedBorder(objOffset);
            }
            break;
        case CrossingMapElement::STATE::STATE_INITIALIZED:
            if (GetMapElement(firstMapNum)->GetOffset() > objOffset) {
                LOG_CROSSING_MAP(DEBUG) << "AddObject - state of the map num " << firstMapNum
                                        << " is INITIALIZED, but this object is the first in it."
                                        << " Initialize it with new offset " << objOffset;
                GetMapElement(firstMapNum)->SetInitialized(objOffset);
            }
            break;
        default:
            LOG_CROSSING_MAP(FATAL) << "Unknown state!";
    }
    if constexpr (CROSSING_MAP_MANAGE_CROSSED_BORDER) {
        void *lastObjByte = ToVoidPtr(ToUintPtr(objAddr) + objSize - 1U);
        size_t finalMapNum = GetMapNumFromAddr(lastObjByte);
        if (finalMapNum != firstMapNum) {
            UpdateCrossedBorderOnAdding(firstMapNum + 1U, finalMapNum);
        }
    }
}

void CrossingMap::UpdateCrossedBorderOnAdding(const size_t firstCrossedBorderMap, const size_t lastCrossedBorderMap)
{
    ASSERT(lastCrossedBorderMap >= firstCrossedBorderMap);
    // Iterate over maps which are fully covered by this object
    // i.e. from second to last minus one map
    size_t mapOffset = 1U;
    for (size_t i = firstCrossedBorderMap; i + 1U <= lastCrossedBorderMap; i++) {
        LOG_CROSSING_MAP(DEBUG) << "AddObject - set CROSSED_BORDER to map num " << i << " with offset " << mapOffset;
        GetMapElement(i)->SetCrossedBorder(mapOffset);
        // If map_offset exceeds the limit, we will set max value to each map after that.
        // When we want to find the element, which crosses the borders of a map,
        // we will iterate before we meet a map with non-CROSSED_BORDER state.
        if (mapOffset < CrossingMapElement::GetMaxOffsetValue()) {
            mapOffset++;
        }
    }
    // Set up last map:
    switch (GetMapElement(lastCrossedBorderMap)->GetState()) {
        case CrossingMapElement::STATE::STATE_UNINITIALIZED:
            GetMapElement(lastCrossedBorderMap)->SetCrossedBorder(mapOffset);
            break;
        case CrossingMapElement::STATE::STATE_INITIALIZED:
            GetMapElement(lastCrossedBorderMap)
                ->SetInitializedAndCrossedBorder(GetMapElement(lastCrossedBorderMap)->GetOffset());
            break;
        default:
            LOG_CROSSING_MAP(FATAL) << "Unknown state!";
    }
    LOG_CROSSING_MAP(DEBUG) << "AddObject - set CROSSED_BORDER or INITIALIZED_AND_CROSSED_BORDERS to final map num "
                            << lastCrossedBorderMap << " with offset " << mapOffset;
}

void CrossingMap::RemoveObject(const void *objAddr, size_t objSize, const void *nextObjAddr, const void *prevObjAddr,
                               size_t prevObjSize)
{
    LOG_CROSSING_MAP(DEBUG) << "Try to RemoveObject with addr " << std::hex << objAddr << " and size " << std::dec
                            << objSize;
    ASSERT(objAddr != nullptr);
    // Let's set all maps, which are related to this object, as uninitialized
    size_t firstMapNum = GetMapNumFromAddr(objAddr);
    size_t objOffset = GetOffsetFromAddr(objAddr);
    ASSERT(GetMapElement(firstMapNum)->GetState() == CrossingMapElement::STATE::STATE_INITIALIZED ||
           GetMapElement(firstMapNum)->GetState() == CrossingMapElement::STATE::STATE_INITIALIZED_AND_CROSSED_BORDERS);
    // We need to check that first object in this map is a pointer to this object
    size_t mapOffset = GetMapElement(firstMapNum)->GetOffset();
    ASSERT(mapOffset <= objOffset);
    if (mapOffset == objOffset) {
        LOG_CROSSING_MAP(DEBUG) << "RemoveObject - it is the first object in map num " << firstMapNum
                                << ". So, just uninitialize it.";
        GetMapElement(firstMapNum)->SetUninitialized();
    }

    if constexpr (CROSSING_MAP_MANAGE_CROSSED_BORDER) {
        void *lastObjByte = ToVoidPtr(ToUintPtr(objAddr) + objSize - 1U);
        size_t finalMapNum = GetMapNumFromAddr(lastObjByte);
        ASSERT(finalMapNum >= firstMapNum);
        // Set all pages, which fully covered by this object, as Uninitialized;
        // and for last map (we will set it up correctly later)
        for (size_t i = firstMapNum + 1U; i <= finalMapNum; i++) {
            LOG_CROSSING_MAP(DEBUG) << "RemoveObject - Set uninitialized to map num " << i;
            GetMapElement(i)->SetUninitialized();
        }
    }

    // Set up map for next element (because we could set it as uninitialized)
    if (nextObjAddr != nullptr) {
        size_t nextObjMapNum = GetMapNumFromAddr(nextObjAddr);
        if (GetMapElement(nextObjMapNum)->GetState() == CrossingMapElement::STATE::STATE_UNINITIALIZED) {
            LOG_CROSSING_MAP(DEBUG) << "RemoveObject - Set up map " << nextObjMapNum << " for next object with addr "
                                    << std::hex << nextObjAddr << " as INITIALIZED with offset " << std::dec
                                    << GetOffsetFromAddr(nextObjAddr);
            GetMapElement(nextObjMapNum)->SetInitialized(GetOffsetFromAddr(nextObjAddr));
        }
    }
    // Set up map for last byte of prev element (because it can cross the page borders)
    if constexpr (CROSSING_MAP_MANAGE_CROSSED_BORDER) {
        if (prevObjAddr != nullptr) {
            void *prevObjLastByte = ToVoidPtr(ToUintPtr(prevObjAddr) + prevObjSize - 1U);
            size_t prevObjLastMap = GetMapNumFromAddr(prevObjLastByte);
            size_t prevObjFirstMap = GetMapNumFromAddr(prevObjAddr);
            if ((prevObjLastMap == firstMapNum) && (prevObjFirstMap != firstMapNum)) {
                UpdateCrossedBorderOnRemoving(prevObjLastMap);
            }
        }
    }
}

void CrossingMap::UpdateCrossedBorderOnRemoving(const size_t crossedBorderMap)
{
    switch (GetMapElement(crossedBorderMap)->GetState()) {
        case CrossingMapElement::STATE::STATE_UNINITIALIZED: {
            // This situation can only happen when removed object was the first object in a corresponding page map
            // and next_obj_addr is not located in the same page map.
            ASSERT(crossedBorderMap > 0);
            // Calculate offset for crossed border map
            size_t offset = GetMapElement(crossedBorderMap - 1U)->GetOffset();
            CrossingMapElement::STATE prevMapState = GetMapElement(crossedBorderMap - 1U)->GetState();
            switch (prevMapState) {
                case CrossingMapElement::STATE::STATE_INITIALIZED:
                case CrossingMapElement::STATE::STATE_INITIALIZED_AND_CROSSED_BORDERS:
                    offset = 1U;
                    break;
                case CrossingMapElement::STATE::STATE_CROSSED_BORDER:
                    if (offset < CrossingMapElement::GetMaxOffsetValue()) {
                        offset++;
                    }
                    break;
                default:
                    LOG_CROSSING_MAP(FATAL) << "Incorrect state!";
            }
            GetMapElement(crossedBorderMap)->SetCrossedBorder(offset);
            break;
        }
        case CrossingMapElement::STATE::STATE_INITIALIZED: {
            GetMapElement(crossedBorderMap)
                ->SetInitializedAndCrossedBorder(GetMapElement(crossedBorderMap)->GetOffset());
            break;
        }
        default:
            LOG_CROSSING_MAP(FATAL) << "Incorrect state!";
    }
}

void *CrossingMap::FindFirstObject(const void *startAddr, const void *endAddr)
{
    LOG_CROSSING_MAP(DEBUG) << "FindFirstObject for interval [" << std::hex << startAddr << ", " << endAddr << "]";
    size_t firstMap = GetMapNumFromAddr(startAddr);
    size_t lastMap = GetMapNumFromAddr(endAddr);
    LOG_CROSSING_MAP(DEBUG) << "FindFirstObject for maps [" << std::dec << firstMap << ", " << lastMap << "]";
    for (size_t i = firstMap; i <= lastMap; i++) {
        void *objOffset = FindObjInMap(i);
        if (objOffset != nullptr) {
            LOG_CROSSING_MAP(DEBUG) << "Found first object in this interval with addr " << std::hex << objOffset;
            return objOffset;
        }
    }
    LOG_CROSSING_MAP(DEBUG) << "There is no object in this interval, return nullptr";
    return nullptr;
}

void CrossingMap::InitializeCrossingMapForMemory(const void *startAddr, size_t size)
{
    LOG_CROSSING_MAP(DEBUG) << "InitializeCrossingMapForMemory for addr " << std::hex << startAddr << " with size "
                            << size;
    size_t startMap = GetStaticArrayNumFromAddr(startAddr);
    size_t endMap = GetStaticArrayNumFromAddr(ToVoidPtr(ToUintPtr(startAddr) + size - 1));
    ASSERT(startMap <= endMap);
    size_t staticMapSizeInBytes = CROSSING_MAP_COUNT_IN_STATIC_ARRAY_ELEMENT * sizeof(CrossingMapType);
    for (size_t i = startMap; i <= endMap; i++) {
        ASSERT(GetStaticArrayElement(i) == nullptr);
        void *mem = internalAllocator_->Alloc(staticMapSizeInBytes);
        memset_s(mem, staticMapSizeInBytes, 0x0, staticMapSizeInBytes);
        SetStaticArrayElement(i, static_cast<CrossingMapElement *>(mem));
        ASSERT(GetStaticArrayElement(i) != nullptr);
    }
}

void CrossingMap::RemoveCrossingMapForMemory(const void *startAddr, size_t size)
{
    LOG_CROSSING_MAP(DEBUG) << "RemoveCrossingMapForMemory for addr " << std::hex << startAddr << " with size " << size;
    size_t startMap = GetStaticArrayNumFromAddr(startAddr);
    size_t endMap = GetStaticArrayNumFromAddr(ToVoidPtr(ToUintPtr(startAddr) + size - 1));
    ASSERT(startMap <= endMap);
    for (size_t i = startMap; i <= endMap; i++) {
        ASSERT(GetStaticArrayElement(i) != nullptr);
        internalAllocator_->Free(GetStaticArrayElement(i));
        SetStaticArrayElement(i, nullptr);
    }
}

void *CrossingMap::FindObjInMap(size_t mapNum)
{
    LOG_CROSSING_MAP(DEBUG) << "Try to find object for map_num - " << mapNum;
    CrossingMapElement::STATE state = GetMapElement(mapNum)->GetState();
    switch (state) {
        case CrossingMapElement::STATE::STATE_UNINITIALIZED:
            LOG_CROSSING_MAP(DEBUG) << "STATE_UNINITIALIZED, return nullptr";
            return nullptr;
        case CrossingMapElement::STATE::STATE_INITIALIZED:
            LOG_CROSSING_MAP(DEBUG) << "STATE_INITIALIZED, obj addr = " << std::hex
                                    << GetAddrFromOffset(mapNum, GetMapElement(mapNum)->GetOffset());
            return GetAddrFromOffset(mapNum, GetMapElement(mapNum)->GetOffset());
        case CrossingMapElement::STATE::STATE_INITIALIZED_AND_CROSSED_BORDERS: {
            LOG_CROSSING_MAP(DEBUG)
                << "STATE_INITIALIZED_AND_CROSSED_BORDERS, try to find object which crosses the borders";
            ASSERT(mapNum > 0);
            size_t currentMap = mapNum - 1;
            while (GetMapElement(currentMap)->GetState() == CrossingMapElement::STATE::STATE_CROSSED_BORDER) {
                ASSERT(currentMap >= GetMapElement(currentMap)->GetOffset());
                currentMap = currentMap - GetMapElement(currentMap)->GetOffset();
            }
            ASSERT(GetMapElement(currentMap)->GetState() != CrossingMapElement::STATE::STATE_UNINITIALIZED);
            LOG_CROSSING_MAP(DEBUG) << "Found object in map " << currentMap << " with object addr = " << std::hex
                                    << GetAddrFromOffset(currentMap, GetMapElement(currentMap)->GetOffset());
            return GetAddrFromOffset(currentMap, GetMapElement(currentMap)->GetOffset());
        }
        case CrossingMapElement::STATE::STATE_CROSSED_BORDER: {
            LOG_CROSSING_MAP(DEBUG) << "STATE_CROSSED_BORDER, try to find object which crosses the borders";
            ASSERT(mapNum >= GetMapElement(mapNum)->GetOffset());
            size_t currentMap = mapNum - GetMapElement(mapNum)->GetOffset();
            while (GetMapElement(currentMap)->GetState() == CrossingMapElement::STATE::STATE_CROSSED_BORDER) {
                ASSERT(currentMap >= GetMapElement(currentMap)->GetOffset());
                currentMap = currentMap - GetMapElement(currentMap)->GetOffset();
            }
            ASSERT(GetMapElement(currentMap)->GetState() != CrossingMapElement::STATE::STATE_UNINITIALIZED);
            LOG_CROSSING_MAP(DEBUG) << "Found object in map " << currentMap << " with object addr = " << std::hex
                                    << GetAddrFromOffset(currentMap, GetMapElement(currentMap)->GetOffset());
            return GetAddrFromOffset(currentMap, GetMapElement(currentMap)->GetOffset());
        }
        default:
            LOG_CROSSING_MAP(ERROR) << "Undefined map state";
            return nullptr;
    }
}

}  // namespace ark::mem
