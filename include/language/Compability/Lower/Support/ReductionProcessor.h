//===-- Lower/OpenMP/ReductionProcessor.h -----------------------*- C++ -*-===//
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

#ifndef LANGUAGE_COMPABILITY_LOWER_REDUCTIONPROCESSOR_H
#define LANGUAGE_COMPABILITY_LOWER_REDUCTIONPROCESSOR_H

#include "language/Compability/Lower/OpenMP/Clauses.h"
#include "language/Compability/Optimizer/Builder/FIRBuilder.h"
#include "language/Compability/Optimizer/Dialect/FIRType.h"
#include "language/Compability/Parser/parse-tree.h"
#include "language/Compability/Semantics/symbol.h"
#include "language/Compability/Semantics/type.h"
#include "mlir/IR/Location.h"
#include "mlir/IR/Types.h"

namespace mlir {
namespace omp {
class DeclareReductionOp;
} // namespace omp
} // namespace mlir

namespace language::Compability {
namespace lower {
class AbstractConverter;
} // namespace lower
} // namespace language::Compability

namespace language::Compability {
namespace lower {
namespace omp {

class ReductionProcessor {
public:
  // TODO: Move this enumeration to the OpenMP dialect
  enum ReductionIdentifier {
    ID,
    USER_DEF_OP,
    ADD,
    SUBTRACT,
    MULTIPLY,
    AND,
    OR,
    EQV,
    NEQV,
    MAX,
    MIN,
    IAND,
    IOR,
    IEOR
  };

  static ReductionIdentifier
  getReductionType(const omp::clause::ProcedureDesignator &pd);

  static ReductionIdentifier
  getReductionType(omp::clause::DefinedOperator::IntrinsicOperator intrinsicOp);

  static ReductionIdentifier
  getReductionType(const fir::ReduceOperationEnum &pd);

  static bool
  supportedIntrinsicProcReduction(const omp::clause::ProcedureDesignator &pd);

  static const semantics::SourceName
  getRealName(const semantics::Symbol *symbol);

  static const semantics::SourceName
  getRealName(const omp::clause::ProcedureDesignator &pd);

  static std::string getReductionName(toolchain::StringRef name,
                                      const fir::KindMapping &kindMap,
                                      mlir::Type ty, bool isByRef);

  static std::string getReductionName(ReductionIdentifier redId,
                                      const fir::KindMapping &kindMap,
                                      mlir::Type ty, bool isByRef);

  /// This function returns the identity value of the operator \p
  /// reductionOpName. For example:
  ///    0 + x = x,
  ///    1 * x = x
  static int getOperationIdentity(ReductionIdentifier redId,
                                  mlir::Location loc);

  static mlir::Value getReductionInitValue(mlir::Location loc, mlir::Type type,
                                           ReductionIdentifier redId,
                                           fir::FirOpBuilder &builder);

  template <typename FloatOp, typename IntegerOp>
  static mlir::Value getReductionOperation(fir::FirOpBuilder &builder,
                                           mlir::Type type, mlir::Location loc,
                                           mlir::Value op1, mlir::Value op2);
  template <typename FloatOp, typename IntegerOp, typename ComplexOp>
  static mlir::Value getReductionOperation(fir::FirOpBuilder &builder,
                                           mlir::Type type, mlir::Location loc,
                                           mlir::Value op1, mlir::Value op2);

  static mlir::Value createScalarCombiner(fir::FirOpBuilder &builder,
                                          mlir::Location loc,
                                          ReductionIdentifier redId,
                                          mlir::Type type, mlir::Value op1,
                                          mlir::Value op2);

  /// Creates an OpenMP reduction declaration and inserts it into the provided
  /// symbol table. The declaration has a constant initializer with the neutral
  /// value `initValue`, and the reduction combiner carried over from `reduce`.
  /// TODO: add atomic region.
  template <typename OpType>
  static OpType createDeclareReduction(AbstractConverter &builder,
                                       toolchain::StringRef reductionOpName,
                                       const ReductionIdentifier redId,
                                       mlir::Type type, mlir::Location loc,
                                       bool isByRef);

  /// Creates a reduction declaration and associates it with an OpenMP block
  /// directive.
  template <typename OpType, typename RedOperatorListTy>
  static bool processReductionArguments(
      mlir::Location currentLocation, lower::AbstractConverter &converter,
      const RedOperatorListTy &redOperatorList,
      toolchain::SmallVectorImpl<mlir::Value> &reductionVars,
      toolchain::SmallVectorImpl<bool> &reduceVarByRef,
      toolchain::SmallVectorImpl<mlir::Attribute> &reductionDeclSymbols,
      const toolchain::SmallVectorImpl<const semantics::Symbol *> &reductionSymbols);
};

template <typename FloatOp, typename IntegerOp>
mlir::Value
ReductionProcessor::getReductionOperation(fir::FirOpBuilder &builder,
                                          mlir::Type type, mlir::Location loc,
                                          mlir::Value op1, mlir::Value op2) {
  type = fir::unwrapRefType(type);
  assert(type.isIntOrIndexOrFloat() &&
         "only integer, float and complex types are currently supported");
  if (type.isIntOrIndex())
    return IntegerOp::create(builder, loc, op1, op2);
  return FloatOp::create(builder, loc, op1, op2);
}

template <typename FloatOp, typename IntegerOp, typename ComplexOp>
mlir::Value
ReductionProcessor::getReductionOperation(fir::FirOpBuilder &builder,
                                          mlir::Type type, mlir::Location loc,
                                          mlir::Value op1, mlir::Value op2) {
  assert((type.isIntOrIndexOrFloat() || fir::isa_complex(type)) &&
         "only integer, float and complex types are currently supported");
  if (type.isIntOrIndex())
    return IntegerOp::create(builder, loc, op1, op2);
  if (fir::isa_real(type))
    return FloatOp::create(builder, loc, op1, op2);
  return ComplexOp::create(builder, loc, op1, op2);
}

} // namespace omp
} // namespace lower
} // namespace language::Compability

#endif // FORTRAN_LOWER_REDUCTIONPROCESSOR_H
