//===-- Lower/ConvertType.h -- lowering of types ----------------*- C++ -*-===//
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
///
/// Conversion of front-end TYPE, KIND, ATTRIBUTE (TKA) information to FIR/MLIR.
/// This is meant to be the single point of truth (SPOT) for all type
/// conversions when lowering to FIR.  This implements all lowering of parse
/// tree TKA to the FIR type system. If one is converting front-end types and
/// not using one of the routines provided here, it's being done wrong.
///
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_COMPABILITY_LOWER_CONVERT_TYPE_H
#define LANGUAGE_COMPABILITY_LOWER_CONVERT_TYPE_H

#include "language/Compability/Evaluate/type.h"
#include "language/Compability/Support/Fortran.h"
#include "mlir/IR/BuiltinTypes.h"

namespace mlir {
class Location;
class MLIRContext;
class Type;
} // namespace mlir

namespace language::Compability {
namespace common {
template <typename>
class Reference;
} // namespace common

namespace evaluate {
template <typename>
class Expr;
template <typename>
class FunctionRef;
struct SomeType;
} // namespace evaluate

namespace semantics {
class Symbol;
class DerivedTypeSpec;
class DerivedTypeDetails;
class Scope;
} // namespace semantics

namespace lower {
class AbstractConverter;
namespace pft {
struct Variable;
}

using SomeExpr = evaluate::Expr<evaluate::SomeType>;
using SymbolRef = common::Reference<const semantics::Symbol>;

// Type for compile time constant length type parameters.
using LenParameterTy = std::int64_t;

/// Get a FIR type based on a category and kind.
mlir::Type getFIRType(mlir::MLIRContext *ctxt, common::TypeCategory tc,
                      int kind, toolchain::ArrayRef<LenParameterTy>);

/// Get a FIR type for a derived type
mlir::Type
translateDerivedTypeToFIRType(language::Compability::lower::AbstractConverter &,
                              const language::Compability::semantics::DerivedTypeSpec &);

/// Translate a SomeExpr to an mlir::Type.
mlir::Type translateSomeExprToFIRType(language::Compability::lower::AbstractConverter &,
                                      const SomeExpr &expr);

/// Translate a language::Compability::semantics::Symbol to an mlir::Type.
mlir::Type translateSymbolToFIRType(language::Compability::lower::AbstractConverter &,
                                    const SymbolRef symbol);

/// Translate a language::Compability::lower::pft::Variable to an mlir::Type.
mlir::Type translateVariableToFIRType(language::Compability::lower::AbstractConverter &,
                                      const pft::Variable &variable);

/// Translate a REAL of KIND to the mlir::Type.
mlir::Type convertReal(mlir::MLIRContext *ctxt, int KIND);

bool isDerivedTypeWithLenParameters(const semantics::Symbol &);

template <typename T>
class TypeBuilder {
public:
  static mlir::Type genType(language::Compability::lower::AbstractConverter &,
                            const language::Compability::evaluate::FunctionRef<T> &);
};
using namespace evaluate;
FOR_EACH_SPECIFIC_TYPE(extern template class TypeBuilder, )

/// A helper class to reverse iterate through the component names of a derived
/// type, including the parent component and the component of the parents. This
/// is useful to deal with StructureConstructor lowering.
class ComponentReverseIterator {
public:
  ComponentReverseIterator(const language::Compability::semantics::DerivedTypeSpec &derived) {
    setCurrentType(derived);
  }
  /// Does the current type has a component with \name (does not look-up the
  /// components of the parent if any)? If there is a match, the iterator
  /// is advanced to the search result.
  bool lookup(const language::Compability::parser::CharBlock &name) {
    componentIt = std::find(componentIt, componentItEnd, name);
    return componentIt != componentItEnd;
  };

  /// Advance iterator to the last components of the current type parent.
  const language::Compability::semantics::DerivedTypeSpec &advanceToParentType();

  /// Get the parent component symbol for the current type.
  const language::Compability::semantics::Symbol *getParentComponent() const;

private:
  void setCurrentType(const language::Compability::semantics::DerivedTypeSpec &derived);
  const language::Compability::semantics::DerivedTypeSpec *currentParentType = nullptr;
  const language::Compability::semantics::DerivedTypeDetails *currentTypeDetails = nullptr;
  using name_iterator =
      std::list<language::Compability::parser::CharBlock>::const_reverse_iterator;
  name_iterator componentIt{};
  name_iterator componentItEnd{};
};
} // namespace lower
} // namespace language::Compability

#endif // FORTRAN_LOWER_CONVERT_TYPE_H
