//===--- USRFindingAction.h - Clang refactoring library -------------------===//
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
/// Provides an action to find all relevant USRs at a point.
///
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_TOOLING_REFACTORING_RENAME_USRFINDINGACTION_H
#define LANGUAGE_CORE_TOOLING_REFACTORING_RENAME_USRFINDINGACTION_H

#include "language/Core/Basic/LLVM.h"
#include "toolchain/ADT/ArrayRef.h"

#include <string>
#include <vector>

namespace language::Core {
class ASTConsumer;
class ASTContext;
class NamedDecl;

namespace tooling {

/// Returns the canonical declaration that best represents a symbol that can be
/// renamed.
///
/// The following canonicalization rules are currently used:
///
/// - A constructor is canonicalized to its class.
/// - A destructor is canonicalized to its class.
const NamedDecl *getCanonicalSymbolDeclaration(const NamedDecl *FoundDecl);

/// Returns the set of USRs that correspond to the given declaration.
std::vector<std::string> getUSRsForDeclaration(const NamedDecl *ND,
                                               ASTContext &Context);

struct USRFindingAction {
  USRFindingAction(ArrayRef<unsigned> SymbolOffsets,
                   ArrayRef<std::string> QualifiedNames, bool Force)
      : SymbolOffsets(SymbolOffsets), QualifiedNames(QualifiedNames),
        ErrorOccurred(false), Force(Force) {}
  std::unique_ptr<ASTConsumer> newASTConsumer();

  ArrayRef<std::string> getUSRSpellings() { return SpellingNames; }
  ArrayRef<std::vector<std::string>> getUSRList() { return USRList; }
  bool errorOccurred() { return ErrorOccurred; }

private:
  std::vector<unsigned> SymbolOffsets;
  std::vector<std::string> QualifiedNames;
  std::vector<std::string> SpellingNames;
  std::vector<std::vector<std::string>> USRList;
  bool ErrorOccurred;
  bool Force;
};

} // end namespace tooling
} // end namespace language::Core

#endif // LANGUAGE_CORE_TOOLING_REFACTORING_RENAME_USRFINDINGACTION_H
