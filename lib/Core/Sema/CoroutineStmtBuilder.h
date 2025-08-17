//===- CoroutineStmtBuilder.h - Implicit coroutine stmt builder -*- C++ -*-===//
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
//===----------------------------------------------------------------------===//
//
//  This file defines CoroutineStmtBuilder, a class for building the implicit
//  statements required for building a coroutine body.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_LIB_SEMA_COROUTINESTMTBUILDER_H
#define LANGUAGE_CORE_LIB_SEMA_COROUTINESTMTBUILDER_H

#include "language/Core/AST/Decl.h"
#include "language/Core/AST/ExprCXX.h"
#include "language/Core/AST/StmtCXX.h"
#include "language/Core/Lex/Preprocessor.h"
#include "language/Core/Sema/SemaInternal.h"

namespace language::Core {

class CoroutineStmtBuilder : public CoroutineBodyStmt::CtorArgs {
  Sema &S;
  FunctionDecl &FD;
  sema::FunctionScopeInfo &Fn;
  bool IsValid = true;
  SourceLocation Loc;
  SmallVector<Stmt *, 4> ParamMovesVector;
  const bool IsPromiseDependentType;
  CXXRecordDecl *PromiseRecordDecl = nullptr;

public:
  /// Construct a CoroutineStmtBuilder and initialize the promise
  /// statement and initial/final suspends from the FunctionScopeInfo.
  CoroutineStmtBuilder(Sema &S, FunctionDecl &FD, sema::FunctionScopeInfo &Fn,
                       Stmt *Body);

  /// Build the coroutine body statements, including the
  /// "promise dependent" statements when the promise type is not dependent.
  bool buildStatements();

  /// Build the coroutine body statements that require a non-dependent
  /// promise type in order to construct.
  ///
  /// For example different new/delete overloads are selected depending on
  /// if the promise type provides `unhandled_exception()`, and therefore they
  /// cannot be built until the promise type is complete so that we can perform
  /// name lookup.
  bool buildDependentStatements();

  bool isInvalid() const { return !this->IsValid; }

private:
  bool makePromiseStmt();
  bool makeInitialAndFinalSuspend();
  bool makeNewAndDeleteExpr();
  bool makeOnFallthrough();
  bool makeOnException();
  bool makeReturnObject();
  bool makeGroDeclAndReturnStmt();
  bool makeReturnOnAllocFailure();
};

} // end namespace language::Core

#endif // LANGUAGE_CORE_LIB_SEMA_COROUTINESTMTBUILDER_H
