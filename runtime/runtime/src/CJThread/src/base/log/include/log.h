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


#ifndef MRT_CODETHREAD_LOG_H
#define MRT_CODETHREAD_LOG_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include "schedule_rename.h"
#include "Codira.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#ifndef VOSFILENAME
#define VOSFILENAME  __FILE__
#endif

// Log Enable is from MRT_LOG_CODETHREAD. Default value is false.
extern bool g_logEnable;

/**
 * @brief Hook function for writing log.
 */
typedef void (*LogFunc)(const char *message);

/**
 * @brief log level.
 */
enum ThreadLogLevel {
    LOG_LEVEL_DEBUG   = RTLogLevel::RTLOG_DEBUG,
    LOG_LEVEL_INFO    = RTLogLevel::RTLOG_INFO,
    LOG_LEVEL_WARNING = RTLogLevel::RTLOG_WARNING,
    LOG_LEVEL_ERROR   = RTLogLevel::RTLOG_ERROR,
    LOG_LEVEL_FATAL   = RTLogLevel::RTLOG_FATAL,
};

/**
 * @brief Function for recording debug log.
 * @param  errorCode    [IN]  error code
 * @param  fmt          [IN]  log format
 * @param  args         [IN]  args
 */
#define LOG_DEBUG(errorCode, fmt, args...) \
    do { \
        LogWrite(ThreadLogLevel::LOG_LEVEL_DEBUG, (errorCode), VOSFILENAME, __LINE__, fmt, ##args); \
    } while (0)

/**
 * @brief Function for recording info log.
 * @param  errorCode    [IN]  error code
 * @param  fmt          [IN]  log format
 * @param  args         [IN]  args
 */
#define LOG_INFO(errorCode, fmt, args...) \
    do { \
        LogWrite(ThreadLogLevel::LOG_LEVEL_INFO, (errorCode), VOSFILENAME, __LINE__, fmt, ##args); \
    } while (0)

/**
 * @brief Function for recording warning log.
 * @param  errorCode    [IN]  error code
 * @param  fmt          [IN]  log format
 * @param  args         [IN]  args
 */
#define LOG_WARN(errorCode, fmt, args...) \
    do { \
        LogWrite(ThreadLogLevel::LOG_LEVEL_WARNING, (errorCode), VOSFILENAME, __LINE__, fmt, ##args); \
    } while (0)

/**
 * @brief Function for recording error log.
 * @param  errorCode    [IN]  error code
 * @param  fmt          [IN]  log format
 * @param  args         [IN]  args
 */
#define LOG_ERROR(errorCode, fmt, args...) \
    do { \
        LogWrite(ThreadLogLevel::LOG_LEVEL_ERROR, (errorCode), VOSFILENAME, __LINE__, fmt, ##args); \
    } while (0)

/**
 * @brief Function for recording fatal log.
 * @param  errorCode    [IN]  error code
 * @param  fmt          [IN]  log format
 * @param  args         [IN]  args
 */
#define LOG_FATAL(errorCode, fmt, args...) \
    do { \
        LogWrite(ThreadLogLevel::LOG_LEVEL_FATAL, (errorCode), VOSFILENAME, __LINE__, fmt, ##args); \
    } while (0)

/**
 * @brief Function for recording error log on the file or system log.
 * @param  errorCode    [IN]  error code
 * @param  fmt          [IN]  log format
 * @param  args         [IN]  args
 */
#define HILOG_WARN(errorCode, fmt, args...) \
    do { \
        if (g_logEnable) { \
            LogWrite(ThreadLogLevel::LOG_LEVEL_WARNING, (errorCode), VOSFILENAME, __LINE__, fmt, ##args); \
            break; \
        } \
        HiLogWrite(RTLogLevel::RTLOG_WARNING, fmt, ##args); \
    } while (0)

/**
 * @brief Function for recording error log on the file or system log.
 * @param  errorCode    [IN]  error code
 * @param  fmt          [IN]  log format
 * @param  args         [IN]  args
 */
#define HILOG_ERROR(errorCode, fmt, args...) \
    do { \
        if (g_logEnable) { \
            LogWrite(ThreadLogLevel::LOG_LEVEL_ERROR, (errorCode), VOSFILENAME, __LINE__, fmt, ##args); \
            break; \
        } \
        HiLogWrite(RTLogLevel::RTLOG_ERROR, fmt, ##args); \
    } while (0)

/**
 * @brief Function for recording fatal log on the file or system log.
 * @param  errorCode    [IN]  error code
 * @param  fmt          [IN]  log format
 * @param  args         [IN]  args
 */
#define HILOG_FATAL(errorCode, fmt, args...) \
    do { \
        if (g_logEnable) { \
            LogWrite(ThreadLogLevel::LOG_LEVEL_FATAL, (errorCode), VOSFILENAME, __LINE__, fmt, ##args); \
            break; \
        } \
        HiLogWrite(RTLogLevel::RTLOG_FATAL, fmt, ##args); \
    } while (0)

/**
 * @brief Write log information to the file.
 * @attention
 * @param  level        [IN]  level
 * @param  errorCode    [IN]  error code
 * @param  fileName     [IN]  file
 * @param  line         [IN]  line
 * @param  fmt          [IN]  format
 * @retval void
 */
void LogWrite(ThreadLogLevel level,
              unsigned int errorCode,
              const char *fileName,
              unsigned short line,
              const char *fmt,
              ...);

/**
 * @brief Write log information to system log.
 * @attention
 * @param  level        [IN]  level
 * @param  fmt          [IN]  format
 * @retval void
 */
void HiLogWrite(RTLogLevel level, const char *fmt, ...);

/**
 * @brief Registering the Log Hook Function. If func is nullptr, use printf to write log.
 * @param  func   [IN]  hook function for writing log
 * @param  enable [IN]  whether enable codethread log
 * @param  lebel  [IN]  log level
 */
void LogRegister(LogFunc func, bool enable, int level);

/**
 * @brief Check whether info level logs are recorded.
 */
bool LogInfoWritable(void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif /* MRT_CODETHREAD_LOG_H */
