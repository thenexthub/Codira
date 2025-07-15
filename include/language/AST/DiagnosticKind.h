//===-- AST/DiagnosticKind.h ------------------------------------*- C++ -*-===//
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

#ifndef LANGUAGE_AST_DIAGNOSTIC_KIND_H
#define LANGUAGE_AST_DIAGNOSTIC_KIND_H

/// This header is included in a bridging header. Be *very* careful with what
/// you include here! See include caveats in `ASTBridging.h`.
#include "language/Basic/LanguageBridging.h"
#include <stdint.h>

namespace language {

/// Describes the kind of diagnostic.
enum class ENUM_EXTENSIBILITY_ATTR(open) DiagnosticKind : uint8_t {
  Error LANGUAGE_NAME("error"),
  Warning LANGUAGE_NAME("warning"),
  Remark LANGUAGE_NAME("remark"),
  Note LANGUAGE_NAME("note")
};

} // namespace language

#endif // LANGUAGE_AST_DIAGNOSTIC_KIND_H
