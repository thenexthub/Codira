//===--- LibcShims.cpp ----------------------------------------------------===//
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

#if defined(__APPLE__)
#define _REENTRANT
#include <math.h>
#endif

#if defined(_WIN32) && !defined(__CYGWIN__)
#include <io.h>
#include <stdlib.h>
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

#include <stdio.h>
#include <stdlib.h>

#include "language/shims/LibcShims.h"

#if defined(_WIN32)
static void __attribute__((__constructor__))
_swift_stdlib_configure_console_mode(void) {
  static UINT uiPrevConsoleCP = GetConsoleOutputCP();
  atexit([]() { SetConsoleOutputCP(uiPrevConsoleCP); });
  SetConsoleOutputCP(CP_UTF8);
}
#endif

SWIFT_RUNTIME_STDLIB_INTERNAL
__swift_size_t _swift_stdlib_fwrite_stdout(const void *ptr,
                                           __swift_size_t size,
                                           __swift_size_t nitems) {
  return fwrite(ptr, size, nitems, stdout);
}
