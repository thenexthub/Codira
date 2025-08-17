//===- ASTImporterSharedState.h - ASTImporter specific state --*- C++ -*---===//
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
//  This file defines the ASTImporter specific state, which may be shared
//  amongst several ASTImporter objects.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_AST_ASTIMPORTERSHAREDSTATE_H
#define LANGUAGE_CORE_AST_ASTIMPORTERSHAREDSTATE_H

#include "language/Core/AST/ASTImportError.h"
#include "language/Core/AST/ASTImporterLookupTable.h"
#include "language/Core/AST/Decl.h"
#include "toolchain/ADT/DenseMap.h"
#include <optional>

namespace language::Core {

class TranslationUnitDecl;

/// Importer specific state, which may be shared amongst several ASTImporter
/// objects.
class ASTImporterSharedState {

  /// Pointer to the import specific lookup table.
  std::unique_ptr<ASTImporterLookupTable> LookupTable;

  /// Mapping from the already-imported declarations in the "to"
  /// context to the error status of the import of that declaration.
  /// This map contains only the declarations that were not correctly
  /// imported. The same declaration may or may not be included in
  /// ImportedFromDecls. This map is updated continuously during imports and
  /// never cleared (like ImportedFromDecls).
  toolchain::DenseMap<Decl *, ASTImportError> ImportErrors;

  /// Set of the newly created declarations.
  toolchain::DenseSet<Decl *> NewDecls;

  // FIXME put ImportedFromDecls here!
  // And from that point we can better encapsulate the lookup table.

public:
  ASTImporterSharedState() = default;

  ASTImporterSharedState(TranslationUnitDecl &ToTU) {
    LookupTable = std::make_unique<ASTImporterLookupTable>(ToTU);
  }

  ASTImporterLookupTable *getLookupTable() { return LookupTable.get(); }

  void addDeclToLookup(Decl *D) {
    if (LookupTable)
      if (auto *ND = dyn_cast<NamedDecl>(D))
        LookupTable->add(ND);
  }

  void removeDeclFromLookup(Decl *D) {
    if (LookupTable)
      if (auto *ND = dyn_cast<NamedDecl>(D))
        LookupTable->remove(ND);
  }

  std::optional<ASTImportError> getImportDeclErrorIfAny(Decl *ToD) const {
    auto Pos = ImportErrors.find(ToD);
    if (Pos != ImportErrors.end())
      return Pos->second;
    else
      return std::nullopt;
  }

  void setImportDeclError(Decl *To, ASTImportError Error) {
    ImportErrors[To] = Error;
  }

  bool isNewDecl(const Decl *ToD) const { return NewDecls.count(ToD); }

  void markAsNewDecl(Decl *ToD) { NewDecls.insert(ToD); }
};

} // namespace language::Core
#endif // LANGUAGE_CORE_AST_ASTIMPORTERSHAREDSTATE_H
