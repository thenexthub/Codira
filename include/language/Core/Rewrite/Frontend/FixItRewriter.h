//===- FixItRewriter.h - Fix-It Rewriter Diagnostic Client ------*- C++ -*-===//
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
// This is a diagnostic client adaptor that performs rewrites as
// suggested by code modification hints attached to diagnostics. It
// then forwards any diagnostics to the adapted diagnostic client.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_REWRITE_FRONTEND_FIXITREWRITER_H
#define LANGUAGE_CORE_REWRITE_FRONTEND_FIXITREWRITER_H

#include "language/Core/Basic/Diagnostic.h"
#include "language/Core/Basic/LLVM.h"
#include "language/Core/Basic/SourceLocation.h"
#include "language/Core/Edit/EditedSource.h"
#include "language/Core/Rewrite/Core/Rewriter.h"
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace language::Core {

class LangOptions;
class SourceManager;

class FixItOptions {
public:
  FixItOptions() = default;
  virtual ~FixItOptions();

  /// This file is about to be rewritten. Return the name of the file
  /// that is okay to write to.
  ///
  /// \param fd out parameter for file descriptor. After the call it may be set
  /// to an open file descriptor for the returned filename, or it will be -1
  /// otherwise.
  virtual std::string RewriteFilename(const std::string &Filename, int &fd) = 0;

  /// True if files should be updated in place. RewriteFilename is only called
  /// if this is false.
  bool InPlace = false;

  /// Whether to abort fixing a file when not all errors could be fixed.
  bool FixWhatYouCan = false;

  /// Whether to only fix warnings and not errors.
  bool FixOnlyWarnings = false;

  /// If true, only pass the diagnostic to the actual diagnostic consumer
  /// if it is an error or a fixit was applied as part of the diagnostic.
  /// It basically silences warnings without accompanying fixits.
  bool Silent = false;
};

class FixItRewriter : public DiagnosticConsumer {
  /// The diagnostics machinery.
  DiagnosticsEngine &Diags;

  edit::EditedSource Editor;

  /// The rewriter used to perform the various code
  /// modifications.
  Rewriter Rewrite;

  /// The diagnostic client that performs the actual formatting
  /// of error messages.
  DiagnosticConsumer *Client;
  std::unique_ptr<DiagnosticConsumer> Owner;

  /// Turn an input path into an output path. NULL implies overwriting
  /// the original.
  FixItOptions *FixItOpts;

  /// The number of rewriter failures.
  unsigned NumFailures = 0;

  /// Whether the previous diagnostic was not passed to the consumer.
  bool PrevDiagSilenced = false;

public:
  /// Initialize a new fix-it rewriter.
  FixItRewriter(DiagnosticsEngine &Diags, SourceManager &SourceMgr,
                const LangOptions &LangOpts, FixItOptions *FixItOpts);

  /// Destroy the fix-it rewriter.
  ~FixItRewriter() override;

  /// Check whether there are modifications for a given file.
  bool IsModified(FileID ID) const {
    return Rewrite.getRewriteBufferFor(ID) != nullptr;
  }

  using iterator = Rewriter::buffer_iterator;

  // Iteration over files with changes.
  iterator buffer_begin() { return Rewrite.buffer_begin(); }
  iterator buffer_end() { return Rewrite.buffer_end(); }

  /// Write a single modified source file.
  ///
  /// \returns true if there was an error, false otherwise.
  bool WriteFixedFile(FileID ID, raw_ostream &OS);

  /// Write the modified source files.
  ///
  /// \returns true if there was an error, false otherwise.
  bool WriteFixedFiles(
    std::vector<std::pair<std::string, std::string>> *RewrittenFiles = nullptr);

  /// IncludeInDiagnosticCounts - This method (whose default implementation
  /// returns true) indicates whether the diagnostics handled by this
  /// DiagnosticConsumer should be included in the number of diagnostics
  /// reported by DiagnosticsEngine.
  bool IncludeInDiagnosticCounts() const override;

  /// HandleDiagnostic - Handle this diagnostic, reporting it to the user or
  /// capturing it to a log as needed.
  void HandleDiagnostic(DiagnosticsEngine::Level DiagLevel,
                        const Diagnostic &Info) override;

  /// Emit a diagnostic via the adapted diagnostic client.
  void Diag(SourceLocation Loc, unsigned DiagID);
};

} // namespace language::Core

#endif // LANGUAGE_CORE_REWRITE_FRONTEND_FIXITREWRITER_H
