//===--- ParentMap.h - Mappings from Stmts to their Parents -----*- C++ -*-===//
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
//  This file defines the ParentMap class.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_AST_PARENTMAP_H
#define LANGUAGE_CORE_AST_PARENTMAP_H

namespace language::Core {
class Stmt;
class Expr;

class ParentMap {
  void* Impl;
public:
  ParentMap(Stmt* ASTRoot);
  ~ParentMap();

  /// Adds and/or updates the parent/child-relations of the complete
  /// stmt tree of S. All children of S including indirect descendants are
  /// visited and updated or inserted but not the parents of S.
  void addStmt(Stmt* S);

  /// Manually sets the parent of \p S to \p Parent.
  ///
  /// If \p S is already in the map, this method will update the mapping.
  void setParent(const Stmt *S, const Stmt *Parent);

  Stmt *getParent(Stmt*) const;
  Stmt *getParentIgnoreParens(Stmt *) const;
  Stmt *getParentIgnoreParenCasts(Stmt *) const;
  Stmt *getParentIgnoreParenImpCasts(Stmt *) const;
  Stmt *getOuterParenParent(Stmt *) const;

  const Stmt *getParent(const Stmt* S) const {
    return getParent(const_cast<Stmt*>(S));
  }

  const Stmt *getParentIgnoreParens(const Stmt *S) const {
    return getParentIgnoreParens(const_cast<Stmt*>(S));
  }

  const Stmt *getParentIgnoreParenCasts(const Stmt *S) const {
    return getParentIgnoreParenCasts(const_cast<Stmt*>(S));
  }

  bool hasParent(const Stmt *S) const { return getParent(S) != nullptr; }

  bool isConsumedExpr(Expr *E) const;

  bool isConsumedExpr(const Expr *E) const {
    return isConsumedExpr(const_cast<Expr*>(E));
  }
};

} // end clang namespace
#endif
