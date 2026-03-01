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
#ifndef PANDA_RUNTIME_MEM_GC_GC_THRESHOLD_H
#define PANDA_RUNTIME_MEM_GC_GC_THRESHOLD_H

#include <cstddef>
#include <cstdint>
#include <string_view>

#include "libarkbase/macros.h"
#include "libarkbase/utils/ring_buffer.h"
#include "runtime/mem/gc/gc.h"

namespace ark {

class RuntimeOptions;

namespace test {
class GCTriggerTest;
}  // namespace test

namespace mem {

enum class GCTriggerType {
    INVALID_TRIGGER,
    HEAP_TRIGGER_TEST,        // TRIGGER with low thresholds for tests
    HEAP_TRIGGER,             // Standard TRIGGER with production ready thresholds
    ADAPTIVE_HEAP_TRIGGER,    // TRIGGER with adaptive strategy for heap increasing
    NO_GC_FOR_START_UP,       // A non-production strategy, TRIGGER GC after the app starts up
    TRIGGER_HEAP_OCCUPANCY,   // Trigger full GC by heap size threshold
    DEBUG,                    // Debug TRIGGER which always returns true
    DEBUG_NEVER,              // Trigger for testing which never triggers (young-gc can still trigger), for test purpose
    ON_NTH_ALLOC,             // Triggers GC on n-th allocation
    PAUSE_TIME_GOAL_TRIGGER,  // Triggers concurrent marking by heap size threshold and ask GC to change eden size to
                              // satisfy pause time goal
    XGC,                      // Trigger for cross VM references GC
};

class GCTriggerConfig {
public:
    GCTriggerConfig(const RuntimeOptions &options, panda_file::SourceLang lang);

    std::string_view GetGCTriggerType() const
    {
        return gcTriggerType_;
    }

    uint64_t GetDebugStart() const
    {
        return debugStart_;
    }

    uint32_t GetPercentThreshold() const
    {
        return percentThreshold_;
    }

    uint32_t GetAdaptiveMultiplier() const
    {
        return adaptiveMultiplier_;
    }

    size_t GetMinExtraHeapSize() const
    {
        return minExtraHeapSize_;
    }

    size_t GetMaxExtraHeapSize() const
    {
        return maxExtraHeapSize_;
    }

    uint32_t GetMaxTriggerPercent() const
    {
        return maxTriggerPercent_;
    }

    uint32_t GetSkipStartupGcCount() const
    {
        return skipStartupGcCount_;
    }

    bool IsUseNthAllocTrigger() const
    {
        return useNthAllocTrigger_;
    }

private:
    std::string gcTriggerType_;
    uint64_t debugStart_;
    uint32_t percentThreshold_;
    uint32_t adaptiveMultiplier_;
    size_t minExtraHeapSize_;
    size_t maxExtraHeapSize_;
    uint32_t maxTriggerPercent_;
    uint32_t skipStartupGcCount_;
    bool useNthAllocTrigger_;
};

class GCTrigger : public GCListener {
public:
    GCTrigger() = default;
    ~GCTrigger() override = default;

    NO_COPY_SEMANTIC(GCTrigger);
    NO_MOVE_SEMANTIC(GCTrigger);

    /**
     * @brief Checks if GC required
     * @return returns true if GC should be executed
     */
    virtual void TriggerGcIfNeeded(GC *gc) = 0;
    virtual GCTriggerType GetType() const = 0;
    virtual void SetMinTargetFootprint([[maybe_unused]] size_t heapSize) {}
    virtual void RestoreMinTargetFootprint() {}

private:
    friend class GC;
};

/// Triggers when heap increased by predefined %
class GCTriggerHeap : public GCTrigger {
public:
    explicit GCTriggerHeap(MemStatsType *memStats, HeapSpace *heapSpace);
    explicit GCTriggerHeap(MemStatsType *memStats, HeapSpace *heapSpace, size_t minHeapSize, uint8_t percentThreshold,
                           size_t minExtraSize, size_t maxExtraSize, uint32_t skipGcTimes = 0);

    GCTriggerType GetType() const override
    {
        return GCTriggerType::HEAP_TRIGGER;
    }

