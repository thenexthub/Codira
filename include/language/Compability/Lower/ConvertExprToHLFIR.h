//===-- Lower/ConvertExprToHLFIR.h -- lowering of expressions ----*- C++-*-===//
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
/// Implements the conversion from language::Compability::evaluate::Expr trees to HLFIR.
///
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_COMPABILITY_LOWER_CONVERTEXPRTOHLFIR_H
#define LANGUAGE_COMPABILITY_LOWER_CONVERTEXPRTOHLFIR_H

#include "language/Compability/Lower/StatementContext.h"
#include "language/Compability/Lower/Support/Utils.h"
#include "language/Compability/Optimizer/Builder/FIRBuilder.h"
#include "language/Compability/Optimizer/Builder/HLFIRTools.h"
#include "language/Compability/Optimizer/Dialect/FIRDialect.h"

namespace mlir {
class Location;
} // namespace mlir

namespace hlfir {
class ElementalAddrOp;
}

namespace language::Compability::lower {

class AbstractConverter;
class SymMap;

hlfir::EntityWithAttributes
convertExprToHLFIR(mlir::Location loc, language::Compability::lower::AbstractConverter &,
                   const language::Compability::lower::SomeExpr &, language::Compability::lower::SymMap &,
                   language::Compability::lower::StatementContext &);

inline fir::ExtendedValue translateToExtendedValue(
    mlir::Location loc, fir::FirOpBuilder &builder, hlfir::Entity entity,
    language::Compability::lower::StatementContext &context, bool contiguityHint = false) {
  auto [exv, exvCleanup] =
      hlfir::translateToExtendedValue(loc, builder, entity, contiguityHint);
  if (exvCleanup)
    context.attachCleanup(*exvCleanup);
  return exv;
}

/// Lower an evaluate::Expr object to a fir.box, and a procedure designator to a
/// fir.boxproc<>
fir::ExtendedValue convertExprToBox(mlir::Location loc,
                                    language::Compability::lower::AbstractConverter &,
                                    const language::Compability::lower::SomeExpr &,
                                    language::Compability::lower::SymMap &,
                                    language::Compability::lower::StatementContext &);
fir::ExtendedValue convertToBox(mlir::Location loc,
                                language::Compability::lower::AbstractConverter &,
                                hlfir::Entity entity,
                                language::Compability::lower::StatementContext &,
                                mlir::Type fortranType);

/// Lower an evaluate::Expr to fir::ExtendedValue address.
/// The address may be a raw fir.ref<T>, or a fir.box<T>/fir.class<T>, or a
/// fir.boxproc<>. Pointers and allocatable are dereferenced.
/// - If the expression is a procedure designator, it is lowered to fir.boxproc
/// (with an extra length for character function procedure designators).
/// - If expression is not a variable, or is a designator with vector
///   subscripts, a temporary is created to hold the expression value and
///   is returned as:
///   - a fir.class<T> if the expression is polymorphic.
///   - otherwise, a fir.box<T> if it is a derived type with length
///     parameters (not yet implemented).
///   - otherwise, a fir.ref<T>
/// - If the expression is a variable that is not a designator with
///   vector subscripts, it is lowered without creating a temporary and
///   is returned as:
///   - a fir.class<T> if the variable is polymorphic.
///   - otherwise, a fir.box<T> if it is a derived type with length
///     parameters (not yet implemented), or if it is not a simply
///     contiguous.
///   - otherwise, a fir.ref<T>
///
/// Beware that this is different from the previous createSomeExtendedAddress
/// that had a non-trivial behaviour and would create contiguous temporary for
/// array sections `x(:, :)`, but not for `x` even if x is not simply
/// contiguous.
fir::ExtendedValue convertExprToAddress(mlir::Location loc,
                                        language::Compability::lower::AbstractConverter &,
                                        const language::Compability::lower::SomeExpr &,
                                        language::Compability::lower::SymMap &,
                                        language::Compability::lower::StatementContext &);
fir::ExtendedValue convertToAddress(mlir::Location loc,
                                    language::Compability::lower::AbstractConverter &,
                                    hlfir::Entity entity,
                                    language::Compability::lower::StatementContext &,
                                    mlir::Type fortranType);

/// Lower an evaluate::Expr to a fir::ExtendedValue value.
fir::ExtendedValue convertExprToValue(mlir::Location loc,
                                      language::Compability::lower::AbstractConverter &,
                                      const language::Compability::lower::SomeExpr &,
                                      language::Compability::lower::SymMap &,
                                      language::Compability::lower::StatementContext &);
fir::ExtendedValue convertToValue(mlir::Location loc,
                                  language::Compability::lower::AbstractConverter &,
                                  hlfir::Entity entity,
                                  language::Compability::lower::StatementContext &);

fir::ExtendedValue convertDataRefToValue(mlir::Location loc,
                                         language::Compability::lower::AbstractConverter &,
                                         const language::Compability::evaluate::DataRef &,
                                         language::Compability::lower::SymMap &,
                                         language::Compability::lower::StatementContext &);

/// Lower an evaluate::Expr to a fir::MutableBoxValue value.
/// This can only be called if the Expr is a POINTER or ALLOCATABLE,
/// otherwise, this will crash.
fir::MutableBoxValue
convertExprToMutableBox(mlir::Location loc, language::Compability::lower::AbstractConverter &,
                        const language::Compability::lower::SomeExpr &,
                        language::Compability::lower::SymMap &);
/// Lower a designator containing vector subscripts into an
/// hlfir::ElementalAddrOp that will allow looping on the elements to assign
/// them values. This only intends to cover the cases where such designator
/// appears on the left-hand side of an assignment or appears in an input IO
/// statement. These are the only contexts in Fortran where a vector subscripted
/// entity may be modified. Otherwise, there is no need to do anything special
/// about vector subscripts, they are automatically turned into array expression
/// values via an hlfir.elemental in the convertExprToXXX calls.
hlfir::ElementalAddrOp convertVectorSubscriptedExprToElementalAddr(
    mlir::Location loc, language::Compability::lower::AbstractConverter &,
    const language::Compability::lower::SomeExpr &, language::Compability::lower::SymMap &,
    language::Compability::lower::StatementContext &);

/// Lower a designator containing vector subscripts, creating a hlfir::Entity
/// representing the first element in the vector subscripted array. This is a
/// helper which calls convertVectorSubscriptedExprToElementalAddr and lowers
/// the hlfir::ElementalAddrOp.
hlfir::Entity genVectorSubscriptedDesignatorFirstElementAddress(
    mlir::Location loc, language::Compability::lower::AbstractConverter &converter,
    const language::Compability::lower::SomeExpr &expr, language::Compability::lower::SymMap &symMap,
    language::Compability::lower::StatementContext &stmtCtx);

} // namespace language::Compability::lower

#endif // FORTRAN_LOWER_CONVERTEXPRTOHLFIR_H
