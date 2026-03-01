/**
 * Copyright (c) 2023-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_LOGGER_H_
#define PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_LOGGER_H_

#include <string>
#include <cstdarg>
#include <securec.h>
#include "libarkbase/mem/mem.h"

// NOLINTBEGIN(cppcoreguidelines-macro-usage)

#define INTEROP_LOG(level) LOG(level, ETS_INTEROP_JS)

#ifdef PANDA_TARGET_OHOS
#include <hilog/log.h>

#define INTEROP_LOG_DEBUG(msg) OH_LOG_Print(LOG_APP, LOG_DEBUG, 0xFF00, "ts2ets", "%s", msg)
#define INTEROP_LOG_DEBUG_A(msg, ...) OH_LOG_Print(LOG_APP, LOG_DEBUG, 0xFF00, "ts2ets", msg, __VA_ARGS__)
#define INTEROP_LOG_INFO(msg) OH_LOG_Print(LOG_APP, LOG_INFO, 0xFF00, "ts2ets", "%s", msg)
#define INTEROP_LOG_INFO_A(msg, ...) OH_LOG_Print(LOG_APP, LOG_INFO, 0xFF00, "ts2ets", msg, __VA_ARGS__)
#define INTEROP_LOG_ERROR(msg) OH_LOG_Print(LOG_APP, LOG_ERROR, 0xFF00, "ts2ets", msg)
#define INTEROP_LOG_ERROR_A(msg, ...) OH_LOG_Print(LOG_APP, LOG_ERROR, 0xFF00, "ts2ets", msg, __VA_ARGS__)

#else

// CC-OFFNXT(G.FUD.06) solid logic, ODR
inline std::string EtsLogMakeString(const char *fmt, ...)
{
    va_list ap;      // NOLINT(cppcoreguidelines-pro-type-vararg)
    va_list apCopy;  // NOLINT(cppcoreguidelines-pro-type-vararg)
    va_start(ap, fmt);
    va_copy(apCopy, ap);

    int len = vsnprintf_s(nullptr, 0, PANDA_PAGE_SIZE, fmt, ap);
    if (len < 0) {
        LOG(FATAL, ETS) << "interop_js: Cannot convert message to log buffer";
        UNREACHABLE();
    }

    std::string res;
    res.resize(static_cast<size_t>(len));
    if (vsnprintf_s(res.data(), len + 1, len, fmt, apCopy) < 0) {
        LOG(FATAL, ETS) << "interop_js: Cannot convert message to result string";
        UNREACHABLE();
    }

    va_end(apCopy);
    va_end(ap);
    return res;
}

#define TS2ETS_LOGGER(level, ...)                                    \
    do {                                                             \
        std::string msgTs2etsLogger = EtsLogMakeString(__VA_ARGS__); \
        LOG(level, ETS) << "interop_js: " << msgTs2etsLogger;        \
    } while (0)

#define INTEROP_LOG_DEBUG(msg) TS2ETS_LOGGER(DEBUG, msg)
#define INTEROP_LOG_DEBUG_A(msg, ...) TS2ETS_LOGGER(DEBUG, msg, __VA_ARGS__)
#define INTEROP_LOG_INFO(msg) TS2ETS_LOGGER(INFO, msg)
#define INTEROP_LOG_INFO_A(msg, ...) TS2ETS_LOGGER(INFO, msg, __VA_ARGS__)
#define INTEROP_LOG_ERROR(msg) TS2ETS_LOGGER(INFO, msg)
#define INTEROP_LOG_ERROR_A(msg, ...) TS2ETS_LOGGER(INFO, msg, __VA_ARGS__)
#endif

// NOLINTEND(cppcoreguidelines-macro-usage)

#endif  // PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_LOGGER_H_
