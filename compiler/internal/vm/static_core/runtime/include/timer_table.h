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
#ifndef PANDA_RUNTIME_TIMER_TABLE_H_
#define PANDA_RUNTIME_TIMER_TABLE_H_

#ifdef SAFEPOINT_TIME_CHECKER_ENABLED
#include <chrono>

#include "libarkbase/os/mutex.h"
#include "runtime/include/mem/panda_containers.h"

namespace ark {

class TimerTable {
public:
    using ThreadId = uint64_t;
    using TimeDiff = long long;
    using TimePoint = std::chrono::time_point<std::chrono::high_resolution_clock>;

    /// @param recordingTime - the minimal time that will be recorded
    explicit TimerTable(TimeDiff recordingTime) : recordingTime_(recordingTime) {}

    void RegisterTimer(ThreadId id, std::string_view name);

    void DisarmTimer(ThreadId id, std::string_view name);

    void ResetTimers(ThreadId id, bool needRecord);

    void Report(std::stringstream &ss);

private:
    static TimePoint GetTime()
    {
        return std::chrono::high_resolution_clock::now();
    }

    class Timer {
    public:
        explicit Timer(std::string_view name) : name_(name), start_(GetTime()) {}

        std::string_view GetName() const
        {
            return name_;
        }

        TimePoint GetStart() const
        {
            return start_;
        }

        void Reset(TimePoint time)
        {
            start_ = time;
        }

    private:
        std::string_view name_;
        TimePoint start_;
    };

    void RecordTimer(const Timer &timer, TimePoint end);

    TimeDiff recordingTime_;
    os::memory::Mutex tableLock_;
    PandaUnorderedMap<ThreadId, PandaVector<Timer>> activeTimers_;
    PandaUnorderedMap<std::string_view, TimeDiff> timeRecords_;
};

}  // namespace ark
#endif  // SAFEPOINT_TIME_CHECKER_ENABLED

#endif  // PANDA_RUNTIME_TIMER_TABLE_H_
