//===-- Lower/Mangler.h -- name mangling ------------------------*- C++ -*-===//
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
// Coding style: https://mlir.toolchain.org/getting_started/DeveloperGuide/
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_COMPABILITY_LOWER_MANGLER_H
#define LANGUAGE_COMPABILITY_LOWER_MANGLER_H

#include "language/Compability/Evaluate/expression.h"
#include "language/Compability/Optimizer/Dialect/FIRType.h"
#include "mlir/IR/BuiltinTypes.h"
#include "toolchain/ADT/StringRef.h"
#include <string>

namespace language::Compability {
namespace common {
template <typename>
class Reference;
}

namespace semantics {
class Scope;
class Symbol;
class DerivedTypeSpec;
} // namespace semantics

namespace lower::mangle {

using ScopeBlockIdMap =
    toolchain::DenseMap<language::Compability::semantics::Scope *, std::int64_t>;

/// Convert a front-end symbol to a unique internal name.
/// A symbol that could be in a block scope must provide a ScopeBlockIdMap.
/// If \p keepExternalInScope is true, mangling an external symbol retains
/// the scope of the symbol. This is useful when setting the attributes of
/// a symbol where all the Fortran context is needed. Otherwise, external
/// symbols are mangled outside of any scope.
std::string mangleName(const semantics::Symbol &, ScopeBlockIdMap &,
                       bool keepExternalInScope = false,
                       bool underscoring = true);
std::string mangleName(const semantics::Symbol &,
                       bool keepExternalInScope = false,
                       bool underscoring = true);

/// Convert a derived type instance to an internal name.
std::string mangleName(const semantics::DerivedTypeSpec &, ScopeBlockIdMap &);

/// Add a scope specific mangling prefix to a compiler generated name.
std::string mangleName(std::string &, const language::Compability::semantics::Scope &,
                       ScopeBlockIdMap &);

/// Recover the bare name of the original symbol from an internal name.
std::string demangleName(toolchain::StringRef name);

std::string
mangleArrayLiteral(size_t size,
                   const language::Compability::evaluate::ConstantSubscripts &shape,
                   language::Compability::common::TypeCategory cat, int kind = 0,
                   language::Compability::common::ConstantSubscript charLen = -1,
                   toolchain::StringRef derivedName = {});

template <language::Compability::common::TypeCategory TC, int KIND>
std::string mangleArrayLiteral(
    mlir::Type,
    const language::Compability::evaluate::Constant<language::Compability::evaluate::Type<TC, KIND>> &x) {
  return mangleArrayLiteral(x.values().size() * sizeof(x.values()[0]),
                            x.shape(), TC, KIND);
}

template <int KIND>
std::string
mangleArrayLiteral(mlir::Type,
                   const language::Compability::evaluate::Constant<language::Compability::evaluate::Type<
                       language::Compability::common::TypeCategory::Character, KIND>> &x) {
  return mangleArrayLiteral(x.values().size() * sizeof(x.values()[0]),
                            x.shape(), language::Compability::common::TypeCategory::Character,
                            KIND, x.LEN());
}

inline std::string mangleArrayLiteral(
    mlir::Type eleTy,
    const language::Compability::evaluate::Constant<language::Compability::evaluate::SomeDerived> &x) {
  return mangleArrayLiteral(x.values().size() * sizeof(x.values()[0]),
                            x.shape(), language::Compability::common::TypeCategory::Derived,
                            /*kind=*/0, /*charLen=*/-1,
                            mlir::cast<fir::RecordType>(eleTy).getName());
}

/// Return the compiler-generated name of a static namelist variable descriptor.
std::string globalNamelistDescriptorName(const language::Compability::semantics::Symbol &sym);

/// Return the field name for a derived type component inside a fir.record type.
/// It is the component name if the component is not private. Otherwise it is
/// mangled with the component parent type to avoid any name clashes in type
/// extensions.
std::string getRecordTypeFieldName(const language::Compability::semantics::Symbol &component,
                                   ScopeBlockIdMap &);

} // namespace lower::mangle
} // namespace language::Compability

#endif // FORTRAN_LOWER_MANGLER_H
