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

#include "libarkbase/utils/logger.h"
#include "runtime/include/managed_thread.h"
#include "runtime/include/runtime.h"
#include "runtime/include/safepoint_timer.h"

namespace ark {

TimerTable *SafepointTimerTable::safepointTimers_ = nullptr;

void SafepointTimerTable::Initialize(TimerTable::TimeDiff recordingTime, std::string reportFilepath)
{
    ASSERT(!IsInitialized());
    reportFilepath_ = std::move(reportFilepath);
    safepointTimers_ = Runtime::GetCurrent()->GetInternalAllocator()->New<TimerTable>(recordingTime);
}

void SafepointTimerTable::Destroy()
{
    ASSERT(IsInitialized());

    std::stringstream ss;
    SafepointTimerTable::Report(ss);

    Runtime::GetCurrent()->GetInternalAllocator()->Delete(safepointTimers_);
    safepointTimers_ = nullptr;

    auto timerData = ss.str();
    if (timerData.empty()) {
        return;
    }

    if (reportFilepath_.empty()) {
        LOG(INFO, RUNTIME) << "Safepoint checker report:\n" << timerData;
        return;
    }

    if (std::ofstream file {reportFilepath_}; file) {
        file << timerData;
    } else {
        LOG(ERROR, RUNTIME) << "Failed to open safepoint checker report file so no report will be provided";
    }
}

SafepointTimer::SafepointTimer(std::string_view name) : name_(name)
{
    SafepointTimerTable::RegisterTimer(ManagedThread::GetCurrent()->GetInternalId(), name_);
}

SafepointTimer::~SafepointTimer()
{
    SafepointTimerTable::DisarmTimer(ManagedThread::GetCurrent()->GetInternalId(), name_);
}

}  // namespace ark
