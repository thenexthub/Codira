//===--- TokenAnalyzer.cpp - Analyze Token Streams --------------*- C++ -*-===//
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
///
/// \file
/// This file implements an abstract TokenAnalyzer and associated helper
/// classes. TokenAnalyzer can be extended to generate replacements based on
/// an annotated and pre-processed token stream.
///
//===----------------------------------------------------------------------===//

#include "TokenAnalyzer.h"
#include "AffectedRangeManager.h"
#include "Encoding.h"
#include "FormatToken.h"
#include "FormatTokenLexer.h"
#include "TokenAnnotator.h"
#include "UnwrappedLineParser.h"
#include "language/Core/Basic/Diagnostic.h"
#include "language/Core/Basic/DiagnosticOptions.h"
#include "language/Core/Basic/SourceManager.h"
#include "language/Core/Format/Format.h"
#include "toolchain/ADT/SmallVector.h"
#include "toolchain/Support/Debug.h"

#define DEBUG_TYPE "format-formatter"

namespace language::Core {
namespace format {

// FIXME: Instead of printing the diagnostic we should store it and have a
// better way to return errors through the format APIs.
class FatalDiagnosticConsumer : public DiagnosticConsumer {
public:
  void HandleDiagnostic(DiagnosticsEngine::Level DiagLevel,
                        const Diagnostic &Info) override {
    if (DiagLevel == DiagnosticsEngine::Fatal) {
      Fatal = true;
      toolchain::SmallVector<char, 128> Message;
      Info.FormatDiagnostic(Message);
      toolchain::errs() << Message << "\n";
    }
  }

  bool fatalError() const { return Fatal; }

private:
  bool Fatal = false;
};

std::unique_ptr<Environment>
Environment::make(StringRef Code, StringRef FileName,
                  ArrayRef<tooling::Range> Ranges, unsigned FirstStartColumn,
                  unsigned NextStartColumn, unsigned LastStartColumn) {
  auto Env = std::make_unique<Environment>(Code, FileName, FirstStartColumn,
                                           NextStartColumn, LastStartColumn);
  FatalDiagnosticConsumer Diags;
  Env->SM.getDiagnostics().setClient(&Diags, /*ShouldOwnClient=*/false);
  SourceLocation StartOfFile = Env->SM.getLocForStartOfFile(Env->ID);
  for (const tooling::Range &Range : Ranges) {
    SourceLocation Start = StartOfFile.getLocWithOffset(Range.getOffset());
    SourceLocation End = Start.getLocWithOffset(Range.getLength());
    Env->CharRanges.push_back(CharSourceRange::getCharRange(Start, End));
  }
  // Validate that we can get the buffer data without a fatal error.
  Env->SM.getBufferData(Env->ID);
  if (Diags.fatalError())
    return nullptr;
  return Env;
}

Environment::Environment(StringRef Code, StringRef FileName,
                         unsigned FirstStartColumn, unsigned NextStartColumn,
                         unsigned LastStartColumn)
    : VirtualSM(new SourceManagerForFile(FileName, Code)), SM(VirtualSM->get()),
      ID(VirtualSM->get().getMainFileID()), FirstStartColumn(FirstStartColumn),
      NextStartColumn(NextStartColumn), LastStartColumn(LastStartColumn) {}

TokenAnalyzer::TokenAnalyzer(const Environment &Env, const FormatStyle &Style)
    : Style(Style), LangOpts(getFormattingLangOpts(Style)), Env(Env),
      AffectedRangeMgr(Env.getSourceManager(), Env.getCharRanges()),
      UnwrappedLines(1),
      Encoding(encoding::detectEncoding(
          Env.getSourceManager().getBufferData(Env.getFileID()))) {
  LLVM_DEBUG(
      toolchain::dbgs() << "File encoding: "
                   << (Encoding == encoding::Encoding_UTF8 ? "UTF8" : "unknown")
                   << "\n");
  LLVM_DEBUG(toolchain::dbgs() << "Language: " << getLanguageName(Style.Language)
                          << "\n");
}

std::pair<tooling::Replacements, unsigned>
TokenAnalyzer::process(bool SkipAnnotation) {
  tooling::Replacements Result;
  toolchain::SpecificBumpPtrAllocator<FormatToken> Allocator;
  IdentifierTable IdentTable(LangOpts);
  FormatTokenLexer Lex(Env.getSourceManager(), Env.getFileID(),
                       Env.getFirstStartColumn(), Style, Encoding, Allocator,
                       IdentTable);
  ArrayRef<FormatToken *> Toks(Lex.lex());
  SmallVector<FormatToken *, 10> Tokens(Toks);
  UnwrappedLineParser Parser(Env.getSourceManager(), Style, Lex.getKeywords(),
                             Env.getFirstStartColumn(), Tokens, *this,
                             Allocator, IdentTable);
  Parser.parse();
  assert(UnwrappedLines.back().empty());
  unsigned Penalty = 0;
  for (unsigned Run = 0, RunE = UnwrappedLines.size(); Run + 1 != RunE; ++Run) {
    const auto &Lines = UnwrappedLines[Run];
    LLVM_DEBUG(toolchain::dbgs() << "Run " << Run << "...\n");
    SmallVector<AnnotatedLine *, 16> AnnotatedLines;
    AnnotatedLines.reserve(Lines.size());

    TokenAnnotator Annotator(Style, Lex.getKeywords());
    for (const UnwrappedLine &Line : Lines) {
      AnnotatedLines.push_back(new AnnotatedLine(Line));
      if (!SkipAnnotation)
        Annotator.annotate(*AnnotatedLines.back());
    }

    std::pair<tooling::Replacements, unsigned> RunResult =
        analyze(Annotator, AnnotatedLines, Lex);

    LLVM_DEBUG({
      toolchain::dbgs() << "Replacements for run " << Run << ":\n";
      for (const tooling::Replacement &Fix : RunResult.first)
        toolchain::dbgs() << Fix.toString() << "\n";
    });
    for (AnnotatedLine *Line : AnnotatedLines)
      delete Line;

    Penalty += RunResult.second;
    for (const auto &R : RunResult.first) {
      auto Err = Result.add(R);
      // FIXME: better error handling here. For now, simply return an empty
      // Replacements to indicate failure.
      if (Err) {
        toolchain::errs() << toolchain::toString(std::move(Err)) << "\n";
        return {tooling::Replacements(), 0};
      }
    }
  }
  return {Result, Penalty};
}

void TokenAnalyzer::consumeUnwrappedLine(const UnwrappedLine &TheLine) {
  assert(!UnwrappedLines.empty());
  UnwrappedLines.back().push_back(TheLine);
}

void TokenAnalyzer::finishRun() {
  UnwrappedLines.push_back(SmallVector<UnwrappedLine, 16>());
}

} // end namespace format
} // end namespace language::Core
