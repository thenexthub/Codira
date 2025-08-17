//===------ SemaM68k.cpp -------- M68k target-specific routines -----------===//
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
//  This file implements semantic analysis functions specific to M68k.
//
//===----------------------------------------------------------------------===//

#include "language/Core/Sema/SemaM68k.h"
#include "language/Core/AST/ASTContext.h"
#include "language/Core/AST/Attr.h"
#include "language/Core/AST/DeclBase.h"
#include "language/Core/Basic/DiagnosticSema.h"
#include "language/Core/Sema/ParsedAttr.h"

namespace language::Core {
SemaM68k::SemaM68k(Sema &S) : SemaBase(S) {}

void SemaM68k::handleInterruptAttr(Decl *D, const ParsedAttr &AL) {
  if (!AL.checkExactlyNumArgs(SemaRef, 1))
    return;

  if (!AL.isArgExpr(0)) {
    Diag(AL.getLoc(), diag::err_attribute_argument_type)
        << AL << AANT_ArgumentIntegerConstant;
    return;
  }

  // FIXME: Check for decl - it should be void ()(void).

  Expr *NumParamsExpr = AL.getArgAsExpr(0);
  auto MaybeNumParams = NumParamsExpr->getIntegerConstantExpr(getASTContext());
  if (!MaybeNumParams) {
    Diag(AL.getLoc(), diag::err_attribute_argument_type)
        << AL << AANT_ArgumentIntegerConstant
        << NumParamsExpr->getSourceRange();
    return;
  }

  unsigned Num = MaybeNumParams->getLimitedValue(255);
  if ((Num & 1) || Num > 30) {
    Diag(AL.getLoc(), diag::err_attribute_argument_out_of_bounds)
        << AL << (int)MaybeNumParams->getSExtValue()
        << NumParamsExpr->getSourceRange();
    return;
  }

  D->addAttr(::new (getASTContext())
                 M68kInterruptAttr(getASTContext(), AL, Num));
  D->addAttr(UsedAttr::CreateImplicit(getASTContext()));
}
} // namespace language::Core
