/*
 * Copyright (C) 2025 The Android Open Source Project
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

#include <err.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void report_err(bool show_errno, const char* fmt, va_list args) {
  int error = errno;
  fflush(stdout);

  fprintf(stderr, "%s: ", getprogname());
  if (fmt) {
    vfprintf(stderr, fmt, args);
    if (show_errno) fprintf(stderr, ": ");
  }
  if (show_errno) fprintf(stderr, "%s", strerror(error));
  putc('\n', stderr);
  fflush(stderr);
}

void err(int status, const char* _Nullable fmt, ...) {
  va_list va;
  va_start(va, fmt);
  verr(status, fmt, va);
}

void verr(int status, const char* _Nullable fmt, va_list args) {
  report_err(true, fmt, args);
  exit(status);
}

void errx(int status, const char* _Nullable fmt, ...) {
  va_list va;
  va_start(va, fmt);
  verrx(status, fmt, va);
}

void verrx(int status, const char* _Nullable fmt, va_list args) {
  report_err(false, fmt, args);
  exit(status);
}

void warn(const char* _Nullable fmt, ...) {
  va_list va;
  va_start(va, fmt);
  vwarn(fmt, va);
  va_end(va);
}

void vwarn(const char* _Nullable fmt, va_list args) {
  report_err(true, fmt, args);
}

void warnx(const char* _Nullable fmt, ...) {
  va_list va;
  va_start(va, fmt);
  vwarnx(fmt, va);
  va_end(va);
}

void vwarnx(const char* _Nullable fmt, va_list args) {
  report_err(false, fmt, args);
}
