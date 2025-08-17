//===--- ParseAST.cpp - Provide the language::Core::ParseAST method ----------------===//
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
// This file implements the language::Core::ParseAST method.
//
//===----------------------------------------------------------------------===//

#include "language/Core/Parse/ParseAST.h"
#include "language/Core/AST/ASTConsumer.h"
#include "language/Core/AST/ASTContext.h"
#include "language/Core/AST/ExternalASTSource.h"
#include "language/Core/AST/Stmt.h"
#include "language/Core/Parse/Parser.h"
#include "language/Core/Sema/CodeCompleteConsumer.h"
#include "language/Core/Sema/EnterExpressionEvaluationContext.h"
#include "language/Core/Sema/Sema.h"
#include "language/Core/Sema/SemaConsumer.h"
#include "language/Core/Sema/TemplateInstCallback.h"
#include "toolchain/Support/CrashRecoveryContext.h"
#include "toolchain/Support/TimeProfiler.h"
#include <cstdio>
#include <memory>

using namespace language::Core;

namespace {

/// Resets LLVM's pretty stack state so that stack traces are printed correctly
/// when there are nested CrashRecoveryContexts and the inner one recovers from
/// a crash.
class ResetStackCleanup
    : public toolchain::CrashRecoveryContextCleanupBase<ResetStackCleanup,
                                                   const void> {
public:
  ResetStackCleanup(toolchain::CrashRecoveryContext *Context, const void *Top)
      : toolchain::CrashRecoveryContextCleanupBase<ResetStackCleanup, const void>(
            Context, Top) {}
  void recoverResources() override {
    toolchain::RestorePrettyStackState(resource);
  }
};

/// If a crash happens while the parser is active, an entry is printed for it.
class PrettyStackTraceParserEntry : public toolchain::PrettyStackTraceEntry {
  const Parser &P;
public:
  PrettyStackTraceParserEntry(const Parser &p) : P(p) {}
  void print(raw_ostream &OS) const override;
};

/// If a crash happens while the parser is active, print out a line indicating
/// what the current token is.
void PrettyStackTraceParserEntry::print(raw_ostream &OS) const {
  const Token &Tok = P.getCurToken();
  if (Tok.is(tok::eof)) {
    OS << "<eof> parser at end of file\n";
    return;
  }

  if (Tok.getLocation().isInvalid()) {
    OS << "<unknown> parser at unknown location\n";
    return;
  }

  const Preprocessor &PP = P.getPreprocessor();
  Tok.getLocation().print(OS, PP.getSourceManager());
  if (Tok.isAnnotation()) {
    OS << ": at annotation token\n";
  } else {
    // Do the equivalent of PP.getSpelling(Tok) except for the parts that would
    // allocate memory.
    bool Invalid = false;
    const SourceManager &SM = P.getPreprocessor().getSourceManager();
    unsigned Length = Tok.getLength();
    const char *Spelling = SM.getCharacterData(Tok.getLocation(), &Invalid);
    if (Invalid) {
      OS << ": unknown current parser token\n";
      return;
    }
    OS << ": current parser token '" << StringRef(Spelling, Length) << "'\n";
  }
}

}  // namespace

//===----------------------------------------------------------------------===//
// Public interface to the file
//===----------------------------------------------------------------------===//

/// ParseAST - Parse the entire file specified, notifying the ASTConsumer as
/// the file is parsed.  This inserts the parsed decls into the translation unit
/// held by Ctx.
///
void language::Core::ParseAST(Preprocessor &PP, ASTConsumer *Consumer,
                     ASTContext &Ctx, bool PrintStats,
                     TranslationUnitKind TUKind,
                     CodeCompleteConsumer *CompletionConsumer,
                     bool SkipFunctionBodies) {

  std::unique_ptr<Sema> S(
      new Sema(PP, Ctx, *Consumer, TUKind, CompletionConsumer));

  // Recover resources if we crash before exiting this method.
  toolchain::CrashRecoveryContextCleanupRegistrar<Sema> CleanupSema(S.get());

  ParseAST(*S, PrintStats, SkipFunctionBodies);
}

