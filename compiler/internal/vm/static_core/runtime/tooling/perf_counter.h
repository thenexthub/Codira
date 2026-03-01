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

#ifndef PANDA_RUNTIME_TOOLING_PERF_COUNTER_H
#define PANDA_RUNTIME_TOOLING_PERF_COUNTER_H

#include <memory>
#include <vector>
#include <unordered_map>
#include <cstring>
#include <optional>

#include "libarkbase/os/time.h"
#include "libarkbase/utils/logger.h"
#include "libarkbase/utils/type_converter.h"

#include <unistd.h>
#include <linux/perf_event.h>
#include <linux/hw_breakpoint.h>
#include <sys/syscall.h>
#include <sys/ioctl.h>

namespace ark::tooling {

/*
  See perf_counter.md
 */

class CounterValue {
public:
    static CounterValue CreateUnavailable();
    static CounterValue CreateExact(uint64_t value);
    static CounterValue CreateApprox(uint64_t value, double accuracy);

    uint64_t GetValue() const;
    bool IsExact() const;
    bool IsAvailable() const;
    double GetAccuracy() const;

private:
    CounterValue(uint64_t value, bool exact, bool available, double accuracy);
    uint64_t value_;
    bool exact_;
    bool available_;
    double accuracy_;
};

class CounterAccumulator {
public:
    void Reset();
    void Add(uint64_t value);
    void AddMissing();
    CounterValue GetValue() const;
    bool IsAvailable() const;
    bool HasMissing() const;
    uint64_t GetExact() const;
    uint64_t GetApprox() const;
    double GetAccuracy() const;

private:
    std::atomic_uint64_t value_ {};
    std::atomic_uint64_t missing_ {};
    std::atomic_uint64_t total_ {};
};

class PerfFileHandler {
public:
    ~PerfFileHandler();

    NO_COPY_SEMANTIC(PerfFileHandler);

    PerfFileHandler(PerfFileHandler &&other);
    PerfFileHandler &operator=(PerfFileHandler &&other);

    void Reset();
    void Enable();
    void Disable();

    std::optional<uint64_t> GetData() const;

private:
    static int OpenDescriptor(uint32_t type, uint64_t config);
    PerfFileHandler(uint32_t type, uint64_t config);
    int fd_;
    friend class PerfCounterDescriptor;
};

class Perf;
class PerfCounterDescriptor;

class PerfCollector {
public:
    NO_MOVE_SEMANTIC(PerfCollector);
    NO_COPY_SEMANTIC(PerfCollector);

    PerfCollector(Perf *p, std::vector<const PerfCounterDescriptor *> &list, bool isWallTime);
    ~PerfCollector();

    void Reset();
    void Enable();
    void Disable();

private:
    Perf *perf_;
    std::unordered_map<const PerfCounterDescriptor *, PerfFileHandler> counters_;
    bool isWallTime_ {false};
    uint64_t startTime_;
};

class CounterReporter;

class PerfCounterDescriptor {
public:
    NO_COPY_SEMANTIC(PerfCounterDescriptor);
    NO_MOVE_SEMANTIC(PerfCounterDescriptor);

    ~PerfCounterDescriptor() = default;

    PerfFileHandler CreatePerfFileHandler() const;

    const char *GetName() const;
    void Report(std::ostream &out, const Perf *p) const;

    static const PerfCounterDescriptor TASK_CLOCK;
    static const PerfCounterDescriptor CONTEXT_SWITCHES;
    static const PerfCounterDescriptor CPU_MIGRATION;
    static const PerfCounterDescriptor PAGE_FAULT;
    static const PerfCounterDescriptor TOTAL_CPU_CYCLES;
    static const PerfCounterDescriptor STALLED_FRONTEND_CYCLES;
    static const PerfCounterDescriptor STALLED_BACKEND_CYCLES;
    static const PerfCounterDescriptor INSTRUCTIONS_COUNT;
    static const PerfCounterDescriptor BRANCHES;
    static const PerfCounterDescriptor BRANCH_MISSES;
    static const PerfCounterDescriptor COUNT;

private:
    PerfCounterDescriptor(const char *name, uint32_t type, uint64_t config, std::unique_ptr<CounterReporter> reporter);

    const char *name_;
    uint32_t type_;
    uint64_t config_;
    std::unique_ptr<CounterReporter> reporter_;
};

class Perf {
public:
    Perf();
    Perf(std::initializer_list<const PerfCounterDescriptor *> list);
    ~Perf() = default;

    NO_COPY_SEMANTIC(Perf);
    NO_MOVE_SEMANTIC(Perf);

    PerfCollector CreateCollector(bool isWallTime = false);
    void Reset();
    void Add(const PerfCounterDescriptor *desc, uint64_t value);
    void AddMissing(const PerfCounterDescriptor *desc);
    void AddWallTime(uint64_t time);
    CounterValue Get(const PerfCounterDescriptor *desc) const;
    void Report(std::ostream &out) const;

private:
    std::vector<const PerfCounterDescriptor *> perfDescriptors_;
    std::unordered_map<const PerfCounterDescriptor *, CounterAccumulator> counters_;
    uint64_t wallTime_ {0};
};

std::ostream &operator<<(std::ostream &out, const Perf &p);

extern Perf g_perf;
}  // namespace ark::tooling

#endif
