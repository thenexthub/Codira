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

#include <algorithm>
#include <atomic>

#include "libarkbase/macros.h"
#include "runtime/include/runtime.h"
#include "runtime/include/runtime_options.h"
#include "runtime/include/panda_vm.h"
#include "runtime/include/gc_task.h"
#include "runtime/mem/gc/gc_trigger.h"
#include "libarkbase/utils/logger.h"

namespace ark::mem {

static constexpr size_t PERCENT_100 = 100;

GCTriggerConfig::GCTriggerConfig(const RuntimeOptions &options, panda_file::SourceLang lang)
{
    auto runtimeLang = plugins::LangToRuntimeType(lang);
    gcTriggerType_ = options.GetGcTriggerType(runtimeLang);
    debugStart_ = options.GetGcDebugTriggerStart(runtimeLang);
    percentThreshold_ = std::min(options.GetGcTriggerPercentThreshold(), PERCENT_100_U32);
    adaptiveMultiplier_ = options.GetGcTriggerAdaptiveMultiplier();
    minExtraHeapSize_ = options.GetMinExtraHeapSize();
    maxExtraHeapSize_ = options.GetMaxExtraHeapSize();
    maxTriggerPercent_ = std::min(options.GetMaxTriggerPercent(), PERCENT_100_U32);
    skipStartupGcCount_ = options.GetSkipStartupGcCount(runtimeLang);
    useNthAllocTrigger_ = options.IsGcUseNthAllocTrigger();

    if (options.IsRunGcEverySafepoint()) {
        ASSERT_PRINT(gcTriggerType_ == "debug",
                     "Option 'run-gc-every-safepoint' must be used with 'gc-trigger-type=debug'");
        ASSERT_PRINT(options.IsRunGcInPlace(runtimeLang),
                     "Option 'run-gc-every-safepoint' must be used with 'run-gc-in-place'");
    }
}

GCTriggerHeap::GCTriggerHeap(MemStatsType *memStats, HeapSpace *heapSpace) : heapSpace_(heapSpace), memStats_(memStats)
{
}

GCTriggerHeap::GCTriggerHeap(MemStatsType *memStats, HeapSpace *heapSpace, size_t minHeapSize, uint8_t percentThreshold,
                             size_t minExtraSize, size_t maxExtraSize, uint32_t skipGcTimes)
    : heapSpace_(heapSpace), memStats_(memStats), skipGcCount_(skipGcTimes)
{
    percentThreshold_ = percentThreshold;
    minExtraSize_ = minExtraSize;
    maxExtraSize_ = maxExtraSize;
    // If we have min_heap_size < 100, we get false positives in TriggerGcIfNeeded, since we divide by 100 first
    ASSERT(minHeapSize >= 100);
    // Atomic with relaxed order reason: data race with target_footprint_ with no synchronization or ordering
    // constraints imposed on other reads or writes
    targetFootprint_.store((minHeapSize / PERCENT_100) * percentThreshold_, std::memory_order_relaxed);
    LOG(DEBUG, GC_TRIGGER) << "GCTriggerHeap created, min heap size " << minHeapSize << ", percent threshold "
                           << percentThreshold << ", min_extra_size " << minExtraSize << ", max_extra_size "
                           << maxExtraSize;
}

void GCTriggerHeap::SetMinTargetFootprint(size_t targetSize)
{
    LOG(DEBUG, GC_TRIGGER) << "SetTempTargetFootprint target_footprint = " << targetSize;
    minTargetFootprint_ = targetSize;
    // Atomic with relaxed order reason: data race with target_footprint_ with no synchronization or ordering
    // constraints imposed on other reads or writes
    targetFootprint_.store(targetSize, std::memory_order_relaxed);
}

void GCTriggerHeap::RestoreMinTargetFootprint()
{
    minTargetFootprint_ = DEFAULT_MIN_TARGET_FOOTPRINT;
}

void GCTriggerHeap::ComputeNewTargetFootprint(const GCTask &task, size_t heapSizeBeforeGc, size_t heapSize)
{
    GC *gc = Thread::GetCurrent()->GetVM()->GetGC();
    if (gc->IsGenerational() && task.reason == GCTaskCause::YOUNG_GC_CAUSE &&
        task.collectionType != GCCollectionType::MIXED) {
        // we don't want to update heap-trigger on young-gc
        return;
    }

    size_t target = this->ComputeTarget(heapSizeBeforeGc, heapSize);

    // Atomic with relaxed order reason: data race with target_footprint_ with no synchronization or ordering
    // constraints imposed on other reads or writes
    targetFootprint_.store(target, std::memory_order_relaxed);

    LOG(DEBUG, GC_TRIGGER) << "ComputeNewTargetFootprint target_footprint = " << target;
}

size_t GCTriggerHeap::ComputeTarget(size_t heapSizeBeforeGc, size_t heapSize)
{
    // Note: divide by 100 first to avoid overflow
    size_t delta = (heapSize / PERCENT_100) * percentThreshold_;

    // heap increased corresponding with previous gc
    if (heapSize > heapSizeBeforeGc) {
        delta = std::min(delta, maxExtraSize_);
    } else {
        // if heap was squeeze from 200mb to 100mb we want to set a target to 150mb, not just 100mb*percent_threshold_
        delta = std::max(delta, (heapSizeBeforeGc - heapSize) / 2);
    }
    return heapSize + std::max(delta, minExtraSize_);
}

void GCTriggerHeap::FinishPostponeGCIfNeeded(GC *gc)
{
    if (gc->IsPostponeEnabled() && ++gcPostponeCount_ == POSTPONE_GC_LIMIT) {
        gc->PostponeGCEnd();
        gcPostponeCount_ = 0;
    }
}

void GCTriggerHeap::TriggerGcIfNeeded(GC *gc)
{
    if (skipGcCount_ > 0) {
        skipGcCount_--;
        return;
    }

    if (!gc->CanAddGCTask()) {
        FinishPostponeGCIfNeeded(gc);
        return;
    }

    size_t bytesInHeap = memStats_->GetFootprintHeap();
    // Atomic with relaxed order reason: data race with target_footprint_ with no synchronization or ordering
    // constraints imposed on other reads or writes
    if (UNLIKELY(bytesInHeap >= targetFootprint_.load(std::memory_order_relaxed))) {
        LOG(DEBUG, GC_TRIGGER) << "GCTriggerHeap triggered";
        ASSERT(gc != nullptr);
        gc->PendingGC();
        auto task = MakePandaUnique<GCTask>(GCTaskCause::HEAP_USAGE_THRESHOLD_CAUSE, time::GetCurrentTimeInNanos());
        gc->Trigger(std::move(task));
    }
}

GCAdaptiveTriggerHeap::GCAdaptiveTriggerHeap(MemStatsType *memStats, HeapSpace *heapSpace, size_t minHeapSize,
                                             uint8_t percentThreshold, uint32_t adaptiveMultiplier, size_t minExtraSize,
                                             size_t maxExtraSize, uint32_t skipGcTimes)
    : GCTriggerHeap(memStats, heapSpace, minHeapSize, percentThreshold, minExtraSize, maxExtraSize, skipGcTimes),
      adaptiveMultiplier_(adaptiveMultiplier)
{
    // Atomic with relaxed order reason: data race with target_footprint_ with no synchronization or ordering
    // constraints imposed on other reads or writes
    recentTargetThresholds_.push_back(targetFootprint_.load(std::memory_order_relaxed));
}

size_t GCAdaptiveTriggerHeap::ComputeTarget(size_t heapSizeBeforeGc, size_t heapSize)
{
    auto delta = static_cast<size_t>(static_cast<double>(heapSize) / PERCENT_100_D * percentThreshold_);

    const auto [min_threshold, max_threshold] =
        std::minmax_element(recentTargetThresholds_.begin(), recentTargetThresholds_.end());
    size_t window = *max_threshold - *min_threshold;

    // if recent thresholds localize in "small" window then we need to get out from a location to avoid too many trigger
    if (window <= maxExtraSize_) {
        delta = std::max(delta, adaptiveMultiplier_ * maxExtraSize_);
        delta = std::min(delta, heapSize);
    } else if (heapSize > heapSizeBeforeGc) {  // heap increased corresponding with previous gc
        delta = std::min(delta, maxExtraSize_);
    } else {
        // if heap was squeeze from 200mb to 100mb we want to set a target to 150mb, not just 100mb*percent_threshold_
        delta = std::max(delta, (heapSizeBeforeGc - heapSize) / 2);
    }
    delta = std::max(delta, minExtraSize_);
    size_t target = heapSize + delta;

    recentTargetThresholds_.push_back(target);

    return target;
}

GCTriggerType GetTriggerType(std::string_view gcTriggerType)
{
    auto triggerType = GCTriggerType::INVALID_TRIGGER;
    if (gcTriggerType == "heap-trigger-test") {
        triggerType = GCTriggerType::HEAP_TRIGGER_TEST;
    } else if (gcTriggerType == "heap-trigger") {
        triggerType = GCTriggerType::HEAP_TRIGGER;
    } else if (gcTriggerType == "adaptive-heap-trigger") {
        triggerType = GCTriggerType::ADAPTIVE_HEAP_TRIGGER;
    } else if (gcTriggerType == "trigger-heap-occupancy") {
        triggerType = GCTriggerType::TRIGGER_HEAP_OCCUPANCY;
    } else if (gcTriggerType == "debug") {
        triggerType = GCTriggerType::DEBUG;
    } else if (gcTriggerType == "no-gc-for-start-up") {
        triggerType = GCTriggerType::NO_GC_FOR_START_UP;
    } else if (gcTriggerType == "debug-never") {
        triggerType = GCTriggerType::DEBUG_NEVER;
    } else if (gcTriggerType == "pause-time-goal-trigger") {
        triggerType = GCTriggerType::PAUSE_TIME_GOAL_TRIGGER;
    }
    return triggerType;
}

GCTrigger *CreateGCTrigger(MemStatsType *memStats, HeapSpace *heapSpace, const GCTriggerConfig &config,
                           InternalAllocatorPtr allocator)
{
    uint32_t skipGcTimes = config.GetSkipStartupGcCount();

    constexpr size_t DEFAULT_HEAP_SIZE = 8_MB;
    auto triggerType = GetTriggerType(config.GetGCTriggerType());

    GCTrigger *ret {nullptr};
    switch (triggerType) {  // NOLINT(hicpp-multiway-paths-covered)
        case GCTriggerType::HEAP_TRIGGER_TEST:
            // NOTE(dtrubenkov): replace with permanent allocator when we get it
            ret = allocator->New<GCTriggerHeap>(memStats, heapSpace);
            break;
        case GCTriggerType::HEAP_TRIGGER:
            ret = allocator->New<GCTriggerHeap>(memStats, heapSpace, DEFAULT_HEAP_SIZE, config.GetPercentThreshold(),
                                                config.GetMinExtraHeapSize(), config.GetMaxExtraHeapSize());
            break;
        case GCTriggerType::ADAPTIVE_HEAP_TRIGGER:
            ret = allocator->New<GCAdaptiveTriggerHeap>(memStats, heapSpace, DEFAULT_HEAP_SIZE,
                                                        config.GetPercentThreshold(), config.GetAdaptiveMultiplier(),
                                                        config.GetMinExtraHeapSize(), config.GetMaxExtraHeapSize());
            break;
        case GCTriggerType::NO_GC_FOR_START_UP:
            ret =
                allocator->New<GCTriggerHeap>(memStats, heapSpace, DEFAULT_HEAP_SIZE, config.GetPercentThreshold(),
                                              config.GetMinExtraHeapSize(), config.GetMaxExtraHeapSize(), skipGcTimes);
            break;
        case GCTriggerType::TRIGGER_HEAP_OCCUPANCY:
            ret = allocator->New<GCTriggerHeapOccupancy>(heapSpace, config.GetMaxTriggerPercent());
            break;
        case GCTriggerType::DEBUG:
            ret = allocator->New<GCTriggerDebug>(config.GetDebugStart(), heapSpace);
            break;
        case GCTriggerType::DEBUG_NEVER:
            ret = allocator->New<GCNeverTrigger>();
            break;
        case GCTriggerType::PAUSE_TIME_GOAL_TRIGGER:
            ret = allocator->New<PauseTimeGoalTrigger>(memStats, DEFAULT_HEAP_SIZE, config.GetPercentThreshold(),
                                                       config.GetMinExtraHeapSize(), config.GetMaxExtraHeapSize());
            break;
        default:
            LOG(FATAL, GC) << "Wrong GCTrigger type";
            break;
    }
    if (config.IsUseNthAllocTrigger()) {
        ret = allocator->New<SchedGCOnNthAllocTrigger>(ret);
    }
    return ret;
}

void GCTriggerHeap::GCStarted([[maybe_unused]] const GCTask &task, [[maybe_unused]] size_t heapSize)
{
    heapSpace_->SetIsWorkGC(true);
}

void GCTriggerHeap::GCFinished(const GCTask &task, size_t heapSizeBeforeGc, size_t heapSize)
{
    this->ComputeNewTargetFootprint(task, heapSizeBeforeGc, heapSize);
    heapSpace_->ComputeNewSize();
}

GCTriggerDebug::GCTriggerDebug(uint64_t debugStart, HeapSpace *heapSpace)
    : heapSpace_(heapSpace), debugStart_(debugStart)
{
    LOG(DEBUG, GC_TRIGGER) << "GCTriggerDebug created";
}

void GCTriggerDebug::TriggerGcIfNeeded(GC *gc)
{
    static std::atomic<uint64_t> counter = 0;
    LOG(DEBUG, GC_TRIGGER) << "GCTriggerDebug counter " << counter;
    if (counter >= debugStart_) {
        LOG(DEBUG, GC_TRIGGER) << "GCTriggerDebug triggered";
        auto task = MakePandaUnique<GCTask>(GCTaskCause::HEAP_USAGE_THRESHOLD_CAUSE, time::GetCurrentTimeInNanos());
        gc->Trigger(std::move(task));
    }
    counter++;
}

void GCTriggerDebug::GCStarted([[maybe_unused]] const GCTask &task, [[maybe_unused]] size_t heapSize)
{
    heapSpace_->SetIsWorkGC(true);
}

void GCTriggerDebug::GCFinished([[maybe_unused]] const GCTask &task, [[maybe_unused]] size_t heapSizeBeforeGc,
                                [[maybe_unused]] size_t heapSize)
{
    heapSpace_->ComputeNewSize();
}

GCTriggerHeapOccupancy::GCTriggerHeapOccupancy(HeapSpace *heapSpace, uint32_t maxTriggerPercent)
    : heapSpace_(heapSpace), maxTriggerPercent_(maxTriggerPercent / PERCENT_100_D)
{
    LOG(DEBUG, GC_TRIGGER) << "GCTriggerHeapOccupancy created";
}

void GCTriggerHeapOccupancy::GCStarted([[maybe_unused]] const GCTask &task, [[maybe_unused]] size_t heapSize)
{
    heapSpace_->SetIsWorkGC(true);
}

void GCTriggerHeapOccupancy::GCFinished([[maybe_unused]] const GCTask &task, [[maybe_unused]] size_t heapSizeBeforeGc,
                                        [[maybe_unused]] size_t heapSize)
{
    heapSpace_->ComputeNewSize();
}

void GCTriggerHeapOccupancy::TriggerGcIfNeeded(GC *gc)
{
    size_t currentHeapSize = heapSpace_->GetHeapSize();
    size_t minHeapSize = MemConfig::GetInitialHeapSizeLimit();
    size_t maxHeapSize = MemConfig::GetHeapSizeLimit();
    size_t threshold = std::min(minHeapSize, static_cast<size_t>(maxTriggerPercent_ * maxHeapSize));
    if (currentHeapSize > threshold) {
        LOG(DEBUG, GC_TRIGGER) << "GCTriggerHeapOccupancy triggered: current heap size = " << currentHeapSize
                               << ", threshold = " << threshold;
        auto task = MakePandaUnique<GCTask>(GCTaskCause::HEAP_USAGE_THRESHOLD_CAUSE, time::GetCurrentTimeInNanos());
        gc->Trigger(std::move(task));
    }
}

SchedGCOnNthAllocTrigger::SchedGCOnNthAllocTrigger(GCTrigger *origin) : origin_(origin) {}

SchedGCOnNthAllocTrigger::~SchedGCOnNthAllocTrigger()
{
    Runtime::GetCurrent()->GetInternalAllocator()->Delete(origin_);
}

void SchedGCOnNthAllocTrigger::TriggerGcIfNeeded(GC *gc)
{
    // Atomic with relaxed order reason: data race with other mutators
    uint32_t value = counter_.load(std::memory_order_relaxed);
    bool trigger = false;
    while (value > 0) {
        if (counter_.compare_exchange_strong(value, value - 1, std::memory_order_release, std::memory_order_relaxed)) {
            --value;
            trigger = (value == 0);
            break;
        }
        // Atomic with relaxed order reason: data race with other mutators
        value = counter_.load(std::memory_order_relaxed);
    }
    if (trigger) {
        gc->WaitForGCInManaged(GCTask(cause_));
        isTriggered_ = true;
    } else {
        origin_->TriggerGcIfNeeded(gc);
    }
}

void SchedGCOnNthAllocTrigger::ScheduleGc(GCTaskCause cause, uint32_t counter)
{
    isTriggered_ = false;
    cause_ = cause;
    counter_ = counter;
}

PauseTimeGoalTrigger::PauseTimeGoalTrigger(MemStatsType *memStats, size_t minHeapSize, uint8_t percentThreshold,
                                           size_t minExtraSize, size_t maxExtraSize)
    : memStats_(memStats),
      percentThreshold_(percentThreshold),
      minExtraSize_(minExtraSize),
      maxExtraSize_(maxExtraSize),
      targetFootprint_(minHeapSize * percentThreshold / PERCENT_100)
{
}

void PauseTimeGoalTrigger::TriggerGcIfNeeded(GC *gc)
{
    bool expectedStartConcurrent = true;
    // Atomic with relaxed order reason: data race with no synchronization or ordering constraints imposed on other
    // reads or writes
    auto startConcurrentMarking = startConcurrentMarking_.compare_exchange_weak(
        expectedStartConcurrent, false, std::memory_order_relaxed, std::memory_order_relaxed);
    if (UNLIKELY(startConcurrentMarking)) {
        LOG(DEBUG, GC_TRIGGER) << "PauseTimeGoalTrigger triggered";
        ASSERT(gc != nullptr);
        gc->PendingGC();
        auto result = gc->Trigger(
            MakePandaUnique<GCTask>(GCTaskCause::HEAP_USAGE_THRESHOLD_CAUSE, time::GetCurrentTimeInNanos()));
        if (!result) {
            // Atomic with relaxed order reason: data race with no synchronization or ordering constraints imposed
            // on other reads or writes
            startConcurrentMarking_.store(true, std::memory_order_relaxed);
        }
    }
}

void PauseTimeGoalTrigger::GCFinished(const GCTask &task, size_t heapSizeBeforeGc, size_t heapSize)
{
    if (task.reason == GCTaskCause::HEAP_USAGE_THRESHOLD_CAUSE) {
        return;
    }

    if (task.collectionType != GCCollectionType::YOUNG && task.collectionType != GCCollectionType::MIXED) {
        return;
    }

    if (task.collectionType == GCCollectionType::YOUNG) {
        auto bytesInHeap = memStats_->GetFootprintHeap();
        if (bytesInHeap >= GetTargetFootprint()) {
            // Atomic with relaxed order reason: data race with no synchronization or ordering constraints imposed
            // on other reads or writes
            startConcurrentMarking_.store(true, std::memory_order_relaxed);
        }
    }

    auto *gc = Thread::GetCurrent()->GetVM()->GetGC();
    gc->ComputeNewSize();

    if (task.collectionType == GCCollectionType::MIXED) {
        // Atomic with relaxed order reason: data race with no synchronization or ordering constraints imposed
        // on other reads or writes
        targetFootprint_.store(ComputeTarget(heapSizeBeforeGc, heapSize), std::memory_order_relaxed);
    }
}

// Curently it is copy of GCTriggerHeap::ComputeTarget. #11945
size_t PauseTimeGoalTrigger::ComputeTarget(size_t heapSizeBeforeGc, size_t heapSize)
{
    // Note: divide by 100 first to avoid overflow
    size_t delta = (heapSize / PERCENT_100) * percentThreshold_;

    // heap increased corresponding with previous gc
    if (heapSize > heapSizeBeforeGc) {
        delta = std::min(delta, maxExtraSize_);
    } else {
        // if heap was squeeze from 200mb to 100mb we want to set a target to 150mb, not just 100mb*percent_threshold_
        delta = std::max(delta, (heapSizeBeforeGc - heapSize) / 2);
    }
    return heapSize + std::max(delta, minExtraSize_);
}
}  // namespace ark::mem
