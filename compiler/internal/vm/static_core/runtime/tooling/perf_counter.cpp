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

#include "runtime/tooling/perf_counter.h"
#include "libarkbase/utils/time.h"
#include <securec.h>

namespace ark::tooling {
// NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
Perf g_perf;

CounterValue::CounterValue(uint64_t value, bool exact, bool available, double accuracy)
    : value_(value), exact_(exact), available_(available), accuracy_(accuracy)
{
}

CounterValue CounterValue::CreateUnavailable()
{
    return CounterValue(0, false, false, 0.0);
}

CounterValue CounterValue::CreateExact(uint64_t value)
{
    return CounterValue(value, true, true, 1.0);
}

CounterValue CounterValue::CreateApprox(uint64_t value, double accuracy)
{
    return CounterValue(value, false, true, accuracy);
}

uint64_t CounterValue::GetValue() const
{
    return value_;
}

bool CounterValue::IsExact() const
{
    return exact_;
}

bool CounterValue::IsAvailable() const
{
    return available_;
}

double CounterValue::GetAccuracy() const
{
    return accuracy_;
}

void CounterAccumulator::Reset()
{
    // Atomic with relaxed order reason: memory order is not required
    value_.store(0, std::memory_order_relaxed);
    // Atomic with relaxed order reason: memory order is not required
    missing_.store(0, std::memory_order_relaxed);
    // Atomic with relaxed order reason: memory order is not required
    total_.store(0, std::memory_order_relaxed);
}

void CounterAccumulator::Add(uint64_t value)
{
    // Atomic with relaxed order reason: memory order is not required
    value_.fetch_add(value, std::memory_order_relaxed);
    // Atomic with relaxed order reason: memory order is not required
    total_.fetch_add(1, std::memory_order_relaxed);
}

void CounterAccumulator::AddMissing()
{
    // Atomic with relaxed order reason: memory order is not required
    missing_.fetch_add(1, std::memory_order_relaxed);
    // Atomic with relaxed order reason: memory order is not required
    total_.fetch_add(1, std::memory_order_relaxed);
}

CounterValue CounterAccumulator::GetValue() const
{
    if (!IsAvailable()) {
        return CounterValue::CreateUnavailable();
    }

    if (HasMissing()) {
        return CounterValue::CreateApprox(GetApprox(), GetAccuracy());
    }

    return CounterValue::CreateExact(GetExact());
}

bool CounterAccumulator::IsAvailable() const
{
    // Atomic with relaxed order reason: memory order is not required
    return total_.load(std::memory_order_relaxed) > 0;
}

bool CounterAccumulator::HasMissing() const
{
    // Atomic with relaxed order reason: memory order is not required
    return missing_.load(std::memory_order_relaxed) > 0;
}

uint64_t CounterAccumulator::GetExact() const
{
    ASSERT(IsAvailable());
    ASSERT(!HasMissing());
    // Atomic with relaxed order reason: memory order is not required
    return value_.load(std::memory_order_relaxed);
}

uint64_t CounterAccumulator::GetApprox() const
{
    ASSERT(IsAvailable());
    ASSERT(HasMissing());
    // Atomic with relaxed order reason: memory order is not required
    return value_.load(std::memory_order_relaxed) * GetAccuracy();
}

double CounterAccumulator::GetAccuracy() const
{
    ASSERT(IsAvailable());
    ASSERT(HasMissing());
    // Atomic with relaxed order reason: memory order is not required
    return static_cast<double>(missing_.load(std::memory_order_relaxed)) / total_.load(std::memory_order_relaxed);
}

int PerfFileHandler::OpenDescriptor(uint32_t type, uint64_t config)
{
    perf_event_attr attr {};
    attr.type = type;
    attr.size = sizeof(attr);
    attr.config = config;
    attr.disabled = 1;
    attr.exclude_kernel = 1;
    attr.exclude_hv = 1;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    return syscall(__NR_perf_event_open, &attr, 0 /*pid*/, -1 /*cpu*/, -1 /*group_fd*/, PERF_FLAG_FD_CLOEXEC);
}

PerfFileHandler::PerfFileHandler(uint32_t type, uint64_t config) : fd_(OpenDescriptor(type, config)) {}

PerfFileHandler::~PerfFileHandler()
{
    if (fd_ >= 0) {
        close(fd_);
    }
}

PerfFileHandler::PerfFileHandler(PerfFileHandler &&other) : fd_(other.fd_)
{
    other.fd_ = -1;
}

PerfFileHandler &PerfFileHandler::operator=(PerfFileHandler &&other)
{
    this->fd_ = other.fd_;
    other.fd_ = -1;
    return *this;
}

void PerfFileHandler::Reset()
{
    if (fd_ >= 0) {
        ioctl(fd_, PERF_EVENT_IOC_RESET, 0);
    }
}

void PerfFileHandler::Enable()
{
    if (fd_ >= 0) {
        ioctl(fd_, PERF_EVENT_IOC_ENABLE, 0);
    }
}

void PerfFileHandler::Disable()
{
    if (fd_ >= 0) {
        ioctl(fd_, PERF_EVENT_IOC_DISABLE, 0);
    }
}

std::optional<uint64_t> PerfFileHandler::GetData() const
{
    if (fd_ < 0) {
        return std::nullopt;
    }

    uint64_t count;
    auto n = read(fd_, &count, sizeof(count));
    if (n != sizeof(count)) {
        return std::nullopt;
    }

    return count;
}

PerfCollector::PerfCollector(Perf *p, std::vector<const PerfCounterDescriptor *> &list, bool isWallTime)
    : perf_(p), isWallTime_(isWallTime)
{
    for (auto *desc : list) {
        counters_.insert(std::make_pair(desc, desc->CreatePerfFileHandler()));
    }

    Enable();

    if (isWallTime_) {
        startTime_ = ark::time::GetCurrentTimeInNanos();
    }
}

PerfCollector::~PerfCollector()
{
    if (isWallTime_) {
        perf_->AddWallTime(ark::time::GetCurrentTimeInNanos() - startTime_);
    }

    Disable();

    for (auto &[desc, counter] : counters_) {
        auto data = counter.GetData();
        if (data) {
            perf_->Add(desc, data.value());
        } else {
            perf_->AddMissing(desc);
        }
    }
}

void PerfCollector::Reset()
{
    for (auto &[desc, counter] : counters_) {
        counter.Reset();
    }
}

void PerfCollector::Enable()
{
    for (auto &[desc, counter] : counters_) {
        counter.Enable();
    }
}

void PerfCollector::Disable()
{
    for (auto &[desc, counter] : counters_) {
        counter.Disable();
    }
}

PerfCounterDescriptor::PerfCounterDescriptor(const char *name, uint32_t type, uint64_t config,
                                             std::unique_ptr<CounterReporter> reporter)
    : name_(name), type_(type), config_(config), reporter_(std::move(reporter))
{
}

PerfFileHandler PerfCounterDescriptor::CreatePerfFileHandler() const
{
    return PerfFileHandler(type_, config_);
}

const char *PerfCounterDescriptor::GetName() const
{
    return name_;
}

class CounterReporter {
public:
    virtual void Report(std::ostream &out, const char *title, CounterValue counter, const Perf *p) const = 0;
    CounterReporter() = default;
    NO_COPY_SEMANTIC(CounterReporter);
    NO_MOVE_SEMANTIC(CounterReporter);
    virtual ~CounterReporter() = default;

