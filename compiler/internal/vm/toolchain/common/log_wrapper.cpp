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

#include "common/log_wrapper.h"

#include <string>
#ifdef ANDROID_PLATFORM
#include <android/log.h>
#else
#include <thread>
#include "securec.h"
#endif

namespace OHOS::ArkCompiler::Toolchain {
void StripString(const std::string& prefix, std::string& str)
{
    for (auto pos = str.find(prefix, 0); pos != std::string::npos; pos = str.find(prefix, pos)) {
        str.erase(pos, prefix.size());
    }
}

std::string StripFormatString(const char* fmt)
{
    std::string newFmt(fmt);
    StripString("{public}", newFmt);
    StripString("{private}", newFmt);
    return newFmt;
}

#ifdef ANDROID_PLATFORM
const char* tag = "ArkCompiler";
void StdLog::PrintLog(LogLevel level, const char* fmt, ...)
{
    std::string formatted = StripFormatString(fmt);
    va_list args;
    va_start(args, fmt);
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wformat-nonliteral"
    __android_log_vprint(static_cast<int>(level), tag, formatted.c_str(), args);
#pragma clang diagnostic pop
    va_end(args);
}
#else
#ifndef ENABLE_HILOG
constexpr int32_t MAX_BUFFER_SIZE = 100;
void StdLog::PrintLog(LogLevel level, const char* fmt, ...)
{
    std::string formatted = StripFormatString(fmt);
    va_list args;
    va_start(args, fmt);

    char buf[MAX_BUFFER_SIZE];
    if (vsnprintf_s(buf, sizeof(buf), sizeof(buf) - 1, formatted.c_str(), args) < 0 && errno == EINVAL) {
        va_end(args);
        return;
    }
    va_end(args);

    char timeBuf[MAX_BUFFER_SIZE];
    const char* domainTag = "[ArkCompiler Debugger]";
    std::string levelTag;
    switch (level) {
        case LogLevel::FATAL:
            levelTag = "[FATAL]";
            break;
        case LogLevel::ERROR:
            levelTag = "[ERROR]";
            break;
        case LogLevel::WARN:
            levelTag = "[WARN]";
            break;
        case LogLevel::INFO:
            levelTag = "[INFO]";
            break;
        case LogLevel::DEBUG:
            levelTag = "[DEBUG]";
            break;
        default:
            levelTag = "[DEFAULT]";
            break;
    }

    if (snprintf_s(timeBuf, sizeof(timeBuf), sizeof(timeBuf) - 1, "%s %s %d",
            domainTag, levelTag.c_str(), std::this_thread::get_id()) < 0) {
        return;
    }

    printf("%s %s\r\n", timeBuf, buf);
}
#endif
#endif
} // namespace OHOS::ArkCompiler::Toolchain