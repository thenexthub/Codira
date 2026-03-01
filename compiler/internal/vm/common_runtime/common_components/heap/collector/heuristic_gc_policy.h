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

#ifndef COMMON_COMPONENTS_HEAP_HEURISTIC_GC_POLICY_H
#define COMMON_COMPONENTS_HEAP_HEURISTIC_GC_POLICY_H

#include "common_components/base/globals.h"
#include "common_interfaces/base_runtime.h"
#include "common_components/taskpool/taskpool.h"
#include "common_components/log/log.h"

namespace common {
enum class StartupStatus: uint8_t {
    BEFORE_STARTUP,
    COLD_STARTUP,
    COLD_STARTUP_PARTIALLY_FINISH,
    COLD_STARTUP_FINISH,
};

enum AppSensitiveStatus : uint8_t {
    NORMAL_SCENE,
    ENTER_HIGH_SENSITIVE,
    EXIT_HIGH_SENSITIVE,
};

class StartupStatusManager {
public:
    static std::atomic<StartupStatus> startupStatus_;
    static constexpr size_t STARTUP_DURATION_MS = 2000;
    static constexpr size_t STARTUP_FINISH_DURATION_MS = 8000;

    static void OnAppStartup();

    static void SetStartupStatus(StartupStatus status)
    {
        startupStatus_ = status;
    }

    static StartupStatus GetStartupStatus()
    {
        return startupStatus_.load(std::memory_order_relaxed);
    }

private:
    class StartupFinishTask : public common::Task {
    public:
        StartupFinishTask(uint32_t id) : Task(id) {}
        bool Run([[maybe_unused]] uint32_t threadIndex) override
        {
            StartupStatusManager::SetStartupStatus(StartupStatus::COLD_STARTUP_FINISH);
            return true;
        }
    };

    class StartupTask : public common::Task {
    public:
        StartupTask(uint32_t id, Taskpool *pool, size_t delay)
            : Task(id), threadPool_(pool), delay_(delay) {}
        bool Run([[maybe_unused]] uint32_t threadIndex) override
        {
            StartupStatusManager::SetStartupStatus(StartupStatus::COLD_STARTUP_PARTIALLY_FINISH);
            size_t startupFinishDelay = STARTUP_FINISH_DURATION_MS - delay_;
            threadPool_->PostDelayedTask(std::make_unique<StartupFinishTask>(0), startupFinishDelay);
            return true;
        }

    private:
        Taskpool *threadPool_ {nullptr};
        size_t delay_ {0};
    };
};

static constexpr size_t NATIVE_MULTIPLIER = 2;
// Use interval to avoid gc request too frequent.
static constexpr size_t NOTIFY_NATIVE_INTERVAL = 32;
#if defined(PANDA_TARGET_32)
    static constexpr size_t NATIVE_INIT_THRESHOLD = 100 * MB;
    static constexpr size_t MAX_GLOBAL_NATIVE_LIMIT = 512 * MB;
    static constexpr size_t MAX_NATIVE_STEP = 64 * MB;
    static constexpr size_t MAX_NATIVE_SIZE_INC = 256 * MB;
    static constexpr size_t NATIVE_IMMEDIATE_THRESHOLD = 300 * KB;
#else
    static constexpr size_t NATIVE_INIT_THRESHOLD = 200 * MB;
    static constexpr size_t MAX_GLOBAL_NATIVE_LIMIT = 2 * GB;
    static constexpr size_t MAX_NATIVE_STEP = 200 * MB;
    static constexpr size_t MAX_NATIVE_SIZE_INC = 1 * GB;
    static constexpr size_t NATIVE_IMMEDIATE_THRESHOLD = 2 * MB;
#endif
static constexpr size_t URGENCY_NATIVE_LIMIT = MAX_NATIVE_SIZE_INC + MAX_NATIVE_STEP * 2; // 2 is double.

class HeuristicGCPolicy {
public:
    static constexpr double COLD_STARTUP_PHASE1_GC_THRESHOLD_RATIO = 0.3;
    static constexpr double COLD_STARTUP_PHASE2_GC_THRESHOLD_RATIO = 0.125;
    void Init();

    bool ShouldRestrainGCOnStartupOrSensitive();

