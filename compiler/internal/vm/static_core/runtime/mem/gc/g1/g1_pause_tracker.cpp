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
#include <algorithm>
#include "g1_pause_tracker.h"
#include "libarkbase/utils/logger.h"
#include "libarkbase/utils/type_converter.h"
#include "libarkbase/os/time.h"

namespace ark::mem {
G1PauseTracker::G1PauseTracker(int64_t gcPauseIntervalMs, int64_t maxGcTimeMs)
    : gcPauseIntervalUs_(gcPauseIntervalMs * ark::os::time::MILLIS_TO_MICRO),
      maxGcTimeUs_(maxGcTimeMs * ark::os::time::MILLIS_TO_MICRO)
{
}

bool G1PauseTracker::AddPauseInNanos(int64_t startTimeNs, int64_t endTimeNs)
{
    return AddPause(startTimeNs / ark::os::time::MICRO_TO_NANO, endTimeNs / ark::os::time::MICRO_TO_NANO);
}

bool G1PauseTracker::AddPause(int64_t startTimeUs, int64_t endTimeUs)
{
    RemoveOutOfIntervalEntries(endTimeUs);
    pauses_.push_back(PauseEntry(startTimeUs, endTimeUs));
    auto gcTime = CalculateIntervalPauseInMicros(endTimeUs);
    if (gcTime > maxGcTimeUs_) {
        LOG(DEBUG, GC) << "Target GC pause was exceeded: "
                       << ark::helpers::TimeConverter(gcTime * ark::os::time::MICRO_TO_NANO) << " > "
                       << ark::helpers::TimeConverter(maxGcTimeUs_ * ark::os::time::MICRO_TO_NANO)
                       << " in time interval "
                       << ark::helpers::TimeConverter(gcPauseIntervalUs_ * ark::os::time::MICRO_TO_NANO);
        return false;
    }
    return true;
}

void G1PauseTracker::RemoveOutOfIntervalEntries(int64_t nowUs)
{
    auto oldestIntervalTime = nowUs - gcPauseIntervalUs_;
    while (!pauses_.empty()) {
        auto oldestPause = pauses_.front();
        if (oldestPause.GetEndTimeInMicros() > oldestIntervalTime) {
            break;
        }
        pauses_.pop_front();
    }
}

int64_t G1PauseTracker::CalculateIntervalPauseInMicros(int64_t nowUs)
{
    int64_t gcTime = 0;
    auto oldestIntervalTime = nowUs - gcPauseIntervalUs_;
    auto end = pauses_.cend();
    for (auto it = pauses_.cbegin(); it != end; ++it) {
        if (it->GetEndTimeInMicros() > oldestIntervalTime) {
            gcTime += it->DurationInMicros(oldestIntervalTime);
        }
    }
    return gcTime;
}

int64_t G1PauseTracker::MinDelayBeforePauseInMicros(int64_t nowUs, int64_t pauseTimeUs)
{
    pauseTimeUs = std::min(pauseTimeUs, maxGcTimeUs_);
    auto gcBudget = maxGcTimeUs_ - pauseTimeUs;
    auto newIntervalTime = nowUs + pauseTimeUs;
    auto oldestIntervalTime = newIntervalTime - gcPauseIntervalUs_;

    auto rend = pauses_.crend();
    for (auto it = pauses_.crbegin(); it != rend; ++it) {
        if (it->GetEndTimeInMicros() <= oldestIntervalTime) {
            break;
        }

        auto duration = it->DurationInMicros(oldestIntervalTime);
        if (duration > gcBudget) {
            auto newOldestIntervalTime = it->GetEndTimeInMicros() - gcBudget;
            ASSERT(newOldestIntervalTime >= oldestIntervalTime);
            return newOldestIntervalTime - oldestIntervalTime;
        }

        gcBudget -= duration;
    }

    return 0;
}

int64_t G1PauseTracker::MinDelayBeforeMaxPauseInMicros(int64_t nowUs)
{
    return MinDelayBeforePauseInMicros(nowUs, maxGcTimeUs_);
}
}  // namespace ark::mem
