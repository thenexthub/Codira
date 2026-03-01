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

#include "runtime/include/time_utils.h"

#include <iomanip>

#include "libarkbase/utils/time.h"

namespace ark::time {

constexpr size_t TIME_BUFF_LENGTH = 100;

Timer::Timer(uint64_t *duration, bool needRestart) : duration_(duration), startTime_(GetCurrentTimeInNanos())
{
    if (needRestart) {
        *duration = 0;
    }
}

Timer::~Timer()
{
    *duration_ += GetCurrentTimeInNanos() - startTime_;
}

PandaString GetCurrentTimeString()
{
    PandaOStringStream resultStream;
    auto timeNow = GetCurrentTimeInMillis(true);
    auto millisecond = static_cast<time_t>(timeNow % MILLISECONDS_IN_SECOND);
    auto seconds = static_cast<time_t>(timeNow / MILLISECONDS_IN_SECOND);

    constexpr int DATE_BUFFER_SIZE = 16;
    PandaString dateBuffer;
    dateBuffer.resize(DATE_BUFFER_SIZE);
    std::tm *now = std::localtime(&seconds);
    ASSERT(now != nullptr);
    if (std::strftime(dateBuffer.data(), DATE_BUFFER_SIZE, "%b %d %T", now) == 0U) {
        return "";
    }
    // Because strftime returns a string in C format
    dateBuffer[DATE_BUFFER_SIZE - 1] = '.';
    resultStream << dateBuffer << std::setfill('0') << std::setw(PRECISION_FOR_TIME) << millisecond;
    return resultStream.str();
}

PandaString GetCurrentTimeString(const char *format)
{
    std::string date {};
    std::time_t time = std::time(nullptr);
    std::tm *now = std::localtime(&time);
    ASSERT(now != nullptr);
    std::array<char, TIME_BUFF_LENGTH> buffer {};
    if (std::strftime(buffer.data(), buffer.size(), format, now) != 0) {
        date = std::string {buffer.data()};
    }
    return ConvertToString(!date.empty() ? date : "1970-01-01 00:00:00");
}

}  // namespace ark::time
