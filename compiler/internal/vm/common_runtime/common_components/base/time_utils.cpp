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

#include "common_components/base/time_utils.h"

#include <array>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <thread>

#include "securec.h"

namespace common {
namespace TimeUtil {
using Sec = std::chrono::seconds;
using Ms = std::chrono::milliseconds;
using Us = std::chrono::microseconds;
using Ns = std::chrono::nanoseconds;

template<typename Unit>
uint64_t ClockTime(clockid_t clock)
{
    struct timespec time = { 0, 0 };
    clock_gettime(clock, &time);
    auto duration = Sec{ time.tv_sec } + Ns{ time.tv_nsec };
    return std::chrono::duration_cast<Unit>(duration).count();
}

uint64_t NanoSeconds() { return ClockTime<Ns>(CLOCK_MONOTONIC); }

uint64_t MilliSeconds() { return ClockTime<Ms>(CLOCK_MONOTONIC); }

uint64_t MicroSeconds() noexcept { return ClockTime<Us>(CLOCK_MONOTONIC); }

void SleepForNano(uint64_t ns) { std::this_thread::sleep_for(std::chrono::nanoseconds(ns)); }

#ifndef NDEBUG
// format: yyyy-mm-dd hh:mm::ss
static inline CString GetDate(const char* format)
{
    std::time_t time = std::time(nullptr);
    std::tm* now = std::localtime(&time);
    if (UNLIKELY_CC(now == nullptr)) {
        return CString();
    }

    std::array<char, 100> buffer; // the array length is 100
    if (std::strftime(buffer.data(), buffer.size(), format, now)) {
        return CString(buffer.data());
    }
    return CString();
}

// yyymmdd.hhmmss format
CString GetDigitDate()
{
    CString date = GetDate("%Y%m%d_%H%M%S");
    return !date.IsEmpty() ? date : "19700101.000000";
}
#endif

CString GetTimestamp()
{
    // yyyy-mm-dd hh:mm::ss.ms format
    std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
    std::chrono::microseconds us = std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch());
    constexpr size_t microSecondsPerSecond = 1000000;
    auto rem = us.count() % microSecondsPerSecond;

    time_t time = std::chrono::system_clock::to_time_t(now);
    struct tm tm;
#ifdef _WIN64
    (void)localtime_s(&tm, &time);
#else
    (void)localtime_r(&time, &tm);
#endif

    constexpr int epochYears = 1900;
    constexpr size_t bufSize = 64;
    char buf[bufSize];
    (void)sprintf_s(buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d:%02d.%06zu", tm.tm_year + epochYears,
                    tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, rem);
    return CString(buf);
}
} // namespace TimeUtil
} // namespace common
