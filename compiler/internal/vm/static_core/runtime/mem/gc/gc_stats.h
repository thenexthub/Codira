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
#ifndef PANDA_RUNTIME_MEM_GC_GC_STATS_H
#define PANDA_RUNTIME_MEM_GC_GC_STATS_H

#include "libarkbase/os/mutex.h"
#include "runtime/include/histogram-inl.h"
#include "runtime/include/mem/panda_containers.h"
#include "runtime/include/mem/panda_string.h"
#include "runtime/include/time_utils.h"
#include "runtime/mem/gc/gc_types.h"

#include <algorithm>
#include <atomic>
#include <ratio>

namespace ark::mem {

namespace test {
class MemStatsGenGCTest;
}  // namespace test

class GCStats;
class HeapManager;

enum class ObjectTypeStats : size_t {
    YOUNG_FREED_OBJECTS = 0,
    MOVED_OBJECTS,
    ALL_FREED_OBJECTS,

    OBJECT_TYPE_STATS_LAST
};

constexpr size_t ToIndex(ObjectTypeStats type)
{
    return static_cast<size_t>(type);
}

constexpr size_t OBJECT_TYPE_STATS_SIZE = static_cast<size_t>(ObjectTypeStats::OBJECT_TYPE_STATS_LAST);

enum class MemoryTypeStats : size_t {
    YOUNG_FREED_BYTES = 0,
    MOVED_BYTES,
    ALL_FREED_BYTES,

    MEMORY_TYPE_STATS_LAST
};

constexpr size_t ToIndex(MemoryTypeStats type)
{
    return static_cast<size_t>(type);
}

constexpr size_t MEMORY_TYPE_STATS_SIZE = static_cast<size_t>(MemoryTypeStats::MEMORY_TYPE_STATS_LAST);

enum class TimeTypeStats : size_t {
    YOUNG_PAUSED_TIME = 0,
    YOUNG_TOTAL_TIME,
    ALL_PAUSED_TIME,
    ALL_TOTAL_TIME,

    TIME_TYPE_STATS_LAST
};

constexpr size_t ToIndex(TimeTypeStats type)
{
    return static_cast<size_t>(type);
}

constexpr size_t TIME_TYPE_STATS_SIZE = static_cast<size_t>(TimeTypeStats::TIME_TYPE_STATS_LAST);

enum class PauseTypeStats : size_t {
    COMMON_PAUSE = 0,
    INITIAL_MARK_PAUSE,
    REMARK_PAUSE,

    PAUSE_TYPE_STATS_LAST
};

constexpr size_t ToIndex(PauseTypeStats type)
{
    return static_cast<size_t>(type);
}

constexpr PauseTypeStats ToPauseTypeStats(size_t type)
{
    return static_cast<PauseTypeStats>(type);
}

constexpr std::string_view ToString(PauseTypeStats type)
{
    switch (type) {
        case PauseTypeStats::COMMON_PAUSE:
            return "COMMON_PAUSE";
        case PauseTypeStats::INITIAL_MARK_PAUSE:
            return "INITIAL_MARK_PAUSE";
        case PauseTypeStats::REMARK_PAUSE:
            return "REMARK_PAUSE";
        default:
            return "UnknownPauseType";
    }
}

constexpr size_t PAUSE_TYPE_STATS_SIZE = static_cast<size_t>(PauseTypeStats::PAUSE_TYPE_STATS_LAST);

// scoped specific for GC Stats
class GCInstanceStats {
public:
    GCInstanceStats()
    {
        std::fill(begin(objectsStats_), end(objectsStats_),
                  SimpleHistogram<uint64_t>(helpers::ValueType::VALUE_TYPE_OBJECT));
        std::fill(begin(memoryStats_), end(memoryStats_),
                  SimpleHistogram<uint64_t>(helpers::ValueType::VALUE_TYPE_MEMORY));
        std::fill(begin(timeStats_), end(timeStats_), SimpleHistogram<uint64_t>(helpers::ValueType::VALUE_TYPE_TIME));
    }

    void AddObjectsValue(uint64_t value, ObjectTypeStats memoryType)
    {
        auto index = static_cast<size_t>(memoryType);
        objectsStats_[index].AddValue(value);
    }

    void AddMemoryValue(uint64_t value, MemoryTypeStats memoryType)
    {
        auto index = static_cast<size_t>(memoryType);
        memoryStats_[index].AddValue(value);
    }

    void AddTimeValue(uint64_t value, TimeTypeStats timeType)
    {
        auto index = static_cast<size_t>(timeType);
        timeStats_[index].AddValue(value);
    }

    void AddReclaimRatioValue(double value)
    {
        reclaimBytes_.AddValue(value);
    }

    void AddCopiedRatioValue(double value)
    {
        copiedBytes_.AddValue(value);
    }

    PandaString GetDump(GCType gcType);

    virtual ~GCInstanceStats() = default;

    NO_COPY_SEMANTIC(GCInstanceStats);
    NO_MOVE_SEMANTIC(GCInstanceStats);

private:
    PandaString GetYoungSpaceDump(GCType gcType);
    PandaString GetAllSpacesDump(GCType gcType);
    std::array<SimpleHistogram<uint64_t>, OBJECT_TYPE_STATS_SIZE> objectsStats_;
    std::array<SimpleHistogram<uint64_t>, MEMORY_TYPE_STATS_SIZE> memoryStats_;
    std::array<SimpleHistogram<uint64_t>, TIME_TYPE_STATS_SIZE> timeStats_;
    SimpleHistogram<double> reclaimBytes_;
    SimpleHistogram<double> copiedBytes_;
};

// scoped all field GCStats except pause_
class GCScopedStats {
public:
    explicit GCScopedStats(GCStats *stats, GCInstanceStats *instanceStats = nullptr);

