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

#include "runtime/include/timer_table.h"

#include "libarkbase/os/mutex.h"

namespace ark {

void TimerTable::RegisterTimer(ThreadId id, std::string_view name)
{
    os::memory::LockHolder hlock(tableLock_);
    activeTimers_[id].emplace_back(name);
}

void TimerTable::DisarmTimer(ThreadId id, [[maybe_unused]] std::string_view name)
{
    auto end = GetTime();

    os::memory::LockHolder hlock(tableLock_);
    ASSERT(!activeTimers_[id].empty());
    auto &timer = activeTimers_[id].back();
    ASSERT(timer.GetName() == name);
    RecordTimer(timer, end);
    activeTimers_[id].pop_back();
}

void TimerTable::ResetTimers(ThreadId id, bool needRecord)
{
    auto end = GetTime();

    os::memory::LockHolder hlock(tableLock_);
    for (auto &timer : activeTimers_[id]) {
        if (needRecord) {
            RecordTimer(timer, end);
        }
        timer.Reset(end);
    }
}

void TimerTable::Report(std::stringstream &ss)
{
    os::memory::LockHolder hlock(tableLock_);
    for (auto &[name, timeDiff] : timeRecords_) {
        ss << name << ": " << timeDiff << std::endl;
    }
}

void TimerTable::RecordTimer(const Timer &timer, TimePoint end)
{
    auto timeDiff = std::chrono::duration_cast<std::chrono::microseconds>(end - timer.GetStart()).count();
    if (timeDiff < recordingTime_) {
        return;
    }
    auto [recordIt, isInserted] = timeRecords_.insert({timer.GetName(), timeDiff});
    if (!isInserted) {
        recordIt->second = std::max<long long>(recordIt->second, timeDiff);
    }
}

}  // namespace ark
