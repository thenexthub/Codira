//===-- lib/Semantics/check-purity.cpp ------------------------------------===//
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

#include "check-purity.h"
#include "language/Compability/Parser/parse-tree.h"
#include "language/Compability/Semantics/tools.h"

namespace language::Compability::semantics {
void PurityChecker::Enter(const parser::ExecutableConstruct &exec) {
  if (InPureSubprogram() && IsImageControlStmt(exec)) {
    context_.Say(GetImageControlStmtLocation(exec),
        "An image control statement may not appear in a pure subprogram"_err_en_US);
  }
}
void PurityChecker::Enter(const parser::SubroutineSubprogram &subr) {
  const auto &stmt{std::get<parser::Statement<parser::SubroutineStmt>>(subr.t)};
  Entered(
      stmt.source, std::get<std::list<parser::PrefixSpec>>(stmt.statement.t));
}

void PurityChecker::Leave(const parser::SubroutineSubprogram &) { Left(); }

void PurityChecker::Enter(const parser::FunctionSubprogram &func) {
  const auto &stmt{std::get<parser::Statement<parser::FunctionStmt>>(func.t)};
  Entered(
      stmt.source, std::get<std::list<parser::PrefixSpec>>(stmt.statement.t));
}

void PurityChecker::Leave(const parser::FunctionSubprogram &func) { Left(); }

bool PurityChecker::InPureSubprogram() const {
  return pureDepth_ >= 0 && depth_ >= pureDepth_;
}

bool PurityChecker::HasPurePrefix(
    const std::list<parser::PrefixSpec> &prefixes) const {
  bool result{false};
  for (const parser::PrefixSpec &prefix : prefixes) {
    if (std::holds_alternative<parser::PrefixSpec::Impure>(prefix.u)) {
      return false;
    } else if (std::holds_alternative<parser::PrefixSpec::Pure>(prefix.u) ||
        std::holds_alternative<parser::PrefixSpec::Elemental>(prefix.u)) {
      result = true;
    }
  }
  return result;
}

void PurityChecker::Entered(
    parser::CharBlock source, const std::list<parser::PrefixSpec> &prefixes) {
  if (depth_ == 2) {
    context_.messages().Say(source,
        "An internal subprogram may not contain an internal subprogram"_err_en_US);
  }
  if (HasPurePrefix(prefixes)) {
    if (pureDepth_ < 0) {
      pureDepth_ = depth_;
    }
  } else if (InPureSubprogram()) {
    context_.messages().Say(source,
        "An internal subprogram of a pure subprogram must also be pure"_err_en_US);
  }
  ++depth_;
}

void PurityChecker::Left() {
  if (pureDepth_ == --depth_) {
    pureDepth_ = -1;
  }
}

} // namespace language::Compability::semantics
