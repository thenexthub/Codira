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


#ifndef CODIRARUNTIME_ARRAYREFERENCECOUNTER_H
#define CODIRARUNTIME_ARRAYREFERENCECOUNTER_H

#include <cstdint>
#include <map>

#include "Base/Log.h"
#include "Base/LogFile.h"
#include "Base/SpinLock.h"

namespace MapleRuntime {

constexpr uintptr_t PAGE_SCALE = 12;

template<typename T>
class ArrayReferenceCounter {
public:
    ArrayReferenceCounter(const void* addr, size_t size, uintptr_t scale = PAGE_SCALE)
    {
        heapAddress = reinterpret_cast<uintptr_t>(addr);
        heapLength = reinterpret_cast<uintptr_t>(size);
        mapScale = scale;
        mapLenth = CalculateIndex(heapAddress + heapLength) + 1;
        mapList = new std::map<uintptr_t, std::pair<int64_t, T>>[mapLenth]();
        lockList = new SpinLock[mapLenth]();
    };

    ~ArrayReferenceCounter()
    {
        delete[] mapList;
        delete[] lockList;
    }

    std::pair<int64_t, T> CounterAddOrInsert(uintptr_t addr, T defaultVal)
    {
        auto index = CalculateIndex(addr);
        DLOG(SANITIZER, "locate counter map at %d for 0x%lx", index, addr);
        auto map = mapList + index;
        auto it = map->find(addr);
        if (it == map->end()) {
            auto counterData = std::pair<int64_t, T>{ 1, defaultVal };
            map->insert({ addr, counterData });
            return counterData;
        }

        auto count = it->second.first++;
        CHECK_DETAIL(count > 0, "invalid array reference with negative value");
        return it->second;
    }

    std::pair<int64_t, T> CounterSubOrDelete(uintptr_t addr)
    {
        auto index = CalculateIndex(addr);
        DLOG(SANITIZER, "locate counter map at %d for 0x%x", index, addr);
        // use pointer to change value in map
        auto map = mapList + index;
        auto it = map->find(addr);
        CHECK_DETAIL(it != map->end(), "invalid array release with zero reference");

        it->second.first--;
        auto currentValue = it->second;
        CHECK_DETAIL(it->second.first >= 0, "invalid array reference with negative value or zero");
        if (it->second.first == 0) {
            map->erase(it);
        }
        return currentValue;
    }

    inline void Lock(uintptr_t addr) { lockList[CalculateIndex(addr)].Lock(); }

    inline void Unlock(uintptr_t addr) { lockList[CalculateIndex(addr)].Unlock(); }

    bool Empty()
    {
        for (uintptr_t i = 0; i < mapLenth; ++i) {
            if (!mapList[i].empty()) {
                return false;
            }
        }
        return true;
    }

    void DiagnoseUnreleased(const std::function<void(uintptr_t, std::pair<int64_t, T>)>& callback)
    {
        for (uintptr_t i = 0; i < mapLenth; ++i) {
            if (mapList[i].empty()) {
                continue;
            }

            for (auto& it : mapList[i]) {
                callback(it.first, it.second);
            }
        }
    }

    bool inline constexpr AddressInRange(uintptr_t addr)
    {
        return addr >= heapAddress && CalculateIndex(addr) < mapLenth;
    }

private:
    inline constexpr uintptr_t CalculateIndex(uintptr_t addr) const { return (addr - heapAddress) >> mapScale; }

    std::map<uintptr_t, std::pair<int64_t, T>>* mapList;
    SpinLock* lockList;
    uintptr_t mapScale;
    uintptr_t mapLenth;
    uintptr_t heapAddress;
    uintptr_t heapLength;
};
} // namespace MapleRuntime

#endif // CODIRARUNTIME_ARRAYREFERENCECOUNTER_H
