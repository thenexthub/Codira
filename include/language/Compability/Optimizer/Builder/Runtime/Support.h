//===-- Support.h - generate support runtime API calls ----------*- C++ -*-===//
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

#ifndef LANGUAGE_COMPABILITY_OPTIMIZER_BUILDER_RUNTIME_SUPPORT_H
#define LANGUAGE_COMPABILITY_OPTIMIZER_BUILDER_RUNTIME_SUPPORT_H

namespace mlir {
class Value;
class Location;
} // namespace mlir

namespace fir {
class FirOpBuilder;
}

namespace fir::runtime {

/// Generate call to `CopyAndUpdateDescriptor` runtime routine.
void genCopyAndUpdateDescriptor(fir::FirOpBuilder &builder, mlir::Location loc,
                                mlir::Value to, mlir::Value from,
                                mlir::Value newDynamicType,
                                mlir::Value newAttribute,
                                mlir::Value newLowerBounds);

/// Generate call to `IsAssumedSize` runtime routine.
mlir::Value genIsAssumedSize(fir::FirOpBuilder &builder, mlir::Location loc,
                             mlir::Value box);

} // namespace fir::runtime
#endif // FORTRAN_OPTIMIZER_BUILDER_RUNTIME_SUPPORT_H
