//===-- DNBTimer.h ----------------------------------------------*- C++ -*-===//
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
//  Created by Greg Clayton on 12/13/07.
//
//===----------------------------------------------------------------------===//

#ifndef LLDB_TOOLS_DEBUGSERVER_SOURCE_DNBTIMER_H
#define LLDB_TOOLS_DEBUGSERVER_SOURCE_DNBTIMER_H

#include "DNBDefs.h"
#include <cstdint>
#include <mutex>
#include <optional>
#include <sys/time.h>

class DNBTimer {
public:
  // Constructors and Destructors
  DNBTimer(bool threadSafe) {
    if (threadSafe)
      m_mutex.emplace();
    Reset();
  }

  DNBTimer(const DNBTimer &rhs) {
    // Create a new mutex to make this timer thread safe as well if
    // the timer we are copying is thread safe
    if (rhs.IsThreadSafe())
      m_mutex.emplace();
    m_timeval = rhs.m_timeval;
  }

  DNBTimer &operator=(const DNBTimer &rhs) {
    // Create a new mutex to make this timer thread safe as well if
    // the timer we are copying is thread safe
    if (rhs.IsThreadSafe())
      m_mutex.emplace();
    m_timeval = rhs.m_timeval;
    return *this;
  }

  ~DNBTimer() {}

  bool IsThreadSafe() const { return m_mutex.has_value(); }
  // Reset the time value to now
  void Reset() {
    auto lock = m_mutex ? std::unique_lock<std::recursive_mutex>(*m_mutex)
                        : std::unique_lock<std::recursive_mutex>();
    gettimeofday(&m_timeval, NULL);
  }
  // Get the total microseconds since Jan 1, 1970
  uint64_t TotalMicroSeconds() const {
    std::unique_lock<std::recursive_mutex> lock =
        m_mutex ? std::unique_lock<std::recursive_mutex>(*m_mutex)
                : std::unique_lock<std::recursive_mutex>();
    return (uint64_t)(m_timeval.tv_sec) * 1000000ull +
           (uint64_t)m_timeval.tv_usec;
  }

  void GetTime(uint64_t &sec, uint32_t &usec) const {
    auto lock = m_mutex ? std::unique_lock<std::recursive_mutex>(*m_mutex)
                        : std::unique_lock<std::recursive_mutex>();
    sec = m_timeval.tv_sec;
    usec = m_timeval.tv_usec;
  }
  // Return the number of microseconds elapsed between now and the
  // m_timeval
  uint64_t ElapsedMicroSeconds(bool update) {
    std::unique_lock<std::recursive_mutex> lock =
        m_mutex ? std::unique_lock<std::recursive_mutex>(*m_mutex)
                : std::unique_lock<std::recursive_mutex>();
    struct timeval now;
    gettimeofday(&now, NULL);
    uint64_t now_usec =
        (uint64_t)(now.tv_sec) * 1000000ull + (uint64_t)now.tv_usec;
    uint64_t this_usec =
        (uint64_t)(m_timeval.tv_sec) * 1000000ull + (uint64_t)m_timeval.tv_usec;
    uint64_t elapsed = now_usec - this_usec;
    // Update the timer time value if requeseted
    if (update)
      m_timeval = now;
    return elapsed;
  }

  static uint64_t GetTimeOfDay() {
    struct timeval now;
    gettimeofday(&now, NULL);
    uint64_t now_usec =
        (uint64_t)(now.tv_sec) * 1000000ull + (uint64_t)now.tv_usec;
    return now_usec;
  }

  static void OffsetTimeOfDay(struct timespec *ts,
                              __darwin_time_t sec_offset = 0,
                              long nsec_offset = 0) {
    if (ts == NULL)
      return;
    // Get the current time in a timeval structure
    struct timeval now;
    gettimeofday(&now, NULL);
    // Morph it into a timespec
    TIMEVAL_TO_TIMESPEC(&now, ts);
    // Offset the timespec if requested
    if (sec_offset != 0 || nsec_offset != 0) {
      // Offset the nano seconds
      ts->tv_nsec += nsec_offset;
      // Offset the seconds taking into account a nano-second overflow
      ts->tv_sec = ts->tv_sec + ts->tv_nsec / 1000000000 + sec_offset;
      // Trim the nanoseconds back there was an overflow
      ts->tv_nsec = ts->tv_nsec % 1000000000;
    }
  }
  static bool TimeOfDayLaterThan(struct timespec &ts) {
    struct timespec now;
    OffsetTimeOfDay(&now);
    if (now.tv_sec > ts.tv_sec)
      return true;
    else if (now.tv_sec < ts.tv_sec)
      return false;
    else {
      if (now.tv_nsec > ts.tv_nsec)
        return true;
      else
        return false;
    }
  }

protected:
  // Classes that inherit from DNBTimer can see and modify these
  mutable std::optional<std::recursive_mutex> m_mutex;
  struct timeval m_timeval;
};

#endif // LLDB_TOOLS_DEBUGSERVER_SOURCE_DNBTIMER_H
