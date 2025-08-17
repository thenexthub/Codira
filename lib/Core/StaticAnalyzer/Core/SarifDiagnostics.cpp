//===--- SarifDiagnostics.cpp - Sarif Diagnostics for Paths -----*- C++ -*-===//
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
//  This file defines the SarifDiagnostics object.
//
//===----------------------------------------------------------------------===//

#include "language/Core/Analysis/MacroExpansionContext.h"
#include "language/Core/Analysis/PathDiagnostic.h"
#include "language/Core/Basic/Sarif.h"
#include "language/Core/Basic/SourceManager.h"
#include "language/Core/Basic/Version.h"
#include "language/Core/Lex/Preprocessor.h"
#include "language/Core/StaticAnalyzer/Core/PathDiagnosticConsumers.h"
#include "toolchain/ADT/StringMap.h"
#include "toolchain/Support/ConvertUTF.h"
#include "toolchain/Support/JSON.h"
#include <memory>

using namespace toolchain;
using namespace language::Core;
using namespace ento;

namespace {
class SarifDiagnostics : public PathDiagnosticConsumer {
  std::string OutputFile;
  const LangOptions &LO;
  SarifDocumentWriter SarifWriter;

public:
  SarifDiagnostics(const std::string &Output, const LangOptions &LO,
                   const SourceManager &SM)
      : OutputFile(Output), LO(LO), SarifWriter(SM) {}
  ~SarifDiagnostics() override = default;

  void FlushDiagnosticsImpl(std::vector<const PathDiagnostic *> &Diags,
                            FilesMade *FM) override;

  StringRef getName() const override { return "SarifDiagnostics"; }
  PathGenerationScheme getGenerationScheme() const override { return Minimal; }
  bool supportsLogicalOpControlFlow() const override { return true; }
  bool supportsCrossFileDiagnostics() const override { return true; }
};
} // end anonymous namespace

void ento::createSarifDiagnosticConsumer(
    PathDiagnosticConsumerOptions DiagOpts, PathDiagnosticConsumers &C,
    const std::string &Output, const Preprocessor &PP,
    const cross_tu::CrossTranslationUnitContext &CTU,
    const MacroExpansionContext &MacroExpansions) {

  // TODO: Emit an error here.
  if (Output.empty())
    return;

  C.push_back(std::make_unique<SarifDiagnostics>(Output, PP.getLangOpts(),
                                                 PP.getSourceManager()));
  createTextMinimalPathDiagnosticConsumer(std::move(DiagOpts), C, Output, PP,
                                          CTU, MacroExpansions);
}

static StringRef getRuleDescription(StringRef CheckName) {
  return toolchain::StringSwitch<StringRef>(CheckName)
#define GET_CHECKERS
#define CHECKER(FULLNAME, CLASS, HELPTEXT, DOC_URI, IS_HIDDEN)                 \
  .Case(FULLNAME, HELPTEXT)
#include "language/Core/StaticAnalyzer/Checkers/Checkers.inc"
#undef CHECKER
#undef GET_CHECKERS
      ;
}

static StringRef getRuleHelpURIStr(StringRef CheckName) {
  return toolchain::StringSwitch<StringRef>(CheckName)
#define GET_CHECKERS
#define CHECKER(FULLNAME, CLASS, HELPTEXT, DOC_URI, IS_HIDDEN)                 \
  .Case(FULLNAME, DOC_URI)
#include "language/Core/StaticAnalyzer/Checkers/Checkers.inc"
#undef CHECKER
#undef GET_CHECKERS
      ;
}

static ThreadFlowImportance
calculateImportance(const PathDiagnosticPiece &Piece) {
  switch (Piece.getKind()) {
  case PathDiagnosticPiece::Call:
  case PathDiagnosticPiece::Macro:
  case PathDiagnosticPiece::Note:
  case PathDiagnosticPiece::PopUp:
    // FIXME: What should be reported here?
    break;
  case PathDiagnosticPiece::Event:
    return Piece.getTagStr() == "ConditionBRVisitor"
               ? ThreadFlowImportance::Important
               : ThreadFlowImportance::Essential;
  case PathDiagnosticPiece::ControlFlow:
    return ThreadFlowImportance::Unimportant;
  }
  return ThreadFlowImportance::Unimportant;
}

