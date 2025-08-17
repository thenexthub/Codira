//===-- SymbolMap.cpp -----------------------------------------------------===//
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
//
// Pretty printers for symbol boxes, etc.
//
//===----------------------------------------------------------------------===//

#include "language/Compability/Lower/SymbolMap.h"
#include "language/Compability/Optimizer/Builder/Todo.h"
#include "mlir/IR/BuiltinTypes.h"
#include "toolchain/Support/Debug.h"
#include <optional>

#define DEBUG_TYPE "flang-lower-symbol-map"

void language::Compability::lower::SymMap::addSymbol(language::Compability::semantics::SymbolRef sym,
                                       const fir::ExtendedValue &exv,
                                       bool force) {
  exv.match([&](const fir::UnboxedValue &v) { addSymbol(sym, v, force); },
            [&](const fir::CharBoxValue &v) { makeSym(sym, v, force); },
            [&](const fir::ArrayBoxValue &v) { makeSym(sym, v, force); },
            [&](const fir::CharArrayBoxValue &v) { makeSym(sym, v, force); },
            [&](const fir::BoxValue &v) { makeSym(sym, v, force); },
            [&](const fir::MutableBoxValue &v) { makeSym(sym, v, force); },
            [&](const fir::PolymorphicValue &v) { makeSym(sym, v, force); },
            [](auto) {
              toolchain::report_fatal_error("value not added to symbol table");
            });
}

language::Compability::lower::SymbolBox
language::Compability::lower::SymMap::lookupSymbol(language::Compability::semantics::SymbolRef symRef) {
  auto *sym = symRef->HasLocalLocality() ? &*symRef : &symRef->GetUltimate();
  for (auto jmap = symbolMapStack.rbegin(), jend = symbolMapStack.rend();
       jmap != jend; ++jmap) {
    auto iter = jmap->find(sym);
    if (iter != jmap->end())
      return iter->second;
  }
  return SymbolBox::None{};
}

language::Compability::lower::SymbolBox language::Compability::lower::SymMap::shallowLookupSymbol(
    language::Compability::semantics::SymbolRef symRef) {
  auto *sym = symRef->HasLocalLocality() ? &*symRef : &symRef->GetUltimate();
  auto &map = symbolMapStack.back();
  auto iter = map.find(sym);
  if (iter != map.end())
    return iter->second;
  return SymbolBox::None{};
}

/// Skip one level when looking up the symbol. The use case is such as looking
/// up the host variable symbol box by skipping the associated level in
/// host-association in OpenMP code.
language::Compability::lower::SymbolBox language::Compability::lower::SymMap::lookupOneLevelUpSymbol(
    language::Compability::semantics::SymbolRef symRef) {
  auto *sym = symRef->HasLocalLocality() ? &*symRef : &symRef->GetUltimate();
  auto jmap = symbolMapStack.rbegin();
  auto jend = symbolMapStack.rend();
  if (jmap == jend)
    return SymbolBox::None{};
  // Skip one level in symbol map stack.
  for (++jmap; jmap != jend; ++jmap) {
    auto iter = jmap->find(sym);
    if (iter != jmap->end())
      return iter->second;
  }
  return SymbolBox::None{};
}

mlir::Value
language::Compability::lower::SymMap::lookupImpliedDo(language::Compability::lower::SymMap::AcDoVar var) {
  for (auto [marker, binding] : toolchain::reverse(impliedDoStack))
    if (var == marker)
      return binding;
  return {};
}

void language::Compability::lower::SymbolBox::dump() const { toolchain::errs() << *this << '\n'; }

void language::Compability::lower::SymMap::dump() const { toolchain::errs() << *this << '\n'; }

toolchain::raw_ostream &
language::Compability::lower::operator<<(toolchain::raw_ostream &os,
                           const language::Compability::lower::SymbolBox &symBox) {
  symBox.match(
      [&](const language::Compability::lower::SymbolBox::None &box) {
        os << "** symbol not properly mapped **\n";
      },
      [&](const language::Compability::lower::SymbolBox::Intrinsic &val) {
        os << val.getAddr() << '\n';
      },
      [&](const auto &box) { os << box << '\n'; });
  return os;
}

toolchain::raw_ostream &
language::Compability::lower::operator<<(toolchain::raw_ostream &os,
                           const language::Compability::lower::SymMap &symMap) {
  os << "Symbol map:\n";
  for (auto i : toolchain::enumerate(symMap.symbolMapStack)) {
    os << " level " << i.index() << "<{\n";
    for (auto iter : i.value()) {
      os << "  symbol @" << static_cast<const void *>(iter.first) << " ["
         << *iter.first << "] ->\n    ";
      os << iter.second;
    }
    os << " }>\n";
  }
  return os;
}
