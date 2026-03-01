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

#ifndef ARKCOMPILER_TOOLCHAIN_COMMON_LOG_WRAPPER_H
#define ARKCOMPILER_TOOLCHAIN_COMMON_LOG_WRAPPER_H

#include "common/macros.h"

#if defined(ENABLE_HILOG)
#if defined(__clang__)
#pragma clang diagnostic ignored "-Wgnu-zero-variadic-macro-arguments"
#elif defined(__GNUC__)
#pragma GCC diagnostic ignored "-Wgnu-zero-variadic-macro-arguments"
#endif
#include "hilog/log.h"
#endif
#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD003F00
#undef LOG_TAG
#define LOG_TAG "ArkCompiler"

namespace OHOS::ArkCompiler::Toolchain {
#ifdef LOGF
#undef LOGF
#endif
#ifdef LOGE
#undef LOGE
#endif
#ifdef LOGW
#undef LOGW
#endif
#ifdef LOGI
#undef LOGI
#endif
#ifdef LOGD
#undef LOGD
#endif

#if !defined(ENABLE_HILOG)
enum class LogLevel {
    UNKNOWN,
    DEFAULT,
    VERBOSE,
    DEBUG,
    INFO,
    WARN,
    ERROR,
    FATAL
};
class TOOLCHAIN_EXPORT StdLog {
public:
    StdLog() = default;
    ~StdLog() = default;

    static void PrintLog(LogLevel level, const char* fmt, ...);
};
#endif

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wgnu-zero-variadic-macro-arguments"
#if !defined(ENABLE_HILOG)
#define LOGF(fmt, ...) StdLog::PrintLog(LogLevel::FATAL, fmt, ##__VA_ARGS__)
#define LOGE(fmt, ...) StdLog::PrintLog(LogLevel::ERROR, fmt, ##__VA_ARGS__)
#define LOGW(fmt, ...) StdLog::PrintLog(LogLevel::WARN, fmt, ##__VA_ARGS__)
#define LOGI(fmt, ...) StdLog::PrintLog(LogLevel::INFO, fmt, ##__VA_ARGS__)
#define LOGD(fmt, ...) StdLog::PrintLog(LogLevel::DEBUG, fmt, ##__VA_ARGS__)
#else
#define LOGF(fmt, ...) HILOG_FATAL(LOG_CORE, fmt, ##__VA_ARGS__)
#define LOGE(fmt, ...) HILOG_ERROR(LOG_CORE, fmt, ##__VA_ARGS__)
#define LOGW(fmt, ...) HILOG_WARN(LOG_CORE, fmt, ##__VA_ARGS__)
#define LOGI(fmt, ...) HILOG_INFO(LOG_CORE, fmt, ##__VA_ARGS__)
#define LOGD(fmt, ...) HILOG_DEBUG(LOG_CORE, fmt, ##__VA_ARGS__)
#endif
#pragma clang diagnostic pop
} // namespace OHOS::ArkCompiler::Toolchain
#endif // ARKCOMPILER_TOOLCHAIN_COMMON_LOG_WRAPPER_H
