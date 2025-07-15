//===--- Concurrency.h ----------------------------------------------------===//
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

#ifndef LANGUAGE_AST_CONCURRENCY_H
#define LANGUAGE_AST_CONCURRENCY_H

#include "language/AST/DiagnosticEngine.h"

#include <optional>

namespace language {

/// Find the imported module that treats the given nominal type as "preconcurrency", or return `nullptr`
/// if there is no such module.
ModuleDecl *moduleImportForPreconcurrency(NominalTypeDecl *nominal,
                                          const DeclContext *fromDC);

/// Determinate the appropriate diagnostic behavior to used when emitting
/// concurrency diagnostics when referencing the given nominal type from the
/// given declaration context.
std::optional<DiagnosticBehavior>
getConcurrencyDiagnosticBehaviorLimit(NominalTypeDecl *nominal,
                                      const DeclContext *fromDC,
                                      bool ignoreExplicitConformance = false);

/// Determine whether the given nominal type has an explicit Sendable
/// conformance (regardless of its availability).
bool hasExplicitSendableConformance(NominalTypeDecl *nominal,
                                    bool applyModuleDefault = true);

} // namespace language

#endif
