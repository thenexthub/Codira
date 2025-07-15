//===----------------------------------------------------------------------===//
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

#include "RefactoringActions.h"

using namespace language::refactoring;

/// Get the source file that corresponds to the given buffer.
SourceFile *getContainingFile(ModuleDecl *M, RangeConfig Range) {
  auto &SM = M->getASTContext().SourceMgr;
  // TODO: We should add an ID -> SourceFile mapping.
  return M->getSourceFileContainingLocation(
      SM.getRangeForBuffer(Range.BufferID).getStart());
}

RefactoringAction::RefactoringAction(ModuleDecl *MD, RefactoringOptions &Opts,
                                     SourceEditConsumer &EditConsumer,
                                     DiagnosticConsumer &DiagConsumer)
    : MD(MD), TheFile(getContainingFile(MD, Opts.Range)),
      EditConsumer(EditConsumer), Ctx(MD->getASTContext()),
      SM(MD->getASTContext().SourceMgr), DiagEngine(SM),
      StartLoc(Lexer::getLocForStartOfToken(SM, Opts.Range.getStart(SM))),
      PreferredName(Opts.PreferredName) {
  DiagEngine.addConsumer(DiagConsumer);
}

TokenBasedRefactoringAction::TokenBasedRefactoringAction(
    ModuleDecl *MD, RefactoringOptions &Opts, SourceEditConsumer &EditConsumer,
    DiagnosticConsumer &DiagConsumer)
    : RefactoringAction(MD, Opts, EditConsumer, DiagConsumer) {
  // Resolve the sema token and save it for later use.
  CursorInfo =
      evaluateOrDefault(TheFile->getASTContext().evaluator,
                        CursorInfoRequest{CursorInfoOwner(TheFile, StartLoc)},
                        new ResolvedCursorInfo());
}

RangeBasedRefactoringAction::RangeBasedRefactoringAction(
    ModuleDecl *MD, RefactoringOptions &Opts, SourceEditConsumer &EditConsumer,
    DiagnosticConsumer &DiagConsumer)
    : RefactoringAction(MD, Opts, EditConsumer, DiagConsumer),
      RangeInfo(evaluateOrDefault(
          MD->getASTContext().evaluator,
          RangeInfoRequest(RangeInfoOwner(TheFile, Opts.Range.getStart(SM),
                                          Opts.Range.getEnd(SM))),
          ResolvedRangeInfo())) {}
