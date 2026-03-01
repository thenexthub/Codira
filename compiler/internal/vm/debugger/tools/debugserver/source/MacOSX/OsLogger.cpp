//===-- OsLogger.cpp --------------------------------------------*- C++ -*-===//
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

#include "OsLogger.h"
#include <Availability.h>

#if (LLDB_USE_OS_LOG) && (__MAC_OS_X_VERSION_MAX_ALLOWED >= 101200)

#include <os/log.h>

#include "DNBDefs.h"
#include "DNBLog.h"

#define LLDB_OS_LOG_MAX_BUFFER_LENGTH 256

namespace {
// Darwin os_log logging callback that can be registered with
// DNBLogSetLogCallback
void DarwinLogCallback(void *baton, uint32_t flags, const char *format,
                       va_list args) {
  if (format == nullptr)
    return;

  static os_log_t g_logger;
  if (!g_logger) {
    g_logger = os_log_create("com.apple.dt.lldb", "debugserver");
    if (!g_logger)
      return;
  }

  os_log_type_t log_type;
  if (flags & DNBLOG_FLAG_FATAL)
    log_type = OS_LOG_TYPE_FAULT;
  else if (flags & DNBLOG_FLAG_ERROR)
    log_type = OS_LOG_TYPE_ERROR;
  else if (flags & DNBLOG_FLAG_WARNING)
    log_type = OS_LOG_TYPE_DEFAULT;
  else if (flags & DNBLOG_FLAG_VERBOSE)
    log_type = OS_LOG_TYPE_DEBUG;
  else
    log_type = OS_LOG_TYPE_DEFAULT;

  // This code is unfortunate.  os_log* only takes static strings, but
  // our current log API isn't set up to make use of that style.
  char buffer[LLDB_OS_LOG_MAX_BUFFER_LENGTH];
  vsnprintf(buffer, sizeof(buffer), format, args);
  os_log_with_type(g_logger, log_type, "%{public}s", buffer);
}
}

DNBCallbackLog OsLogger::GetLogFunction() { return DarwinLogCallback; }

#else

DNBCallbackLog OsLogger::GetLogFunction() { return nullptr; }

#endif

