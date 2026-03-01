//===-- DNBLog.h ------------------------------------------------*- C++ -*-===//
//
// Copyright (c) NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// Author: Tunjay Akbarli
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
// Middletown, DE 19709, New Castle County, USA.
//
//===----------------------------------------------------------------------===//
//
//  Created by Greg Clayton on 6/18/07.
//
//===----------------------------------------------------------------------===//

#ifndef LLDB_TOOLS_DEBUGSERVER_SOURCE_DNBLOG_H
#define LLDB_TOOLS_DEBUGSERVER_SOURCE_DNBLOG_H

#include "DNBDefs.h"
#include <cstdint>
#include <cstdio>

extern "C" {

// Flags that get filled in automatically before calling the log callback
// function
#define DNBLOG_FLAG_FATAL (1u << 0)
#define DNBLOG_FLAG_ERROR (1u << 1)
#define DNBLOG_FLAG_WARNING (1u << 2)
#define DNBLOG_FLAG_DEBUG (1u << 3)
#define DNBLOG_FLAG_VERBOSE (1u << 4)
#define DNBLOG_FLAG_THREADED (1u << 5)

#define DNBLOG_ENABLED

#if defined(DNBLOG_ENABLED)

void _DNBLog(uint32_t flags, const char *format, ...)
    __attribute__((format(printf, 2, 3)));
void _DNBLogDebug(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
void _DNBLogDebugVerbose(const char *fmt, ...)
    __attribute__((format(printf, 1, 2)));
void _DNBLogThreaded(const char *fmt, ...)
    __attribute__((format(printf, 1, 2)));
void _DNBLogThreadedIf(uint32_t mask, const char *fmt, ...)
    __attribute__((format(printf, 2, 3)));
void _DNBLogError(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
void _DNBLogFatalError(int err, const char *fmt, ...)
    __attribute__((format(printf, 2, 3)));
void _DNBLogVerbose(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
void _DNBLogWarning(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
void _DNBLogWarningVerbose(const char *fmt, ...)
    __attribute__((format(printf, 1, 2)));
bool DNBLogCheckLogBit(uint32_t bit);
uint32_t DNBLogSetLogMask(uint32_t mask);
uint32_t DNBLogGetLogMask();
void DNBLogSetLogCallback(DNBCallbackLog callback, void *baton);
DNBCallbackLog DNBLogGetLogCallback();
bool DNBLogEnabled();
bool DNBLogEnabledForAny(uint32_t mask);
int DNBLogGetDebug();
void DNBLogSetDebug(int g);
int DNBLogGetVerbose();
void DNBLogSetVerbose(int g);

#define DNBLog(fmt, ...)                                                       \
  do {                                                                         \
    if (DNBLogEnabled()) {                                                     \
      _DNBLog(0, fmt, ##__VA_ARGS__);                                          \
    }                                                                          \
  } while (0)
#define DNBLogDebug(fmt, ...)                                                  \
  do {                                                                         \
    if (DNBLogEnabled()) {                                                     \
      _DNBLogDebug(fmt, ##__VA_ARGS__);                                        \
    }                                                                          \
  } while (0)
#define DNBLogDebugVerbose(fmt, ...)                                           \
  do {                                                                         \
    if (DNBLogEnabled()) {                                                     \
      _DNBLogDebugVerbose(fmt, ##__VA_ARGS__);                                 \
    }                                                                          \
  } while (0)
#define DNBLogThreaded(fmt, ...)                                               \
  do {                                                                         \
    if (DNBLogEnabled()) {                                                     \
      _DNBLogThreaded(fmt, ##__VA_ARGS__);                                     \
    }                                                                          \
  } while (0)
#define DNBLogThreadedIf(mask, fmt, ...)                                       \
  do {                                                                         \
    if (DNBLogEnabledForAny(mask)) {                                           \
      _DNBLogThreaded(fmt, ##__VA_ARGS__);                                     \
    }                                                                          \
  } while (0)
#define DNBLogError(fmt, ...)                                                  \
  do {                                                                         \
    if (DNBLogEnabled()) {                                                     \
      _DNBLogError(fmt, ##__VA_ARGS__);                                        \
    }                                                                          \
  } while (0)
#define DNBLogFatalError(err, fmt, ...)                                        \
  do {                                                                         \
    if (DNBLogEnabled()) {                                                     \
      _DNBLogFatalError(err, fmt, ##__VA_ARGS__);                              \
    }                                                                          \
  } while (0)
#define DNBLogVerbose(fmt, ...)                                                \
  do {                                                                         \
    if (DNBLogEnabled()) {                                                     \
      _DNBLogVerbose(fmt, ##__VA_ARGS__);                                      \
    }                                                                          \
  } while (0)
#define DNBLogWarning(fmt, ...)                                                \
  do {                                                                         \
    if (DNBLogEnabled()) {                                                     \
      _DNBLogWarning(fmt, ##__VA_ARGS__);                                      \
    }                                                                          \
  } while (0)
#define DNBLogWarningVerbose(fmt, ...)                                         \
  do {                                                                         \
    if (DNBLogEnabled()) {                                                     \
      _DNBLogWarningVerbose(fmt, ##__VA_ARGS__);                               \
    }                                                                          \
  } while (0)

#else // #if defined(DNBLOG_ENABLED)

#define DNBLogDebug(...) ((void)0)
#define DNBLogDebugVerbose(...) ((void)0)
#define DNBLogThreaded(...) ((void)0)
#define DNBLogThreadedIf(...) ((void)0)
#define DNBLogError(...) ((void)0)
#define DNBLogFatalError(...) ((void)0)
#define DNBLogVerbose(...) ((void)0)
#define DNBLogWarning(...) ((void)0)
#define DNBLogWarningVerbose(...) ((void)0)
#define DNBLogGetLogFile() ((FILE *)NULL)
#define DNBLogSetLogFile(f) ((void)0)
#define DNBLogCheckLogBit(bit) ((bool)false)
#define DNBLogSetLogMask(mask) ((uint32_t)0u)
#define DNBLogGetLogMask() ((uint32_t)0u)
#define DNBLogToASL() ((void)0)
#define DNBLogToFile() ((void)0)
#define DNBLogCloseLogFile() ((void)0)

#endif // #else defined(DNBLOG_ENABLED)
}

#endif // LLDB_TOOLS_DEBUGSERVER_SOURCE_DNBLOG_H
