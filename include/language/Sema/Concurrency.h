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
///
/// \file
///
/// This file defines concurrency utility routines from Sema that are used by
/// later parts of the compiler like the pass pipeline.
///
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_SEMA_CONCURRENCY_H
#define LANGUAGE_SEMA_CONCURRENCY_H

#include <optional>

namespace language {

class DeclContext;
class SourceFile;
class NominalTypeDecl;
class VarDecl;

struct DiagnosticBehavior;

/// If any of the imports in this source file was @preconcurrency but there were
/// no diagnostics downgraded or suppressed due to that @preconcurrency, suggest
/// that the attribute be removed.
void diagnoseUnnecessaryPreconcurrencyImports(SourceFile &sf);

/// Diagnose the use of an instance property of non-Sendable type from an
/// nonisolated deinitializer within an actor-isolated type.
///
/// \returns true iff a diagnostic was emitted for this reference.
bool diagnoseNonSendableFromDeinit(
    SourceLoc refLoc, VarDecl *var, DeclContext *dc);

} // namespace language

#endif
