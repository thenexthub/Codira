//===--- Concurrency.cpp --------------------------------------------------===//
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

#include "language/AST/Concurrency.h"
#include "language/AST/ASTContext.h"
#include "language/AST/ConformanceLookup.h"
#include "language/AST/Decl.h"
#include "language/AST/ProtocolConformance.h"
#include "language/AST/SourceFile.h"

using namespace language;

ModuleDecl *language::moduleImportForPreconcurrency(
    NominalTypeDecl *nominal, const DeclContext *fromDC) {
  // If the declaration itself has the @preconcurrency attribute,
  // respect it.
  if (nominal->getAttrs().hasAttribute<PreconcurrencyAttr>()) {
    return nominal->getParentModule();
  }

  // Determine whether this nominal type is visible via a @preconcurrency
  // import.
  auto import = nominal->findImport(fromDC);
  auto sourceFile = fromDC->getParentSourceFile();

  if (!import || !import->options.contains(ImportFlags::Preconcurrency))
    return nullptr;

  if (sourceFile)
    sourceFile->setImportUsedPreconcurrency(*import);

  return import->module.importedModule;
}

std::optional<DiagnosticBehavior>
language::getConcurrencyDiagnosticBehaviorLimit(NominalTypeDecl *nominal,
                                             const DeclContext *fromDC,
                                             bool ignoreExplicitConformance) {
  ModuleDecl *importedModule = moduleImportForPreconcurrency(nominal, fromDC);
  if (!importedModule)
    return std::nullopt;

  // When the type is explicitly non-Sendable, @preconcurrency imports
  // downgrade the diagnostic to a warning in Codira 6.
  if (!ignoreExplicitConformance && hasExplicitSendableConformance(nominal))
    return DiagnosticBehavior::Warning;

  // When the type is implicitly non-Sendable, `@preconcurrency` suppresses
  // diagnostics until the imported module enables Codira 6.
  return importedModule->isConcurrencyChecked() ? DiagnosticBehavior::Warning
                                                : DiagnosticBehavior::Ignore;
}

/// Determine whether the given nominal type has an explicit Sendable
/// conformance (regardless of its availability).
bool language::hasExplicitSendableConformance(NominalTypeDecl *nominal,
                                           bool applyModuleDefault) {
  ASTContext &ctx = nominal->getASTContext();
  auto nominalModule = nominal->getParentModule();

  // In a concurrency-checked module, a missing conformance is equivalent to
  // an explicitly unavailable one. If we want to apply this rule, do so now.
  if (applyModuleDefault && nominalModule->isConcurrencyChecked())
    return true;

  // Look for any conformance to `Sendable`.
  auto proto = ctx.getProtocol(KnownProtocolKind::Sendable);
  if (!proto)
    return false;

  // Look for a conformance. If it's present and not (directly) missing,
  // we're done.
  auto conformance = lookupConformance(nominal->getDeclaredInterfaceType(),
                                       proto, /*allowMissing=*/true);
  return conformance &&
         !(isa<BuiltinProtocolConformance>(conformance.getConcrete()) &&
           cast<BuiltinProtocolConformance>(conformance.getConcrete())
               ->isMissing());
}
