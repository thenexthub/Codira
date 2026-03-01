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

#include "common_components/log/log.h"

namespace common {
#ifdef ENABLE_HILOG
Level ConvertToLevel(LogLevel hilogLevel)
{
    Level level = Level::ERROR;
    std::string logLevel;
    switch (hilogLevel) {         // LCOV_EXCL_BR_LINE
        case LogLevel::LOG_INFO:  // LCOV_EXCL_BR_LINE
            level = Level::INFO;
            break;
        case LogLevel::LOG_WARN:  // LCOV_EXCL_BR_LINE
            level = Level::WARN;
            break;
        case LogLevel::LOG_ERROR:  // LCOV_EXCL_BR_LINE
            level = Level::ERROR;
            break;
        case LogLevel::LOG_FATAL:      // LCOV_EXCL_BR_LINE
        case LogLevel::LOG_LEVEL_MAX:  // LCOV_EXCL_BR_LINE
            level = Level::FATAL;
            break;
        case LogLevel::LOG_DEBUG:  // LCOV_EXCL_BR_LINE
        default:                   // LCOV_EXCL_BR_LINE
            level = Level::DEBUG;
            break;
    }

    return level;
}

LogLevel GetHiLogLevel()
{
    for (int32_t level = LogLevel::LOG_LEVEL_MIN; level <= LogLevel::LOG_LEVEL_MAX; level++) {  // LCOV_EXCL_BR_LINE
        if (HiLogIsLoggable(LOG_DOMAIN, LOG_TAG, static_cast<LogLevel>(level))) {               // LCOV_EXCL_BR_LINE
            return static_cast<LogLevel>(level);
        }
    }
    return LogLevel::LOG_LEVEL_MAX;
}
#endif

Level Log::level_ = Level::ERROR;
ComponentMark Log::components_ = static_cast<ComponentMark>(Component::ALL);

Level Log::ConvertFromRuntime(LOG_LEVEL level)
{
    Level logLevel = Level::INFO;
    switch (level) {             // LCOV_EXCL_BR_LINE
        case LOG_LEVEL::FOLLOW:  // LCOV_EXCL_BR_LINE
#ifdef ENABLE_HILOG
            logLevel = ConvertToLevel(GetHiLogLevel());
            break;
#endif
        case LOG_LEVEL::INFO:  // LCOV_EXCL_BR_LINE
            logLevel = Level::INFO;
            break;
        case LOG_LEVEL::WARN:  // LCOV_EXCL_BR_LINE
            logLevel = Level::WARN;
            break;
        case LOG_LEVEL::ERROR:  // LCOV_EXCL_BR_LINE
            logLevel = Level::ERROR;
            break;
        case LOG_LEVEL::FATAL:  // LCOV_EXCL_BR_LINE
            logLevel = Level::FATAL;
            break;
        case LOG_LEVEL::DEBUG:  // LCOV_EXCL_BR_LINE
        default:                // LCOV_EXCL_BR_LINE
            logLevel = Level::DEBUG;
            break;
    }

    return logLevel;
}

std::string Log::LevelToString(Level level)
{
    std::string logLevel;
    switch (level) {       // LCOV_EXCL_BR_LINE
        case Level::INFO:  // LCOV_EXCL_BR_LINE
            logLevel = "info";
            break;
        case Level::WARN:  // LCOV_EXCL_BR_LINE
            logLevel = "warning";
            break;
        case Level::ERROR:  // LCOV_EXCL_BR_LINE
            logLevel = "error";
            break;
        case Level::FATAL:  // LCOV_EXCL_BR_LINE
            logLevel = "fatal";
            break;
        case Level::DEBUG:  // LCOV_EXCL_BR_LINE
        default:            // LCOV_EXCL_BR_LINE
            logLevel = "debug";
            break;
    }

    return logLevel;
}

void Log::Initialize(const LogOptions &options)
{
    // For ArkTS runtime log
    level_ = options.level;
    components_ = options.component;
}

// Orders of magnitudes.  Note: The upperbound of uint64_t is 16E (16 * (1024 ^ 6))
const char *g_orderOfMagnitude[] = {"", "K", "M", "G", "T", "P", "E"};

// Orders of magnitudes.  Note: The upperbound of uint64_t is 16E (16 * (1024 ^ 6))
const char *g_orderOfMagnitudeFromNano[] = {"n", "u", "m", nullptr};

// number of digits in a pretty format segment (100,000,000 each has three digits)
constexpr int NUM_DIGITS_PER_SEGMENT = 3;

std::string Pretty(uint64_t number) noexcept
{
    std::string orig = std::to_string(number);
    int pos = static_cast<int>(orig.length()) - NUM_DIGITS_PER_SEGMENT;
    while (pos > 0) {
        orig.insert(pos, ",");
        pos -= NUM_DIGITS_PER_SEGMENT;
    }
    return orig;
}

// Useful for informatic units, such as KiB, MiB, GiB, ...
std::string PrettyOrderInfo(uint64_t number, const char *unit)
{
    size_t order = 0;
    const uint64_t factor = 1024;

    while (number > factor) {
        number /= factor;
        order += 1;
    }

    const char *prefix = g_orderOfMagnitude[order];
    const char *infix = order > 0 ? "i" : "";  // 1KiB = 1024B, but there is no "1iB"

    return std::to_string(number) + prefix + infix + unit;
}

// Useful for scientific units where number is in nanos: ns, us, ms, s
std::string PrettyOrderMathNano(uint64_t number, const char *unit)
{
    size_t order = 0;
    const uint64_t factor = 1000;  // show in us if under 10ms

    while (number > factor && g_orderOfMagnitudeFromNano[order] != nullptr) {
        number /= factor;
        order += 1;
    }

    const char *prefix = g_orderOfMagnitudeFromNano[order];
    if (prefix == nullptr) {
        prefix = "";
    }

    return std::to_string(number) + prefix + unit;
}

constexpr size_t LOG_BUFFER_SIZE = 1024;

std::string FormatLogMessage(const char *format, va_list agrs) noexcept
{
    char buf[LOG_BUFFER_SIZE];
    int ret = vsprintf_s(buf, sizeof(buf), format, agrs);
    if (ret < 0) {
        return std::string("Log format error: ") + strerror(errno);
    }
    return std::string(buf, ret);
}

std::string FormatLog(const char *format, ...) noexcept
{
    va_list args;
    va_start(args, format);
    auto msg = FormatLogMessage(format, args);
    va_end(args);
    return "[CMC GC] " + msg;
}
}  // namespace common
