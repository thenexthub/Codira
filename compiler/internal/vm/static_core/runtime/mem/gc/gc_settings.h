/**
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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
#ifndef PANDA_RUNTIME_MEM_GC_GC_SETTINGS_H
#define PANDA_RUNTIME_MEM_GC_GC_SETTINGS_H

#include <cstddef>
#include <string_view>
#include <cstdint>

namespace ark {
class RuntimeOptions;
}  // namespace ark

namespace ark::mem {

enum class NativeGcTriggerType { INVALID_NATIVE_GC_TRIGGER, NO_NATIVE_GC_TRIGGER, SIMPLE_STRATEGY };
inline NativeGcTriggerType NativeGcTriggerTypeFromString(std::string_view nativeGcTriggerTypeStr)
{
    if (nativeGcTriggerTypeStr == "no-native-gc-trigger") {
        return NativeGcTriggerType::NO_NATIVE_GC_TRIGGER;
    }
    if (nativeGcTriggerTypeStr == "simple-strategy") {
        return NativeGcTriggerType::SIMPLE_STRATEGY;
    }
    return NativeGcTriggerType::INVALID_NATIVE_GC_TRIGGER;
}

class GCSettings {
public:
    GCSettings() = default;
    explicit GCSettings(const RuntimeOptions &options, panda_file::SourceLang lang);

    /// @brief if true then enable tracing
    bool IsGcEnableTracing() const;

    /// @brief type of native trigger
    NativeGcTriggerType GetNativeGcTriggerType() const;

    /// @brief dump heap at the beginning and the end of GC
    bool IsDumpHeap() const;

    /// @brief true if concurrency enabled
    bool IsConcurrencyEnabled() const;

    /// @brief true if explicit GC should be running in concurrent
    bool IsExplicitConcurrentGcEnabled() const;

    /// @brief true if GC should be running in place
    bool RunGCInPlace() const;

    /// @brief use FastHeapVerifier if true
    bool EnableFastHeapVerifier() const;

    /// @brief true if heap verification before GC enabled
    bool PreGCHeapVerification() const;

    /// @brief true if heap verification during GC enabled
    bool IntoGCHeapVerification() const;

    /// @brief true if heap verification after GC enabled
    bool PostGCHeapVerification() const;

    /// true if heap verification before G1-GC concurrent phase enabled
    bool BeforeG1ConcurrentHeapVerification() const;

    /// @brief if true then fail execution if heap verifier found heap corruption
    bool FailOnHeapVerification() const;

    /// @brief if true then run gc every savepoint
    bool RunGCEverySafepoint() const;

    /// @brief garbage rate threshold of a tenured region to be included into a mixed collection
    double G1RegionGarbageRateThreshold() const;

    /// @brief runs full collection one of N times in GC thread
    uint32_t FullGCBombingFrequency() const;

    /// @brief specify a max number of tenured regions which can be collected at mixed collection in G1GC.
    uint32_t GetG1NumberOfTenuredRegionsAtMixedCollection() const;

    /// @brief minimum percentage of alive bytes in young region to promote it into tenured without moving for G1GC
    double G1PromotionRegionAliveRate() const;

    /**
     * @brief minimum percentage of not used bytes in tenured region to collect it on full even if we have no garbage
     * inside
     */
    double G1FullGCRegionFragmentationRate() const;

    /**
     * @brief Specify if we need to track removing objects
     * (i.e. update objects count in memstats and log removed objects) during the G1GC collection or not.
     */
    bool G1TrackFreedObjects() const;

    /// @brief if not zero and gc supports workers, create a gc workers and try to use them
    size_t GCWorkersCount() const;

    void SetGCWorkersCount(size_t value);

    bool ManageGcThreadsAffinity() const;

    bool UseWeakCpuForGcConcurrent() const;

    /// @return true if thread pool is used, false - if task manager is used
    bool UseThreadPoolForGC() const;

    /// @return true if task manager is used, false - if thread pool is used
    bool UseTaskManagerForGC() const;

    /**
     * @brief Limit the creation rate of tasks during marking in nanoseconds.
     * if average task creation is less than this value - it increases the stack size limit to create tasks less
     * frequently.
     */
    size_t GCMarkingStackNewTasksFrequency() const;

    /**
     * @brief Max stack size for marking in main thread, if it exceeds we will send a new task to workers,
     * 0 means unlimited.
     */
    size_t GCRootMarkingStackMaxSize() const;

    /**
     * @brief Max stack size for marking in a gc worker, if it exceeds we will send a new task to workers,
     * 0 means unlimited.
     */
    size_t GCWorkersMarkingStackMaxSize() const;

    /// @brief size of young-space
    uint64_t YoungSpaceSize() const;

    /// @brief true if print INFO log to get more detailed information in GC.
    bool LogDetailedGCInfoEnabled() const;

    /// @brief true if print INFO log to get more detailed compaction/promotion information per region in GC.
    bool LogDetailedGCCompactionInfoEnabled() const;

    /// @brief true if we want to do marking phase in multithreading mode.
    bool ParallelMarkingEnabled() const;

    void SetParallelMarkingEnabled(bool value);

    /// @brief true if we want to do compacting phase in multithreading mode.
    bool ParallelCompactingEnabled() const;

    void SetParallelCompactingEnabled(bool value);

    /// @brief true if we want to do ref updating phase in multithreading mode
    bool ParallelRefUpdatingEnabled() const;

    void SetParallelRefUpdatingEnabled(bool value);

    /// @brief true if G1 should updates remsets concurrently
    bool G1EnableConcurrentUpdateRemset() const;

    /// @brief
    size_t G1MinConcurrentCardsToProcess() const;

    size_t G1HotCardsProcessingFrequency() const;

    bool G1EnablePauseTimeGoal() const;

    uint32_t GetG1MaxGcPauseInMillis() const;

    uint32_t GetG1GcPauseIntervalInMillis() const;

    bool G1SinglePassCompactionEnabled() const;

