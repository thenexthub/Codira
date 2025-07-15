//===--- DiagnosticHelper.h - Diagnostic Helper -----------------*- C++ -*-===//
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
// This file exposes helper class to emit diagnostics from language-frontend.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_FRONTEND_DIAGNOSTIC_HELPER_H
#define LANGUAGE_FRONTEND_DIAGNOSTIC_HELPER_H

#include "language/Basic/Toolchain.h"
#include "toolchain/Support/raw_ostream.h"

namespace language {
class CompilerInstance;
class CompilerInvocation;

class DiagnosticHelper {
private:
  class Implementation;
  Implementation &Impl;

public:
  /// Create a DiagnosticHelper class to emit diagnostics from frontend actions.
  /// OS is the stream to print diagnostics. useQuasiPID determines if using
  /// real PID when priting parseable output.
  static DiagnosticHelper create(CompilerInstance &instance,
                                 const CompilerInvocation &invocation,
                                 ArrayRef<const char *> args,
                                 toolchain::raw_pwrite_stream &OS = toolchain::errs(),
                                 bool useQuasiPID = false);

  /// Begin emitting the message, specifically the parseable output message.
  void beginMessage();

  /// End emitting all diagnostics. This has to be called if beginMessage() is
  /// called.
  void endMessage(int retCode);

  /// Set if printing output should be suppressed.
  void setSuppressOutput(bool suppressOutput);

  /// Helper function to emit fatal error.
  void diagnoseFatalError(const char *reason, bool shouldCrash);

  DiagnosticHelper(const DiagnosticHelper &) = delete;
  DiagnosticHelper(DiagnosticHelper &&) = delete;
  DiagnosticHelper &operator=(const DiagnosticHelper &) = delete;
  DiagnosticHelper &operator=(DiagnosticHelper &&) = delete;
  ~DiagnosticHelper();

private:
  DiagnosticHelper(CompilerInstance &instance,
                   const CompilerInvocation &invocation,
                   ArrayRef<const char *> args, toolchain::raw_pwrite_stream &OS,
                   bool useQuasiPID);
};

} // namespace language

#endif
