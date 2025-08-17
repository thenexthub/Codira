//===-- lib/Semantics/check-arithmeticif.cpp ------------------------------===//
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

#include "check-arithmeticif.h"
#include "language/Compability/Parser/message.h"
#include "language/Compability/Parser/parse-tree.h"
#include "language/Compability/Semantics/tools.h"

namespace language::Compability::semantics {

bool IsNumericExpr(const SomeExpr &expr) {
  auto dynamicType{expr.GetType()};
  return dynamicType && common::IsNumericTypeCategory(dynamicType->category());
}

void ArithmeticIfStmtChecker::Leave(
    const parser::ArithmeticIfStmt &arithmeticIfStmt) {
  // Arithmetic IF statements have been removed from Fortran 2018.
  // The constraints and requirements here refer to the 2008 spec.
  // R853 Check for a scalar-numeric-expr
  // C849 that shall not be of type complex.
  auto &parsedExpr{std::get<parser::Expr>(arithmeticIfStmt.t)};
  if (const auto *expr{GetExpr(context_, parsedExpr)}) {
    if (expr->Rank() > 0) {
      context_.Say(parsedExpr.source,
          "ARITHMETIC IF expression must be a scalar expression"_err_en_US);
    } else if (ExprHasTypeCategory(*expr, common::TypeCategory::Complex)) {
      context_.Say(parsedExpr.source,
          "ARITHMETIC IF expression must not be a COMPLEX expression"_err_en_US);
    } else if (ExprHasTypeCategory(*expr, common::TypeCategory::Unsigned)) {
      context_.Say(parsedExpr.source,
          "ARITHMETIC IF expression must not be an UNSIGNED expression"_err_en_US);
    } else if (!IsNumericExpr(*expr)) {
      context_.Say(parsedExpr.source,
          "ARITHMETIC IF expression must be a numeric expression"_err_en_US);
    }
  }
  // The labels have already been checked in resolve-labels.
  // TODO: Really?  Check that they are really branch target
  // statements and in the same inclusive scope.
}

} // namespace language::Compability::semantics
