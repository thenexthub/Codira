//===--- ClangDiagnosticConsumer.h - Handle Clang diagnostics ---*- C++ -*-===//
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
#ifndef LANGUAGE_CLANG_DIAGNOSTIC_CONSUMER_H
#define LANGUAGE_CLANG_DIAGNOSTIC_CONSUMER_H

#include "language/ClangImporter/ClangImporter.h"
#include "language/Core/Basic/Diagnostic.h"
#include "language/Core/Basic/IdentifierTable.h"
#include "language/Core/Frontend/TextDiagnosticPrinter.h"
#include "toolchain/Support/MemoryBuffer.h"

namespace language {

class ClangDiagnosticConsumer : public language::Core::TextDiagnosticPrinter {
  struct LoadModuleRAII {
    ClangDiagnosticConsumer *Consumer;
    language::Core::DiagnosticsEngine *Engine;
    bool DiagEngineClearPriorToLookup;

  public:
    LoadModuleRAII(ClangDiagnosticConsumer &consumer,
                   language::Core::DiagnosticsEngine &engine,
                   const language::Core::IdentifierInfo *import)
        : Consumer(&consumer), Engine(&engine) {
      assert(import);
      assert(!Consumer->CurrentImport);
      assert(!Consumer->CurrentImportNotFound);
      Consumer->CurrentImport = import;
      DiagEngineClearPriorToLookup = !engine.hasErrorOccurred();
    }

    LoadModuleRAII(LoadModuleRAII &) = delete;
    LoadModuleRAII &operator=(LoadModuleRAII &) = delete;

    LoadModuleRAII(LoadModuleRAII &&other) {
      *this = std::move(other);
    }
    LoadModuleRAII &operator=(LoadModuleRAII &&other) {
      Consumer = other.Consumer;
      other.Consumer = nullptr;
      return *this;
    }

    ~LoadModuleRAII() {
      if (Consumer) {
        // We must reset Clang's diagnostic engine here since we know that only
        // the module lookup errors have been emitted. While the
        // ClangDiagnosticConsumer takes care of filtering out the diagnostics
        // from the output and from being handled by Codira's DiagnosticEngine,
        // we must ensure to also reset Clang's DiagnosticEngine because its
        // state is queried in later stages of compilation and errors emitted on
        // "module_not_found" should not be counted. Use a soft reset that only
        // clear the errors but not reset the states.
        // FIXME: We must instead allow for module loading in Clang to fail
        // without needing to emit a diagnostic.
        if (Engine && Consumer->CurrentImportNotFound &&
            DiagEngineClearPriorToLookup)
          Engine->Reset(/*soft=*/true);
        Consumer->CurrentImport = nullptr;
        Consumer->CurrentImportNotFound = false;
      }
    }
  };

private:
  friend struct LoadModuleRAII;

  ClangImporter::Implementation &ImporterImpl;

  const language::Core::IdentifierInfo *CurrentImport = nullptr;
  bool CurrentImportNotFound = false;
  SourceLoc DiagLoc;
  const bool DumpToStderr;

public:
  ClangDiagnosticConsumer(ClangImporter::Implementation &impl,
                          language::Core::DiagnosticOptions &clangDiagOptions,
                          bool dumpToStderr);

  LoadModuleRAII handleImport(const language::Core::IdentifierInfo *name,
                              language::Core::DiagnosticsEngine &engine,
                              SourceLoc diagLoc) {
    DiagLoc = diagLoc;
    return LoadModuleRAII(*this, engine, name);
  }

  void HandleDiagnostic(language::Core::DiagnosticsEngine::Level diagLevel,
                        const language::Core::Diagnostic &info) override;
};

} // end namespace language

#endif
