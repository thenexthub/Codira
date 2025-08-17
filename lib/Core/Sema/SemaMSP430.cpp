//===------ SemaMSP430.cpp ----- MSP430 target-specific routines ----------===//
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
//  This file implements semantic analysis functions specific to NVPTX.
//
//===----------------------------------------------------------------------===//

#include "language/Core/Sema/SemaMSP430.h"
#include "language/Core/AST/ASTContext.h"
#include "language/Core/AST/Attr.h"
#include "language/Core/AST/DeclBase.h"
#include "language/Core/Basic/DiagnosticSema.h"
#include "language/Core/Sema/Attr.h"
#include "language/Core/Sema/ParsedAttr.h"

namespace language::Core {

SemaMSP430::SemaMSP430(Sema &S) : SemaBase(S) {}

void SemaMSP430::handleInterruptAttr(Decl *D, const ParsedAttr &AL) {
  // MSP430 'interrupt' attribute is applied to
  // a function with no parameters and void return type.
  if (!isFuncOrMethodForAttrSubject(D)) {
    Diag(D->getLocation(), diag::warn_attribute_wrong_decl_type)
        << AL << AL.isRegularKeywordAttribute() << ExpectedFunctionOrMethod;
    return;
  }

  if (hasFunctionProto(D) && getFunctionOrMethodNumParams(D) != 0) {
    Diag(D->getLocation(), diag::warn_interrupt_signal_attribute_invalid)
        << /*MSP430*/ 1 << /*interrupt*/ 0 << 0;
    return;
  }

  if (!getFunctionOrMethodResultType(D)->isVoidType()) {
    Diag(D->getLocation(), diag::warn_interrupt_signal_attribute_invalid)
        << /*MSP430*/ 1 << /*interrupt*/ 0 << 1;
    return;
  }

  // The attribute takes one integer argument.
  if (!AL.checkExactlyNumArgs(SemaRef, 1))
    return;

  if (!AL.isArgExpr(0)) {
    Diag(AL.getLoc(), diag::err_attribute_argument_type)
        << AL << AANT_ArgumentIntegerConstant;
    return;
  }

  Expr *NumParamsExpr = AL.getArgAsExpr(0);
  std::optional<toolchain::APSInt> NumParams = toolchain::APSInt(32);
  if (!(NumParams = NumParamsExpr->getIntegerConstantExpr(getASTContext()))) {
    Diag(AL.getLoc(), diag::err_attribute_argument_type)
        << AL << AANT_ArgumentIntegerConstant
        << NumParamsExpr->getSourceRange();
    return;
  }
  // The argument should be in range 0..63.
  unsigned Num = NumParams->getLimitedValue(255);
  if (Num > 63) {
    Diag(AL.getLoc(), diag::err_attribute_argument_out_of_bounds)
        << AL << (int)NumParams->getSExtValue()
        << NumParamsExpr->getSourceRange();
    return;
  }

  D->addAttr(::new (getASTContext())
                 MSP430InterruptAttr(getASTContext(), AL, Num));
  D->addAttr(UsedAttr::CreateImplicit(getASTContext()));
}

} // namespace language::Core
