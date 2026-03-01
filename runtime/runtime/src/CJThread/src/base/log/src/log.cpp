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


#include <libgen.h>
#include "securec.h"
#include "log.h"
#include "Base/Log.h"

#ifdef __cplusplus
extern "C" {
#endif

const int LOG_BUF_SIZE = 1024;

LogFunc g_logFunc = nullptr;

// Log Enable is from MRT_LOG_CODETHREAD. Default value is false.
bool g_logEnable = false;

// Log level is from MRT_LOG_LEVEL. Default value is error.
int g_logLevel = ThreadLogLevel::LOG_LEVEL_ERROR;

bool LogInfoWritable(void)
{
    return g_logLevel >= ThreadLogLevel::LOG_LEVEL_INFO;
}

const char *ErrorLeverString(ThreadLogLevel error)
{
    switch (error) {
        case ThreadLogLevel::LOG_LEVEL_DEBUG:
            return "D";
        case ThreadLogLevel::LOG_LEVEL_INFO:
            return "I";
        case ThreadLogLevel::LOG_LEVEL_WARNING:
            return "W";
        case ThreadLogLevel::LOG_LEVEL_ERROR:
            return "E";
        case ThreadLogLevel::LOG_LEVEL_FATAL:
            return "F";
        default:
            return "U";
    }
    return "U";
}

void LogWrite(ThreadLogLevel level,
              unsigned int errorCode,
              const char *fileName,
              unsigned short line,
              const char *fmt,
              ...)
{
    if (!g_logEnable || level < g_logLevel) {
        return;
    }
    va_list alist;
    char output[LOG_BUF_SIZE];
    int ret;
    int len;
    LogFunc func;

#ifdef MRT_WINDOWS
    fileName = strdup(fileName);
#endif
    ret = snprintf_s(output, LOG_BUF_SIZE, LOG_BUF_SIZE - 1,
        "%s File=%s:%u Error=0x%x ",
        ErrorLeverString(level), basename(const_cast<char *>(fileName)), line, errorCode);
    if (ret < 0) {
        printf("LogWrite failed: %d\n", errno);
        return;
    }
    len = ret;
#ifdef MRT_WINDOWS
    free((void *)fileName);
#endif

    va_start(alist, fmt);
    ret = vsnprintf_s(output + len, LOG_BUF_SIZE - len, LOG_BUF_SIZE - len - 1, fmt, alist);
    va_end(alist);
    if (ret < 0) {
        printf("LogWrite failed: %d\n", errno);
        printf("%s\r\n", output);
        return;
    }

    func = g_logFunc;
    if (func != nullptr) {
        func(output);
    } else {
        printf("%s\r\n", output);
    }

    if (level == ThreadLogLevel::LOG_LEVEL_FATAL) {
        abort();
    }
}

void HiLogWrite(RTLogLevel level, const char *fmt, ...)
{
#if defined (__OHOS__) || defined(__ANDROID__)
    va_list args;
    va_start(args, fmt);
    MapleRuntime::HiLogForCODEThread(level, fmt, args);
    va_end(args);
    return;
#else
    if (level == RTLogLevel::RTLOG_FATAL) {
        va_list alist;
        char output[LOG_BUF_SIZE];
        va_start(alist, fmt);
        int ret = vsnprintf_s(output, LOG_BUF_SIZE, LOG_BUF_SIZE - 1, fmt, alist);
        va_end(alist);
        if (ret < 0) {
            printf("HiLogWrite failed: %d\n", errno);
            printf("%s\r\n", output);
            return;
        }
        printf("%s\r\n", output);
    }
#endif
}

void LogRegister(LogFunc logFunc, bool enable, int level)
{
    g_logFunc = logFunc;
    g_logEnable = enable;
    g_logLevel = level;
}

#ifdef __cplusplus
}
#endif
