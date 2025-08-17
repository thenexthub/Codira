//===- VerifyDiagnosticConsumer.h - Verifying Diagnostic Client -*- C++ -*-===//
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

#ifndef LANGUAGE_CORE_FRONTEND_VERIFYDIAGNOSTICCONSUMER_H
#define LANGUAGE_CORE_FRONTEND_VERIFYDIAGNOSTICCONSUMER_H

#include "language/Core/Basic/Diagnostic.h"
#include "language/Core/Basic/FileManager.h"
#include "language/Core/Basic/LLVM.h"
#include "language/Core/Basic/SourceLocation.h"
#include "language/Core/Lex/Preprocessor.h"
#include "toolchain/ADT/DenseMap.h"
#include "toolchain/ADT/PointerIntPair.h"
#include "toolchain/ADT/StringRef.h"
#include <cassert>
#include <limits>
#include <memory>
#include <string>
#include <vector>

namespace language::Core {

class FileEntry;
class LangOptions;
class SourceManager;
class TextDiagnosticBuffer;

/// VerifyDiagnosticConsumer - Create a diagnostic client which will use
/// markers in the input source to check that all the emitted diagnostics match
/// those expected. See clang/docs/InternalsManual.rst for details about how to
/// write tests to verify diagnostics.
///
class VerifyDiagnosticConsumer: public DiagnosticConsumer,
                                public CommentHandler {
public:
  /// Directive - Abstract class representing a parsed verify directive.
  ///
  class Directive {
  public:
    static std::unique_ptr<Directive>
    create(bool RegexKind, SourceLocation DirectiveLoc,
           SourceLocation DiagnosticLoc, StringRef Spelling,
           bool MatchAnyFileAndLine, bool MatchAnyLine, StringRef Text,
           unsigned Min, unsigned Max);

  public:
    /// Constant representing n or more matches.
    static const unsigned MaxCount = std::numeric_limits<unsigned>::max();

    SourceLocation DirectiveLoc;
    SourceLocation DiagnosticLoc;
    const std::string Spelling;
    const std::string Text;
    unsigned Min, Max;
    bool MatchAnyLine;
    bool MatchAnyFileAndLine; // `MatchAnyFileAndLine` implies `MatchAnyLine`.

    Directive(const Directive &) = delete;
    Directive &operator=(const Directive &) = delete;
    virtual ~Directive() = default;

    // Returns true if directive text is valid.
    // Otherwise returns false and populates E.
    virtual bool isValid(std::string &Error) = 0;

    // Returns true on match.
    virtual bool match(StringRef S) = 0;

  protected:
    Directive(SourceLocation DirectiveLoc, SourceLocation DiagnosticLoc,
              StringRef Spelling, bool MatchAnyFileAndLine, bool MatchAnyLine,
              StringRef Text, unsigned Min, unsigned Max)
        : DirectiveLoc(DirectiveLoc), DiagnosticLoc(DiagnosticLoc),
          Spelling(Spelling), Text(Text), Min(Min), Max(Max),
          MatchAnyLine(MatchAnyLine || MatchAnyFileAndLine),
          MatchAnyFileAndLine(MatchAnyFileAndLine) {
      assert(!DirectiveLoc.isInvalid() && "DirectiveLoc is invalid!");
      assert((!DiagnosticLoc.isInvalid() || MatchAnyLine) &&
             "DiagnosticLoc is invalid!");
    }
  };

  using DirectiveList = std::vector<std::unique_ptr<Directive>>;

  /// ExpectedData - owns directive objects and deletes on destructor.
  struct ExpectedData {
    DirectiveList Errors;
    DirectiveList Warnings;
    DirectiveList Remarks;
    DirectiveList Notes;

    void Reset() {
      Errors.clear();
      Warnings.clear();
      Remarks.clear();
      Notes.clear();
    }
  };

  enum DirectiveStatus {
    HasNoDirectives,
    HasNoDirectivesReported,
    HasExpectedNoDiagnostics,
    HasOtherExpectedDirectives
  };

  struct ParsingState {
    DirectiveStatus Status;
    std::string FirstNoDiagnosticsDirective;
  };

  class MarkerTracker;

private:
  DiagnosticsEngine &Diags;
  DiagnosticConsumer *PrimaryClient;
  std::unique_ptr<DiagnosticConsumer> PrimaryClientOwner;
  std::unique_ptr<TextDiagnosticBuffer> Buffer;
  std::unique_ptr<MarkerTracker> Markers;
  const Preprocessor *CurrentPreprocessor = nullptr;
  const LangOptions *LangOpts = nullptr;
  SourceManager *SrcManager = nullptr;
  unsigned ActiveSourceFiles = 0;
  ParsingState State;
  ExpectedData ED;

  void CheckDiagnostics();

  void setSourceManager(SourceManager &SM) {
    assert((!SrcManager || SrcManager == &SM) && "SourceManager changed!");
    SrcManager = &SM;
  }

  // These facilities are used for validation in debug builds.
  class UnparsedFileStatus {
    OptionalFileEntryRef File;
    bool FoundDirectives;

  public:
    UnparsedFileStatus(OptionalFileEntryRef File, bool FoundDirectives)
        : File(File), FoundDirectives(FoundDirectives) {}

    OptionalFileEntryRef getFile() const { return File; }
    bool foundDirectives() const { return FoundDirectives; }
  };

  using ParsedFilesMap = toolchain::DenseMap<FileID, const FileEntry *>;
  using UnparsedFilesMap = toolchain::DenseMap<FileID, UnparsedFileStatus>;

  ParsedFilesMap ParsedFiles;
  UnparsedFilesMap UnparsedFiles;

public:
  /// Create a new verifying diagnostic client, which will issue errors to
  /// the currently-attached diagnostic client when a diagnostic does not match
  /// what is expected (as indicated in the source file).
  VerifyDiagnosticConsumer(DiagnosticsEngine &Diags);
  ~VerifyDiagnosticConsumer() override;

  void BeginSourceFile(const LangOptions &LangOpts,
                       const Preprocessor *PP) override;

  void EndSourceFile() override;

  enum ParsedStatus {
    /// File has been processed via HandleComment.
    IsParsed,

    /// File has diagnostics and may have directives.
    IsUnparsed,

    /// File has diagnostics but guaranteed no directives.
    IsUnparsedNoDirectives
  };

  /// Update lists of parsed and unparsed files.
  void UpdateParsedFileStatus(SourceManager &SM, FileID FID, ParsedStatus PS);

  bool HandleComment(Preprocessor &PP, SourceRange Comment) override;

  void HandleDiagnostic(DiagnosticsEngine::Level DiagLevel,
                        const Diagnostic &Info) override;
};

} // namespace language::Core

#endif // LANGUAGE_CORE_FRONTEND_VERIFYDIAGNOSTICCONSUMER_H