    NO_COPY_SEMANTIC(GCScopedStats);
    NO_MOVE_SEMANTIC(GCScopedStats);

    ~GCScopedStats();

private:
    uint64_t startTime_;
    GCInstanceStats *instanceStats_;
    GCStats *stats_;
};

// scoped field GCStats while GC in pause
class GCScopedPauseStats {
public:
    explicit GCScopedPauseStats(GCStats *stats, GCInstanceStats *instanceStats = nullptr,
                                PauseTypeStats pauseType = PauseTypeStats::COMMON_PAUSE);

    NO_COPY_SEMANTIC(GCScopedPauseStats);
    NO_MOVE_SEMANTIC(GCScopedPauseStats);

    ~GCScopedPauseStats();

private:
    uint64_t startTime_;
    GCInstanceStats *instanceStats_;
    GCStats *stats_;
    PauseTypeStats pauseType_;
};

class GCStats {
public:
    explicit GCStats(MemStatsType *memStats, GCType gcTypeFromRuntime, InternalAllocatorPtr allocator);
    ~GCStats();

    NO_COPY_SEMANTIC(GCStats);
    NO_MOVE_SEMANTIC(GCStats);

    PandaString GetStatistics();

    PandaString GetFinalStatistics(HeapManager *heapManager);

    PandaString GetPhasePauseStat(PauseTypeStats pauseType);

    uint64_t GetPhasePause(PauseTypeStats pauseType);
    void ResetLastPause();

    size_t GetObjectsFreedBytes()
    {
#ifdef PANDA_TARGET_64
        static_assert(sizeof(objectsFreedBytes_) == sizeof(std::atomic_uint64_t));
        // Atomic with seq_cst order reason: data race with objects_freed_bytes_ with requirement for sequentially
        // consistent order where threads observe all modifications in the same order
        return reinterpret_cast<std::atomic_uint64_t *>(&objectsFreedBytes_)->load(std::memory_order_seq_cst);
#endif
#ifdef PANDA_TARGET_32
        static_assert(sizeof(objectsFreedBytes_) == sizeof(std::atomic_uint32_t));
        // Atomic with seq_cst order reason: data race with objects_freed_bytes_ with requirement for sequentially
        // consistent order where threads observe all modifications in the same order
        return reinterpret_cast<std::atomic_uint32_t *>(&objectsFreedBytes_)->load(std::memory_order_seq_cst);
#endif
        UNREACHABLE();
    }

    uint64_t GetObjectsFreedCount()
    {
        return objectsFreed_;
    }

    uint64_t GetLargeObjectsFreedBytes()
    {
        return largeObjectsFreedBytes_;
    }

    uint64_t GetLargeObjectsFreedCount()
    {
        return largeObjectsFreed_;
    }

    uint64_t GetTotalDuration() const
    {
        return totalDuration_;
    }

    MemStatsType *GetMemStats() const
    {
        return memStats_;
    }

    void StartMutatorLock();
    void StopMutatorLock();

private:
    // For convert from nano to 10 seconds
    using PERIOD = std::deca;
    GCType gcType_ {GCType::INVALID_GC};
    size_t objectsFreed_ {0};
    size_t objectsFreedBytes_ {0};
    size_t largeObjectsFreed_ {0};
    size_t largeObjectsFreedBytes_ {0};
    uint64_t startTime_ {0};
    // CC-OFFNXT(G.FMT.03) project code style
    size_t countMutator_ GUARDED_BY(mutatorStatsLock_) {0};
    // CC-OFFNXT(G.FMT.03) project code style
    uint64_t mutatorStartTime_ GUARDED_BY(mutatorStatsLock_) {0};

    uint64_t lastDuration_ {0};
    uint64_t totalDuration_ {0};
    uint64_t totalPause_ {0};
    // CC-OFFNXT(G.FMT.03) project code style
    uint64_t totalMutatorPause_ GUARDED_BY(mutatorStatsLock_) {0};

    uint64_t lastStartDuration_ {0};
    // GC in the last PERIOD
    uint64_t countGcPeriod_ {0};
    // GC number of times every PERIOD
    PandaVector<uint64_t> *allNumberDurations_ {nullptr};

    std::array<uint64_t, PAUSE_TYPE_STATS_SIZE> lastPause_ {};

    os::memory::Mutex mutatorStatsLock_;
    MemStatsType *memStats_;

    void StartCollectStats();
    void StopCollectStats(GCInstanceStats *instanceStats);

    void AddPause(uint64_t pause, GCInstanceStats *instanceStats, PauseTypeStats pauseType);

    void RecordDuration(uint64_t duration, GCInstanceStats *instanceStats);

    uint64_t ConvertTimeToPeriod(uint64_t timeInNanos, bool ceil = false);

    InternalAllocatorPtr allocator_ {nullptr};

#ifndef NDEBUG
    PauseTypeStats prevPauseType_ {PauseTypeStats::COMMON_PAUSE};
#endif

    friend GCScopedPauseStats;
    friend GCScopedStats;
    friend test::MemStatsGenGCTest;
};

}  // namespace ark::mem

#endif  // PANDA_RUNTIME_MEM_GC_GC_STATS_H