void language::Core::ParseAST(Sema &S, bool PrintStats, bool SkipFunctionBodies) {
  // Collect global stats on Decls/Stmts (until we have a module streamer).
  if (PrintStats) {
    Decl::EnableStatistics();
    Stmt::EnableStatistics();
  }

  // Also turn on collection of stats inside of the Sema object.
  bool OldCollectStats = PrintStats;
  std::swap(OldCollectStats, S.CollectStats);

  // Initialize the template instantiation observer chain.
  // FIXME: See note on "finalize" below.
  initialize(S.TemplateInstCallbacks, S);

  ASTConsumer *Consumer = &S.getASTConsumer();

  std::unique_ptr<Parser> ParseOP(
      new Parser(S.getPreprocessor(), S, SkipFunctionBodies));
  Parser &P = *ParseOP;

  toolchain::CrashRecoveryContextCleanupRegistrar<const void, ResetStackCleanup>
      CleanupPrettyStack(toolchain::SavePrettyStackState());
  PrettyStackTraceParserEntry CrashInfo(P);

  // Recover resources if we crash before exiting this method.
  toolchain::CrashRecoveryContextCleanupRegistrar<Parser>
    CleanupParser(ParseOP.get());

  S.getPreprocessor().EnterMainSourceFile();
  ExternalASTSource *External = S.getASTContext().getExternalSource();
  if (External)
    External->StartTranslationUnit(Consumer);

  // If a PCH through header is specified that does not have an include in
  // the source, or a PCH is being created with #pragma hdrstop with nothing
  // after the pragma, there won't be any tokens or a Lexer.
  bool HaveLexer = S.getPreprocessor().getCurrentLexer();

  if (HaveLexer) {
    toolchain::TimeTraceScope TimeScope("Frontend", [&]() {
      toolchain::TimeTraceMetadata M;
      if (toolchain::isTimeTraceVerbose()) {
        const SourceManager &SM = S.getSourceManager();
        if (const auto *FE = SM.getFileEntryForID(SM.getMainFileID()))
          M.File = FE->tryGetRealPathName();
      }
      return M;
    });
    P.Initialize();
    Parser::DeclGroupPtrTy ADecl;
    Sema::ModuleImportState ImportState;
    EnterExpressionEvaluationContext PotentiallyEvaluated(
        S, Sema::ExpressionEvaluationContext::PotentiallyEvaluated);

    for (bool AtEOF = P.ParseFirstTopLevelDecl(ADecl, ImportState); !AtEOF;
         AtEOF = P.ParseTopLevelDecl(ADecl, ImportState)) {
      // If we got a null return and something *was* parsed, ignore it.  This
      // is due to a top-level semicolon, an action override, or a parse error
      // skipping something.
      if (ADecl && !Consumer->HandleTopLevelDecl(ADecl.get()))
        return;
    }
  }

  // Process any TopLevelDecls generated by #pragma weak.
  for (Decl *D : S.WeakTopLevelDecls())
    Consumer->HandleTopLevelDecl(DeclGroupRef(D));

  Consumer->HandleTranslationUnit(S.getASTContext());

  // Finalize the template instantiation observer chain.
  // FIXME: This (and init.) should be done in the Sema class, but because
  // Sema does not have a reliable "Finalize" function (it has a
  // destructor, but it is not guaranteed to be called ("-disable-free")).
  // So, do the initialization above and do the finalization here:
  finalize(S.TemplateInstCallbacks, S);

  std::swap(OldCollectStats, S.CollectStats);
  if (PrintStats) {
    toolchain::errs() << "\nSTATISTICS:\n";
    if (HaveLexer) P.getActions().PrintStats();
    S.getASTContext().PrintStats();
    Decl::PrintStats();
    Stmt::PrintStats();
    Consumer->PrintStats();
  }
}
