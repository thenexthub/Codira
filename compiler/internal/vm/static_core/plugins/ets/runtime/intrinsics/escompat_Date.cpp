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

#include <chrono>
#include <ctime>
#include <clocale>
#include <sstream>
#include "unicode/timezone.h"
#include "plugins/ets/runtime/types/ets_string.h"

namespace ark::ets::intrinsics {

constexpr const int32_t MS_IN_MINUTES = 60000;

extern "C" int64_t EscompatDateNow()
{
    auto now = std::chrono::system_clock::now();
    auto nowMs = std::chrono::time_point_cast<std::chrono::milliseconds>(now);
    return nowMs.time_since_epoch().count();
}

extern "C" int64_t EscompatDateGetLocalTimezoneOffset(int64_t ms)
{
    UErrorCode success = U_ZERO_ERROR;
    int32_t stdOffset;
    int32_t dstOffset;
    icu::TimeZone *tzlocal = icu::TimeZone::createDefault();
    tzlocal->getOffset(ms, 1, stdOffset, dstOffset, success);
    delete tzlocal;
    return -(stdOffset + dstOffset) / MS_IN_MINUTES;
}

extern "C" EtsString *EscompatDateGetTimezoneName(int64_t ms)
{
    UErrorCode success = U_ZERO_ERROR;
    int32_t stdOffset;
    int32_t dstOffset;
    icu::TimeZone *tzlocal = icu::TimeZone::createDefault();
    icu::UnicodeString s;
    std::string result;
    tzlocal->getOffset(ms, 0, stdOffset, dstOffset, success);
    bool inDayLight = (dstOffset != 0);
    tzlocal->getDisplayName(static_cast<UBool>(inDayLight), icu::TimeZone::EDisplayType::LONG, s);
    s.toUTF8String(result);
    delete tzlocal;
    return EtsString::CreateFromMUtf8(result.c_str());
}

extern "C" int64_t ChronoGetCpuTime()
{
    // NOTE(ipetrov, #15499): Need to change approach when coroutine can migrate to other thread
    return ark::os::time::GetClockTimeInThreadCpuTime();
}

}  // namespace ark::ets::intrinsics
