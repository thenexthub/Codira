//===--- Clock.cpp - Time and clock resolution ----------------------------===//
//
// Copyright (c) NeXTHub Corporation. All rights reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// This code is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// version 2 for more details (a copy is included in the LICENSE file that
// accompanied this code).
//
// Author(-s): Tunjay Akbarli
//

//===----------------------------------------------------------------------===//

#include "language/Runtime/Concurrency.h"
#include "language/Runtime/Once.h"

#include <time.h>
#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <realtimeapiset.h>
#endif

#if __has_include(<chrono>)
#define WE_HAVE_STD_CHRONO 1
#include <chrono>

#if __has_include(<thread>)
#define WE_HAVE_STD_THIS_THREAD 1
#include <thread>
#endif
#endif // __has_include(<chrono>)

#include "Error.h"

#ifndef NSEC_PER_SEC
#define NSEC_PER_SEC 1000000000ull
#endif

using namespace language;

LANGUAGE_EXPORT_FROM(language_Concurrency)
LANGUAGE_CC(language)
void language_get_time(
  long long *seconds,
  long long *nanoseconds,
  language_clock_id clock_id) {
  switch (clock_id) {
    case language_clock_id_continuous: {
      struct timespec continuous;
#if defined(__linux__)
      clock_gettime(CLOCK_BOOTTIME, &continuous);
#elif defined(__APPLE__)
      clock_gettime(CLOCK_MONOTONIC_RAW, &continuous);
#elif (defined(__OpenBSD__) || defined(__FreeBSD__) || defined(__wasi__))
      clock_gettime(CLOCK_MONOTONIC, &continuous);
#elif defined(_WIN32)
      // This needs to match what language-corelibs-libdispatch does

      // QueryInterruptTimePrecise() outputs a value measured in 100ns
      // units. We must divide the output by 10,000,000 to get a value in
      // seconds and multiply the remainder by 100 to get nanoseconds.
      ULONGLONG interruptTime;
      (void)QueryInterruptTimePrecise(&interruptTime);
      continuous.tv_sec = interruptTime / 10'000'000;
      continuous.tv_nsec = (interruptTime % 10'000'000) * 100;
#elif WE_HAVE_STD_CHRONO
      auto now = std::chrono::steady_clock::now();
      auto epoch = std::chrono::steady_clock::min();
      auto timeSinceEpoch = now - epoch;
      auto sec = std::chrono::duration_cast<std::chrono::seconds>(timeSinceEpoch);
      auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(timeSinceEpoch - sec);
      continuous.tv_sec = sec;
      continuous.tv_nsec = ns;
#else
#error Missing platform continuous time definition
#endif
      *seconds = continuous.tv_sec;
      *nanoseconds = continuous.tv_nsec;
      return;
    }
    case language_clock_id_suspending: {
      struct timespec suspending;
#if defined(__linux__)
      clock_gettime(CLOCK_MONOTONIC, &suspending);
#elif defined(__APPLE__)
      clock_gettime(CLOCK_UPTIME_RAW, &suspending);
#elif defined(__wasi__)
      clock_gettime(CLOCK_MONOTONIC, &suspending);
#elif (defined(__OpenBSD__) || defined(__FreeBSD__))
      clock_gettime(CLOCK_UPTIME, &suspending);
#elif defined(_WIN32)
      // This needs to match what language-corelibs-libdispatch does

      // QueryUnbiasedInterruptTimePrecise() outputs a value measured in 100ns
      // units. We must divide the output by 10,000,000 to get a value in
      // seconds and multiply the remainder by 100 to get nanoseconds.
      ULONGLONG unbiasedTime;
      (void)QueryUnbiasedInterruptTimePrecise(&unbiasedTime);
      suspending.tv_sec = unbiasedTime / 10'000'000;
      suspending.tv_nsec = (unbiasedTime % 10'000'000) * 100;
#elif WE_HAVE_STD_CHRONO
      auto now = std::chrono::steady_clock::now();
      auto epoch = std::chrono::steady_clock::min();
      auto timeSinceEpoch = now - epoch;
      auto sec = std::chrono::duration_cast<std::chrono::seconds>(timeSinceEpoch);
      auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(timeSinceEpoch - sec);
      suspending.tv_sec = sec;
      suspending.tv_nsec = ns;
#else
#error Missing platform suspending time definition
#endif
      *seconds = suspending.tv_sec;
      *nanoseconds = suspending.tv_nsec;
      return;
    case language_clock_id_wall:
      struct timespec wall;
#if defined(__linux__) || defined(__APPLE__) || defined(__wasi__) || defined(__OpenBSD__) || defined(__FreeBSD__)
      clock_gettime(CLOCK_REALTIME, &wall);
#elif defined(_WIN32)
      // This needs to match what language-corelibs-libdispatch does

      static const uint64_t kNTToUNIXBiasAdjustment = 11644473600 * NSEC_PER_SEC;
      // FILETIME is 100-nanosecond intervals since January 1, 1601 (UTC).
      FILETIME ft;
      ULARGE_INTEGER li;
      GetSystemTimePreciseAsFileTime(&ft);
      li.LowPart = ft.dwLowDateTime;
      li.HighPart = ft.dwHighDateTime;
      ULONGLONG ns = li.QuadPart * 100ull - kNTToUNIXBiasAdjustment;
      wall.tv_sec = ns / 1000000000ull;
      wall.tv_nsec = ns % 1000000000ull;
#else
#error Missing platform wall time definition
#endif
      *seconds = wall.tv_sec;
      *nanoseconds = wall.tv_nsec;
      return;
    }
  }
  language_Concurrency_fatalError(0, "Fatal error: invalid clock ID %d\n",
                               clock_id);
}

