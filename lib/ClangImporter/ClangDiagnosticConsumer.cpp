//===--- ClangDiagnosticConsumer.cpp - Handle Clang diagnostics -----------===//
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

#include "ClangDiagnosticConsumer.h"
#include "ClangSourceBufferImporter.h"
#include "ImporterImpl.h"
#include "language/AST/ASTContext.h"
#include "language/AST/DiagnosticEngine.h"
#include "language/AST/DiagnosticsClangImporter.h"
#include "language/Basic/Assertions.h"
#include "language/Core/AST/ASTContext.h"
#include "language/Core/Frontend/DiagnosticRenderer.h"
#include "language/Core/Frontend/FrontendDiagnostic.h"
#include "language/Core/Lex/LexDiagnostic.h"
#include "toolchain/ADT/STLExtras.h"

using namespace language;
using namespace language::importer;

namespace {
  class ClangDiagRenderer final : public language::Core::DiagnosticNoteRenderer {
    const toolchain::function_ref<void(language::Core::FullSourceLoc,
                                  language::Core::DiagnosticsEngine::Level,
                                  StringRef)> callback;

  public:
    ClangDiagRenderer(const language::Core::LangOptions &langOpts,
                      language::Core::DiagnosticOptions *diagOpts,
                      decltype(callback) fn)
       : DiagnosticNoteRenderer(langOpts, diagOpts),
         callback(fn) {}

  private:
    /// Is this a diagnostic that doesn't do the user any good to show if it
    /// is located in one of Codira's synthetic buffers? If so, returns true to
    /// suppress it.
    static bool shouldSuppressDiagInCodiraBuffers(language::Core::DiagOrStoredDiag info) {
      if (info.isNull())
        return false;

      unsigned ID;
      if (auto *activeDiag = info.dyn_cast<const language::Core::Diagnostic *>())
        ID = activeDiag->getID();
      else
        ID = info.get<const language::Core::StoredDiagnostic *>()->getID();
      return ID == language::Core::diag::note_module_import_here ||
             ID == language::Core::diag::err_module_not_built;
    }

    /// Returns true if \p loc is inside one of Codira's synthetic buffers.
    static bool isInCodiraBuffers(language::Core::FullSourceLoc loc) {
      StringRef bufName = StringRef(loc.getManager().getBufferName(loc));
      return bufName == ClangImporter::Implementation::moduleImportBufferName ||
             bufName == ClangImporter::Implementation::bridgingHeaderBufferName;
    }

    void emitDiagnosticMessage(language::Core::FullSourceLoc Loc,
                               language::Core::PresumedLoc PLoc,
                               language::Core::DiagnosticsEngine::Level Level,
                               StringRef Message,
                               ArrayRef<language::Core::CharSourceRange> Ranges,
                               language::Core::DiagOrStoredDiag Info) override {
      if (isInCodiraBuffers(Loc)) {
        // FIXME: Ideally, we'd report non-suppressed diagnostics on synthetic
        // buffers, printing their names (eg. <language-imported-modules>:...) but
        // this risks printing _excerpts_ of those buffers to stderr too; at
        // present the synthetic buffers are "large blocks of null bytes" which
        // we definitely don't want to print out. So until we have some clever
        // way to print the name but suppress printing excerpts, we just replace
        // the Loc with an invalid one here, which suppresses both.
        Loc = language::Core::FullSourceLoc();
        if (shouldSuppressDiagInCodiraBuffers(Info))
          return;
      }
      callback(Loc, Level, Message);
    }

    void emitDiagnosticLoc(language::Core::FullSourceLoc Loc, language::Core::PresumedLoc PLoc,
                           language::Core::DiagnosticsEngine::Level Level,
                           ArrayRef<language::Core::CharSourceRange> Ranges) override {}

    void emitCodeContext(language::Core::FullSourceLoc Loc,
                         language::Core::DiagnosticsEngine::Level Level,
                         SmallVectorImpl<language::Core::CharSourceRange>& Ranges,
                         ArrayRef<language::Core::FixItHint> Hints) override {}

