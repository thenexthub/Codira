/**
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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
#ifndef PANDA_RUNTIME_MEM_GC_G1_G1_PAUSE_TRACKER_H
#define PANDA_RUNTIME_MEM_GC_G1_G1_PAUSE_TRACKER_H

#include <cstdint>
#include <array>
#include "libarkbase/macros.h"
#include "libarkbase/utils/time.h"
#include "libarkbase/utils/ring_buffer.h"

namespace ark::mem {

/// Track G1 GC pause goal: pauses sum does not exceed max_gc_time in gc_pause_interval
class G1PauseTracker {
public:
    NO_COPY_SEMANTIC(G1PauseTracker);
    NO_MOVE_SEMANTIC(G1PauseTracker);

    G1PauseTracker(int64_t gcPauseIntervalMs, int64_t maxGcTimeMs);

    ~G1PauseTracker() = default;

    bool AddPauseInNanos(int64_t startTimeNs, int64_t endTimeNs);

    bool AddPause(int64_t startTimeUs, int64_t endTimeUs);

    /// @return minimum delay for pause from now to achieve pause goal
    int64_t MinDelayBeforePauseInMicros(int64_t nowUs, int64_t pauseTimeUs);

    /// @return minimum delay for maximum allowed pause from now to achieve pause goal
    int64_t MinDelayBeforeMaxPauseInMicros(int64_t nowUs);

    class Scope {
    public:
        ~Scope()
        {
            owner_->AddPause(startTimeUs_, ark::time::GetCurrentTimeInMicros());
        }

        NO_COPY_SEMANTIC(Scope);
        NO_MOVE_SEMANTIC(Scope);

    private:
        explicit Scope(G1PauseTracker *owner) : owner_(owner), startTimeUs_(ark::time::GetCurrentTimeInMicros()) {}

        G1PauseTracker *owner_;
        int64_t startTimeUs_;

        friend class G1PauseTracker;
    };

    Scope CreateScope()
    {
        return Scope(this);
    }

private:
    class PauseEntry {
    public:
        PauseEntry() : PauseEntry(0, 0) {}
        PauseEntry(int64_t startTimeUs, int64_t endTimeUs) : startTimeUs_(startTimeUs), endTimeUs_(endTimeUs)
        {
            ASSERT(endTimeUs_ >= startTimeUs_);
        }
        int64_t DurationInMicros() const
        {
            return endTimeUs_ - startTimeUs_;
        }
        int64_t DurationInMicros(int64_t oldestIntervalTime) const
        {
            return GetStartTimeInMicros() > oldestIntervalTime ? DurationInMicros()
                                                               : GetEndTimeInMicros() - oldestIntervalTime;
        }
        int64_t GetStartTimeInMicros() const
        {
            return startTimeUs_;
        }
        int64_t GetEndTimeInMicros() const
        {
            return endTimeUs_;
        }

    private:
        int64_t startTimeUs_;
        int64_t endTimeUs_;
    };

    int64_t CalculateIntervalPauseInMicros(int64_t nowUs);
    void RemoveOutOfIntervalEntries(int64_t nowUs);

    // Need to analyze real apps what size is required
    static constexpr int SIZE = 16;
    ark::RingBuffer<PauseEntry, SIZE> pauses_;
    int64_t gcPauseIntervalUs_;
    int64_t maxGcTimeUs_;
};
}  // namespace ark::mem

#endif  // PANDA_RUNTIME_MEM_GC_G1_G1_PAUSE_TRACKER_H
