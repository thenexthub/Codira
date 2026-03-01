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
#ifndef PANDA_RUNTIME_MEM_STATS_H
#define PANDA_RUNTIME_MEM_STATS_H

#include <chrono>
#include <cstdint>
#include <cstring>

#include "libarkbase/macros.h"
#include "libarkbase/mem/base_mem_stats.h"
#include "libarkbase/os/mutex.h"
#include "runtime/include/mem/panda_string.h"
#include "runtime/mem/gc/gc_phase.h"

namespace ark {
class BaseClass;
}  // namespace ark

namespace ark::mem {

class HeapManager;

/**
 * Class for recording memory usage in the VM. Allocators use this class for both cases: object allocation in heap and
 * raw memory for VM needs as well. This class uses CRTP for storing additional information in DEBUG mode.
 */
template <typename T>
class MemStats : public BaseMemStats {
public:
    NO_COPY_SEMANTIC(MemStats);
    NO_MOVE_SEMANTIC(MemStats);

    MemStats() = default;

    ~MemStats() override = default;

    void RecordAllocateObject(size_t size, SpaceType typeMem);

    void RecordAllocateObjects(size_t totalObjectNum, size_t totalObjectSize, SpaceType typeMem);

    void RecordYoungMovedObjects(size_t youngObjectNum, size_t size, SpaceType typeMem);

    void RecordTenuredMovedObjects(size_t tenuredObjectNum, size_t size, SpaceType typeMem);

    void RecordFreeObject(size_t objectSize, SpaceType typeMem);

    void RecordFreeObjects(size_t totalObjectNum, size_t totalObjectSize, SpaceType typeMem);

    /// Number of allocated objects for all time
    [[nodiscard]] uint64_t GetTotalObjectsAllocated() const;

    /// Number of freed objects for all time
    [[nodiscard]] uint64_t GetTotalObjectsFreed() const;

    /// Number of allocated large and regular (size <= FREELIST_MAX_ALLOC_SIZE) objects for all time
    [[nodiscard]] uint64_t GetTotalRegularObjectsAllocated() const;

    /// Number of freed large and regular (size <= FREELIST_MAX_ALLOC_SIZE) objects for all time
    [[nodiscard]] uint64_t GetTotalRegularObjectsFreed() const;

    /// Number of allocated humongous (size > FREELIST_MAX_ALLOC_SIZE) objects for all time
    [[nodiscard]] uint64_t GetTotalHumongousObjectsAllocated() const;

    /// Number of freed humongous (size > FREELIST_MAX_ALLOC_SIZE) objects for all time
    [[nodiscard]] uint64_t GetTotalHumongousObjectsFreed() const;

    /// Number of alive objects now
    [[nodiscard]] uint64_t GetObjectsCountAlive() const;

    /// Number of alive large and regular (size <= FREELIST_MAX_ALLOC_SIZE) objects now
    [[nodiscard]] uint64_t GetRegularObjectsCountAlive() const;

    /// Number of alive humongous (size > FREELIST_MAX_ALLOC_SIZE) objects now
    [[nodiscard]] uint64_t GetHumonguousObjectsCountAlive() const;

    /// Number of moved bytes in last gc
    [[nodiscard]] uint64_t GetLastYoungObjectsMovedBytes() const;

    void ClearLastYoungObjectsMovedBytes()
    {
        lastYoungObjectsMovedBytes_ = 0;
    }

    PandaString GetStatistics();

private:
    std::atomic_uint64_t lastYoungObjectsMovedBytes_ = 0;

    // make groups of different parts of the VM (JIT, interpreter, etc)
    std::atomic_uint64_t objectsAllocated_ = 0;
    std::atomic_uint64_t objectsFreed_ = 0;

    std::atomic_uint64_t humongousObjectsAllocated_ = 0;
    std::atomic_uint64_t humongousObjectsFreed_ = 0;
};

}  // namespace ark::mem
#endif  // PANDA_RUNTIME_MEM_STATS_H
