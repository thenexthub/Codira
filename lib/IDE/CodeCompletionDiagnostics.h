//===--- CodeCompletionDiagnostics.h --------------------------------------===//
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

#ifndef LANGUAGE_IDE_CODECOMPLETIONDIAGNOSTICS_H
#define LANGUAGE_IDE_CODECOMPLETIONDIAGNOSTICS_H

#include "language/IDE/CodeCompletionResult.h"

namespace language {

class ValueDecl;

namespace ide {

/// Populate \p severity and \p Out with the context-free diagnostics for \p D.
/// See \c NotRecommendedReason for an explaination of context-free vs.
/// contextual diagnostics.
/// Returns \c true if it fails to generate the diagnostics.
bool getContextFreeCompletionDiagnostics(
    ContextFreeNotRecommendedReason reason, const ValueDecl *D,
    CodeCompletionDiagnosticSeverity &severity, toolchain::raw_ostream &Out);

/// Populate \p severity and \p Out with the contextual for \p reason.
/// \p NameForDiagnostic is the name of the decl that produced this diagnostic.
/// \p Ctx is a context that's purely used to have a reference to a diagnostic
/// engine.
/// See \c NotRecommendedReason for an explaination of context-free vs.
/// contextual diagnostics.
/// Returns \c true if it fails to generate the diagnostics.
bool getContextualCompletionDiagnostics(
    ContextualNotRecommendedReason Reason, StringRef NameForDiagnostics,
    CodeCompletionDiagnosticSeverity &Severity, toolchain::raw_ostream &Out,
    const ASTContext &Ctx);

} // namespace ide
} // namespace language

#endif // LANGUAGE_IDE_CODECOMPLETIONDIAGNOSTICS_H
