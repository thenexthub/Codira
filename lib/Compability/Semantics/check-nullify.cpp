//===-- lib/Semantics/check-nullify.cpp -----------------------------------===//
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

#include "check-nullify.h"
#include "definable.h"
#include "language/Compability/Evaluate/expression.h"
#include "language/Compability/Parser/message.h"
#include "language/Compability/Parser/parse-tree.h"
#include "language/Compability/Semantics/expression.h"
#include "language/Compability/Semantics/tools.h"

namespace language::Compability::semantics {

void NullifyChecker::Leave(const parser::NullifyStmt &nullifyStmt) {
  CHECK(context_.location());
  const Scope &scope{context_.FindScope(*context_.location())};
  for (const parser::PointerObject &pointerObject : nullifyStmt.v) {
    common::visit(
        common::visitors{
            [&](const parser::Name &name) {
              if (name.symbol) {
                if (auto whyNot{WhyNotDefinable(name.source, scope,
                        DefinabilityFlags{DefinabilityFlag::PointerDefinition},
                        *name.symbol)}) {
                  context_.messages()
                      .Say(name.source,
                          "'%s' may not appear in NULLIFY"_err_en_US,
                          name.source)
                      .Attach(std::move(
                          whyNot->set_severity(parser::Severity::Because)));
                }
              }
            },
            [&](const parser::StructureComponent &structureComponent) {
              const auto &component{structureComponent.component};
              SourceName at{component.source};
              if (const auto *checkedExpr{GetExpr(context_, pointerObject)}) {
                if (auto whyNot{WhyNotDefinable(at, scope,
                        DefinabilityFlags{DefinabilityFlag::PointerDefinition},
                        *checkedExpr)}) {
                  context_.messages()
                      .Say(at, "'%s' may not appear in NULLIFY"_err_en_US, at)
                      .Attach(std::move(
                          whyNot->set_severity(parser::Severity::Because)));
                }
              }
            },
        },
        pointerObject.u);
  }
  // From 9.7.3.1(1)
  //   A pointer-object shall not depend on the value,
  //   bounds, or association status of another pointer-
  //   object in the same NULLIFY statement.
  // This restriction is the programmer's responsibility.
  // Some dependencies can be found compile time or at
  // runtime, but for now we choose to skip such checks.
}
} // namespace language::Compability::semantics
