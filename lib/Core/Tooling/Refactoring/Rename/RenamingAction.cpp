//===--- RenamingAction.cpp - Clang refactoring library -------------------===//
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
/// Provides an action to rename every symbol at a point.
///
//===----------------------------------------------------------------------===//

#include "language/Core/Tooling/Refactoring/Rename/RenamingAction.h"
#include "language/Core/AST/ASTConsumer.h"
#include "language/Core/AST/ASTContext.h"
#include "language/Core/Frontend/CompilerInstance.h"
#include "language/Core/Lex/Lexer.h"
#include "language/Core/Tooling/Refactoring/Rename/SymbolName.h"
#include "language/Core/Tooling/Refactoring/Rename/USRFinder.h"
#include "language/Core/Tooling/Refactoring/Rename/USRFindingAction.h"
#include "language/Core/Tooling/Refactoring/Rename/USRLocFinder.h"
#include "toolchain/ADT/STLExtras.h"
#include "toolchain/Support/Errc.h"
#include "toolchain/Support/Error.h"
#include <string>
#include <vector>

using namespace toolchain;

namespace language::Core {
namespace tooling {

namespace {

Expected<SymbolOccurrences>
findSymbolOccurrences(const NamedDecl *ND, RefactoringRuleContext &Context) {
  std::vector<std::string> USRs =
      getUSRsForDeclaration(ND, Context.getASTContext());
  std::string PrevName = ND->getNameAsString();
  return getOccurrencesOfUSRs(USRs, PrevName,
                              Context.getASTContext().getTranslationUnitDecl());
}

} // end anonymous namespace

const RefactoringDescriptor &RenameOccurrences::describe() {
  static const RefactoringDescriptor Descriptor = {
      "local-rename",
      "Rename",
      "Finds and renames symbols in code with no indexer support",
  };
  return Descriptor;
}

Expected<RenameOccurrences>
RenameOccurrences::initiate(RefactoringRuleContext &Context,
                            SourceRange SelectionRange, std::string NewName) {
  const NamedDecl *ND =
      getNamedDeclAt(Context.getASTContext(), SelectionRange.getBegin());
  if (!ND)
    return Context.createDiagnosticError(
        SelectionRange.getBegin(), diag::err_refactor_selection_no_symbol);
  return RenameOccurrences(getCanonicalSymbolDeclaration(ND),
                           std::move(NewName));
}

const NamedDecl *RenameOccurrences::getRenameDecl() const { return ND; }

Expected<AtomicChanges>
RenameOccurrences::createSourceReplacements(RefactoringRuleContext &Context) {
  Expected<SymbolOccurrences> Occurrences = findSymbolOccurrences(ND, Context);
  if (!Occurrences)
    return Occurrences.takeError();
  // FIXME: Verify that the new name is valid.
  SymbolName Name(NewName);
  return createRenameReplacements(
      *Occurrences, Context.getASTContext().getSourceManager(), Name);
}

Expected<QualifiedRenameRule>
QualifiedRenameRule::initiate(RefactoringRuleContext &Context,
                              std::string OldQualifiedName,
                              std::string NewQualifiedName) {
  const NamedDecl *ND =
      getNamedDeclFor(Context.getASTContext(), OldQualifiedName);
  if (!ND)
    return toolchain::make_error<toolchain::StringError>("Could not find symbol " +
                                                   OldQualifiedName,
                                               toolchain::errc::invalid_argument);
  return QualifiedRenameRule(ND, std::move(NewQualifiedName));
}

const RefactoringDescriptor &QualifiedRenameRule::describe() {
  static const RefactoringDescriptor Descriptor = {
      /*Name=*/"local-qualified-rename",
      /*Title=*/"Qualified Rename",
      /*Description=*/
      R"(Finds and renames qualified symbols in code within a translation unit.
It is used to move/rename a symbol to a new namespace/name:
  * Supported symbols: classes, class members, functions, enums, and type alias.
  * Renames all symbol occurrences from the old qualified name to the new
    qualified name. All symbol references will be correctly qualified; For
    symbol definitions, only name will be changed.
For example, rename "A::Foo" to "B::Bar":
  Old code:
    namespace foo {
    class A {};
    }

    namespace bar {
    void f(foo::A a) {}
    }

  New code after rename:
    namespace foo {
    class B {};
    }

    namespace bar {
    void f(B b) {}
    })"
  };
  return Descriptor;
}

Expected<AtomicChanges>
QualifiedRenameRule::createSourceReplacements(RefactoringRuleContext &Context) {
  auto USRs = getUSRsForDeclaration(ND, Context.getASTContext());
  assert(!USRs.empty());
  return tooling::createRenameAtomicChanges(
      USRs, NewQualifiedName, Context.getASTContext().getTranslationUnitDecl());
}

Expected<std::vector<AtomicChange>>
createRenameReplacements(const SymbolOccurrences &Occurrences,
                         const SourceManager &SM, const SymbolName &NewName) {
  // FIXME: A true local rename can use just one AtomicChange.
  std::vector<AtomicChange> Changes;
  for (const auto &Occurrence : Occurrences) {
    ArrayRef<SourceRange> Ranges = Occurrence.getNameRanges();
    assert(NewName.getNamePieces().size() == Ranges.size() &&
           "Mismatching number of ranges and name pieces");
    AtomicChange Change(SM, Ranges[0].getBegin());
    for (const auto &Range : toolchain::enumerate(Ranges)) {
      auto Error =
          Change.replace(SM, CharSourceRange::getCharRange(Range.value()),
                         NewName.getNamePieces()[Range.index()]);
      if (Error)
        return std::move(Error);
    }
    Changes.push_back(std::move(Change));
  }
  return std::move(Changes);
}

/// Takes each atomic change and inserts its replacements into the set of
/// replacements that belong to the appropriate file.
static void convertChangesToFileReplacements(
    ArrayRef<AtomicChange> AtomicChanges,
    std::map<std::string, tooling::Replacements> *FileToReplaces) {
  for (const auto &AtomicChange : AtomicChanges) {
    for (const auto &Replace : AtomicChange.getReplacements()) {
      toolchain::Error Err =
          (*FileToReplaces)[std::string(Replace.getFilePath())].add(Replace);
      if (Err) {
        toolchain::errs() << "Renaming failed in " << Replace.getFilePath() << "! "
                     << toolchain::toString(std::move(Err)) << "\n";
      }
    }
  }
}

class RenamingASTConsumer : public ASTConsumer {
public:
  RenamingASTConsumer(
      const std::vector<std::string> &NewNames,
      const std::vector<std::string> &PrevNames,
      const std::vector<std::vector<std::string>> &USRList,
      std::map<std::string, tooling::Replacements> &FileToReplaces,
      bool PrintLocations)
      : NewNames(NewNames), PrevNames(PrevNames), USRList(USRList),
        FileToReplaces(FileToReplaces), PrintLocations(PrintLocations) {}

