//===--- Refactoring.cpp - Framework for clang refactoring tools ----------===//
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
//  Implements tools to support refactorings.
//
//===----------------------------------------------------------------------===//

#include "language/Core/Tooling/Refactoring.h"
#include "language/Core/Basic/DiagnosticOptions.h"
#include "language/Core/Basic/FileManager.h"
#include "language/Core/Basic/SourceManager.h"
#include "language/Core/Format/Format.h"
#include "language/Core/Frontend/TextDiagnosticPrinter.h"
#include "language/Core/Lex/Lexer.h"
#include "language/Core/Rewrite/Core/Rewriter.h"

namespace language::Core {
namespace tooling {

RefactoringTool::RefactoringTool(
    const CompilationDatabase &Compilations, ArrayRef<std::string> SourcePaths,
    std::shared_ptr<PCHContainerOperations> PCHContainerOps)
    : ClangTool(Compilations, SourcePaths, std::move(PCHContainerOps)) {}

std::map<std::string, Replacements> &RefactoringTool::getReplacements() {
  return FileToReplaces;
}

int RefactoringTool::runAndSave(FrontendActionFactory *ActionFactory) {
  if (int Result = run(ActionFactory)) {
    return Result;
  }

  LangOptions DefaultLangOptions;
  DiagnosticOptions DiagOpts;
  TextDiagnosticPrinter DiagnosticPrinter(toolchain::errs(), DiagOpts);
  DiagnosticsEngine Diagnostics(DiagnosticIDs::create(), DiagOpts,
                                &DiagnosticPrinter, false);
  SourceManager Sources(Diagnostics, getFiles());
  Rewriter Rewrite(Sources, DefaultLangOptions);

  if (!applyAllReplacements(Rewrite)) {
    toolchain::errs() << "Skipped some replacements.\n";
  }

  return saveRewrittenFiles(Rewrite);
}

bool RefactoringTool::applyAllReplacements(Rewriter &Rewrite) {
  bool Result = true;
  for (const auto &Entry : groupReplacementsByFile(
           Rewrite.getSourceMgr().getFileManager(), FileToReplaces))
    Result = tooling::applyAllReplacements(Entry.second, Rewrite) && Result;
  return Result;
}

int RefactoringTool::saveRewrittenFiles(Rewriter &Rewrite) {
  return Rewrite.overwriteChangedFiles() ? 1 : 0;
}

bool formatAndApplyAllReplacements(
    const std::map<std::string, Replacements> &FileToReplaces,
    Rewriter &Rewrite, StringRef Style) {
  SourceManager &SM = Rewrite.getSourceMgr();
  FileManager &Files = SM.getFileManager();

  bool Result = true;
  for (const auto &FileAndReplaces : groupReplacementsByFile(
           Rewrite.getSourceMgr().getFileManager(), FileToReplaces)) {
    const std::string &FilePath = FileAndReplaces.first;
    auto &CurReplaces = FileAndReplaces.second;

    FileEntryRef Entry = toolchain::cantFail(Files.getFileRef(FilePath));
    FileID ID = SM.getOrCreateFileID(Entry, SrcMgr::C_User);
    StringRef Code = SM.getBufferData(ID);

    auto CurStyle = format::getStyle(Style, FilePath, "LLVM");
    if (!CurStyle) {
      toolchain::errs() << toolchain::toString(CurStyle.takeError()) << "\n";
      return false;
    }

    auto NewReplacements =
        format::formatReplacements(Code, CurReplaces, *CurStyle);
    if (!NewReplacements) {
      toolchain::errs() << toolchain::toString(NewReplacements.takeError()) << "\n";
      return false;
    }
    Result = applyAllReplacements(*NewReplacements, Rewrite) && Result;
  }
  return Result;
}

} // end namespace tooling
} // end namespace language::Core