    void emitNote(language::Core::FullSourceLoc Loc, StringRef Message) override {
      // We get invalid note locations when trying to describe where a module
      // is imported and the actual location is in Codira. We also want to ignore
      // things like "in module X imported from <language-imported-modules>".
      if (Loc.isInvalid() || isInCodiraBuffers(Loc))
        return;
      emitDiagnosticMessage(Loc, {}, language::Core::DiagnosticsEngine::Note, Message,
                            {}, {});
    }
  };
} // end anonymous namespace

ClangDiagnosticConsumer::ClangDiagnosticConsumer(
    ClangImporter::Implementation &impl,
    language::Core::DiagnosticOptions &clangDiagOptions,
    bool dumpToStderr)
  : TextDiagnosticPrinter(toolchain::errs(), &clangDiagOptions),
    ImporterImpl(impl), DumpToStderr(dumpToStderr) {}

void ClangDiagnosticConsumer::HandleDiagnostic(
    language::Core::DiagnosticsEngine::Level clangDiagLevel,
    const language::Core::Diagnostic &clangDiag) {
  // Handle the module-not-found diagnostic specially if it's a top-level module
  // we're looking for.
  if (clangDiag.getID() == language::Core::diag::err_module_not_found &&
      CurrentImport && clangDiag.getArgStdStr(0) == CurrentImport->getName()) {
    CurrentImportNotFound = true;
    return;
  }

  if (clangDiag.getID() == language::Core::diag::err_module_not_built &&
      CurrentImport && clangDiag.getArgStdStr(0) == CurrentImport->getName()) {
    HeaderLoc loc(clangDiag.getLocation(), DiagLoc,
                  &clangDiag.getSourceManager());
    ImporterImpl.diagnose(loc, diag::clang_cannot_build_module,
                          ImporterImpl.CodiraContext.LangOpts.EnableObjCInterop,
                          CurrentImport->getName());
    return;
  }

  // Satisfy the default implementation (bookkeeping).
  if (DumpToStderr)
    TextDiagnosticPrinter::HandleDiagnostic(clangDiagLevel, clangDiag);
  else
    DiagnosticConsumer::HandleDiagnostic(clangDiagLevel, clangDiag);

  // FIXME: Map over source ranges in the diagnostic.
  auto emitDiag = [this](language::Core::FullSourceLoc clangNoteLoc,
                         language::Core::DiagnosticsEngine::Level clangDiagLevel,
                         StringRef message) {
    decltype(diag::error_from_clang) diagKind;
    switch (clangDiagLevel) {
    case language::Core::DiagnosticsEngine::Ignored:
      return;
    case language::Core::DiagnosticsEngine::Note:
      diagKind = diag::note_from_clang;
      break;
    case language::Core::DiagnosticsEngine::Remark:
      diagKind = diag::remark_from_clang;
      break;
    case language::Core::DiagnosticsEngine::Warning:
      diagKind = diag::warning_from_clang;
      break;
    case language::Core::DiagnosticsEngine::Error:
    case language::Core::DiagnosticsEngine::Fatal:
      // FIXME: What happens after a fatal error in the importer?
      diagKind = diag::error_from_clang;
      break;
    }

    HeaderLoc noteLoc(clangNoteLoc, SourceLoc(),
              clangNoteLoc.hasManager() ? &clangNoteLoc.getManager() : nullptr);
    ImporterImpl.diagnose(noteLoc, diagKind, message);
  };

  toolchain::SmallString<128> message;
  clangDiag.FormatDiagnostic(message);

  if (clangDiag.getLocation().isInvalid()) {
    // Diagnostic about the compiler arguments.
    emitDiag(language::Core::FullSourceLoc(), clangDiagLevel, message);

  } else {
    assert(clangDiag.hasSourceManager());
    auto clangCI = ImporterImpl.getClangInstance();
    ClangDiagRenderer renderer(clangCI->getLangOpts(),
                               &clangCI->getDiagnosticOpts(), emitDiag);
    language::Core::FullSourceLoc clangDiagLoc(clangDiag.getLocation(),
                                      clangDiag.getSourceManager());
    renderer.emitDiagnostic(clangDiagLoc, clangDiagLevel, message,
                            clangDiag.getRanges(), clangDiag.getFixItHints(),
                            &clangDiag);
  }
}
