//===-- lib/Semantics/check-namelist.cpp ----------------------------------===//
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

#include "check-namelist.h"

namespace language::Compability::semantics {

void NamelistChecker::Leave(const parser::NamelistStmt &nmlStmt) {
  for (const auto &x : nmlStmt.v) {
    if (const auto *nml{std::get<parser::Name>(x.t).symbol}) {
      for (const auto &nmlObjName : std::get<std::list<parser::Name>>(x.t)) {
        const auto *nmlObjSymbol{nmlObjName.symbol};
        if (nmlObjSymbol) {
          if (IsAssumedSizeArray(*nmlObjSymbol)) { // C8104
            context_.Say(nmlObjName.source,
                "A namelist group object '%s' must not be assumed-size"_err_en_US,
                nmlObjSymbol->name());
          }
          if (nml->attrs().test(Attr::PUBLIC) &&
              nmlObjSymbol->attrs().test(Attr::PRIVATE)) { // C8105
            context_.Say(nmlObjName.source,
                "A PRIVATE namelist group object '%s' must not be in a "
                "PUBLIC namelist"_err_en_US,
                nmlObjSymbol->name());
          }
        }
      }
    }
  }
}

void NamelistChecker::Leave(const parser::LocalitySpec::Reduce &x) {
  for (const parser::Name &name : std::get<std::list<parser::Name>>(x.t)) {
    Symbol *sym{name.symbol};
    // This is not disallowed by the standard, but would be difficult to
    // support. This has to go here not with the other checks for locality specs
    // in resolve-names.cpp so that it is done after the InNamelist flag is
    // applied.
    if (sym && sym->GetUltimate().test(Symbol::Flag::InNamelist)) {
      context_.Say(name.source,
          "NAMELIST variable '%s' not allowed in a REDUCE locality-spec"_err_en_US,
          name.ToString());
    }
  }
}

} // namespace language::Compability::semantics
