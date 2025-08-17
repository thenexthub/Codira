//===----- SemaPseudoObject.h --- Semantic Analysis for Pseudo-Objects ----===//
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
/// \file
/// This file declares semantic analysis for expressions involving
//  pseudo-object references.
///
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_SEMA_SEMAPSEUDOOBJECT_H
#define LANGUAGE_CORE_SEMA_SEMAPSEUDOOBJECT_H

#include "language/Core/AST/ASTFwd.h"
#include "language/Core/AST/OperationKinds.h"
#include "language/Core/Basic/SourceLocation.h"
#include "language/Core/Sema/Ownership.h"
#include "language/Core/Sema/SemaBase.h"

namespace language::Core {
class Scope;

class SemaPseudoObject : public SemaBase {
public:
  SemaPseudoObject(Sema &S);

  ExprResult checkIncDec(Scope *S, SourceLocation OpLoc,
                         UnaryOperatorKind Opcode, Expr *Op);
  ExprResult checkAssignment(Scope *S, SourceLocation OpLoc,
                             BinaryOperatorKind Opcode, Expr *LHS, Expr *RHS);
  ExprResult checkRValue(Expr *E);
  Expr *recreateSyntacticForm(PseudoObjectExpr *E);
};

} // namespace language::Core

#endif // LANGUAGE_CORE_SEMA_SEMAPSEUDOOBJECT_H
