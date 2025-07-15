//===--- Assertions.cpp - Assertion macros --------------------------------===//
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
// This file defines implementation details of include/language/Basic/Assertions.h.
//
//===----------------------------------------------------------------------===//

#include "toolchain/ADT/SmallString.h"
#include "toolchain/Support/CommandLine.h"
#include "toolchain/Support/PrettyStackTrace.h"
#include "toolchain/Support/raw_ostream.h"
#include "language/Basic/Assertions.h"
#include <iostream>

toolchain::cl::opt<bool> AssertContinue(
    "assert-continue", toolchain::cl::init(false),
    toolchain::cl::desc("Do not stop on an assertion failure"));

toolchain::cl::opt<bool> AssertHelp(
    "assert-help", toolchain::cl::init(false),
    toolchain::cl::desc("Print help for managing assertions"));

int CONDITIONAL_ASSERT_Global_enable_flag =
#ifdef NDEBUG
  0; // Default to `off` in release builds
#else
  1; // Default to `on` in debug builds
#endif

static void ASSERT_help(toolchain::raw_ostream &out) {
  static int ASSERT_help_shown = 0;
  if (ASSERT_help_shown) {
    return;
  }
  ASSERT_help_shown = 1;

  if (!AssertHelp) {
    out << "(to display assertion configuration options: -Xtoolchain -assert-help)\n";
    return;
  }

  out << "\n";
  out << "Control assertion behavior with one or more of the following options:\n\n";
  out << " -Xtoolchain -assert-continue\n";
  out << "     Continue after any failed assertion\n\n";
}

[[noreturn]]
static inline void _abortWithMessage(toolchain::StringRef message);

void ASSERT_failure(const char *expr, const char *filename, int line, const char *fn) {
  // Find the last component of `filename`
  // Needed on Windows MSVC, which lacks __FILE_NAME__
  // so we have to use __FILE__ instead:
  for (const char *p = filename; *p != '\0'; p++) {
    if ((p[0] == '/' || p[0] == '\\')
       && p[1] != '/' && p[1] != '\\' && p[1] != '\0') {
      filename = p + 1;
    }
  }

  toolchain::SmallString<0> message;
  toolchain::raw_svector_ostream out(message);

  out << "Assertion failed: "
      << "(" << expr << "), "
      << "function " << fn << " at "
      << filename << ":"
      << line << ".\n";

  ASSERT_help(out);

  if (AssertContinue) {
    toolchain::errs() << message;
    toolchain::errs() << "Continuing after failed assertion (-Xtoolchain -assert-continue)\n";
    return;
  }

  _abortWithMessage(message);
}

// This has to be callable in the same way as the macro version,
// so we can't put it inside a namespace.
#undef CONDITIONAL_ASSERT_enabled
int CONDITIONAL_ASSERT_enabled() {
  return (CONDITIONAL_ASSERT_Global_enable_flag != 0);
}

// MARK: ABORT

namespace {
/// Similar to PrettyStackTraceString, but formats multi-line strings for
/// the stack trace.
class PrettyStackTraceMultilineString : public toolchain::PrettyStackTraceEntry {
  toolchain::StringRef Str;

public:
  PrettyStackTraceMultilineString(toolchain::StringRef str) : Str(str) {}
  void print(toolchain::raw_ostream &OS) const override {
    // For each line, add a leading character and indentation to better match
    // the formatting of the stack trace.
    for (auto c : Str.rtrim('\n')) {
      OS << c;
      if (c == '\n')
        OS << "| \t";
    }
    OS << '\n';
  }
};
} // end anonymous namespace

static void _abortWithMessage(toolchain::StringRef message) {
  // Use a pretty stack trace to ensure the message gets picked up the
  // crash reporter.
  PrettyStackTraceMultilineString trace(message);

  // Also dump to stderr in case pretty backtracing is disabled, and to
  // allow the message to be seen while attached with a debugger.
  toolchain::errs() << message << '\n';

  abort();
}

void _ABORT(const char *file, int line, const char *fn,
            toolchain::function_ref<void(toolchain::raw_ostream &)> message) {
  toolchain::SmallString<0> errorStr;
  toolchain::raw_svector_ostream out(errorStr);
  out << "Abort: " << "function " << fn << " at "
      << file << ":" << line << "\n";
  message(out);

  _abortWithMessage(errorStr);
}

void _ABORT(const char *file, int line, const char *fn,
            toolchain::StringRef message) {
  _ABORT(file, line, fn, [&](auto &out) { out << message; });
}
