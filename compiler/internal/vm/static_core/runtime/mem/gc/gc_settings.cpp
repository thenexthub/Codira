/**
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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

#include "runtime/include/runtime.h"
#include "runtime/include/runtime_options.h"
#include "runtime/mem/gc/gc_settings.h"
#include "libarkbase/globals.h"

namespace ark::mem {

GCSettings::GCSettings(const RuntimeOptions &options, panda_file::SourceLang lang)
{
    auto runtimeLang = plugins::LangToRuntimeType(lang);
    isDumpHeap_ = options.IsGcDumpHeap(runtimeLang);
    isConcurrencyEnabled_ = options.IsConcurrentGcEnabled(runtimeLang);
    isGcEnableTracing_ = options.IsGcEnableTracing(runtimeLang);
    isExplicitConcurrentGcEnabled_ = options.IsExplicitConcurrentGcEnabled();
    runGcInPlace_ = options.IsRunGcInPlace(runtimeLang);
    fullGcBombingFrequency_ = options.GetFullGcBombingFrequency(runtimeLang);
    nativeGcTriggerType_ = NativeGcTriggerTypeFromString(options.GetNativeGcTriggerType(runtimeLang));
    enableFastHeapVerifier_ = options.IsEnableFastHeapVerifier(runtimeLang);
    auto hvParams = options.GetHeapVerifier(runtimeLang);
    preGcHeapVerification_ = std::find(hvParams.begin(), hvParams.end(), "pre") != hvParams.end();
    intoGcHeapVerification_ = std::find(hvParams.begin(), hvParams.end(), "into") != hvParams.end();
    postGcHeapVerification_ = std::find(hvParams.begin(), hvParams.end(), "post") != hvParams.end();
    beforeG1ConcurrentHeapVerification_ =
        std::find(hvParams.begin(), hvParams.end(), "before_g1_concurrent") != hvParams.end();
    failOnHeapVerification_ = std::find(hvParams.begin(), hvParams.end(), "fail_on_verification") != hvParams.end();
    runGcEverySafepoint_ = options.IsRunGcEverySafepoint();
    g1RegionGarbageRateThreshold_ = options.GetG1RegionGarbageRateThreshold() / PERCENT_100_D;
    g1NumberOfTenuredRegionsAtMixedCollection_ = options.GetG1NumberOfTenuredRegionsAtMixedCollection();
    g1PromotionRegionAliveRate_ = options.GetG1PromotionRegionAliveRate();
    g1FullGcRegionFragmentationRate_ = options.GetG1FullGcRegionFragmentationRate() / PERCENT_100_D;
    g1TrackFreedObjects_ = options.IsG1TrackFreedObjects();
    gcWorkersCount_ = options.GetGcWorkersCount();
    manageGcThreadsAffinity_ = options.IsManageGcThreadsAffinity();
    useWeakCpuForGcConcurrent_ = options.IsUseWeakCpuForGcConcurrent();
    useThreadPoolForGcWorkers_ = !Runtime::IsTaskManagerUsed();
    gcMarkingStackNewTasksFrequency_ = options.GetGcMarkingStackNewTasksFrequency();
    gcRootMarkingStackMaxSize_ = options.GetGcRootMarkingStackMaxSize();
    gcWorkersMarkingStackMaxSize_ = options.GetGcWorkersMarkingStackMaxSize();
    youngSpaceSize_ = options.GetYoungSpaceSize();
    logDetailedGcInfoEnabled_ = options.IsLogDetailedGcInfoEnabled();
    logDetailedGcCompactionInfoEnabled_ = options.IsLogDetailedGcCompactionInfoEnabled();
    parallelMarkingEnabled_ = options.IsGcParallelMarkingEnabled() && (options.GetGcWorkersCount() != 0);
    parallelCompactingEnabled_ = options.IsGcParallelCompactingEnabled() && (options.GetGcWorkersCount() != 0);
    parallelRefUpdatingEnabled_ = options.IsGcParallelRefUpdatingEnabled() && (options.GetGcWorkersCount() != 0);
    g1EnableConcurrentUpdateRemset_ = options.IsG1EnableConcurrentUpdateRemset();
    g1MinConcurrentCardsToProcess_ = options.GetG1MinConcurrentCardsToProcess();
    g1HotCardsProcessingFrequency_ = options.GetG1HotCardsProcessingFrequency();
    g1EnablePauseTimeGoal_ = options.IsG1PauseTimeGoal();
    g1MaxGcPauseMs_ = options.GetG1PauseTimeGoalMaxGcPause();
    g1GcPauseIntervalMs_ = options.WasSetG1PauseTimeGoalGcPauseInterval() ? options.GetG1PauseTimeGoalGcPauseInterval()
                                                                          : g1MaxGcPauseMs_ + 1;
    g1SinglePassCompactionEnabled_ = options.IsG1SinglePassCompactionEnabled();
    LOG_IF(FullGCBombingFrequency() && RunGCInPlace(), FATAL, GC)
        << "full-gc-bombimg-frequency and run-gc-in-place options can't be used together";
}

bool GCSettings::IsGcEnableTracing() const
{
    return isGcEnableTracing_;
}

NativeGcTriggerType GCSettings::GetNativeGcTriggerType() const
{
    return nativeGcTriggerType_;
}

bool GCSettings::IsDumpHeap() const
{
    return isDumpHeap_;
}

bool GCSettings::IsConcurrencyEnabled() const
{
    return isConcurrencyEnabled_;
}

bool GCSettings::IsExplicitConcurrentGcEnabled() const
{
    return isExplicitConcurrentGcEnabled_;
}

bool GCSettings::RunGCInPlace() const
{
    return runGcInPlace_;
}

bool GCSettings::EnableFastHeapVerifier() const
{
    return enableFastHeapVerifier_;
}

bool GCSettings::PreGCHeapVerification() const
{
    return preGcHeapVerification_;
}

bool GCSettings::IntoGCHeapVerification() const
{
    return intoGcHeapVerification_;
}

bool GCSettings::PostGCHeapVerification() const
{
    return postGcHeapVerification_;
}

bool GCSettings::BeforeG1ConcurrentHeapVerification() const
{
    return beforeG1ConcurrentHeapVerification_;
}

bool GCSettings::FailOnHeapVerification() const
{
    return failOnHeapVerification_;
}

bool GCSettings::RunGCEverySafepoint() const
{
    return runGcEverySafepoint_;
}

double GCSettings::G1RegionGarbageRateThreshold() const
{
    return g1RegionGarbageRateThreshold_;
}

uint32_t GCSettings::FullGCBombingFrequency() const
{
    return fullGcBombingFrequency_;
}

uint32_t GCSettings::GetG1NumberOfTenuredRegionsAtMixedCollection() const
{
    return g1NumberOfTenuredRegionsAtMixedCollection_;
}

double GCSettings::G1PromotionRegionAliveRate() const
{
    return g1PromotionRegionAliveRate_;
}

double GCSettings::G1FullGCRegionFragmentationRate() const
{
    return g1FullGcRegionFragmentationRate_;
}

bool GCSettings::G1TrackFreedObjects() const
{
    return g1TrackFreedObjects_;
}

size_t GCSettings::GCWorkersCount() const
{
    return gcWorkersCount_;
}

bool GCSettings::ManageGcThreadsAffinity() const
{
    return manageGcThreadsAffinity_;
}

bool GCSettings::UseWeakCpuForGcConcurrent() const
{
    return useWeakCpuForGcConcurrent_;
}

void GCSettings::SetGCWorkersCount(size_t value)
{
    gcWorkersCount_ = value;
}

bool GCSettings::UseThreadPoolForGC() const
{
    return useThreadPoolForGcWorkers_;
}

bool GCSettings::UseTaskManagerForGC() const
{
    return !useThreadPoolForGcWorkers_;
}

size_t GCSettings::GCMarkingStackNewTasksFrequency() const
{
    return gcMarkingStackNewTasksFrequency_;
}

size_t GCSettings::GCRootMarkingStackMaxSize() const
{
    return gcRootMarkingStackMaxSize_;
}

size_t GCSettings::GCWorkersMarkingStackMaxSize() const
{
    return gcWorkersMarkingStackMaxSize_;
}

uint64_t GCSettings::YoungSpaceSize() const
{
    return youngSpaceSize_;
}

bool GCSettings::LogDetailedGCInfoEnabled() const
{
    return logDetailedGcInfoEnabled_;
}

bool GCSettings::LogDetailedGCCompactionInfoEnabled() const
{
    return logDetailedGcCompactionInfoEnabled_;
}

bool GCSettings::ParallelMarkingEnabled() const
{
    return parallelMarkingEnabled_;
}

void GCSettings::SetParallelMarkingEnabled(bool value)
{
    parallelMarkingEnabled_ = value;
}

bool GCSettings::ParallelCompactingEnabled() const
{
    return parallelCompactingEnabled_;
}

void GCSettings::SetParallelCompactingEnabled(bool value)
{
    parallelCompactingEnabled_ = value;
}

bool GCSettings::ParallelRefUpdatingEnabled() const
{
    return parallelRefUpdatingEnabled_;
}

void GCSettings::SetParallelRefUpdatingEnabled(bool value)
{
    parallelRefUpdatingEnabled_ = value;
}

bool GCSettings::G1EnableConcurrentUpdateRemset() const
{
    return g1EnableConcurrentUpdateRemset_;
}

size_t GCSettings::G1MinConcurrentCardsToProcess() const
{
    return g1MinConcurrentCardsToProcess_;
}

size_t GCSettings::G1HotCardsProcessingFrequency() const
{
    return g1HotCardsProcessingFrequency_;
}

bool GCSettings::G1EnablePauseTimeGoal() const
{
    return g1EnablePauseTimeGoal_;
}

uint32_t GCSettings::GetG1MaxGcPauseInMillis() const
{
    return g1MaxGcPauseMs_;
}

uint32_t GCSettings::GetG1GcPauseIntervalInMillis() const
{
    return g1GcPauseIntervalMs_;
}

bool GCSettings::G1SinglePassCompactionEnabled() const
{
    return g1SinglePassCompactionEnabled_;
}

}  // namespace ark::mem
