//===-- lib/Semantics/check-if-stmt.cpp -----------------------------------===//
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

#include "check-if-stmt.h"
#include "language/Compability/Parser/message.h"
#include "language/Compability/Parser/parse-tree.h"
#include "language/Compability/Semantics/tools.h"

namespace language::Compability::semantics {

void IfStmtChecker::Leave(const parser::IfStmt &ifStmt) {
  // C1143 Check that the action stmt is not an if stmt
  const auto &body{
      std::get<parser::UnlabeledStatement<parser::ActionStmt>>(ifStmt.t)};
  if (std::holds_alternative<common::Indirection<parser::IfStmt>>(
          body.statement.u)) {
    context_.Say(
        body.source, "IF statement is not allowed in IF statement"_err_en_US);
  }
}

} // namespace language::Compability::semantics
