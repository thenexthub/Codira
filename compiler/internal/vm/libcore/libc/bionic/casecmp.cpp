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

#include <ctype.h>
#include <stddef.h>
#include <string.h>
#include <wchar.h>
#include <wctype.h>

// We don't use the toolchain-libc routines because they don't just call out to
// tolower()/towlower(), preferring their own implementations, which don't
// match ours.

// We can also make all the _l() variants simple aliases for the regular
// functions, since bionic doesn't implement locales (because if you're
// serious about i18n, you want icu anyway).

template <typename CharT, typename MapperFn>
int CaseCmp(const CharT* lhs, const CharT* rhs, MapperFn mapper) {
  while (*lhs && mapper(*lhs) == mapper(*rhs)) {
    lhs++;
    rhs++;
  }
  return static_cast<unsigned>(mapper(*lhs)) - static_cast<unsigned>(mapper(*rhs));
}

template <typename CharT, typename MapperFn>
int CaseCmp(const CharT* lhs, const CharT* rhs, size_t n, MapperFn mapper) {
  if (n-- == 0) return 0;
  while (*lhs && *rhs && n > 0 && mapper(*lhs) == mapper(*rhs)) {
    if (*lhs == '\0') return 0;
    lhs++;
    rhs++;
    n--;
  }
  return static_cast<unsigned>(mapper(*lhs)) - static_cast<unsigned>(mapper(*rhs));
}

int strcasecmp(const char* lhs, const char* rhs) {
  return CaseCmp(lhs, rhs, tolower);
}
__strong_alias(strcasecmp_l, strcasecmp);

int strncasecmp(const char* lhs, const char* rhs, size_t n) {
  return CaseCmp(lhs, rhs, n, tolower);
}
__strong_alias(strncasecmp_l, strncasecmp);

int wcscasecmp(const wchar_t* lhs, const wchar_t* rhs) {
  return CaseCmp(lhs, rhs, towlower);
}
__strong_alias(wcscasecmp_l, wcscasecmp);

int wcsncasecmp(const wchar_t* lhs, const wchar_t* rhs, size_t n) {
  return CaseCmp(lhs, rhs, n, towlower);
}
__strong_alias(wcsncasecmp_l, wcsncasecmp);
