//===--- RenamingAction.h - Clang refactoring library ---------------------===//
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

#ifndef LANGUAGE_CORE_TOOLING_REFACTORING_RENAME_RENAMINGACTION_H
#define LANGUAGE_CORE_TOOLING_REFACTORING_RENAME_RENAMINGACTION_H

#include "language/Core/Tooling/Refactoring.h"
#include "language/Core/Tooling/Refactoring/AtomicChange.h"
#include "language/Core/Tooling/Refactoring/RefactoringActionRules.h"
#include "language/Core/Tooling/Refactoring/RefactoringOptions.h"
#include "language/Core/Tooling/Refactoring/Rename/SymbolOccurrences.h"
#include "toolchain/Support/Error.h"

namespace language::Core {
class ASTConsumer;

namespace tooling {

class RenamingAction {
public:
  RenamingAction(const std::vector<std::string> &NewNames,
                 const std::vector<std::string> &PrevNames,
                 const std::vector<std::vector<std::string>> &USRList,
                 std::map<std::string, tooling::Replacements> &FileToReplaces,
                 bool PrintLocations = false)
      : NewNames(NewNames), PrevNames(PrevNames), USRList(USRList),
        FileToReplaces(FileToReplaces), PrintLocations(PrintLocations) {}

  std::unique_ptr<ASTConsumer> newASTConsumer();

private:
  const std::vector<std::string> &NewNames, &PrevNames;
  const std::vector<std::vector<std::string>> &USRList;
  std::map<std::string, tooling::Replacements> &FileToReplaces;
  bool PrintLocations;
};

class RenameOccurrences final : public SourceChangeRefactoringRule {
public:
  static Expected<RenameOccurrences> initiate(RefactoringRuleContext &Context,
                                              SourceRange SelectionRange,
                                              std::string NewName);

  static const RefactoringDescriptor &describe();

  const NamedDecl *getRenameDecl() const;

private:
  RenameOccurrences(const NamedDecl *ND, std::string NewName)
      : ND(ND), NewName(std::move(NewName)) {}

  Expected<AtomicChanges>
  createSourceReplacements(RefactoringRuleContext &Context) override;

  const NamedDecl *ND;
  std::string NewName;
};

class QualifiedRenameRule final : public SourceChangeRefactoringRule {
public:
  static Expected<QualifiedRenameRule> initiate(RefactoringRuleContext &Context,
                                                std::string OldQualifiedName,
                                                std::string NewQualifiedName);

  static const RefactoringDescriptor &describe();

private:
  QualifiedRenameRule(const NamedDecl *ND,
                      std::string NewQualifiedName)
      : ND(ND), NewQualifiedName(std::move(NewQualifiedName)) {}

  Expected<AtomicChanges>
  createSourceReplacements(RefactoringRuleContext &Context) override;

  // A NamedDecl which identifies the symbol being renamed.
  const NamedDecl *ND;
  // The new qualified name to change the symbol to.
  std::string NewQualifiedName;
};

/// Returns source replacements that correspond to the rename of the given
/// symbol occurrences.
toolchain::Expected<std::vector<AtomicChange>>
createRenameReplacements(const SymbolOccurrences &Occurrences,
                         const SourceManager &SM, const SymbolName &NewName);

/// Rename all symbols identified by the given USRs.
class QualifiedRenamingAction {
public:
  QualifiedRenamingAction(
      const std::vector<std::string> &NewNames,
      const std::vector<std::vector<std::string>> &USRList,
      std::map<std::string, tooling::Replacements> &FileToReplaces)
      : NewNames(NewNames), USRList(USRList), FileToReplaces(FileToReplaces) {}

  std::unique_ptr<ASTConsumer> newASTConsumer();

private:
  /// New symbol names.
  const std::vector<std::string> &NewNames;

  /// A list of USRs. Each element represents USRs of a symbol being renamed.
  const std::vector<std::vector<std::string>> &USRList;

  /// A file path to replacements map.
  std::map<std::string, tooling::Replacements> &FileToReplaces;
};

} // end namespace tooling
} // end namespace language::Core

#endif // LANGUAGE_CORE_TOOLING_REFACTORING_RENAME_RENAMINGACTION_H
