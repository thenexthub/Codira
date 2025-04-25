//===--- DiagnosticsDriver.h - Diagnostic Definitions -----------*- C++ -*-===//
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
/// \file
/// This file defines diagnostics for the driver.
/// \note Diagnostics shared between the driver and frontend are defined in
/// \ref DiagnosticsFrontend.h.
//
//===----------------------------------------------------------------------===//

#ifndef SWIFT_DIAGNOSTICSDRIVER_H
#define SWIFT_DIAGNOSTICSDRIVER_H

#include "language/AST/DiagnosticsCommon.h"

namespace language {
  namespace diag {
  // Declare common diagnostics objects with their appropriate types.
#define DIAG(KIND, ID, Group, Options, Text, Signature)                      \
    extern detail::DiagWithArguments<void Signature>::type ID;
#include "DiagnosticsDriver.def"
  }
}

#endif
