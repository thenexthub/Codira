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

#include "libarkbase/utils/time.h"
#include "libarkbase/utils/type_converter.h"
#include "runtime/include/runtime.h"
#include "runtime/include/panda_vm.h"
#include "runtime/mem/gc/gc_stats.h"
#include "runtime/mem/mem_stats.h"
#include "runtime/mem/gc/gc_settings.h"

namespace ark::mem {

PandaString GCStats::GetStatistics()
{
    PandaStringStream statistic;
    statistic << time::GetCurrentTimeString() << " ";

    statistic << GC_NAMES[ToIndex(gcType_)] << " ";
    auto *gc = Runtime::GetCurrent()->GetPandaVM()->GetGC();
    static bool g1TrackFreedObjects = gc->GetSettings()->G1TrackFreedObjects();
    if (g1TrackFreedObjects) {
        statistic << "freed " << objectsFreed_ << "(" << helpers::MemoryConverter(objectsFreedBytes_) << "), ";
        statistic << largeObjectsFreed_ << "(" << helpers::MemoryConverter(largeObjectsFreedBytes_)
                  << ") LOS objects, ";
    } else {
        statistic << "freed " << helpers::MemoryConverter(objectsFreedBytes_) << ", ";
        statistic << helpers::MemoryConverter(largeObjectsFreedBytes_) << " LOS objects, ";
    }
    constexpr uint16_t MAX_PERCENT = 100;
    size_t totalHeap = mem::MemConfig::GetHeapSizeLimit();
    ASSERT(totalHeap != 0);
    size_t allocatedNow = memStats_->GetFootprintHeap();
    ASSERT(allocatedNow <= totalHeap);
    uint16_t percent = round((1 - (allocatedNow * 1.0 / totalHeap)) * MAX_PERCENT);
    statistic << percent << "% free, " << helpers::MemoryConverter(allocatedNow) << "/"
              << helpers::MemoryConverter(totalHeap) << ", ";
    bool initialMarkPause = GetPhasePause(PauseTypeStats::INITIAL_MARK_PAUSE) != 0U;
    bool remarkPause = IsGenerationalGCType(gcType_) && (GetPhasePause(PauseTypeStats::REMARK_PAUSE) != 0U);
    statistic << GetPhasePauseStat(PauseTypeStats::COMMON_PAUSE)
              << (initialMarkPause ? GetPhasePauseStat(PauseTypeStats::INITIAL_MARK_PAUSE) : "")
              << (remarkPause ? GetPhasePauseStat(PauseTypeStats::REMARK_PAUSE) : "")
              << " total: " << helpers::TimeConverter(lastDuration_);
    return statistic.str();
}

PandaString GCStats::GetFinalStatistics(HeapManager *heapManager)
{
    auto totalTime = ConvertTimeToPeriod(time::GetCurrentTimeInNanos() - startTime_, true);
    auto totalTimeGc = helpers::TimeConverter(totalDuration_);
    auto totalAllocated = memStats_->GetAllocatedHeap();
    auto totalFreed = memStats_->GetFreedHeap();
    auto totalObjects = memStats_->GetTotalObjectsAllocated();

    auto currentMemory = memStats_->GetFootprintHeap();
    auto totalMemory = heapManager->GetTotalMemory();
    auto maxMemory = heapManager->GetMaxMemory();

    Histogram<uint64_t> durationInfo(allNumberDurations_->begin(), allNumberDurations_->end());

    if (countGcPeriod_ != 0U) {
        durationInfo.AddValue(countGcPeriod_);
    }
    if (totalTime > durationInfo.GetCountDifferent()) {
        durationInfo.AddValue(0, totalTime - durationInfo.GetCountDifferent());
    }
    PandaStringStream statistic;

    statistic << heapManager->GetGC()->DumpStatistics() << "\n";

    statistic << "Total time spent in GC: " << totalTimeGc << "\n";

    statistic << "Mean GC size throughput " << helpers::MemoryConverter(totalAllocated / totalTimeGc.GetDoubleValue())
              << "/" << totalTimeGc.GetLiteral() << "\n";
    statistic << "Mean GC object throughput: " << std::scientific << totalObjects / totalTimeGc.GetDoubleValue()
              << " objects/" << totalTimeGc.GetLiteral() << "\n";
    statistic << "Total number of allocations " << totalObjects << "\n";
    statistic << "Total bytes allocated " << helpers::MemoryConverter(totalAllocated) << "\n";
    statistic << "Total bytes freed " << helpers::MemoryConverter(totalFreed) << "\n\n";

    statistic << "Free memory " << helpers::MemoryConverter(helpers::UnsignedDifference(totalMemory, currentMemory))
              << "\n";
    statistic << "Free memory until GC " << helpers::MemoryConverter(heapManager->GetFreeMemory()) << "\n";
    statistic << "Free memory until OOME "
              << helpers::MemoryConverter(helpers::UnsignedDifference(maxMemory, currentMemory)) << "\n";
    statistic << "Total memory " << helpers::MemoryConverter(totalMemory) << "\n";

    {
        os::memory::LockHolder lock(mutatorStatsLock_);
        statistic << "Total mutator paused time: " << helpers::TimeConverter(totalMutatorPause_) << "\n";
    }
    statistic << "Total time waiting for GC to complete: " << helpers::TimeConverter(totalPause_) << "\n";
    statistic << "Total GC count: " << durationInfo.GetSum() << "\n";
    statistic << "Total GC time: " << totalTimeGc << "\n";
    statistic << "Total blocking GC count: " << durationInfo.GetSum() << "\n";
    statistic << "Total blocking GC time: " << totalTimeGc << "\n";
    statistic << "Histogram of GC count per 10000 ms: " << durationInfo.GetTopDump() << "\n";
    statistic << "Histogram of blocking GC count per 10000 ms: " << durationInfo.GetTopDump() << "\n";

    statistic << "Native bytes registered: " << heapManager->GetGC()->GetNativeBytesRegistered() << "\n\n";

    statistic << "Max memory " << helpers::MemoryConverter(maxMemory) << "\n";

    return statistic.str();
}

PandaString GCStats::GetPhasePauseStat(PauseTypeStats pauseType)
{
    PandaStringStream statistic;
    statistic << " phase: " << ToString(pauseType)
              << " paused: " << helpers::TimeConverter(lastPause_[ToIndex(pauseType)]);
    return statistic.str();
}

uint64_t GCStats::GetPhasePause(PauseTypeStats pauseType)
{
    return lastPause_[ToIndex(pauseType)];
}

GCStats::GCStats(MemStatsType *memStats, GCType gcTypeFromRuntime, InternalAllocatorPtr allocator)
    : memStats_(memStats), allocator_(allocator)
{
    startTime_ = time::GetCurrentTimeInNanos();
    allNumberDurations_ = allocator_->New<PandaVector<uint64_t>>(allocator_->Adapter());
    gcType_ = gcTypeFromRuntime;
}

GCStats::~GCStats()
{
    gcType_ = GCType::INVALID_GC;

    if (allNumberDurations_ != nullptr) {
        allocator_->Delete(allNumberDurations_);
    }
    allNumberDurations_ = nullptr;
}

void GCStats::StartMutatorLock()
{
    os::memory::LockHolder lock(mutatorStatsLock_);
    if (countMutator_ == 0) {
        mutatorStartTime_ = time::GetCurrentTimeInNanos();
    }
    ++countMutator_;
}

void GCStats::StopMutatorLock()
{
    os::memory::LockHolder lock(mutatorStatsLock_);
    if (countMutator_ == 0) {
        return;
    }
    if (countMutator_ == 1) {
        totalMutatorPause_ += time::GetCurrentTimeInNanos() - mutatorStartTime_;
        mutatorStartTime_ = 0;
    }
    --countMutator_;
}

void GCStats::StartCollectStats()
{
    memStats_->ClearLastYoungObjectsMovedBytes();
    objectsFreed_ = memStats_->GetTotalObjectsFreed();
    objectsFreedBytes_ = memStats_->GetFootprintHeap();
    largeObjectsFreed_ = memStats_->GetTotalHumongousObjectsFreed();
    largeObjectsFreedBytes_ = memStats_->GetFootprint(SpaceType::SPACE_TYPE_HUMONGOUS_OBJECT);
}

void GCStats::StopCollectStats(GCInstanceStats *instanceStats)
{
    objectsFreed_ = memStats_->GetTotalObjectsFreed() - objectsFreed_;
    largeObjectsFreed_ = memStats_->GetTotalHumongousObjectsFreed() - largeObjectsFreed_;

    size_t currentObjectsFreedBytes = memStats_->GetFootprintHeap();
    size_t currentLargeObjectsFreedBytes = memStats_->GetFootprint(SpaceType::SPACE_TYPE_HUMONGOUS_OBJECT);

    if (objectsFreedBytes_ < currentObjectsFreedBytes) {
        objectsFreedBytes_ = 0;
    } else {
        objectsFreedBytes_ -= currentObjectsFreedBytes;
    }

    if (largeObjectsFreedBytes_ < currentLargeObjectsFreedBytes) {
        largeObjectsFreedBytes_ = 0;
    } else {
        largeObjectsFreedBytes_ -= currentLargeObjectsFreedBytes;
    }
    if ((instanceStats != nullptr) && (objectsFreed_ > 0)) {
        instanceStats->AddMemoryValue(objectsFreedBytes_, MemoryTypeStats::ALL_FREED_BYTES);
        instanceStats->AddObjectsValue(objectsFreed_, ObjectTypeStats::ALL_FREED_OBJECTS);
    }
}

uint64_t GCStats::ConvertTimeToPeriod(uint64_t timeInNanos, bool ceil)
{
    std::chrono::nanoseconds nanos(timeInNanos);
    if (ceil) {
        using ResultDuration = std::chrono::duration<double, PERIOD>;
        return std::ceil(std::chrono::duration_cast<ResultDuration>(nanos).count());
    }
    using ResultDuration = std::chrono::duration<uint64_t, PERIOD>;
    return std::chrono::duration_cast<ResultDuration>(nanos).count();
}

void GCStats::AddPause(uint64_t pause, GCInstanceStats *instanceStats, PauseTypeStats pauseType)
{
    // COMMON_PAUSE can be accounted in different methods but it cannot be interleaved with other pause types
    ASSERT(prevPauseType_ == PauseTypeStats::COMMON_PAUSE || pauseType != PauseTypeStats::COMMON_PAUSE);
    if ((instanceStats != nullptr) && (pause > 0)) {
        instanceStats->AddTimeValue(pause, TimeTypeStats::ALL_PAUSED_TIME);
    }
    auto &lastPause = lastPause_[ToIndex(pauseType)];
    // allow accounting in different methods for COMMON_PAUSE only
    ASSERT(lastPause == 0 || pauseType == PauseTypeStats::COMMON_PAUSE);
    lastPause += pause;
    totalPause_ += pause;
#ifndef NDEBUG
    prevPauseType_ = pauseType;
#endif
}

void GCStats::ResetLastPause()
{
    for (auto pauseType = PauseTypeStats::COMMON_PAUSE; pauseType != PauseTypeStats::PAUSE_TYPE_STATS_LAST;
         pauseType = ToPauseTypeStats(ToIndex(pauseType) + 1)) {
        lastPause_[ToIndex(pauseType)] = 0U;
    }
#ifndef NDEBUG
    prevPauseType_ = PauseTypeStats::COMMON_PAUSE;
#endif
}

void GCStats::RecordDuration(uint64_t duration, GCInstanceStats *instanceStats)
{
    uint64_t startTimeDuration = ConvertTimeToPeriod(time::GetCurrentTimeInNanos() - startTime_ - duration);
    // every PERIOD
    if ((countGcPeriod_ != 0U) && (lastStartDuration_ != startTimeDuration)) {
        allNumberDurations_->push_back(countGcPeriod_);
        countGcPeriod_ = 0U;
    }
    lastStartDuration_ = startTimeDuration;
    ++countGcPeriod_;
    if ((instanceStats != nullptr) && (duration > 0)) {
        instanceStats->AddTimeValue(duration, TimeTypeStats::ALL_TOTAL_TIME);
    }
    lastDuration_ = duration;
    totalDuration_ += duration;
}

GCScopedStats::GCScopedStats(GCStats *stats, GCInstanceStats *instanceStats)
    : startTime_(time::GetCurrentTimeInNanos()), instanceStats_(instanceStats), stats_(stats)
{
    stats_->StartCollectStats();
}

GCScopedStats::~GCScopedStats()
{
    stats_->StopCollectStats(instanceStats_);
    stats_->RecordDuration(time::GetCurrentTimeInNanos() - startTime_, instanceStats_);
}

GCScopedPauseStats::GCScopedPauseStats(GCStats *stats, GCInstanceStats *instanceStats, PauseTypeStats pauseType)
    : startTime_(time::GetCurrentTimeInNanos()), instanceStats_(instanceStats), stats_(stats), pauseType_(pauseType)
{
}

GCScopedPauseStats::~GCScopedPauseStats()
{
    stats_->AddPause(time::GetCurrentTimeInNanos() - startTime_, instanceStats_, pauseType_);
}

PandaString GCInstanceStats::GetDump(GCType gcType)
{
    PandaStringStream statistic;

    bool youngSpace = timeStats_[ToIndex(TimeTypeStats::YOUNG_TOTAL_TIME)].GetSum() > 0U;
    bool allSpace = timeStats_[ToIndex(TimeTypeStats::ALL_TOTAL_TIME)].GetSum() > 0U;
    bool minorGc = copiedBytes_.GetCount() > 0U;
    bool wasDeleted = reclaimBytes_.GetCount() > 0U;
    bool wasMoved = memoryStats_[ToIndex(MemoryTypeStats::MOVED_BYTES)].GetCount() > 0U;

    if (youngSpace) {
        statistic << GetYoungSpaceDump(gcType);
    } else if (allSpace) {
        statistic << GetAllSpacesDump(gcType);
    }

    if (wasDeleted) {
        statistic << "Average GC reclaim bytes ratio " << reclaimBytes_.GetAvg() << " over " << reclaimBytes_.GetCount()
                  << " GC cycles \n";
    }

    if (minorGc) {
        statistic << "Average minor GC copied live bytes ratio " << copiedBytes_.GetAvg() << " over "
                  << copiedBytes_.GetCount() << " minor GCs \n";
    }

    if (wasMoved) {
        statistic << "Cumulative bytes moved "
                  << memoryStats_[ToIndex(MemoryTypeStats::MOVED_BYTES)].GetGeneralStatistic() << "\n";
        statistic << "Cumulative objects moved "
                  << objectsStats_[ToIndex(ObjectTypeStats::MOVED_OBJECTS)].GetGeneralStatistic() << "\n";
    }

    return statistic.str();
}

PandaString GCInstanceStats::GetYoungSpaceDump(GCType gcType)
{
    PandaStringStream statistic;
    statistic << "young " << GC_NAMES[ToIndex(gcType)]
              << " paused: " << timeStats_[ToIndex(TimeTypeStats::YOUNG_PAUSED_TIME)].GetGeneralStatistic() << "\n";

    auto &youngTotalTimeHist = timeStats_[ToIndex(TimeTypeStats::YOUNG_TOTAL_TIME)];
    auto youngTotalTime = helpers::TimeConverter(youngTotalTimeHist.GetSum());
    auto youngTotalFreedObj = objectsStats_[ToIndex(ObjectTypeStats::YOUNG_FREED_OBJECTS)].GetSum();
    auto youngTotalFreedBytes = memoryStats_[ToIndex(MemoryTypeStats::YOUNG_FREED_BYTES)].GetSum();

    statistic << "young " << GC_NAMES[ToIndex(gcType)] << " total time: " << youngTotalTime
              << " mean time: " << helpers::TimeConverter(youngTotalTimeHist.GetAvg()) << "\n";
    statistic << "young " << GC_NAMES[ToIndex(gcType)] << " freed: " << youngTotalFreedObj << " with total size "
              << helpers::MemoryConverter(youngTotalFreedBytes) << "\n";

    statistic << "young " << GC_NAMES[ToIndex(gcType)] << " throughput: " << std::scientific
              << youngTotalFreedObj / youngTotalTime.GetDoubleValue() << "objects/" << youngTotalTime.GetLiteral()
              << " / " << helpers::MemoryConverter(youngTotalFreedBytes / youngTotalTime.GetDoubleValue()) << "/"
              << youngTotalTime.GetLiteral() << "\n";

    return statistic.str();
}

PandaString GCInstanceStats::GetAllSpacesDump(GCType gcType)
{
    PandaStringStream statistic;

    statistic << GC_NAMES[ToIndex(gcType)]
              << " paused: " << timeStats_[ToIndex(TimeTypeStats::ALL_PAUSED_TIME)].GetGeneralStatistic() << "\n";

    auto &totalTimeHist = timeStats_[ToIndex(TimeTypeStats::ALL_TOTAL_TIME)];
    auto totalTime = helpers::TimeConverter(totalTimeHist.GetSum());
    auto totalFreedObj = objectsStats_[ToIndex(ObjectTypeStats::ALL_FREED_OBJECTS)].GetSum();
    auto totalFreedBytes = memoryStats_[ToIndex(MemoryTypeStats::ALL_FREED_BYTES)].GetSum();

    statistic << GC_NAMES[ToIndex(gcType)] << " total time: " << totalTime
              << " mean time: " << helpers::TimeConverter(totalTimeHist.GetAvg()) << "\n";
    statistic << GC_NAMES[ToIndex(gcType)] << " freed: " << totalFreedObj << " with total size "
              << helpers::MemoryConverter(totalFreedBytes) << "\n";

    statistic << GC_NAMES[ToIndex(gcType)] << " throughput: " << std::scientific
              << totalFreedObj / totalTime.GetDoubleValue() << "objects/" << totalTime.GetLiteral() << " / "
              << helpers::MemoryConverter(totalFreedBytes / totalTime.GetDoubleValue()) << "/" << totalTime.GetLiteral()
              << "\n";

    return statistic.str();
}

}  // namespace ark::mem
