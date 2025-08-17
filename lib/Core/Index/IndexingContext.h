//===- IndexingContext.h - Indexing context data ----------------*- C++ -*-===//
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

#ifndef LANGUAGE_CORE_LIB_INDEX_INDEXINGCONTEXT_H
#define LANGUAGE_CORE_LIB_INDEX_INDEXINGCONTEXT_H

#include "language/Core/Basic/IdentifierTable.h"
#include "language/Core/Basic/LLVM.h"
#include "language/Core/Index/IndexSymbol.h"
#include "language/Core/Index/IndexingAction.h"
#include "language/Core/Lex/MacroInfo.h"
#include "toolchain/ADT/ArrayRef.h"

namespace language::Core {
  class ASTContext;
  class Decl;
  class DeclGroupRef;
  class ImportDecl;
  class HeuristicResolver;
  class TagDecl;
  class TypeSourceInfo;
  class NamedDecl;
  class ObjCMethodDecl;
  class DeclContext;
  class NestedNameSpecifierLoc;
  class Stmt;
  class Expr;
  class TypeLoc;
  class SourceLocation;

namespace index {
  class IndexDataConsumer;

class IndexingContext {
  IndexingOptions IndexOpts;
  IndexDataConsumer &DataConsumer;
  ASTContext *Ctx = nullptr;
  std::unique_ptr<HeuristicResolver> Resolver;

public:
  IndexingContext(IndexingOptions IndexOpts, IndexDataConsumer &DataConsumer);
  // Defaulted, but defined out of line to avoid a dependency on
  // HeuristicResolver.h (unique_ptr requires a complete type at
  // the point where its destructor is called).
  ~IndexingContext();

  const IndexingOptions &getIndexOpts() const { return IndexOpts; }
  IndexDataConsumer &getDataConsumer() { return DataConsumer; }

  void setASTContext(ASTContext &ctx);

  HeuristicResolver *getResolver() const { return Resolver.get(); }

  bool shouldIndex(const Decl *D);

  const LangOptions &getLangOpts() const;

  bool shouldSuppressRefs() const {
    return false;
  }

  bool shouldIndexFunctionLocalSymbols() const;

  bool shouldIndexImplicitInstantiation() const;

  bool shouldIndexParametersInDeclarations() const;

  bool shouldIndexTemplateParameters() const;

  static bool isTemplateImplicitInstantiation(const Decl *D);

  bool handleDecl(const Decl *D, SymbolRoleSet Roles = SymbolRoleSet(),
                  ArrayRef<SymbolRelation> Relations = {});

  bool handleDecl(const Decl *D, SourceLocation Loc,
                  SymbolRoleSet Roles = SymbolRoleSet(),
                  ArrayRef<SymbolRelation> Relations = {},
                  const DeclContext *DC = nullptr);

  bool handleReference(const NamedDecl *D, SourceLocation Loc,
                       const NamedDecl *Parent, const DeclContext *DC,
                       SymbolRoleSet Roles = SymbolRoleSet(),
                       ArrayRef<SymbolRelation> Relations = {},
                       const Expr *RefE = nullptr);

  void handleMacroDefined(const IdentifierInfo &Name, SourceLocation Loc,
                          const MacroInfo &MI);

  void handleMacroUndefined(const IdentifierInfo &Name, SourceLocation Loc,
                            const MacroInfo &MI);

  void handleMacroReference(const IdentifierInfo &Name, SourceLocation Loc,
                            const MacroInfo &MD);

  bool importedModule(const ImportDecl *ImportD);

  bool indexDecl(const Decl *D);

  void indexTagDecl(const TagDecl *D, ArrayRef<SymbolRelation> Relations = {});

  void indexTypeSourceInfo(TypeSourceInfo *TInfo, const NamedDecl *Parent,
                           const DeclContext *DC = nullptr,
                           bool isBase = false,
                           bool isIBType = false);

  void indexTypeLoc(TypeLoc TL, const NamedDecl *Parent,
                    const DeclContext *DC = nullptr,
                    bool isBase = false,
                    bool isIBType = false);

  void indexNestedNameSpecifierLoc(NestedNameSpecifierLoc NNS,
                                   const NamedDecl *Parent,
                                   const DeclContext *DC = nullptr);

  bool indexDeclContext(const DeclContext *DC);

  void indexBody(const Stmt *S, const NamedDecl *Parent,
                 const DeclContext *DC = nullptr);

  bool indexTopLevelDecl(const Decl *D);
  bool indexDeclGroupRef(DeclGroupRef DG);

private:
  bool shouldIgnoreIfImplicit(const Decl *D);

  bool shouldIndexMacroOccurrence(bool IsRef, SourceLocation Loc);

  bool handleDeclOccurrence(const Decl *D, SourceLocation Loc,
                            bool IsRef, const Decl *Parent,
                            SymbolRoleSet Roles,
                            ArrayRef<SymbolRelation> Relations,
                            const Expr *RefE,
                            const Decl *RefD,
                            const DeclContext *ContainerDC);
};

} // end namespace index
} // end namespace language::Core

#endif