    void TriggerGcIfNeeded(GC *gc) override;
    void GCStarted(const GCTask &task, size_t heapSize) override;
    void GCFinished(const GCTask &task, size_t heapSizeBeforeGc, size_t heapSize) override;
    void SetMinTargetFootprint(size_t targetSize) override;
    void RestoreMinTargetFootprint() override;
    void ComputeNewTargetFootprint(const GCTask &task, size_t heapSizeBeforeGc, size_t heapSize);

    static constexpr uint8_t DEFAULT_PERCENTAGE_THRESHOLD = 20;

protected:
    // NOTE(dtrubenkov): change to the proper value when all triggers will be enabled
    static constexpr size_t MIN_HEAP_SIZE_FOR_TRIGGER = 512;
    static constexpr size_t DEFAULT_MIN_TARGET_FOOTPRINT = 256;
    static constexpr size_t DEFAULT_MIN_EXTRA_HEAP_SIZE = 32;      // For heap-trigger-test
    static constexpr size_t DEFAULT_MAX_EXTRA_HEAP_SIZE = 512_KB;  // For heap-trigger-test

    virtual size_t ComputeTarget(size_t heapSizeBeforeGc, size_t heapSize);

    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    std::atomic<size_t> targetFootprint_ {MIN_HEAP_SIZE_FOR_TRIGGER};

    /**
     * We'll trigger if heap increased by delta, delta = heap_size_after_last_gc * percent_threshold_ %
     * And the constraint on delta is: min_extra_size_ <= delta <= max_extra_size_
     */
    uint8_t percentThreshold_ {DEFAULT_PERCENTAGE_THRESHOLD};  // NOLINT(misc-non-private-member-variables-in-classes)
    size_t minExtraSize_ {DEFAULT_MIN_EXTRA_HEAP_SIZE};        // NOLINT(misc-non-private-member-variables-in-classes)
    size_t maxExtraSize_ {DEFAULT_MAX_EXTRA_HEAP_SIZE};        // NOLINT(misc-non-private-member-variables-in-classes)

private:
    void FinishPostponeGCIfNeeded(GC *gc);

    static constexpr uint32_t POSTPONE_GC_LIMIT = 10;

    HeapSpace *heapSpace_ {nullptr};
    size_t minTargetFootprint_ {DEFAULT_MIN_TARGET_FOOTPRINT};
    MemStatsType *memStats_;
    uint8_t skipGcCount_ {0};
    uint32_t gcPostponeCount_ {0};

    friend class ark::test::GCTriggerTest;
};

/// Triggers when heap increased by adaptive strategy
class GCAdaptiveTriggerHeap : public GCTriggerHeap {
public:
    GCAdaptiveTriggerHeap(MemStatsType *memStats, HeapSpace *heapSpace, size_t minHeapSize, uint8_t percentThreshold,
                          uint32_t adaptiveMultiplier, size_t minExtraSize, size_t maxExtraSize,
                          uint32_t skipGcTimes = 0);
    NO_COPY_SEMANTIC(GCAdaptiveTriggerHeap);
    NO_MOVE_SEMANTIC(GCAdaptiveTriggerHeap);
    ~GCAdaptiveTriggerHeap() override = default;

    GCTriggerType GetType() const override
    {
        return GCTriggerType::ADAPTIVE_HEAP_TRIGGER;
    }

private:
    static constexpr uint32_t DEFAULT_INCREASE_MULTIPLIER = 3U;
    // We save last RECENT_THRESHOLDS_COUNT thresholds for detect too often triggering
    static constexpr size_t RECENT_THRESHOLDS_COUNT = 3;

    size_t ComputeTarget(size_t heapSizeBeforeGc, size_t heapSize) override;

    RingBuffer<size_t, RECENT_THRESHOLDS_COUNT> recentTargetThresholds_;
    uint32_t adaptiveMultiplier_ {DEFAULT_INCREASE_MULTIPLIER};

    friend class ark::test::GCTriggerTest;
};

/// Trigger always returns true after given start
class GCTriggerDebug : public GCTrigger {
public:
    explicit GCTriggerDebug(uint64_t debugStart, HeapSpace *heapSpace);

    GCTriggerType GetType() const override
    {
        return GCTriggerType::DEBUG;
    }

    void TriggerGcIfNeeded(GC *gc) override;
    void GCStarted(const GCTask &task, size_t heapSize) override;
    void GCFinished(const GCTask &task, size_t heapSizeBeforeGc, size_t heapSize) override;

private:
    HeapSpace *heapSpace_ {nullptr};
    uint64_t debugStart_ = 0;
};

class GCTriggerHeapOccupancy : public GCTrigger {
public:
    explicit GCTriggerHeapOccupancy(HeapSpace *heapSpace, uint32_t maxTriggerPercent);

