//===--- TracingCommon.h - Common code for runtime/Concurrency -----*- C++ -*-//
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
//
// Support code shared between swiftCore and swift_Concurrency.
//
//===----------------------------------------------------------------------===//

#ifndef SWIFT_TRACING_COMMON_H
#define SWIFT_TRACING_COMMON_H

#if SWIFT_STDLIB_TRACING

#include "language/Basic/Lazy.h"
#include "language/Runtime/Config.h"
#include <os/signpost.h>

extern "C" const char *__progname;

#if SWIFT_USE_OS_TRACE_LAZY_INIT
extern "C" bool _os_trace_lazy_init_completed_4swift(void);
#endif

namespace language {
namespace runtime {
namespace trace {

static inline bool shouldEnableTracing() {
  if (!SWIFT_RUNTIME_WEAK_CHECK(os_signpost_enabled))
    return false;
  return true;
}

#if SWIFT_USE_OS_TRACE_LAZY_INIT
#if __has_include(<sys/codesign.h>)
#include <sys/codesign.h>
#else
// SPI
#define CS_OPS_STATUS 0
#define CS_PLATFORM_BINARY 0x04000000
extern "C" int csops(pid_t, unsigned int, void *, size_t);
#endif

#include <unistd.h>

static inline bool isPlatformBinary() {
  unsigned int flags = 0;
  int error = csops(getpid(), CS_OPS_STATUS, &flags, sizeof(flags));
  if (error)
    return true; // Fail safe if the call fails, assume it's a platform binary.
  return (flags & CS_PLATFORM_BINARY) != 0;
}

static inline bool tracingReady() {
  // For non-platform binaries, consider tracing to always be ready. We can
  // safely initiate setup if it isn't.
  bool platformBinary = SWIFT_LAZY_CONSTANT(isPlatformBinary());
  if (!platformBinary)
    return true;

  // For platform binaries, we may be on the path that sets up tracing, and
  // making tracing calls may deadlock in that case. Wait until something else
  // set up tracing before using it.
  if (!_os_trace_lazy_init_completed_4swift())
    return false;

  return true;
}

#else

static inline bool tracingReady() { return true; }

#endif

} // namespace trace
} // namespace runtime
} // namespace language

#endif

#endif // SWIFT_TRACING_H