    static constexpr size_t VALUE_ALIGNEMENT = 20;

protected:
    static void FormatLongNumber(uint64_t value, char *out);
    static void ReportNanCounter(std::ostream &out, const char *title);

    static void ReportLongCounter(std::ostream &out, const char *title, CounterValue counter);
    static void ReportLongCounter(std::ostream &out, const char *title, CounterValue counter,
                                  CounterValue denominatorCounter, const char *unit);
    static void ReportTimeCounter(std::ostream &out, const char *title, CounterValue counter);
    static void ReportCounterRatio(std::ostream &out, CounterValue counter, CounterValue denominatorCounter,
                                   const char *unit);
    static constexpr size_t BUFFER_SIZE = VALUE_ALIGNEMENT + 1;
    static constexpr size_t PRECISSION = 3;
};

void CounterReporter::FormatLongNumber(uint64_t value, char *out)
{
    static constexpr size_t THREE_DIGITS = 3;
    static constexpr size_t RADIX10 = 10;

    auto p = VALUE_ALIGNEMENT;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    out[p--] = '\0';

    auto k = THREE_DIGITS;
    while (value != 0) {
        auto d = static_cast<char>((value % RADIX10) + '0');
        value /= RADIX10;
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        out[p--] = d;
        k--;
        if (k == 0) {
            k = THREE_DIGITS;
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            out[p--] = ' ';
        }
    }

    for (size_t i = 0; i <= p; i++) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        out[i] = ' ';
    }
}

void CounterReporter::ReportNanCounter(std::ostream &out, const char *title)
{
    out << std::setw(VALUE_ALIGNEMENT) << "NaN"
        << " " << title << std::endl;
}