/// Accepts a SourceRange corresponding to a pair of the first and last tokens
/// and converts to a Character granular CharSourceRange.
static CharSourceRange convertTokenRangeToCharRange(const SourceRange &R,
                                                    const SourceManager &SM,
                                                    const LangOptions &LO) {
  // Caret diagnostics have the first and last locations pointed at the same
  // location, return these as-is.
  if (R.getBegin() == R.getEnd())
    return CharSourceRange::getCharRange(R);

  SourceLocation BeginCharLoc = R.getBegin();
  // For token ranges, the raw end SLoc points at the first character of the
  // last token in the range. This must be moved to one past the end of the
  // last character using the lexer.
  SourceLocation EndCharLoc =
      Lexer::getLocForEndOfToken(R.getEnd(), /* Offset = */ 0, SM, LO);
  return CharSourceRange::getCharRange(BeginCharLoc, EndCharLoc);
}

static SmallVector<ThreadFlow, 8> createThreadFlows(const PathDiagnostic *Diag,
                                                    const LangOptions &LO) {
  SmallVector<ThreadFlow, 8> Flows;
  const PathPieces &Pieces = Diag->path.flatten(false);
  for (const auto &Piece : Pieces) {
    auto Range = convertTokenRangeToCharRange(
        Piece->getLocation().asRange(), Piece->getLocation().getManager(), LO);
    auto Flow = ThreadFlow::create()
                    .setImportance(calculateImportance(*Piece))
                    .setRange(Range)
                    .setMessage(Piece->getString());
    Flows.push_back(Flow);
  }
  return Flows;
}

static StringMap<uint32_t>
createRuleMapping(const std::vector<const PathDiagnostic *> &Diags,
                  SarifDocumentWriter &SarifWriter) {
  StringMap<uint32_t> RuleMapping;
  toolchain::StringSet<> Seen;

  for (const PathDiagnostic *D : Diags) {
    StringRef CheckName = D->getCheckerName();
    std::pair<toolchain::StringSet<>::iterator, bool> P = Seen.insert(CheckName);
    if (P.second) {
      auto Rule = SarifRule::create()
                      .setName(CheckName)
                      .setRuleId(CheckName)
                      .setDescription(getRuleDescription(CheckName))
                      .setHelpURI(getRuleHelpURIStr(CheckName));
      size_t RuleIdx = SarifWriter.createRule(Rule);
      RuleMapping[CheckName] = RuleIdx;
    }
  }
  return RuleMapping;
}

static SarifResult createResult(const PathDiagnostic *Diag,
                                const StringMap<uint32_t> &RuleMapping,
                                const LangOptions &LO) {

  StringRef CheckName = Diag->getCheckerName();
  uint32_t RuleIdx = RuleMapping.lookup(CheckName);
  auto Range = convertTokenRangeToCharRange(
      Diag->getLocation().asRange(), Diag->getLocation().getManager(), LO);

  SmallVector<ThreadFlow, 8> Flows = createThreadFlows(Diag, LO);
  auto Result = SarifResult::create(RuleIdx)
                    .setRuleId(CheckName)
                    .setDiagnosticMessage(Diag->getVerboseDescription())
                    .setDiagnosticLevel(SarifResultLevel::Warning)
                    .setLocations({Range})
                    .setThreadFlows(Flows);
  return Result;
}

void SarifDiagnostics::FlushDiagnosticsImpl(
    std::vector<const PathDiagnostic *> &Diags, FilesMade *) {
  // We currently overwrite the file if it already exists. However, it may be
  // useful to add a feature someday that allows the user to append a run to an
  // existing SARIF file. One danger from that approach is that the size of the
  // file can become large very quickly, so decoding into JSON to append a run
  // may be an expensive operation.
  std::error_code EC;
  toolchain::raw_fd_ostream OS(OutputFile, EC, toolchain::sys::fs::OF_TextWithCRLF);
  if (EC) {
    toolchain::errs() << "warning: could not create file: " << EC.message() << '\n';
    return;
  }

  std::string ToolVersion = getClangFullVersion();
  SarifWriter.createRun("clang", "clang static analyzer", ToolVersion);
  StringMap<uint32_t> RuleMapping = createRuleMapping(Diags, SarifWriter);
  for (const PathDiagnostic *D : Diags) {
    SarifResult Result = createResult(D, RuleMapping, LO);
    SarifWriter.appendResult(Result);
  }
  auto Document = SarifWriter.createDocument();
  OS << toolchain::formatv("{0:2}\n", json::Value(std::move(Document)));
}
