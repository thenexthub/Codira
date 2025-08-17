//===-- lib/Semantics/check-select-rank.cpp -------------------------------===//
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

#include "check-select-rank.h"
#include "language/Compability/Common/idioms.h"
#include "language/Compability/Parser/message.h"
#include "language/Compability/Parser/tools.h"
#include "language/Compability/Semantics/tools.h"
#include "language/Compability/Support/Fortran.h"
#include <list>
#include <optional>
#include <set>
#include <tuple>
#include <variant>

namespace language::Compability::semantics {

void SelectRankConstructChecker::Leave(
    const parser::SelectRankConstruct &selectRankConstruct) {
  const auto &selectRankStmt{
      std::get<parser::Statement<parser::SelectRankStmt>>(
          selectRankConstruct.t)};
  const auto &selectRankStmtSel{
      std::get<parser::Selector>(selectRankStmt.statement.t)};

  // R1149 select-rank-stmt checks
  const Symbol *saveSelSymbol{nullptr};
  if (const auto selExpr{GetExprFromSelector(selectRankStmtSel)}) {
    if (const Symbol * sel{evaluate::UnwrapWholeSymbolDataRef(*selExpr)}) {
      if (!evaluate::IsAssumedRank(*sel)) { // C1150
        context_.Say(parser::FindSourceLocation(selectRankStmtSel),
            "Selector '%s' is not an assumed-rank array variable"_err_en_US,
            sel->name().ToString());
      } else {
        saveSelSymbol = sel;
      }
    } else {
      context_.Say(parser::FindSourceLocation(selectRankStmtSel),
          "Selector '%s' is not an assumed-rank array variable"_err_en_US,
          parser::FindSourceLocation(selectRankStmtSel).ToString());
    }
  }

  // R1150 select-rank-case-stmt checks
  auto &rankCaseList{std::get<std::list<parser::SelectRankConstruct::RankCase>>(
      selectRankConstruct.t)};
  bool defaultRankFound{false};
  bool starRankFound{false};
  parser::CharBlock prevLocDefault;
  parser::CharBlock prevLocStar;
  std::optional<parser::CharBlock> caseForRank[common::maxRank + 1];

  for (const auto &rankCase : rankCaseList) {
    const auto &rankCaseStmt{
        std::get<parser::Statement<parser::SelectRankCaseStmt>>(rankCase.t)};
    const auto &rank{
        std::get<parser::SelectRankCaseStmt::Rank>(rankCaseStmt.statement.t)};
    common::visit(
        common::visitors{
            [&](const parser::Default &) { // C1153
              if (!defaultRankFound) {
                defaultRankFound = true;
                prevLocDefault = rankCaseStmt.source;
              } else {
                context_
                    .Say(rankCaseStmt.source,
                        "Not more than one of the selectors of SELECT RANK "
                        "statement may be DEFAULT"_err_en_US)
                    .Attach(prevLocDefault, "Previous use"_en_US);
              }
            },
            [&](const parser::Star &) { // C1153
              if (!starRankFound) {
                starRankFound = true;
                prevLocStar = rankCaseStmt.source;
              } else {
                context_
                    .Say(rankCaseStmt.source,
                        "Not more than one of the selectors of SELECT RANK "
                        "statement may be '*'"_err_en_US)
                    .Attach(prevLocStar, "Previous use"_en_US);
              }
              if (saveSelSymbol &&
                  IsAllocatableOrPointer(*saveSelSymbol)) { // F'2023 C1160
                context_.Say(rankCaseStmt.source,
                    "RANK (*) cannot be used when selector is "
                    "POINTER or ALLOCATABLE"_err_en_US);
              }
            },
            [&](const parser::ScalarIntConstantExpr &init) {
              if (auto val{GetIntValue(init)}) {
                // If value is in valid range, then only show
                // value repeat error, else stack smashing occurs
                if (*val < 0 || *val > common::maxRank) { // C1151
                  context_.Say(rankCaseStmt.source,
                      "The value of the selector must be "
                      "between zero and %d"_err_en_US,
                      common::maxRank);

                } else {
                  if (!caseForRank[*val].has_value()) {
                    caseForRank[*val] = rankCaseStmt.source;
                  } else {
                    auto prevloc{caseForRank[*val].value()};
                    context_
                        .Say(rankCaseStmt.source,
                            "Same rank value (%d) not allowed more than once"_err_en_US,
                            *val)
                        .Attach(prevloc, "Previous use"_en_US);
                  }
                }
              }
            },
        },
        rank.u);
  }
}

const SomeExpr *SelectRankConstructChecker::GetExprFromSelector(
    const parser::Selector &selector) {
  return common::visit([](const auto &x) { return GetExpr(x); }, selector.u);
}

} // namespace language::Compability::semantics
