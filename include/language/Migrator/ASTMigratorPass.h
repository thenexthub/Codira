//===--- ASTMigratorPass.h --------------------------------------*- C++ -*-===//
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
// A base class for a syntactic migrator pass that uses the temporary
// swift::migrator::EditorAdapter infrastructure.
//
//===----------------------------------------------------------------------===//

#ifndef SWIFT_MIGRATOR_ASTMIGRATORPASS_H
#define SWIFT_MIGRATOR_ASTMIGRATORPASS_H

#include "language/AST/ASTContext.h"
#include "language/AST/SourceFile.h"
#include "language/Migrator/EditorAdapter.h"

namespace language {
class SourceManager;
struct MigratorOptions;
class DiagnosticEngine;

namespace migrator {
class ASTMigratorPass {
protected:
  EditorAdapter &Editor;
  SourceFile *SF;
  const MigratorOptions &Opts;
  const StringRef Filename;
  const unsigned BufferID;
  SourceManager &SM;
  DiagnosticEngine &Diags;

  ASTMigratorPass(EditorAdapter &Editor, SourceFile *SF,
                  const MigratorOptions &Opts)
    : Editor(Editor), SF(SF), Opts(Opts), Filename(SF->getFilename()),
      BufferID(SF->getBufferID()),
      SM(SF->getASTContext().SourceMgr), Diags(SF->getASTContext().Diags) {}
};

/// Run a general pass to migrate code based on SDK differences in the previous
/// release.
void runAPIDiffMigratorPass(EditorAdapter &Editor,
                            SourceFile *SF,
                            const MigratorOptions &Opts);

/// Run a pass to fix up the new type of 'try?' in Swift 4
void runOptionalTryMigratorPass(EditorAdapter &Editor,
                                SourceFile *SF,
                                const MigratorOptions &Opts);
  
  
} // end namespace migrator
} // end namespace language

#endif // SWIFT_MIGRATOR_ASTMIGRATORPASS_H
