//===-- lib/Semantics/check-return.cpp ------------------------------------===//
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

#include "check-return.h"
#include "language/Compability/Parser/message.h"
#include "language/Compability/Parser/parse-tree.h"
#include "language/Compability/Semantics/semantics.h"
#include "language/Compability/Semantics/tools.h"
#include "language/Compability/Support/Fortran-features.h"

namespace language::Compability::semantics {

static const Scope *FindContainingSubprogram(const Scope &start) {
  const Scope &scope{GetProgramUnitContaining(start)};
  return scope.kind() == Scope::Kind::MainProgram ||
          scope.kind() == Scope::Kind::Subprogram
      ? &scope
      : nullptr;
}

void ReturnStmtChecker::Leave(const parser::ReturnStmt &returnStmt) {
  // R1542 Expression analysis validates the scalar-int-expr
  // C1574 The return-stmt shall be in the inclusive scope of a function or
  // subroutine subprogram.
  // C1575 The scalar-int-expr is allowed only in the inclusive scope of a
  // subroutine subprogram.
  const auto &scope{context_.FindScope(context_.location().value())};
  if (const auto *subprogramScope{FindContainingSubprogram(scope)}) {
    if (returnStmt.v &&
        (subprogramScope->kind() == Scope::Kind::MainProgram ||
            IsFunction(*subprogramScope->GetSymbol()))) {
      context_.Say(
          "RETURN with expression is only allowed in SUBROUTINE subprogram"_err_en_US);
    } else if (subprogramScope->kind() == Scope::Kind::MainProgram) {
      context_.Warn(common::LanguageFeature::ProgramReturn,
          "RETURN should not appear in a main program"_port_en_US);
    }
  }
}

} // namespace language::Compability::semantics
