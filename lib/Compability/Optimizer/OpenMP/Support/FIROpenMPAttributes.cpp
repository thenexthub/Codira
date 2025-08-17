//===-- FIROpenMPAttributes.cpp -------------------------------------------===//
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
/// \file
/// This file implements attribute interfaces that are promised by FIR
/// dialect attributes related to OpenMP.
//===----------------------------------------------------------------------===//

#include "language/Compability/Optimizer/Builder/FIRBuilder.h"
#include "language/Compability/Optimizer/Builder/Todo.h"
#include "language/Compability/Optimizer/Dialect/FIRAttr.h"
#include "language/Compability/Optimizer/Dialect/FIRDialect.h"
#include "mlir/Dialect/OpenMP/OpenMPDialect.h"

namespace fir::omp {
class FortranSafeTempArrayCopyAttrImpl
    : public fir::SafeTempArrayCopyAttrInterface::FallbackModel<
          FortranSafeTempArrayCopyAttrImpl> {
public:
  // SafeTempArrayCopyAttrInterface interface methods.
  static bool isDynamicallySafe() { return false; }

  static mlir::Value genDynamicCheck(mlir::Location loc,
                                     fir::FirOpBuilder &builder,
                                     mlir::Value array) {
    TODO(loc, "fir::omp::FortranSafeTempArrayCopyAttrImpl::genDynamicCheck()");
    return nullptr;
  }

  static void registerTempDeallocation(mlir::Location loc,
                                       fir::FirOpBuilder &builder,
                                       mlir::Value array, mlir::Value temp) {
    TODO(loc, "fir::omp::FortranSafeTempArrayCopyAttrImpl::"
              "registerTempDeallocation()");
  }

  // Extra helper methods.

  /// Attach the implementation to fir::OpenMPSafeTempArrayCopyAttr.
  static void registerExternalModel(mlir::DialectRegistry &registry);

  /// If the methods above create any new operations, this method
  /// must register all the corresponding dialect.
  static void getDependentDialects(mlir::DialectRegistry &registry) {}
};

void FortranSafeTempArrayCopyAttrImpl::registerExternalModel(
    mlir::DialectRegistry &registry) {
  registry.addExtension(
      +[](mlir::MLIRContext *ctx, fir::FIROpsDialect *dialect) {
        fir::OpenMPSafeTempArrayCopyAttr::attachInterface<
            FortranSafeTempArrayCopyAttrImpl>(*ctx);
      });
}

void registerAttrsExtensions(mlir::DialectRegistry &registry) {
  FortranSafeTempArrayCopyAttrImpl::registerExternalModel(registry);
}

void registerTransformationalAttrsDependentDialects(
    mlir::DialectRegistry &registry) {
  FortranSafeTempArrayCopyAttrImpl::getDependentDialects(registry);
}

} // namespace fir::omp