void CounterReporter::ReportLongCounter(std::ostream &out, const char *title, CounterValue counter)
{
    if (!counter.IsAvailable()) {
        ReportNanCounter(out, title);
        return;
    }

    char buf[BUFFER_SIZE];  // NOLINT(modernize-avoid-c-arrays)
    FormatLongNumber(counter.GetValue(), buf);

    out << buf << " " << title;
    if (!counter.IsExact()) {
        out << " approx";
    }
    out << std::endl;
}

void CounterReporter::ReportLongCounter(std::ostream &out, const char *title, CounterValue counter,
                                        CounterValue denominatorCounter, const char *unit)
{
    if (!counter.IsAvailable()) {
        ReportNanCounter(out, title);
        return;
    }

    char buf[BUFFER_SIZE];  // NOLINT(modernize-avoid-c-arrays)
    FormatLongNumber(counter.GetValue(), buf);

    if (!denominatorCounter.IsAvailable() || denominatorCounter.GetValue() == 0) {
        out << buf << " " << title << std::endl;
        return;
    }

    out << buf << " " << title << " (" << std::setprecision(PRECISSION) << std::fixed
        << static_cast<double>(counter.GetValue()) / denominatorCounter.GetValue() << " " << unit << ")";

    if (!counter.IsExact() || !denominatorCounter.IsExact()) {
        out << " approx";
    }
    out << std::endl;
}

void CounterReporter::ReportTimeCounter(std::ostream &out, const char *title, CounterValue counter)
{
    if (!counter.IsAvailable()) {
        ReportNanCounter(out, title);
        return;
    }

    auto prettyTime = ark::helpers::TimeConverter(counter.GetValue());
    out << std::setw(VALUE_ALIGNEMENT - prettyTime.GetLiteral().length()) << prettyTime << " " << title;
    if (!counter.IsExact()) {
        out << " approx";
    }
    out << std::endl;
}

void CounterReporter::ReportCounterRatio(std::ostream &out, CounterValue counter, CounterValue denominatorCounter,
                                         const char *unit)
{
    if (!denominatorCounter.IsAvailable() || denominatorCounter.GetValue() == 0) {
        ReportNanCounter(out, unit);
        return;
    }

    out << std::setw(VALUE_ALIGNEMENT) << std::setprecision(PRECISSION) << std::fixed
        << static_cast<double>(counter.GetValue()) / denominatorCounter.GetValue() << " " << unit << std::endl;
}

void PerfCounterDescriptor::Report(std::ostream &out, const Perf *p) const
{
    reporter_->Report(out, GetName(), p->Get(this), p);
}

Perf::Perf()
    : Perf({&PerfCounterDescriptor::TASK_CLOCK, &PerfCounterDescriptor::TOTAL_CPU_CYCLES,
            &PerfCounterDescriptor::STALLED_BACKEND_CYCLES, &PerfCounterDescriptor::INSTRUCTIONS_COUNT})
{
}

Perf::Perf(std::initializer_list<const PerfCounterDescriptor *> list)
{
    for (auto *e : list) {
        perfDescriptors_.push_back(e);
    }
    Reset();
}

PerfCollector Perf::CreateCollector(bool isWallTime)
{
    return PerfCollector(this, perfDescriptors_, isWallTime);
}

void Perf::Reset()
{
    for (auto *desc : perfDescriptors_) {
        counters_[desc].Reset();
    }
}

void Perf::Add(const PerfCounterDescriptor *desc, uint64_t value)
{
    counters_.at(desc).Add(value);
}

void Perf::AddMissing(const PerfCounterDescriptor *desc)
{
    counters_.at(desc).AddMissing();
}

void Perf::AddWallTime(uint64_t time)
{
    wallTime_ = time;
}

CounterValue Perf::Get(const PerfCounterDescriptor *desc) const
{
    if (counters_.count(desc) > 0) {
        return counters_.at(desc).GetValue();
    }

    return CounterValue::CreateUnavailable();
}

void Perf::Report(std::ostream &out) const
{
    out << std::endl;
    if (wallTime_ > 0) {
        auto prettyTime = ark::helpers::TimeConverter(wallTime_);
        out << std::setw(CounterReporter::VALUE_ALIGNEMENT - prettyTime.GetLiteral().size()) << prettyTime
            << " wall time" << std::endl;
    }
    for (auto *desc : perfDescriptors_) {
        desc->Report(out, this);
    }
}

std::ostream &operator<<(std::ostream &out, const Perf &p)
{
    p.Report(out);
    return out;
}

class LongCounterReporter : public CounterReporter {
public:
    void Report(std::ostream &out, const char *title, CounterValue counter,
                [[maybe_unused]] const Perf *p) const override
    {
        ReportLongCounter(out, title, counter);
    }
};

