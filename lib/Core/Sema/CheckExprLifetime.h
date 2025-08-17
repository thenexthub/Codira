//===- CheckExprLifetime.h -----------------------------------  -*- C++ -*-===//
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
//  This files implements a statement-local lifetime analysis.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_SEMA_CHECK_EXPR_LIFETIME_H
#define LANGUAGE_CORE_SEMA_CHECK_EXPR_LIFETIME_H

#include "language/Core/AST/Expr.h"
#include "language/Core/Sema/Initialization.h"
#include "language/Core/Sema/Sema.h"

namespace language::Core::sema {

// Tells whether the type is annotated with [[gsl::Pointer]].
bool isGLSPointerType(QualType QT);

/// Describes an entity that is being assigned.
struct AssignedEntity {
  // The left-hand side expression of the assignment.
  Expr *LHS = nullptr;
  CXXMethodDecl *AssignmentOperator = nullptr;
};

struct CapturingEntity {
  // In an function call involving a lifetime capture, this would be the
  // argument capturing the lifetime of another argument.
  //    void addToSet(std::string_view sv [[language::Core::lifetime_capture_by(setsv)]],
  //                  set<std::string_view>& setsv);
  //    set<std::string_view> setsv;
  //    addToSet(std::string(), setsv); // Here 'setsv' is the 'Entity'.
  //
  // This is 'nullptr' when the capturing entity is 'global' or 'unknown'.
  Expr *Entity = nullptr;
};

/// Check that the lifetime of the given expr (and its subobjects) is
/// sufficient for initializing the entity, and perform lifetime extension
/// (when permitted) if not.
void checkInitLifetime(Sema &SemaRef, const InitializedEntity &Entity,
                       Expr *Init);

/// Check that the lifetime of the given expr (and its subobjects) is
/// sufficient for assigning to the entity.
void checkAssignmentLifetime(Sema &SemaRef, const AssignedEntity &Entity,
                             Expr *Init);

void checkCaptureByLifetime(Sema &SemaRef, const CapturingEntity &Entity,
                            Expr *Init);

/// Check that the lifetime of the given expr (and its subobjects) is
/// sufficient, assuming that it is passed as an argument to a musttail
/// function.
void checkExprLifetimeMustTailArg(Sema &SemaRef,
                                  const InitializedEntity &Entity, Expr *Init);

bool implicitObjectParamIsLifetimeBound(const FunctionDecl *FD);

} // namespace language::Core::sema

#endif // LANGUAGE_CORE_SEMA_CHECK_EXPR_LIFETIME_H