  void HandleTranslationUnit(ASTContext &Context) override {
    for (unsigned I = 0; I < NewNames.size(); ++I) {
      // If the previous name was not found, ignore this rename request.
      if (PrevNames[I].empty())
        continue;

      HandleOneRename(Context, NewNames[I], PrevNames[I], USRList[I]);
    }
  }

  void HandleOneRename(ASTContext &Context, const std::string &NewName,
                       const std::string &PrevName,
                       const std::vector<std::string> &USRs) {
    const SourceManager &SourceMgr = Context.getSourceManager();

    SymbolOccurrences Occurrences = tooling::getOccurrencesOfUSRs(
        USRs, PrevName, Context.getTranslationUnitDecl());
    if (PrintLocations) {
      for (const auto &Occurrence : Occurrences) {
        FullSourceLoc FullLoc(Occurrence.getNameRanges()[0].getBegin(),
                              SourceMgr);
        errs() << "clang-rename: renamed at: " << SourceMgr.getFilename(FullLoc)
               << ":" << FullLoc.getSpellingLineNumber() << ":"
               << FullLoc.getSpellingColumnNumber() << "\n";
      }
    }
    // FIXME: Support multi-piece names.
    // FIXME: better error handling (propagate error out).
    SymbolName NewNameRef(NewName);
    Expected<std::vector<AtomicChange>> Change =
        createRenameReplacements(Occurrences, SourceMgr, NewNameRef);
    if (!Change) {
      toolchain::errs() << "Failed to create renaming replacements for '" << PrevName
                   << "'! " << toolchain::toString(Change.takeError()) << "\n";
      return;
    }
    convertChangesToFileReplacements(*Change, &FileToReplaces);
  }

private:
  const std::vector<std::string> &NewNames, &PrevNames;
  const std::vector<std::vector<std::string>> &USRList;
  std::map<std::string, tooling::Replacements> &FileToReplaces;
  bool PrintLocations;
};

// A renamer to rename symbols which are identified by a give USRList to
// new name.
//
// FIXME: Merge with the above RenamingASTConsumer.
class USRSymbolRenamer : public ASTConsumer {
public:
  USRSymbolRenamer(const std::vector<std::string> &NewNames,
                   const std::vector<std::vector<std::string>> &USRList,
                   std::map<std::string, tooling::Replacements> &FileToReplaces)
      : NewNames(NewNames), USRList(USRList), FileToReplaces(FileToReplaces) {
    assert(USRList.size() == NewNames.size());
  }

  void HandleTranslationUnit(ASTContext &Context) override {
    for (unsigned I = 0; I < NewNames.size(); ++I) {
      // FIXME: Apply AtomicChanges directly once the refactoring APIs are
      // ready.
      auto AtomicChanges = tooling::createRenameAtomicChanges(
          USRList[I], NewNames[I], Context.getTranslationUnitDecl());
      convertChangesToFileReplacements(AtomicChanges, &FileToReplaces);
    }
  }

private:
  const std::vector<std::string> &NewNames;
  const std::vector<std::vector<std::string>> &USRList;
  std::map<std::string, tooling::Replacements> &FileToReplaces;
};

std::unique_ptr<ASTConsumer> RenamingAction::newASTConsumer() {
  return std::make_unique<RenamingASTConsumer>(NewNames, PrevNames, USRList,
                                                FileToReplaces, PrintLocations);
}

std::unique_ptr<ASTConsumer> QualifiedRenamingAction::newASTConsumer() {
  return std::make_unique<USRSymbolRenamer>(NewNames, USRList, FileToReplaces);
}

} // end namespace tooling
} // end namespace language::Core
