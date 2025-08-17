//===-- lib/Semantics/check-deallocate.cpp --------------------------------===//
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

#include "check-deallocate.h"
#include "definable.h"
#include "language/Compability/Evaluate/type.h"
#include "language/Compability/Parser/message.h"
#include "language/Compability/Parser/parse-tree.h"
#include "language/Compability/Semantics/expression.h"
#include "language/Compability/Semantics/tools.h"

namespace language::Compability::semantics {

void DeallocateChecker::Leave(const parser::DeallocateStmt &deallocateStmt) {
  for (const parser::AllocateObject &allocateObject :
      std::get<std::list<parser::AllocateObject>>(deallocateStmt.t)) {
    common::visit(
        common::visitors{
            [&](const parser::Name &name) {
              const Symbol *symbol{
                  name.symbol ? &name.symbol->GetUltimate() : nullptr};
              ;
              if (context_.HasError(symbol)) {
                // already reported an error
              } else if (!IsVariableName(*symbol)) {
                context_.Say(name.source,
                    "Name in DEALLOCATE statement must be a variable name"_err_en_US);
              } else if (!IsAllocatableOrObjectPointer(symbol)) { // C936
                context_.Say(name.source,
                    "Name in DEALLOCATE statement must have the ALLOCATABLE or POINTER attribute"_err_en_US);
              } else if (auto whyNot{WhyNotDefinable(name.source,
                             context_.FindScope(name.source),
                             {DefinabilityFlag::PointerDefinition,
                                 DefinabilityFlag::AcceptAllocatable,
                                 DefinabilityFlag::PotentialDeallocation},
                             *symbol)}) {
                // Catch problems with non-definability of the
                // pointer/allocatable
                context_
                    .Say(name.source,
                        "Name in DEALLOCATE statement is not definable"_err_en_US)
                    .Attach(std::move(
                        whyNot->set_severity(parser::Severity::Because)));
              } else if (auto whyNot{WhyNotDefinable(name.source,
                             context_.FindScope(name.source),
                             DefinabilityFlags{}, *symbol)}) {
                // Catch problems with non-definability of the dynamic object
                context_
                    .Say(name.source,
                        "Object in DEALLOCATE statement is not deallocatable"_err_en_US)
                    .Attach(std::move(
                        whyNot->set_severity(parser::Severity::Because)));
              } else {
                context_.CheckIndexVarRedefine(name);
              }
            },
            [&](const parser::StructureComponent &structureComponent) {
              // Only perform structureComponent checks if it was successfully
              // analyzed by expression analysis.
              auto source{structureComponent.component.source};
              if (const auto *expr{GetExpr(context_, allocateObject)}) {
                if (const Symbol *
                        symbol{structureComponent.component.symbol
                                ? &structureComponent.component.symbol
                                       ->GetUltimate()
                                : nullptr};
                    !IsAllocatableOrObjectPointer(symbol)) { // F'2023 C936
                  context_.Say(source,
                      "Component in DEALLOCATE statement must have the ALLOCATABLE or POINTER attribute"_err_en_US);
                } else if (auto whyNot{WhyNotDefinable(source,
                               context_.FindScope(source),
                               {DefinabilityFlag::PointerDefinition,
                                   DefinabilityFlag::AcceptAllocatable,
                                   DefinabilityFlag::PotentialDeallocation},
                               *expr)}) {
                  context_
                      .Say(source,
                          "Name in DEALLOCATE statement is not definable"_err_en_US)
                      .Attach(std::move(
                          whyNot->set_severity(parser::Severity::Because)));
                } else if (auto whyNot{WhyNotDefinable(source,
                               context_.FindScope(source), DefinabilityFlags{},
                               *expr)}) {
                  context_
                      .Say(source,
                          "Object in DEALLOCATE statement is not deallocatable"_err_en_US)
                      .Attach(std::move(
                          whyNot->set_severity(parser::Severity::Because)));
                } else if (evaluate::ExtractCoarrayRef(*expr)) { // F'2023 C955
                  context_.Say(source,
                      "Component in DEALLOCATE statement may not be coindexed"_err_en_US);
                }
              }
            },
        },
        allocateObject.u);
  }
  bool gotStat{false}, gotMsg{false};
  for (const parser::StatOrErrmsg &deallocOpt :
      std::get<std::list<parser::StatOrErrmsg>>(deallocateStmt.t)) {
    common::visit(
        common::visitors{
            [&](const parser::StatVariable &) {
              if (gotStat) {
                context_.Say(
                    "STAT may not be duplicated in a DEALLOCATE statement"_err_en_US);
              }
              gotStat = true;
            },
            [&](const parser::MsgVariable &var) {
              WarnOnDeferredLengthCharacterScalar(context_,
                  GetExpr(context_, var), var.v.thing.thing.GetSource(),
                  "ERRMSG=");
              if (gotMsg) {
                context_.Say(
                    "ERRMSG may not be duplicated in a DEALLOCATE statement"_err_en_US);
              }
              gotMsg = true;
            },
        },
        deallocOpt.u);
  }
}

} // namespace language::Compability::semantics
