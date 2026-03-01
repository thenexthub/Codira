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

#include <time.h>

#include <errno.h>
#include <stdio.h>
#include <string.h>

char* asctime(const tm* tm) {
  static char buf[128];
  return asctime_r(tm, buf);
}

char* asctime_r(const tm* tm, char* buf) {
  if (tm == nullptr) {
    errno = EINVAL;
    return strcpy(buf, "??? ??? ?? ??:??:?? ????\n");
  }

  auto pick = [](unsigned n, unsigned max, const char* s) {
    return (n < max) ? s + 3*n : "???";
  };
  const char* day = pick(tm->tm_wday, 7, "SunMonTueWedThuFriSat");
  const char* mon = pick(tm->tm_mon, 12, "JanFebMarAprMayJunJulAugSepOctNovDec");

  // This format string is specified by ISO C.
  char tmp_buf[26];
  int n = snprintf(tmp_buf, 26, "%.3s %.3s%3d %.2d:%.2d:%.2d %d\n", day, mon,
                   tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec, 1900 + tm->tm_year);
  if (n > 25) {
    errno = EOVERFLOW;
    return nullptr;
  }
  strcpy(buf, tmp_buf);
  return buf;
}

char* ctime(const time_t* tp) {
  static char buf[128];
  return ctime_r(tp, buf);
}

char* ctime_r(const time_t* tp, char* buf) {
  struct tm tm;
  if (localtime_r(tp, &tm) == nullptr) return nullptr;
  return asctime_r(&tm, buf);
}

double difftime(time_t t1, time_t t0) {
  return t1 - t0;
}

int timespec_get(timespec* ts, int base) {
  return (clock_gettime(base - 1, ts) != -1) ? base : 0;
}

int timespec_getres(timespec* ts, int base) {
  return (clock_getres(base - 1, ts) != -1) ? base : 0;
}
