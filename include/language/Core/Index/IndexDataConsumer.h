//===--- IndexDataConsumer.h - Abstract index data consumer -----*- C++ -*-===//
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

#ifndef LANGUAGE_CORE_INDEX_INDEXDATACONSUMER_H
#define LANGUAGE_CORE_INDEX_INDEXDATACONSUMER_H

#include "language/Core/Index/IndexSymbol.h"
#include "language/Core/Lex/Preprocessor.h"

namespace language::Core {
  class ASTContext;
  class DeclContext;
  class Expr;
  class FileID;
  class IdentifierInfo;
  class ImportDecl;
  class MacroInfo;

namespace index {

class IndexDataConsumer {
public:
  struct ASTNodeInfo {
    const Expr *OrigE;
    const Decl *OrigD;
    const Decl *Parent;
    const DeclContext *ContainerDC;
  };

  virtual ~IndexDataConsumer() = default;

  virtual void initialize(ASTContext &Ctx) {}

  virtual void setPreprocessor(std::shared_ptr<Preprocessor> PP) {}

  /// \returns true to continue indexing, or false to abort.
  virtual bool handleDeclOccurrence(const Decl *D, SymbolRoleSet Roles,
                                    ArrayRef<SymbolRelation> Relations,
                                    SourceLocation Loc, ASTNodeInfo ASTNode) {
    return true;
  }

  /// \returns true to continue indexing, or false to abort.
  virtual bool handleMacroOccurrence(const IdentifierInfo *Name,
                                     const MacroInfo *MI, SymbolRoleSet Roles,
                                     SourceLocation Loc) {
    return true;
  }

  /// \returns true to continue indexing, or false to abort.
  ///
  /// This will be called for each module reference in an import decl.
  /// For "@import MyMod.SubMod", there will be a call for 'MyMod' with the
  /// 'reference' role, and a call for 'SubMod' with the 'declaration' role.
  virtual bool handleModuleOccurrence(const ImportDecl *ImportD,
                                      const Module *Mod, SymbolRoleSet Roles,
                                      SourceLocation Loc) {
    return true;
  }

  virtual void finish() {}
};

} // namespace index
} // namespace language::Core

#endif
