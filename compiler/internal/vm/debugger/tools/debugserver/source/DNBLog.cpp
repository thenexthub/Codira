//===-- DNBLog.cpp ----------------------------------------------*- C++ -*-===//
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

#include "DNBLog.h"

static int g_debug = 0;
static int g_verbose = 0;

#if defined(DNBLOG_ENABLED)

#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <mach/mach.h>
#include <mutex>
#include <pthread.h>
#include <sys/time.h>
#include <unistd.h>

uint32_t g_log_bits = 0;
static DNBCallbackLog g_log_callback = NULL;
static void *g_log_baton = NULL;

int DNBLogGetDebug() { return g_debug; }

void DNBLogSetDebug(int g) { g_debug = g; }

int DNBLogGetVerbose() { return g_verbose; }

void DNBLogSetVerbose(int v) { g_verbose = v; }

bool DNBLogCheckLogBit(uint32_t bit) { return (g_log_bits & bit) != 0; }

uint32_t DNBLogSetLogMask(uint32_t mask) {
  uint32_t old = g_log_bits;
  g_log_bits = mask;
  return old;
}

uint32_t DNBLogGetLogMask() { return g_log_bits; }

void DNBLogSetLogCallback(DNBCallbackLog callback, void *baton) {
  g_log_callback = callback;
  g_log_baton = baton;
}

DNBCallbackLog DNBLogGetLogCallback() { return g_log_callback; }

bool DNBLogEnabled() { return g_log_callback != NULL; }

bool DNBLogEnabledForAny(uint32_t mask) {
  if (g_log_callback)
    return (g_log_bits & mask) != 0;
  return false;
}
static inline void _DNBLogVAPrintf(uint32_t flags, const char *format,
                                   va_list args) {
  static std::recursive_mutex g_LogThreadedMutex;
  std::lock_guard<std::recursive_mutex> guard(g_LogThreadedMutex);

  if (g_log_callback)
    g_log_callback(g_log_baton, flags, format, args);
}

void _DNBLog(uint32_t flags, const char *format, ...) {
  va_list args;
  va_start(args, format);
  _DNBLogVAPrintf(flags, format, args);
  va_end(args);
}

// Print debug strings if and only if the global g_debug is set to
// a non-zero value.
void _DNBLogDebug(const char *format, ...) {
  if (DNBLogEnabled() && g_debug) {
    va_list args;
    va_start(args, format);
    _DNBLogVAPrintf(DNBLOG_FLAG_DEBUG, format, args);
    va_end(args);
  }
}

// Print debug strings if and only if the global g_debug is set to
// a non-zero value.
void _DNBLogDebugVerbose(const char *format, ...) {
  if (DNBLogEnabled() && g_debug && g_verbose) {
    va_list args;
    va_start(args, format);
    _DNBLogVAPrintf(DNBLOG_FLAG_DEBUG | DNBLOG_FLAG_VERBOSE, format, args);
    va_end(args);
  }
}

static uint32_t g_message_id = 0;

// Prefix the formatted log string with process and thread IDs and
// suffix it with a newline.
void _DNBLogThreaded(const char *format, ...) {
  if (DNBLogEnabled()) {
    // PTHREAD_MUTEX_LOCKER(locker, GetLogThreadedMutex());

    char *arg_msg = NULL;
    va_list args;
    va_start(args, format);
    ::vasprintf(&arg_msg, format, args);
    va_end(args);

    if (arg_msg != NULL) {
      static struct timeval g_timeval = {0, 0};
      static struct timeval tv;
      static struct timeval delta;
      gettimeofday(&tv, NULL);
      if (g_timeval.tv_sec == 0) {
        delta.tv_sec = 0;
        delta.tv_usec = 0;
      } else {
        timersub(&tv, &g_timeval, &delta);
      }
      g_timeval = tv;

      // Calling "mach_port_deallocate()" bumps the reference count on the
      // thread
      // port, so we need to deallocate it. mach_task_self() doesn't bump the
      // ref
      // count.
      thread_port_t thread_self = mach_thread_self();

      _DNBLog(DNBLOG_FLAG_THREADED, "%u +%lu.%06u sec [%4.4x/%4.4x]: %s",
              ++g_message_id, delta.tv_sec, delta.tv_usec, getpid(),
              thread_self, arg_msg);

      mach_port_deallocate(mach_task_self(), thread_self);
      free(arg_msg);
    }
  }
}

