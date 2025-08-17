//===- RegisterOpenMPExtensions.h - OpenMP Extension Registration -*- C++ -*-=//
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

#ifndef FLANG_OPTIMIZER_OPENMP_SUPPORT_REGISTEROPENMPEXTENSIONS_H_
#define FLANG_OPTIMIZER_OPENMP_SUPPORT_REGISTEROPENMPEXTENSIONS_H_

namespace mlir {
class DialectRegistry;
} // namespace mlir

namespace fir::omp {

void registerOpenMPExtensions(mlir::DialectRegistry &registry);

/// Register external models for FIR attributes related to OpenMP.
void registerAttrsExtensions(mlir::DialectRegistry &registry);

/// Register all dialects whose operations may be created
/// by the transformational attributes.
void registerTransformationalAttrsDependentDialects(
    mlir::DialectRegistry &registry);

} // namespace fir::omp

#endif // FLANG_OPTIMIZER_OPENMP_SUPPORT_REGISTEROPENMPEXTENSIONS_H_
