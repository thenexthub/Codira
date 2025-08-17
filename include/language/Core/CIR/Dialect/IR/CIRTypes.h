//===----------------------------------------------------------------------===//
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
// This file declares the types in the CIR dialect.
//
//===----------------------------------------------------------------------===//

#ifndef CLANG_CIR_DIALECT_IR_CIRTYPES_H
#define CLANG_CIR_DIALECT_IR_CIRTYPES_H

#include "mlir/IR/BuiltinAttributes.h"
#include "mlir/IR/Types.h"
#include "mlir/Interfaces/DataLayoutInterfaces.h"
#include "language/Core/CIR/Interfaces/CIRTypeInterfaces.h"

namespace cir {

namespace detail {
struct RecordTypeStorage;
} // namespace detail

bool isValidFundamentalIntWidth(unsigned width);

/// Returns true if the type is a CIR sized type.
///
/// Types are sized if they implement SizedTypeInterface and
/// return true from its method isSized.
///
/// Unsized types are those that do not have a size, such as
/// void, or abstract types.
bool isSized(mlir::Type ty);

} // namespace cir

//===----------------------------------------------------------------------===//
// CIR Dialect Tablegen'd Types
//===----------------------------------------------------------------------===//

namespace cir {

#include "language/Core/CIR/Dialect/IR/CIRTypeConstraints.h.inc"

} // namespace cir

#define GET_TYPEDEF_CLASSES
#include "language/Core/CIR/Dialect/IR/CIROpsTypes.h.inc"

#endif // CLANG_CIR_DIALECT_IR_CIRTYPES_H
