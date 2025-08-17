//===-- lib/Parser/user-state.cpp -----------------------------------------===//
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

#include "language/Compability/Parser/user-state.h"
#include "stmt-parser.h"
#include "type-parsers.h"
#include "language/Compability/Parser/parse-state.h"
#include <optional>

namespace language::Compability::parser {

std::optional<Success> StartNewSubprogram::Parse(ParseState &state) {
  if (auto *ustate{state.userState()}) {
    ustate->NewSubprogram();
  }
  return Success{};
}

std::optional<CapturedLabelDoStmt::resultType> CapturedLabelDoStmt::Parse(
    ParseState &state) {
  static constexpr auto parser{statement(indirect(Parser<LabelDoStmt>{}))};
  auto result{parser.Parse(state)};
  if (result) {
    if (auto *ustate{state.userState()}) {
      ustate->NewDoLabel(std::get<Label>(result->statement.value().t));
    }
  }
  return result;
}

std::optional<EndDoStmtForCapturedLabelDoStmt::resultType>
EndDoStmtForCapturedLabelDoStmt::Parse(ParseState &state) {
  static constexpr auto parser{
      statement(indirect(construct<EndDoStmt>("END DO" >> maybe(name))))};
  if (auto enddo{parser.Parse(state)}) {
    if (enddo->label) {
      if (const auto *ustate{state.userState()}) {
        if (ustate->IsDoLabel(enddo->label.value())) {
          return enddo;
        }
      }
    }
  }
  return std::nullopt;
}

std::optional<Success> EnterNonlabelDoConstruct::Parse(ParseState &state) {
  if (auto *ustate{state.userState()}) {
    ustate->EnterNonlabelDoConstruct();
  }
  return {Success{}};
}

std::optional<Success> LeaveDoConstruct::Parse(ParseState &state) {
  if (auto ustate{state.userState()}) {
    ustate->LeaveDoConstruct();
  }
  return {Success{}};
}

// These special parsers for bits of DEC STRUCTURE capture the names of
// their components and nested structures in the user state so that
// references to these fields with periods can be recognized as special
// cases.

std::optional<Name> OldStructureComponentName::Parse(ParseState &state) {
  if (std::optional<Name> n{name.Parse(state)}) {
    if (const auto *ustate{state.userState()}) {
      if (ustate->IsOldStructureComponent(n->source)) {
        return n;
      }
    }
  }
  return std::nullopt;
}

std::optional<DataComponentDefStmt> StructureComponents::Parse(
    ParseState &state) {
  static constexpr auto stmt{Parser<DataComponentDefStmt>{}};
  std::optional<DataComponentDefStmt> defs{stmt.Parse(state)};
  if (defs) {
    if (auto *ustate{state.userState()}) {
      for (const auto &item : std::get<std::list<ComponentOrFill>>(defs->t)) {
        if (const auto *decl{std::get_if<ComponentDecl>(&item.u)}) {
          ustate->NoteOldStructureComponent(std::get<Name>(decl->t).source);
        }
      }
    }
  }
  return defs;
}

std::optional<StructureStmt> NestedStructureStmt::Parse(ParseState &state) {
  std::optional<StructureStmt> stmt{Parser<StructureStmt>{}.Parse(state)};
  if (stmt) {
    if (auto *ustate{state.userState()}) {
      for (const auto &entity : std::get<std::list<EntityDecl>>(stmt->t)) {
        ustate->NoteOldStructureComponent(std::get<Name>(entity.t).source);
      }
    }
  }
  return stmt;
}
} // namespace language::Compability::parser
