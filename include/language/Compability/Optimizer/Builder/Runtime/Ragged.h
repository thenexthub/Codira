//===-- Ragged.h ------------------------------------------------*- C++ -*-===//
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

#ifndef LANGUAGE_COMPABILITY_OPTIMIZER_BUILDER_RUNTIME_RAGGED_H
#define LANGUAGE_COMPABILITY_OPTIMIZER_BUILDER_RUNTIME_RAGGED_H

namespace mlir {
class Location;
class Value;
class ValueRange;
} // namespace mlir

namespace fir {
class FirOpBuilder;
} // namespace fir

namespace fir::runtime {

/// Generate code to instantiate a section of a ragged array. Calls the runtime
/// to initialize the data buffer. \p header must be a ragged buffer header (on
/// the heap) and will be initialized, if and only if the rank of \p extents is
/// at least 1 and all values in the vector of extents are positive. \p extents
/// must be a vector of Value of type `i64`. \p eleSize is in bytes, not bits.
void genRaggedArrayAllocate(mlir::Location loc, fir::FirOpBuilder &builder,
                            mlir::Value header, bool asHeaders,
                            mlir::Value eleSize, mlir::ValueRange extents);

/// Generate a call to the runtime routine to deallocate a ragged array data
/// structure on the heap.
void genRaggedArrayDeallocate(mlir::Location loc, fir::FirOpBuilder &builder,
                              mlir::Value header);

} // namespace fir::runtime
#endif // FORTRAN_OPTIMIZER_BUILDER_RUNTIME_RAGGED_H
