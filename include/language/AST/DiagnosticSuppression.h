//===--- InstrumenterSupport.cpp - Instrumenter Support -------------------===//
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
//  This file implements the supporting functions for writing instrumenters of
//  the Codira AST.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_AST_DIAGNOSTIC_SUPPRESSION_H
#define LANGUAGE_AST_DIAGNOSTIC_SUPPRESSION_H

#include <vector>

namespace language {

class DiagnosticConsumer;
class DiagnosticEngine;

/// RAII class that suppresses diagnostics by temporarily disabling all of
/// the diagnostic consumers.
class DiagnosticSuppression {
  DiagnosticEngine &diags;
  std::vector<DiagnosticConsumer *> consumers;

  DiagnosticSuppression(const DiagnosticSuppression &) = delete;
  DiagnosticSuppression &operator=(const DiagnosticSuppression &) = delete;

public:
  explicit DiagnosticSuppression(DiagnosticEngine &diags);
  ~DiagnosticSuppression();
  static bool isEnabled(const DiagnosticEngine &diags);
};

}
#endif // LANGUAGE_AST_DIAGNOSTIC_SUPPRESSION_H
