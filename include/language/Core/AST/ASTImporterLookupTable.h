//===- ASTImporterLookupTable.h - ASTImporter specific lookup--*- C++ -*---===//
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
//  This file defines the ASTImporterLookupTable class which implements a
//  lookup procedure for the import mechanism.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_AST_ASTIMPORTERLOOKUPTABLE_H
#define LANGUAGE_CORE_AST_ASTIMPORTERLOOKUPTABLE_H

#include "language/Core/AST/DeclBase.h" // lookup_result
#include "language/Core/AST/DeclarationName.h"
#include "toolchain/ADT/DenseMap.h"
#include "toolchain/ADT/SetVector.h"

namespace language::Core {

class NamedDecl;
class DeclContext;

// There are certain cases when normal C/C++ lookup (localUncachedLookup)
// does not find AST nodes. E.g.:
// Example 1:
//   template <class T>
//   struct X {
//     friend void foo(); // this is never found in the DC of the TU.
//   };
// Example 2:
//   // The fwd decl to Foo is not found in the lookupPtr of the DC of the
//   // translation unit decl.
//   // Here we could find the node by doing a traverse through the list of
//   // the Decls in the DC, but that would not scale.
//   struct A { struct Foo *p; };
// This is a severe problem because the importer decides if it has to create a
// new Decl or not based on the lookup results.
// To overcome these cases we need an importer specific lookup table which
// holds every node and we are not interested in any C/C++ specific visibility
// considerations. Simply, we must know if there is an existing Decl in a
// given DC. Once we found it then we can handle any visibility related tasks.
class ASTImporterLookupTable {

  // We store a list of declarations for each name.
  // And we collect these lists for each DeclContext.
  // We could have a flat map with (DeclContext, Name) tuple as key, but a two
  // level map seems easier to handle.
  using DeclList = toolchain::SmallSetVector<NamedDecl *, 2>;
  using NameMap = toolchain::SmallDenseMap<DeclarationName, DeclList, 4>;
  using DCMap = toolchain::DenseMap<DeclContext *, NameMap>;

  void add(DeclContext *DC, NamedDecl *ND);
  void remove(DeclContext *DC, NamedDecl *ND);

  DCMap LookupTable;

public:
  ASTImporterLookupTable(TranslationUnitDecl &TU);
  void add(NamedDecl *ND);
  void remove(NamedDecl *ND);
  // Sometimes a declaration is created first with a temporarily value of decl
  // context (often the translation unit) and later moved to the final context.
  // This happens for declarations that are created before the final declaration
  // context. In such cases the lookup table needs to be updated.
  // (The declaration is in these cases not added to the temporary decl context,
  // only its parent is set.)
  // FIXME: It would be better to not add the declaration to the temporary
  // context at all in the lookup table, but this requires big change in
  // ASTImporter.
  // The function should be called when the old context is definitely different
  // from the new.
  void update(NamedDecl *ND, DeclContext *OldDC);
  // Same as 'update' but allow if 'ND' is not in the table or the old context
  // is the same as the new.
  // FIXME: The old redeclaration context is not handled.
  void updateForced(NamedDecl *ND, DeclContext *OldDC);
  using LookupResult = DeclList;
  LookupResult lookup(DeclContext *DC, DeclarationName Name) const;
  // Check if the `ND` is within the lookup table (with its current name) in
  // context `DC`. This is intended for debug purposes when the DeclContext of a
  // NamedDecl is changed.
  bool contains(DeclContext *DC, NamedDecl *ND) const;
  void dump(DeclContext *DC) const;
  void dump() const;
};

} // namespace language::Core

#endif // LANGUAGE_CORE_AST_ASTIMPORTERLOOKUPTABLE_H
