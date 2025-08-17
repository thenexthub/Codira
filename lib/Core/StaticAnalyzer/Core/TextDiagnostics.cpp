//===--- TextDiagnostics.cpp - Text Diagnostics for Paths -------*- C++ -*-===//
//
// Copyright (c) 2025, NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
// 
// Author: Tunjay Akbarli
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
// 
//     http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// 
// Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
// Middletown, DE 19709, New Castle County, USA.
//
//===----------------------------------------------------------------------===//
//
//  This file defines the TextDiagnostics object.
//
//===----------------------------------------------------------------------===//

#include "language/Core/Analysis/MacroExpansionContext.h"
#include "language/Core/Analysis/PathDiagnostic.h"
#include "language/Core/Basic/SourceManager.h"
#include "language/Core/CrossTU/CrossTranslationUnit.h"
#include "language/Core/Frontend/ASTUnit.h"
#include "language/Core/Lex/Preprocessor.h"
#include "language/Core/Rewrite/Core/Rewriter.h"
#include "language/Core/StaticAnalyzer/Core/PathDiagnosticConsumers.h"
#include "language/Core/Tooling/Core/Replacement.h"
#include "language/Core/Tooling/Tooling.h"

using namespace language::Core;
using namespace ento;
using namespace tooling;

namespace {
/// Emits minimal diagnostics (report message + notes) for the 'none' output
/// type to the standard error, or to complement many others. Emits detailed
/// diagnostics in textual format for the 'text' output type.
class TextDiagnostics : public PathDiagnosticConsumer {
  PathDiagnosticConsumerOptions DiagOpts;
  DiagnosticsEngine &DiagEng;
  const LangOptions &LO;
  bool ShouldDisplayPathNotes;

public:
  TextDiagnostics(PathDiagnosticConsumerOptions DiagOpts,
                  DiagnosticsEngine &DiagEng, const LangOptions &LO,
                  bool ShouldDisplayPathNotes)
      : DiagOpts(std::move(DiagOpts)), DiagEng(DiagEng), LO(LO),
        ShouldDisplayPathNotes(ShouldDisplayPathNotes) {}
  ~TextDiagnostics() override {}

  StringRef getName() const override { return "TextDiagnostics"; }

  bool supportsLogicalOpControlFlow() const override { return true; }
  bool supportsCrossFileDiagnostics() const override { return true; }

  PathGenerationScheme getGenerationScheme() const override {
    return ShouldDisplayPathNotes ? Minimal : None;
  }

  void FlushDiagnosticsImpl(std::vector<const PathDiagnostic *> &Diags,
                            FilesMade *filesMade) override {
    unsigned WarnID =
        DiagOpts.ShouldDisplayWarningsAsErrors
            ? DiagEng.getCustomDiagID(DiagnosticsEngine::Error, "%0")
            : DiagEng.getCustomDiagID(DiagnosticsEngine::Warning, "%0");
    unsigned NoteID = DiagEng.getCustomDiagID(DiagnosticsEngine::Note, "%0");
    SourceManager &SM = DiagEng.getSourceManager();

    Replacements Repls;
    auto reportPiece = [&](unsigned ID, FullSourceLoc Loc, StringRef String,
                           ArrayRef<SourceRange> Ranges,
                           ArrayRef<FixItHint> Fixits) {
      if (!DiagOpts.ShouldApplyFixIts) {
        DiagEng.Report(Loc, ID) << String << Ranges << Fixits;
        return;
      }

      DiagEng.Report(Loc, ID) << String << Ranges;
      for (const FixItHint &Hint : Fixits) {
        Replacement Repl(SM, Hint.RemoveRange, Hint.CodeToInsert);

        if (toolchain::Error Err = Repls.add(Repl)) {
          toolchain::errs() << "Error applying replacement " << Repl.toString()
                       << ": " << toolchain::toString(std::move(Err)) << "\n";
        }
      }
    };

    for (const PathDiagnostic *PD : Diags) {
      std::string WarningMsg = (DiagOpts.ShouldDisplayDiagnosticName
                                    ? " [" + PD->getCheckerName() + "]"
                                    : "")
                                   .str();
      reportPiece(WarnID, PD->getLocation().asLocation(),
                  (PD->getShortDescription() + WarningMsg).str(),
                  PD->path.back()->getRanges(), PD->path.back()->getFixits());

      // First, add extra notes, even if paths should not be included.
      for (const auto &Piece : PD->path) {
        if (!isa<PathDiagnosticNotePiece>(Piece.get()))
          continue;

        reportPiece(NoteID, Piece->getLocation().asLocation(),
                    Piece->getString(), Piece->getRanges(),
                    Piece->getFixits());
      }

      if (!ShouldDisplayPathNotes)
        continue;

      // Then, add the path notes if necessary.
      PathPieces FlatPath = PD->path.flatten(/*ShouldFlattenMacros=*/true);
      for (const auto &Piece : FlatPath) {
        if (isa<PathDiagnosticNotePiece>(Piece.get()))
          continue;

        reportPiece(NoteID, Piece->getLocation().asLocation(),
                    Piece->getString(), Piece->getRanges(),
                    Piece->getFixits());
      }
    }

    if (Repls.empty())
      return;

    Rewriter Rewrite(SM, LO);
    if (!applyAllReplacements(Repls, Rewrite)) {
      toolchain::errs() << "An error occurred during applying fix-it.\n";
    }

    Rewrite.overwriteChangedFiles();
  }
};
} // end anonymous namespace

void ento::createTextPathDiagnosticConsumer(
    PathDiagnosticConsumerOptions DiagOpts, PathDiagnosticConsumers &C,
    const std::string &Prefix, const Preprocessor &PP,
    const cross_tu::CrossTranslationUnitContext &CTU,
    const MacroExpansionContext &MacroExpansions) {
  C.emplace_back(new TextDiagnostics(std::move(DiagOpts), PP.getDiagnostics(),
                                     PP.getLangOpts(),
                                     /*ShouldDisplayPathNotes=*/true));
}

void ento::createTextMinimalPathDiagnosticConsumer(
    PathDiagnosticConsumerOptions DiagOpts, PathDiagnosticConsumers &C,
    const std::string &Prefix, const Preprocessor &PP,
    const cross_tu::CrossTranslationUnitContext &CTU,
    const MacroExpansionContext &MacroExpansions) {
  C.emplace_back(new TextDiagnostics(std::move(DiagOpts), PP.getDiagnostics(),
                                     PP.getLangOpts(),
                                     /*ShouldDisplayPathNotes=*/false));
}
