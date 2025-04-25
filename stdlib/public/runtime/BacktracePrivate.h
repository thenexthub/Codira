//===--- BacktraceUtils.h - Private backtracing utilities -------*- C++ -*-===//
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
// Private declarations of the Swift runtime's backtracing code.
//
//===----------------------------------------------------------------------===//

#ifndef SWIFT_RUNTIME_BACKTRACE_UTILS_H
#define SWIFT_RUNTIME_BACKTRACE_UTILS_H

#include "language/Runtime/Config.h"
#include "language/Runtime/Backtrace.h"
#include "language/shims/Visibility.h"

#include <inttypes.h>

#ifdef _WIN32
// For DWORD
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

// For wchar_t
#include <cwchar>
#endif

namespace language {
namespace runtime {
namespace backtrace {

#ifdef _WIN32
typedef wchar_t ArgChar;
typedef DWORD ErrorCode;
#else
typedef char ArgChar;
typedef int ErrorCode;
#endif

enum class UnwindAlgorithm {
  Auto = 0,
  Fast = 1,
  Precise = 2
};

enum class OnOffTty {
  Default = -1,
  Off = 0,
  On = 1,
  TTY = 2
};

enum class Preset {
  Auto = -1,
  Friendly = 0,
  Medium = 1,
  Full = 2
};

enum class ThreadsToShow {
  Preset = -1,
  All = 0,
  Crashed = 1
};

enum class RegistersToShow {
  Preset = -1,
  None = 0,
  All = 1,
  Crashed = 2
};

enum class ImagesToShow {
  Preset = -1,
  None = 0,
  All = 1,
  Mentioned = 2
};

enum class SanitizePaths {
  Preset = -1,
  Off = 0,
  On = 1
};

enum class OutputTo {
  Auto = -1,
  Stdout = 0,
  Stderr = 2,
  File = 3
};

enum class Symbolication {
  Off = 0,
  Fast = 1,
  Full = 2,
};

enum class OutputFormat {
  Text = 0,
  JSON = 1
};

struct BacktraceSettings {
  UnwindAlgorithm  algorithm;
  OnOffTty         enabled;
  bool             demangle;
  OnOffTty         interactive;
  OnOffTty         color;
  unsigned         timeout;
  ThreadsToShow    threads;
  RegistersToShow  registers;
  ImagesToShow     images;
  unsigned         limit;
  unsigned         top;
  SanitizePaths    sanitize;
  Preset           preset;
  bool             cache;
  OutputTo         outputTo;
  Symbolication    symbolicate;
  bool             suppressWarnings;
  OutputFormat     format;
  const char      *swiftBacktracePath;
  const char      *outputPath;
};

SWIFT_RUNTIME_STDLIB_INTERNAL BacktraceSettings _swift_backtraceSettings;

inline bool _swift_backtrace_isEnabled() {
  return _swift_backtraceSettings.enabled == OnOffTty::On;
}

SWIFT_RUNTIME_STDLIB_INTERNAL ErrorCode _swift_installCrashHandler();

#ifdef __linux__
SWIFT_RUNTIME_STDLIB_INTERNAL bool _swift_spawnBacktracer(CrashInfo *crashInfo,
                                                          int memserver_fd);
#else
SWIFT_RUNTIME_STDLIB_INTERNAL bool _swift_spawnBacktracer(CrashInfo *crashInfo);
#endif

SWIFT_RUNTIME_STDLIB_INTERNAL void _swift_displayCrashMessage(int signum, const void *pc);

SWIFT_RUNTIME_STDLIB_INTERNAL
void _swift_formatAddress(uintptr_t addr, char buffer[18]);

inline void _swift_formatAddress(const void *ptr, char buffer[18]) {
  _swift_formatAddress(reinterpret_cast<uintptr_t>(ptr), buffer);
}

SWIFT_RUNTIME_STDLIB_INTERNAL
void _swift_formatUnsigned(unsigned u, char buffer[22]);

} // namespace backtrace
} // namespace runtime
} // namespace language

#endif // SWIFT_RUNTIME_BACKTRACE_UTILS_H