    GCTriggerType GetType() const override
    {
        return GCTriggerType::TRIGGER_HEAP_OCCUPANCY;
    }

    void TriggerGcIfNeeded(GC *gc) override;

    void GCStarted(const GCTask &task, [[maybe_unused]] size_t heapSize) override;
    void GCFinished([[maybe_unused]] const GCTask &task, [[maybe_unused]] size_t heapSizeBeforeGc,
                    [[maybe_unused]] size_t heapSize) override;

private:
    HeapSpace *heapSpace_ {nullptr};
    double maxTriggerPercent_ = 0.0;
};

class GCNeverTrigger : public GCTrigger {
public:
    GCTriggerType GetType() const override
    {
        return GCTriggerType::DEBUG_NEVER;
    }

    void TriggerGcIfNeeded([[maybe_unused]] GC *gc) override {}

    void GCStarted([[maybe_unused]] const GCTask &task, [[maybe_unused]] size_t heapSize) override {}
    void GCFinished([[maybe_unused]] const GCTask &task, [[maybe_unused]] size_t heapSizeBeforeGc,
                    [[maybe_unused]] size_t heapSize) override
    {
    }
};

class SchedGCOnNthAllocTrigger : public GCTrigger {
public:
    explicit SchedGCOnNthAllocTrigger(GCTrigger *origin);
    ~SchedGCOnNthAllocTrigger() override;

    GCTriggerType GetType() const override
    {
        return GCTriggerType::ON_NTH_ALLOC;
    }

    bool IsTriggered() const
    {
        return isTriggered_;
    }

    void TriggerGcIfNeeded(GC *gc) override;
    void ScheduleGc(GCTaskCause cause, uint32_t counter);
    void GCStarted([[maybe_unused]] const GCTask &task, [[maybe_unused]] size_t heapSize) override {}
    void GCFinished([[maybe_unused]] const GCTask &task, [[maybe_unused]] size_t heapSizeBeforeGc,
                    [[maybe_unused]] size_t heapSize) override
    {
    }

    NO_COPY_SEMANTIC(SchedGCOnNthAllocTrigger);
    NO_MOVE_SEMANTIC(SchedGCOnNthAllocTrigger);

private:
    GCTrigger *origin_;
    std::atomic<uint32_t> counter_ = 0;
    GCTaskCause cause_ = GCTaskCause::HEAP_USAGE_THRESHOLD_CAUSE;
    bool isTriggered_ = false;
};

class PauseTimeGoalTrigger : public GCTrigger {
public:
    PauseTimeGoalTrigger(MemStatsType *memStats, size_t minHeapSize, uint8_t percentThreshold, size_t minExtraSize,
                         size_t maxExtraSize);
    NO_COPY_SEMANTIC(PauseTimeGoalTrigger);
    NO_MOVE_SEMANTIC(PauseTimeGoalTrigger);
    ~PauseTimeGoalTrigger() override = default;

    GCTriggerType GetType() const override
    {
        return GCTriggerType::PAUSE_TIME_GOAL_TRIGGER;
    }

    void GCFinished(const GCTask &task, size_t heapSizeBeforeGc, size_t heapSize) override;

    void TriggerGcIfNeeded(GC *gc) override;

    size_t GetTargetFootprint() const
    {
        // Atomic with relaxed order reason: data race with target_footprint_ with no synchronization or ordering
        // constraints imposed on other reads or writes
        return targetFootprint_.load(std::memory_order_relaxed);
    }

private:
    size_t ComputeTarget(size_t heapSizeBeforeGc, size_t heapSize);
    MemStatsType *memStats_;
    uint8_t percentThreshold_;
    size_t minExtraSize_;
    size_t maxExtraSize_;
    std::atomic<size_t> targetFootprint_;
    std::atomic<bool> startConcurrentMarking_ {false};
};

GCTrigger *CreateGCTrigger(MemStatsType *memStats, HeapSpace *heapSpace, const GCTriggerConfig &config,
                           InternalAllocatorPtr allocator);

}  // namespace mem
}  // namespace ark

#endif  // PANDA_RUNTIME_MEM_GC_GC_THRESHOLD_H