    void TryHeuristicGC();

    void TryIdleGC();

    bool ShouldRestrainGCInSensitive(size_t currentSize);

    void NotifyHighSensitive(bool isStart)
    {
        OHOS_HITRACE(HITRACE_LEVEL_COMMERCIAL, "SmartGC: set high sensitive status: ",
            std::to_string(isStart).c_str());
        isStart ? SetSensitiveStatus(AppSensitiveStatus::ENTER_HIGH_SENSITIVE)
            : SetSensitiveStatus(AppSensitiveStatus::EXIT_HIGH_SENSITIVE);
        LOG_COMMON(INFO) << "SmartGC: set high sensitive status: " << isStart;
    }

    bool InSensitiveStatus() const
    {
        return GetSensitiveStatus() == AppSensitiveStatus::ENTER_HIGH_SENSITIVE;
    }

    bool OnStartupEvent() const
    {
        return StartupStatusManager::GetStartupStatus() == StartupStatus::COLD_STARTUP ||
            StartupStatusManager::GetStartupStatus() == StartupStatus::COLD_STARTUP_PARTIALLY_FINISH;
    }

    StartupStatus GetStartupStatus() const;

    void SetSensitiveStatus(AppSensitiveStatus status)
    {
        sensitiveStatus_.store(status, std::memory_order_release);
    }

    AppSensitiveStatus GetSensitiveStatus() const
    {
        return sensitiveStatus_.load(std::memory_order_acquire);
    }

    bool CASSensitiveStatus(AppSensitiveStatus expect, AppSensitiveStatus status)
    {
        return sensitiveStatus_.compare_exchange_strong(expect, status, std::memory_order_seq_cst);
    }
    
    void SetRecordHeapObjectSizeBeforeSensitive(size_t objSize)
    {
        recordSizeBeforeSensitive_.store(objSize, std::memory_order_release);
    }

    size_t GetRecordHeapObjectSizeBeforeSensitive() const
    {
        return recordSizeBeforeSensitive_.load(std::memory_order_acquire);
    }

    void NotifyNativeAllocation(size_t bytes);

    void NotifyNativeFree(size_t bytes);

    void NotifyNativeReset(size_t oldBytes, size_t newBytes);

    size_t GetNotifiedNativeSize() const;

    void SetNativeHeapThreshold(size_t newThreshold);

    size_t GetNativeHeapThreshold() const;

    void RecordAliveSizeAfterLastGC(size_t aliveBytes);

    void ChangeGCParams(bool isBackground);

    bool CheckAndTriggerHintGC(MemoryReduceDegree degree);
#if defined(PANDA_TARGET_32)
    static constexpr size_t INC_OBJ_SIZE_IN_SENSITIVE = 40 * MB;
#else
    static constexpr size_t INC_OBJ_SIZE_IN_SENSITIVE = 80 * MB;
#endif
    static constexpr size_t BACKGROUND_LIMIT = 2 * MB;
    static constexpr size_t MIN_BACKGROUND_GC_SIZE = 30 * MB;

    static constexpr double IDLE_MIN_INC_RATIO = 1.1f;
    static constexpr size_t LOW_DEGREE_STEP_IN_IDLE = 5 * MB;
    static constexpr size_t HIGH_DEGREE_STEP_IN_IDLE = 1 * MB;

    static constexpr double IDLE_SPACE_SIZE_MIN_INC_RATIO = 1.1f;
    static constexpr size_t IDLE_SPACE_SIZE_MIN_INC_STEP_FULL = 1 * MB;

private:
    void CheckGCForNative();

    uint64_t heapSize_ {0};
    size_t aliveSizeAfterGC_ {0};

    std::atomic<size_t> notifiedNativeSize_ = 0;
    std::atomic<size_t> nativeHeapThreshold_ = NATIVE_INIT_THRESHOLD;
    std::atomic<size_t> nativeHeapObjects_ = 0;

    std::atomic<AppSensitiveStatus> sensitiveStatus_ {AppSensitiveStatus::NORMAL_SCENE};
    std::atomic<size_t> recordSizeBeforeSensitive_ {0};
};
} // namespace common

#endif // COMMON_COMPONENTS_HEAP_HEURISTIC_GC_POLICY_H
