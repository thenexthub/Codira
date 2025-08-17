//===-- Lower/OpenMP/PrivateReductionUtils.h --------------------*- C++ -*-===//
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

#ifndef LANGUAGE_COMPABILITY_LOWER_OPENMP_PRIVATEREDUCTIONUTILS_H
#define LANGUAGE_COMPABILITY_LOWER_OPENMP_PRIVATEREDUCTIONUTILS_H

#include "mlir/IR/Location.h"
#include "mlir/IR/Value.h"

namespace mlir {
class Region;
} // namespace mlir

namespace language::Compability {
namespace semantics {
class Symbol;
} // namespace semantics
} // namespace language::Compability

namespace fir {
class FirOpBuilder;
class ShapeShiftOp;
} // namespace fir

namespace language::Compability {
namespace lower {
class AbstractConverter;

enum class DeclOperationKind {
  PrivateOrLocal,
  FirstPrivateOrLocalInit,
  Reduction
};
inline bool isPrivatization(DeclOperationKind kind) {
  return (kind == DeclOperationKind::FirstPrivateOrLocalInit) ||
         (kind == DeclOperationKind::PrivateOrLocal);
}
inline bool isReduction(DeclOperationKind kind) {
  return kind == DeclOperationKind::Reduction;
}

/// Generate init and cleanup regions suitable for reduction or privatizer
/// declarations. `scalarInitValue` may be nullptr if there is no default
/// initialization (for privatization). `kind` should be set to indicate
/// what kind of operation definition this initialization belongs to.
void populateByRefInitAndCleanupRegions(
    AbstractConverter &converter, mlir::Location loc, mlir::Type argType,
    mlir::Value scalarInitValue, mlir::Block *initBlock,
    mlir::Value allocatedPrivVarArg, mlir::Value moldArg,
    mlir::Region &cleanupRegion, DeclOperationKind kind,
    const language::Compability::semantics::Symbol *sym = nullptr,
    bool cannotHaveNonDefaultLowerBounds = false, bool isDoConcurrent = false);

/// Generate a fir::ShapeShift op describing the provided boxed array.
/// `cannotHaveNonDefaultLowerBounds` should be set if `box` is known to have
/// default lower bounds. This can improve code generation.
/// `useDefaultLowerBounds` can be set to force the returned fir::ShapeShiftOp
/// to have default lower bounds, which is useful to iterate through array
/// elements without having to adjust each index.
fir::ShapeShiftOp getShapeShift(fir::FirOpBuilder &builder, mlir::Location loc,
                                mlir::Value box,
                                bool cannotHaveNonDefaultLowerBounds = false,
                                bool useDefaultLowerBounds = false);

} // namespace lower
} // namespace language::Compability

#endif // FORTRAN_LOWER_OPENMP_PRIVATEREDUCTIONUTILS_H
