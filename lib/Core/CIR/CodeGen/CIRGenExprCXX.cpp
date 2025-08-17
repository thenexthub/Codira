//===--- CIRGenExprCXX.cpp - Emit CIR Code for C++ expressions ------------===//
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
// This contains code dealing with code generation of C++ expressions
//
//===----------------------------------------------------------------------===//

#include "CIRGenFunction.h"
#include "language/Core/AST/ExprCXX.h"

using namespace language::Core;
using namespace language::Core::CIRGen;

RValue CIRGenFunction::emitCXXPseudoDestructorExpr(
    const CXXPseudoDestructorExpr *expr) {
  QualType destroyedType = expr->getDestroyedType();
  if (destroyedType.hasStrongOrWeakObjCLifetime()) {
    assert(!cir::MissingFeatures::objCLifetime());
    cgm.errorNYI(expr->getExprLoc(),
                 "emitCXXPseudoDestructorExpr: Objective-C lifetime is NYI");
  } else {
    // C++ [expr.pseudo]p1:
    //   The result shall only be used as the operand for the function call
    //   operator (), and the result of such a call has type void. The only
    //   effect is the evaluation of the postfix-expression before the dot or
    //   arrow.
    emitIgnoredExpr(expr->getBase());
  }

  return RValue::get(nullptr);
}
