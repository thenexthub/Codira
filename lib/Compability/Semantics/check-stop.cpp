//===-- lib/Semantics/check-stop.cpp --------------------------------------===//
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

#include "check-stop.h"
#include "language/Compability/Evaluate/expression.h"
#include "language/Compability/Parser/parse-tree.h"
#include "language/Compability/Semantics/semantics.h"
#include "language/Compability/Semantics/tools.h"
#include "language/Compability/Support/Fortran.h"
#include <optional>

namespace language::Compability::semantics {

void StopChecker::Enter(const parser::StopStmt &stmt) {
  const auto &stopCode{std::get<std::optional<parser::StopCode>>(stmt.t)};
  if (const auto *expr{GetExpr(context_, stopCode)}) {
    const parser::CharBlock &source{parser::FindSourceLocation(stopCode)};
    if (ExprHasTypeCategory(*expr, common::TypeCategory::Integer)) {
      // C1171 default kind
      if (!ExprTypeKindIsDefault(*expr, context_)) {
        context_.Say(
            source, "INTEGER stop code must be of default kind"_err_en_US);
      }
    } else if (ExprHasTypeCategory(*expr, common::TypeCategory::Character)) {
      // R1162 spells scalar-DEFAULT-char-expr
      if (!ExprTypeKindIsDefault(*expr, context_)) {
        context_.Say(
            source, "CHARACTER stop code must be of default kind"_err_en_US);
      }
    } else {
      context_.Say(
          source, "Stop code must be of INTEGER or CHARACTER type"_err_en_US);
    }
  }
}

} // namespace language::Compability::semantics
