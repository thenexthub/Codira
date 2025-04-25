//===--- DiagnosticsSema.h - Diagnostic Definitions -------------*- C++ -*-===//
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
/// This file defines diagnostics for semantic analysis.
//
//===----------------------------------------------------------------------===//

#ifndef SWIFT_DIAGNOSTICSSEMA_H
#define SWIFT_DIAGNOSTICSSEMA_H

#include "language/AST/DiagnosticsCommon.h"

namespace language {
  namespace diag {

    /// Describes the kind of requirement in a protocol.
    enum class RequirementKind : uint8_t {
      Constructor,
      Func,
      Var,
      Subscript
    };

  // Declare common diagnostics objects with their appropriate types.
#define DIAG(KIND, ID, Group, Options, Text, Signature)                    \
      extern detail::DiagWithArguments<void Signature>::type ID;
#define FIXIT(ID, Text, Signature)                                         \
      extern detail::StructuredFixItWithArguments<void Signature>::type ID;
#include "DiagnosticsSema.def"
  }
}

#endif