class LongCounterWithRatioReporter : public CounterReporter {
public:
    LongCounterWithRatioReporter(const PerfCounterDescriptor *denominator, const char *unit)
        : denominator_(denominator), unit_(unit)
    {
    }

    void Report(std::ostream &out, const char *title, CounterValue counter, const Perf *p) const override
    {
        ReportLongCounter(out, title, counter, p->Get(denominator_), unit_);
    }

private:
    const PerfCounterDescriptor *denominator_;
    const char *unit_;
};

class InstructionCounterReporter : public CounterReporter {
public:
    void Report(std::ostream &out, const char *title, CounterValue counter, const Perf *p) const override
    {
        ReportLongCounter(out, title, counter);
        ReportCounterRatio(out, counter, p->Get(&PerfCounterDescriptor::TOTAL_CPU_CYCLES), "insn per cycle");
        ReportCounterRatio(out, p->Get(&PerfCounterDescriptor::STALLED_BACKEND_CYCLES), counter,
                           "stalled cycles per insn");
    }
};

class TimeCounterReporter : public CounterReporter {
public:
    void Report(std::ostream &out, const char *title, CounterValue counter,
                [[maybe_unused]] const Perf *p) const override
    {
        ReportTimeCounter(out, title, counter);
    }
};

// NOLINTBEGIN(fuchsia-statically-constructed-objects)
const PerfCounterDescriptor PerfCounterDescriptor::TASK_CLOCK("task-clock", PERF_TYPE_SOFTWARE,
                                                              PERF_COUNT_SW_TASK_CLOCK,
                                                              std::make_unique<TimeCounterReporter>());
const PerfCounterDescriptor PerfCounterDescriptor::CONTEXT_SWITCHES("context switches", PERF_TYPE_SOFTWARE,
                                                                    PERF_COUNT_SW_CONTEXT_SWITCHES,
                                                                    std::make_unique<LongCounterReporter>());
const PerfCounterDescriptor PerfCounterDescriptor::CPU_MIGRATION("cpu-migrations", PERF_TYPE_SOFTWARE,
                                                                 PERF_COUNT_SW_CPU_MIGRATIONS,
                                                                 std::make_unique<LongCounterReporter>());
const PerfCounterDescriptor PerfCounterDescriptor::PAGE_FAULT("page-faults", PERF_TYPE_SOFTWARE,
                                                              PERF_COUNT_SW_PAGE_FAULTS,
                                                              std::make_unique<LongCounterReporter>());
const PerfCounterDescriptor PerfCounterDescriptor::TOTAL_CPU_CYCLES(
    "total cpu cycles", PERF_TYPE_HARDWARE, PERF_COUNT_HW_CPU_CYCLES,
    std::make_unique<LongCounterWithRatioReporter>(&PerfCounterDescriptor::TASK_CLOCK, "GHz"));
const PerfCounterDescriptor PerfCounterDescriptor::STALLED_FRONTEND_CYCLES(
    "stalled frontend cycles", PERF_TYPE_HARDWARE, PERF_COUNT_HW_STALLED_CYCLES_FRONTEND,
    std::make_unique<LongCounterWithRatioReporter>(&PerfCounterDescriptor::TOTAL_CPU_CYCLES, "of cycles"));
const PerfCounterDescriptor PerfCounterDescriptor::STALLED_BACKEND_CYCLES(
    "stalled backend cycles", PERF_TYPE_HARDWARE, PERF_COUNT_HW_STALLED_CYCLES_BACKEND,
    std::make_unique<LongCounterWithRatioReporter>(&PerfCounterDescriptor::TOTAL_CPU_CYCLES, "of cycles"));
const PerfCounterDescriptor PerfCounterDescriptor::INSTRUCTIONS_COUNT("instructions", PERF_TYPE_HARDWARE,
                                                                      PERF_COUNT_HW_INSTRUCTIONS,
                                                                      std::make_unique<InstructionCounterReporter>());
const PerfCounterDescriptor PerfCounterDescriptor::BRANCHES("branches", PERF_TYPE_HARDWARE,
                                                            PERF_COUNT_HW_BRANCH_INSTRUCTIONS,
                                                            std::make_unique<LongCounterReporter>());
const PerfCounterDescriptor PerfCounterDescriptor::BRANCH_MISSES(
    "branch-misses", PERF_TYPE_HARDWARE, PERF_COUNT_HW_BRANCH_MISSES,
    std::make_unique<LongCounterWithRatioReporter>(&PerfCounterDescriptor::BRANCHES, "of branches"));
// NOLINTEND(fuchsia-statically-constructed-objects)
}  // namespace ark::tooling