// Prefix the formatted log string with process and thread IDs and
// suffix it with a newline.
void _DNBLogThreadedIf(uint32_t log_bit, const char *format, ...) {
  if (DNBLogEnabled() && (log_bit & g_log_bits) == log_bit) {
    // PTHREAD_MUTEX_LOCKER(locker, GetLogThreadedMutex());

    char *arg_msg = NULL;
    va_list args;
    va_start(args, format);
    ::vasprintf(&arg_msg, format, args);
    va_end(args);

    if (arg_msg != NULL) {
      static struct timeval g_timeval = {0, 0};
      static struct timeval tv;
      static struct timeval delta;
      gettimeofday(&tv, NULL);
      if (g_timeval.tv_sec == 0) {
        delta.tv_sec = 0;
        delta.tv_usec = 0;
      } else {
        timersub(&tv, &g_timeval, &delta);
      }
      g_timeval = tv;

      // Calling "mach_port_deallocate()" bumps the reference count on the
      // thread
      // port, so we need to deallocate it. mach_task_self() doesn't bump the
      // ref
      // count.
      thread_port_t thread_self = mach_thread_self();

      _DNBLog(DNBLOG_FLAG_THREADED, "%u +%lu.%06u sec [%4.4x/%4.4x]: %s",
              ++g_message_id, delta.tv_sec, delta.tv_usec, getpid(),
              thread_self, arg_msg);

      mach_port_deallocate(mach_task_self(), thread_self);

      free(arg_msg);
    }
  }
}

// Printing of errors that are not fatal.
void _DNBLogError(const char *format, ...) {
  if (DNBLogEnabled()) {
    char *arg_msg = NULL;
    va_list args;
    va_start(args, format);
    ::vasprintf(&arg_msg, format, args);
    va_end(args);

    if (arg_msg != NULL) {
      _DNBLog(DNBLOG_FLAG_ERROR, "error: %s", arg_msg);
      free(arg_msg);
    }
  }
}

// Printing of errors that ARE fatal. Exit with ERR exit code
// immediately.
void _DNBLogFatalError(int err, const char *format, ...) {
  if (DNBLogEnabled()) {
    char *arg_msg = NULL;
    va_list args;
    va_start(args, format);
    ::vasprintf(&arg_msg, format, args);
    va_end(args);

    if (arg_msg != NULL) {
      _DNBLog(DNBLOG_FLAG_ERROR | DNBLOG_FLAG_FATAL, "error: %s", arg_msg);
      free(arg_msg);
    }
    ::exit(err);
  }
}

// Printing of warnings that are not fatal only if verbose mode is
// enabled.
void _DNBLogVerbose(const char *format, ...) {
  if (DNBLogEnabled() && g_verbose) {
    va_list args;
    va_start(args, format);
    _DNBLogVAPrintf(DNBLOG_FLAG_VERBOSE, format, args);
    va_end(args);
  }
}

// Printing of warnings that are not fatal only if verbose mode is
// enabled.
void _DNBLogWarningVerbose(const char *format, ...) {
  if (DNBLogEnabled() && g_verbose) {
    char *arg_msg = NULL;
    va_list args;
    va_start(args, format);
    ::vasprintf(&arg_msg, format, args);
    va_end(args);

    if (arg_msg != NULL) {
      _DNBLog(DNBLOG_FLAG_WARNING | DNBLOG_FLAG_VERBOSE, "warning: %s",
              arg_msg);
      free(arg_msg);
    }
  }
}
// Printing of warnings that are not fatal.
void _DNBLogWarning(const char *format, ...) {
  if (DNBLogEnabled()) {
    char *arg_msg = NULL;
    va_list args;
    va_start(args, format);
    ::vasprintf(&arg_msg, format, args);
    va_end(args);

    if (arg_msg != NULL) {
      _DNBLog(DNBLOG_FLAG_WARNING, "warning: %s", arg_msg);
      free(arg_msg);
    }
  }
}

#endif
