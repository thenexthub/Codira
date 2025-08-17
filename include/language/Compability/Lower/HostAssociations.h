//===-- Lower/HostAssociations.h --------------------------------*- C++ -*-===//
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

#ifndef LANGUAGE_COMPABILITY_LOWER_HOSTASSOCIATIONS_H
#define LANGUAGE_COMPABILITY_LOWER_HOSTASSOCIATIONS_H

#include "mlir/IR/Location.h"
#include "mlir/IR/Types.h"
#include "mlir/IR/Value.h"
#include "toolchain/ADT/SetVector.h"

namespace language::Compability {
namespace semantics {
class Symbol;
class Scope;
} // namespace semantics

namespace lower {
class AbstractConverter;
class SymMap;

/// Internal procedures in Fortran may access variables declared in the host
/// procedure directly. We bundle these variables together in a tuple and pass
/// them as an extra argument.
class HostAssociations {
public:
  /// Returns true iff there are no host associations.
  bool empty() const { return tupleSymbols.empty() && globalSymbols.empty(); }

  /// Returns true iff there are host associations that are conveyed through
  /// an extra tuple argument.
  bool hasTupleAssociations() const { return !tupleSymbols.empty(); }

  /// Adds a set of Symbols that will be the host associated bindings for this
  /// host procedure.
  void addSymbolsToBind(
      const toolchain::SetVector<const language::Compability::semantics::Symbol *> &symbols,
      const language::Compability::semantics::Scope &hostScope);

  /// Code gen the FIR for the local bindings for the host associated symbols
  /// for the host (parent) procedure using `builder`.
  void hostProcedureBindings(AbstractConverter &converter, SymMap &symMap);

  /// Code gen the FIR for the local bindings for the host associated symbols
  /// for an internal (child) procedure using `builder`.
  void internalProcedureBindings(AbstractConverter &converter, SymMap &symMap);

  /// Return the type of the extra argument to add to each internal procedure.
  mlir::Type getArgumentType(AbstractConverter &convert);

  /// Is \p symbol host associated ?
  bool isAssociated(const language::Compability::semantics::Symbol &symbol) const {
    return tupleSymbols.contains(&symbol) || globalSymbols.contains(&symbol);
  }

private:
  /// Canonical vector of host associated local symbols.
  toolchain::SetVector<const language::Compability::semantics::Symbol *> tupleSymbols;

  /// Canonical vector of host associated global symbols.
  toolchain::SetVector<const language::Compability::semantics::Symbol *> globalSymbols;

  /// The type of the extra argument to be added to each internal procedure.
  mlir::Type argType;

  /// Scope of the parent procedure if addSymbolsToBind was called.
  const language::Compability::semantics::Scope *hostScope;
};
} // namespace lower
} // namespace language::Compability

#endif // FORTRAN_LOWER_HOSTASSOCIATIONS_H
