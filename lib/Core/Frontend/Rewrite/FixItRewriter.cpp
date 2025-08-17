//===- FixItRewriter.cpp - Fix-It Rewriter Diagnostic Client --------------===//
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

#include "language/Core/Rewrite/Frontend/FixItRewriter.h"
#include "language/Core/Basic/Diagnostic.h"
#include "language/Core/Basic/LLVM.h"
#include "language/Core/Basic/SourceLocation.h"
#include "language/Core/Basic/SourceManager.h"
#include "language/Core/Edit/Commit.h"
#include "language/Core/Edit/EditsReceiver.h"
#include "language/Core/Frontend/FrontendDiagnostic.h"
#include "language/Core/Rewrite/Core/Rewriter.h"
#include "toolchain/ADT/RewriteBuffer.h"
#include "toolchain/ADT/StringRef.h"
#include "toolchain/Support/FileSystem.h"
#include "toolchain/Support/raw_ostream.h"
#include <cstdio>
#include <memory>
#include <string>
#include <system_error>
#include <utility>

using namespace language::Core;
using toolchain::RewriteBuffer;

FixItRewriter::FixItRewriter(DiagnosticsEngine &Diags, SourceManager &SourceMgr,
                             const LangOptions &LangOpts,
                             FixItOptions *FixItOpts)
    : Diags(Diags), Editor(SourceMgr, LangOpts), Rewrite(SourceMgr, LangOpts),
      FixItOpts(FixItOpts) {
  Owner = Diags.takeClient();
  Client = Diags.getClient();
  Diags.setClient(this, false);
}

FixItRewriter::~FixItRewriter() {
  Diags.setClient(Client, Owner.release() != nullptr);
}

bool FixItRewriter::WriteFixedFile(FileID ID, raw_ostream &OS) {
  const RewriteBuffer *RewriteBuf = Rewrite.getRewriteBufferFor(ID);
  if (!RewriteBuf) return true;
  RewriteBuf->write(OS);
  OS.flush();
  return false;
}

namespace {

class RewritesReceiver : public edit::EditsReceiver {
  Rewriter &Rewrite;

public:
  RewritesReceiver(Rewriter &Rewrite) : Rewrite(Rewrite) {}

  void insert(SourceLocation loc, StringRef text) override {
    Rewrite.InsertText(loc, text);
  }

  void replace(CharSourceRange range, StringRef text) override {
    Rewrite.ReplaceText(range.getBegin(), Rewrite.getRangeSize(range), text);
  }
};

} // namespace

bool FixItRewriter::WriteFixedFiles(
             std::vector<std::pair<std::string, std::string>> *RewrittenFiles) {
  if (NumFailures > 0 && !FixItOpts->FixWhatYouCan) {
    Diag(FullSourceLoc(), diag::warn_fixit_no_changes);
    return true;
  }

  RewritesReceiver Rec(Rewrite);
  Editor.applyRewrites(Rec);

  if (FixItOpts->InPlace) {
    // Overwriting open files on Windows is tricky, but the rewriter can do it
    // for us.
    Rewrite.overwriteChangedFiles();
    return false;
  }

  for (iterator I = buffer_begin(), E = buffer_end(); I != E; ++I) {
    OptionalFileEntryRef Entry =
        Rewrite.getSourceMgr().getFileEntryRefForID(I->first);
    int fd;
    std::string Filename =
        FixItOpts->RewriteFilename(std::string(Entry->getName()), fd);
    std::error_code EC;
    std::unique_ptr<toolchain::raw_fd_ostream> OS;
    if (fd != -1) {
      OS.reset(new toolchain::raw_fd_ostream(fd, /*shouldClose=*/true));
    } else {
      OS.reset(new toolchain::raw_fd_ostream(Filename, EC, toolchain::sys::fs::OF_None));
    }
    if (EC) {
      Diags.Report(language::Core::diag::err_fe_unable_to_open_output) << Filename
                                                              << EC.message();
      continue;
    }
    RewriteBuffer &RewriteBuf = I->second;
    RewriteBuf.write(*OS);
    OS->flush();

    if (RewrittenFiles)
      RewrittenFiles->push_back(
          std::make_pair(std::string(Entry->getName()), Filename));
  }

  return false;
}

bool FixItRewriter::IncludeInDiagnosticCounts() const {
  return Client ? Client->IncludeInDiagnosticCounts() : true;
}

void FixItRewriter::HandleDiagnostic(DiagnosticsEngine::Level DiagLevel,
                                     const Diagnostic &Info) {
  // Default implementation (Warnings/errors count).
  DiagnosticConsumer::HandleDiagnostic(DiagLevel, Info);

  if (!FixItOpts->Silent ||
      DiagLevel >= DiagnosticsEngine::Error ||
      (DiagLevel == DiagnosticsEngine::Note && !PrevDiagSilenced) ||
      (DiagLevel > DiagnosticsEngine::Note && Info.getNumFixItHints())) {
    Client->HandleDiagnostic(DiagLevel, Info);
    PrevDiagSilenced = false;
  } else {
    PrevDiagSilenced = true;
  }

  // Skip over any diagnostics that are ignored or notes.
  if (DiagLevel <= DiagnosticsEngine::Note)
    return;
  // Skip over errors if we are only fixing warnings.
  if (DiagLevel >= DiagnosticsEngine::Error && FixItOpts->FixOnlyWarnings) {
    ++NumFailures;
    return;
  }

  // Make sure that we can perform all of the modifications we
  // in this diagnostic.
  edit::Commit commit(Editor);
  for (unsigned Idx = 0, Last = Info.getNumFixItHints();
       Idx < Last; ++Idx) {
    const FixItHint &Hint = Info.getFixItHint(Idx);

    if (Hint.CodeToInsert.empty()) {
      if (Hint.InsertFromRange.isValid())
        commit.insertFromRange(Hint.RemoveRange.getBegin(),
                           Hint.InsertFromRange, /*afterToken=*/false,
                           Hint.BeforePreviousInsertions);
      else
        commit.remove(Hint.RemoveRange);
    } else {
      if (Hint.RemoveRange.isTokenRange() ||
          Hint.RemoveRange.getBegin() != Hint.RemoveRange.getEnd())
        commit.replace(Hint.RemoveRange, Hint.CodeToInsert);
      else
        commit.insert(Hint.RemoveRange.getBegin(), Hint.CodeToInsert,
                    /*afterToken=*/false, Hint.BeforePreviousInsertions);
    }
  }
  bool CanRewrite = Info.getNumFixItHints() > 0 && commit.isCommitable();

  if (!CanRewrite) {
    if (Info.getNumFixItHints() > 0)
      Diag(Info.getLocation(), diag::note_fixit_in_macro);

    // If this was an error, refuse to perform any rewriting.
    if (DiagLevel >= DiagnosticsEngine::Error) {
      if (++NumFailures == 1)
        Diag(Info.getLocation(), diag::note_fixit_unfixed_error);
    }
    return;
  }

  if (!Editor.commit(commit)) {
    ++NumFailures;
    Diag(Info.getLocation(), diag::note_fixit_failed);
    return;
  }

  Diag(Info.getLocation(), diag::note_fixit_applied);
}

/// Emit a diagnostic via the adapted diagnostic client.
void FixItRewriter::Diag(SourceLocation Loc, unsigned DiagID) {
  // When producing this diagnostic, we temporarily bypass ourselves,
  // and let the downstream client format the diagnostic.
  Diags.setClient(Client, false);
  Diags.Report(Loc, DiagID);
  Diags.setClient(this, false);
}

FixItOptions::~FixItOptions() = default;
