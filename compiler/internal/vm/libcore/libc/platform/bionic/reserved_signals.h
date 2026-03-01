/*
 * Copyright (C) 2018 The Android Open Source Project
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#pragma once

#include <sys/cdefs.h>

#include <signal.h>

#include "macros.h"

// Realtime signals reserved for internal use:

//   32 (__SIGRTMIN + 0)        POSIX timers
#define BIONIC_SIGNAL_POSIX_TIMERS (__SIGRTMIN + 0)

//   33 (__SIGRTMIN + 1)        libbacktrace
#define BIONIC_SIGNAL_BACKTRACE (__SIGRTMIN + 1)

//   34 (__SIGRTMIN + 2)        libcore
// There's no constant for this because it's hard-coded in the OpenJDK source.
// It's used to implement Java's "close() on a Socket wakes blocked readers and
// writers" semantics.

//   35 (__SIGRTMIN + 3)        debuggerd
#define BIONIC_SIGNAL_DEBUGGER (__SIGRTMIN + 3)

//   36 (__SIGRTMIN + 4)        platform profilers (heapprofd, traced_perf)
#define BIONIC_SIGNAL_PROFILER (__SIGRTMIN + 4)

//   37 (__SIGRTMIN + 5)        coverage (libprofile-extras)
// Used by the clang coverage support to flush coverage data to disk.
#define BIONIC_SIGNAL_FLUSH_COVERAGE (__SIGRTMIN + 5)

//   38 (__SIGRTMIN + 6)        heapprofd ART managed heap dumps
// When used in ART for heap dumps, this is handled without crashing.
// In debuggerd, we crash the process with this signal to indicate to init that
// a process has been terminated by an MTEAERR SEGV. This works because there is
// no other reason a process could have terminated with this signal.
// This is to work around the limitation of that it is not possible to get the
// si_code that terminated a process.
#define BIONIC_SIGNAL_ART_PROFILER (__SIGRTMIN + 6)

//   39 (__SIGRTMIN + 7)        fdtrack
#define BIONIC_SIGNAL_FDTRACK (__SIGRTMIN + 7)

//   40 (__SIGRTMIN + 8)        android_run_on_all_threads (bionic/pthread_internal.cpp)
#define BIONIC_SIGNAL_RUN_ON_ALL_THREADS (__SIGRTMIN + 8)

//   41 (__SIGRTMIN + 9)        re-enable MTE on thread
#define BIONIC_ENABLE_MTE (__SIGRTMIN + 9)

#define __SIGRT_RESERVED 10

static inline __always_inline sigset64_t filter_reserved_signals(sigset64_t sigset, int how) {
  int (*block)(sigset64_t*, int);
  int (*unblock)(sigset64_t*, int);
  switch (how) {
    case SIG_BLOCK:
      [[fallthrough]];
    case SIG_SETMASK:
      block = sigaddset64;
      unblock = sigdelset64;
      break;

    case SIG_UNBLOCK:
      block = sigdelset64;
      unblock = sigaddset64;
      break;
  }

  // The POSIX timer signal must be blocked.
  block(&sigset, BIONIC_SIGNAL_POSIX_TIMERS);

  // Everything else must remain unblocked.
  for (int i = 1; i < __SIGRT_RESERVED; ++i) {
    unblock(&sigset, __SIGRTMIN + i);
  }

  return sigset;
}
