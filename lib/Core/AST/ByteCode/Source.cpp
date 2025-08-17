//===--- Source.cpp - Source expression tracking ----------------*- C++ -*-===//
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

#include "Source.h"
#include "language/Core/AST/Expr.h"

using namespace language::Core;
using namespace language::Core::interp;

SourceLocation SourceInfo::getLoc() const {
  if (const Expr *E = asExpr())
    return E->getExprLoc();
  if (const Stmt *S = asStmt())
    return S->getBeginLoc();
  if (const Decl *D = asDecl())
    return D->getBeginLoc();
  return SourceLocation();
}

SourceRange SourceInfo::getRange() const {
  if (const Expr *E = asExpr())
    return E->getSourceRange();
  if (const Stmt *S = asStmt())
    return S->getSourceRange();
  if (const Decl *D = asDecl())
    return D->getSourceRange();
  return SourceRange();
}

const Expr *SourceInfo::asExpr() const {
  if (const auto *S = dyn_cast_if_present<const Stmt *>(Source))
    return dyn_cast<Expr>(S);
  return nullptr;
}

const Expr *SourceMapper::getExpr(const Function *F, CodePtr PC) const {
  if (const Expr *E = getSource(F, PC).asExpr())
    return E;
  return nullptr;
}

SourceLocation SourceMapper::getLocation(const Function *F, CodePtr PC) const {
  return getSource(F, PC).getLoc();
}

SourceRange SourceMapper::getRange(const Function *F, CodePtr PC) const {
  return getSource(F, PC).getRange();
}