private:
    // clang-tidy complains about excessive padding
    /// Garbage rate threshold of a tenured region to be included into a mixed collection
    double g1RegionGarbageRateThreshold_ = 0;
    /**
     * Minimum percentage of not used bytes in tenured region to
     * collect it on full even if we have no garbage inside
     */
    double g1FullGcRegionFragmentationRate_ = 0;
    /**
     * Limit the creation rate of tasks during marking in nanoseconds.
     * If average task creation is less than this value - it increases the stack size limit twice to create tasks less
     * frequently.
     */
    size_t gcMarkingStackNewTasksFrequency_ = 0;
    /// Max stack size for marking in main thread, if it exceeds we will send a new task to workers, 0 means unlimited.
    size_t gcRootMarkingStackMaxSize_ = 0;
    /// Max stack size for marking in a gc worker, if it exceeds we will send a new task to workers, 0 means unlimited.
    size_t gcWorkersMarkingStackMaxSize_ = 0;
    /// If not zero and gc supports workers, create a gc workers and try to use them
    size_t gcWorkersCount_ = 0;
    /// Size of young-space
    uint64_t youngSpaceSize_ = 0;
    size_t g1MinConcurrentCardsToProcess_ = 0;
    /// Frequency of proccessing hot cards in update remset thread
    size_t g1HotCardsProcessingFrequency_ = 0;
    /// Type of native trigger
    NativeGcTriggerType nativeGcTriggerType_ = {NativeGcTriggerType::INVALID_NATIVE_GC_TRIGGER};
    /// Runs full collection one of N times in GC thread
    uint32_t fullGcBombingFrequency_ = 0;
    /// Specify a max number of tenured regions which can be collected at mixed collection in G1GC.
    uint32_t g1NumberOfTenuredRegionsAtMixedCollection_ = 0;
    /// Minimum percentage of alive bytes in young region to promote it into tenured without moving for G1GC
    uint32_t g1PromotionRegionAliveRate_ = 0;
    uint32_t g1MaxGcPauseMs_ = 0;
    uint32_t g1GcPauseIntervalMs_ = 0;
    /// If true then enable tracing
    bool isGcEnableTracing_ = false;
    /// Dump heap at the beginning and the end of GC
    bool isDumpHeap_ = false;
    /// True if concurrency enabled
    bool isConcurrencyEnabled_ = true;
    /// True if explicit GC should be running in concurrent
    bool isExplicitConcurrentGcEnabled_ = true;
    /// True if GC should be running in place
    bool runGcInPlace_ = false;
    /// Use FastHeapVerifier if true
    bool enableFastHeapVerifier_ = true;
    /// True if heap verification before GC enabled
    bool preGcHeapVerification_ = false;
    /// True if heap verification during GC enabled
    bool intoGcHeapVerification_ = false;
    /// True if heap verification after GC enabled
    bool postGcHeapVerification_ = false;
    /// True if heap verification before G1-GC concurrent phase enabled
    bool beforeG1ConcurrentHeapVerification_ = false;
    /// If true then fail execution if heap verifier found heap corruption
    bool failOnHeapVerification_ = false;
    /// If true then run gc every savepoint
    bool runGcEverySafepoint_ = false;
    bool manageGcThreadsAffinity_ = true;
    bool useWeakCpuForGcConcurrent_ = false;
    /// if true then thread pool is used, false - task manager is used
    bool useThreadPoolForGcWorkers_ = true;
    /// If we need to track removing objects during the G1GC collection or not
    bool g1TrackFreedObjects_ = false;
    /// True if print INFO log to get more detailed information in GC
    bool logDetailedGcInfoEnabled_ = false;
    /// True if print INFO log to get more detailed compaction/promotion information per region in GC
    bool logDetailedGcCompactionInfoEnabled_ = false;
    /// True if we want to do marking phase in multithreading mode
    bool parallelMarkingEnabled_ = false;
    /// True if we want to do compacting phase in multithreading mode
    bool parallelCompactingEnabled_ = false;
    /// True if we want to do ref updating phase in multithreading mode
    bool parallelRefUpdatingEnabled_ = false;
    /// True if G1 should updates remsets concurrently
    bool g1EnableConcurrentUpdateRemset_ = false;
    bool g1EnablePauseTimeGoal_ {false};
    bool g1SinglePassCompactionEnabled_ = true;
};

}  // namespace ark::mem

#endif  // PANDA_RUNTIME_MEM_GC_GC_SETTINGS_H
