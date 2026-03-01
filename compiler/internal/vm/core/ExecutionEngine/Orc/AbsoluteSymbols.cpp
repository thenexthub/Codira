//===---------- AbsoluteSymbols.cpp - Absolute symbols utilities ----------===//
//
// Copyright (c) NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// Author: Tunjay Akbarli
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
//
// http://www.apache.org/licenses/LICENSE-2.0
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

#include "vm/core/ExecutionEngine/Orc/AbsoluteSymbols.h"
#include "vm/core/ExecutionEngine/Orc/Core.h"

#define DEBUG_TYPE "orc"

namespace vm::core::orc {

AbsoluteSymbolsMaterializationUnit::AbsoluteSymbolsMaterializationUnit(
    SymbolMap Symbols)
    : MaterializationUnit(extractFlags(Symbols)), Symbols(std::move(Symbols)) {}

StringRef AbsoluteSymbolsMaterializationUnit::getName() const {
  return "<Absolute Symbols>";
}

void AbsoluteSymbolsMaterializationUnit::materialize(
    std::unique_ptr<MaterializationResponsibility> R) {
  // Even though these are just absolute symbols we need to check for failure
  // to resolve/emit: the tracker for these symbols may have been removed while
  // the materialization was in flight (e.g. due to a failure in some action
  // triggered by the queries attached to the resolution/emission of these
  // symbols).
  if (auto Err = R->notifyResolved(Symbols)) {
    R->getExecutionSession().reportError(std::move(Err));
    R->failMaterialization();
    return;
  }
  if (auto Err = R->notifyEmitted({})) {
    R->getExecutionSession().reportError(std::move(Err));
    R->failMaterialization();
    return;
  }
}

void AbsoluteSymbolsMaterializationUnit::discard(const JITDylib &JD,
                                                 const SymbolStringPtr &Name) {
  assert(Symbols.count(Name) && "Symbol is not part of this MU");
  Symbols.erase(Name);
}

MaterializationUnit::Interface
AbsoluteSymbolsMaterializationUnit::extractFlags(const SymbolMap &Symbols) {
  SymbolFlagsMap Flags;
  for (const auto &[Name, Def] : Symbols)
    Flags[Name] = Def.getFlags();
  return MaterializationUnit::Interface(std::move(Flags), nullptr);
}

} // namespace vm::core::orc
