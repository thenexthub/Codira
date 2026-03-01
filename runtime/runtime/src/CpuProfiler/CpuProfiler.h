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


#ifndef MRT_CPU_PROFILER_H
#define MRT_CPU_PROFILER_H

#include <thread>
#include "SamplesRecord.h"

namespace MapleRuntime {
class CpuProfiler {
public:
    static CpuProfiler& GetInstance()
    {
        static CpuProfiler instance;
        return instance;
    }
    bool StartCpuProfilerForFile();
    bool StopCpuProfilerForFile(const int fd);
    SamplesRecord& GetGenerator() { return generator; }
    void TryStopSampling();

private:
    CpuProfiler() {}
    ~CpuProfiler();
    static void SamplingThread(SamplesRecord& generator);
    static void DoSampleStack();
    SamplesRecord generator;
    std::thread tid;
};
} // namespace MapleRuntime
#endif // MRT_CPU_PROFILER_H