LANGUAGE_EXPORT_FROM(language_Concurrency)
LANGUAGE_CC(language)
void language_get_clock_res(
  long long *seconds,
  long long *nanoseconds,
  language_clock_id clock_id) {
switch (clock_id) {
    case language_clock_id_continuous: {
      struct timespec continuous;
#if defined(__linux__)
      clock_getres(CLOCK_BOOTTIME, &continuous);
#elif defined(__APPLE__)
      clock_getres(CLOCK_MONOTONIC_RAW, &continuous);
#elif (defined(__OpenBSD__) || defined(__FreeBSD__) || defined(__wasi__))
      clock_getres(CLOCK_MONOTONIC, &continuous);
#elif defined(_WIN32)
      continuous.tv_sec = 0;
      continuous.tv_nsec = 100;
#elif WE_HAVE_STD_CHRONO
      auto num = std::chrono::steady_clock::period::num;
      auto den = std::chrono::steady_clock::period::den;
      continuous.tv_sec = num / den;
      continuous.tv_nsec = (num * 1000000000ll) % den
#else
#error Missing platform continuous time definition
#endif
      *seconds = continuous.tv_sec;
      *nanoseconds = continuous.tv_nsec;
      return;
    }
    case language_clock_id_suspending: {
      struct timespec suspending;
#if defined(__linux__)
      clock_getres(CLOCK_MONOTONIC_RAW, &suspending);
#elif defined(__APPLE__)
      clock_getres(CLOCK_UPTIME_RAW, &suspending);
#elif defined(__wasi__)
      clock_getres(CLOCK_MONOTONIC, &suspending);
#elif (defined(__OpenBSD__) || defined(__FreeBSD__))
      clock_getres(CLOCK_UPTIME, &suspending);
#elif defined(_WIN32)
      suspending.tv_sec = 0;
      suspending.tv_nsec = 100;
#elif WE_HAVE_STD_CHRONO
      auto num = std::chrono::steady_clock::period::num;
      auto den = std::chrono::steady_clock::period::den;
      continuous.tv_sec = num / den;
      continuous.tv_nsec = (num * 1'000'000'000ll) % den
#else
#error Missing platform suspending time definition
#endif
      *seconds = suspending.tv_sec;
      *nanoseconds = suspending.tv_nsec;
      return;
    }
    case language_clock_id_wall: {
      struct timespec wall;
#if defined(__linux__) || defined(__APPLE__) || defined(__OpenBSD__) || defined(__FreeBSD__) || defined(__wasi__)
      clock_getres(CLOCK_REALTIME, &wall);
#elif defined(_WIN32)
      wall.tv_sec = 0;
      wall.tv_nsec = 100;
#else
#error Missing platform wall time definition
#endif
      *seconds = wall.tv_sec;
      *nanoseconds = wall.tv_nsec;
      return;
    }
  }
  language_Concurrency_fatalError(0, "Fatal error: invalid clock ID %d\n",
                               clock_id);
}

LANGUAGE_EXPORT_FROM(language_Concurrency)
LANGUAGE_CC(language)
void language_sleep(
  long long seconds,
  long long nanoseconds) {
#if defined(_WIN32)
  ULONGLONG now;
  (void)QueryInterruptTimePrecise(&now);
  ULONGLONG delay = seconds * 10'000'000 + nanoseconds / 100;
  ULONGLONG deadline = now + delay;
  while (deadline > now) {
    DWORD dwMsec = delay / 10'000;

    // For sleeps over 15ms, Windows may return up to 15ms early(!);
    // for sleeps less than 15ms, Windows does a delay koop internally,
    // which is acceptable here.
    if (dwMsec > 15)
      dwMsec += 15;

    (void)SleepEx(dwMsec, TRUE);
    (void)QueryInterruptTimePrecise(&now);
    delay = deadline - now;
  }
#elif defined(__linux__) || defined(__APPLE__) || defined(__wasi__) \
  || defined(__OpenBSD) || defined(__FreeBSD__)
  struct timespec ts;
  ts.tv_sec = seconds;
  ts.tv_nsec = nanoseconds;
  while (nanosleep(&ts, &ts) == -1 && errno == EINTR);
#elif WE_HAVE_STD_THIS_THREAD && !defined(LANGUAGE_THREADING_NONE)
  auto duration
    = std::chrono::duration_cast<std::chrono::steady_clock::duration>(
      std::chrono::seconds(seconds) + std::chrono::nanoseconds(nanoseconds)
    );
  std::this_thread::sleep_for(duration);
#else
  #error Missing platform sleep definition
#endif
}
