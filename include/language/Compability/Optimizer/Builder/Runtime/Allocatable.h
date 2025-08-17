//===-- Allocatable.h - generate Allocatable runtime API calls---*- C++ -*-===//
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

#ifndef LANGUAGE_COMPABILITY_OPTIMIZER_BUILDER_RUNTIME_ALLOCATABLE_H
#define LANGUAGE_COMPABILITY_OPTIMIZER_BUILDER_RUNTIME_ALLOCATABLE_H

#include "mlir/IR/Value.h"

namespace mlir {
class Location;
} // namespace mlir

namespace fir {
class FirOpBuilder;
}

namespace fir::runtime {

/// Generate runtime call to assign \p sourceBox to \p destBox.
/// \p destBox must be a fir.ref<fir.box<T>> and \p sourceBox a fir.box<T>.
/// \p destBox Fortran descriptor may be modified if destBox is an allocatable
/// according to Fortran allocatable assignment rules, otherwise it is not
/// modified.
mlir::Value genMoveAlloc(fir::FirOpBuilder &builder, mlir::Location loc,
                         mlir::Value to, mlir::Value from, mlir::Value hasStat,
                         mlir::Value errMsg);

/// Generate runtime call to apply bounds, cobounds, length type
/// parameters and derived type information from \p mold descriptor
/// to \p desc descriptor. The resulting rank of \p desc descriptor
/// is set to \p rank. The resulting descriptor must be initialized
/// and deallocated before the call.
void genAllocatableApplyMold(fir::FirOpBuilder &builder, mlir::Location loc,
                             mlir::Value desc, mlir::Value mold, int rank);

/// Generate runtime call to set the bounds (\p lowerBound and \p upperBound)
/// for the specified dimension \p dimIndex (zero-based) in the given
/// \p desc descriptor.
void genAllocatableSetBounds(fir::FirOpBuilder &builder, mlir::Location loc,
                             mlir::Value desc, mlir::Value dimIndex,
                             mlir::Value lowerBound, mlir::Value upperBound);

/// Generate runtime call to allocate an allocatable entity
/// as described by the given \p desc descriptor.
void genAllocatableAllocate(fir::FirOpBuilder &builder, mlir::Location loc,
                            mlir::Value desc, mlir::Value hasStat = {},
                            mlir::Value errMsg = {});

} // namespace fir::runtime
#endif // FORTRAN_OPTIMIZER_BUILDER_RUNTIME_ALLOCATABLE_H
