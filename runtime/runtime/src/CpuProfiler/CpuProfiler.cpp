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


#include "CpuProfiler.h"
#include "Mutator/MutatorManager.h"

namespace MapleRuntime {
CpuProfiler::~CpuProfiler()
{
    TryStopSampling();
}

bool CpuProfiler::StartCpuProfilerForFile()
{
    if (generator.GetIsStart()) {
        LOG(RTLOG_ERROR, "Start CpuProfiler repeatedly.");
        return false;
    }
    generator.SetIsStart(true);

    tid = std::thread(CpuProfiler::SamplingThread, std::ref(generator));
    if (!tid.joinable()) {
        LOG(RTLOG_ERROR, "Failed to create sampling thread.");
        return false;
    }
    return true;
}

bool CpuProfiler::StopCpuProfilerForFile(const int fd)
{
    if (!generator.GetIsStart()) {
        LOG(RTLOG_ERROR, "CpuProfiler is not in profiling");
        return false;
    }
    // Sample data will be dump before sampling thread exit.
    bool ret = generator.OpenFile(fd);
    if (!ret) {
        LOG(RTLOG_ERROR, "Open file failed");
    }
    TryStopSampling();
    return ret;
}

void CpuProfiler::TryStopSampling()
{
    if (!generator.GetIsStart()) {
        return;
    }
    // Set false to break the sampling loop.
    generator.SetIsStart(false);
    if (tid.joinable()) {
        tid.join();
    }
}

void CpuProfiler::SamplingThread(SamplesRecord& generator)
{
    generator.InitProfileInfo();
    uint32_t interval = generator.GetSamplingInterval();
    uint64_t startTime = SamplesRecord::GetMicrosecondsTimeStamp();
    generator.SetThreadStartTime(startTime);
    uint64_t endTime = startTime;
    while (generator.GetIsStart()) {
        startTime = SamplesRecord::GetMicrosecondsTimeStamp();
        int64_t ts = static_cast<int64_t>(interval) - static_cast<int64_t>(startTime - endTime);
        endTime = startTime;
        // when ts > 0, the time interval is less than the sampling interval.
        if (ts > 0) {
            usleep(ts);
            endTime = SamplesRecord::GetMicrosecondsTimeStamp();
        }
        DoSampleStack();
        generator.ParseSampleData(endTime);
        // Save the sampling data to profileInfo.
        generator.DoSingleTask(endTime);
    }
    // Traverse the task queue until all sampling data is saved to profileInfo.
    generator.RunTaskLoop();
    generator.SetSampleStopTime(SamplesRecord::GetMicrosecondsTimeStamp());
    generator.DumpProfileInfo();
    generator.ReleaseProfileInfo();
}

void CpuProfiler::DoSampleStack()
{
    MutatorManager::Instance().TransitionAllMutatorsToCpuProfile();
}
}
