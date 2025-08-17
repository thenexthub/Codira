//===- FIROpenACCTypeInterfaces.h -------------------------------*- C++ -*-===//
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
// This file contains external dialect interfaces for FIR.
//
//===----------------------------------------------------------------------===//

#ifndef FLANG_OPTIMIZER_OPENACC_FIROPENACCTYPEINTERFACES_H_
#define FLANG_OPTIMIZER_OPENACC_FIROPENACCTYPEINTERFACES_H_

#include "language/Compability/Optimizer/Dialect/FIRType.h"
#include "mlir/Dialect/OpenACC/OpenACC.h"

namespace fir::acc {

template <typename T>
struct OpenACCPointerLikeModel
    : public mlir::acc::PointerLikeType::ExternalModel<
          OpenACCPointerLikeModel<T>, T> {
  mlir::Type getElementType(mlir::Type pointer) const {
    return mlir::cast<T>(pointer).getElementType();
  }
  mlir::acc::VariableTypeCategory
  getPointeeTypeCategory(mlir::Type pointer,
                         mlir::TypedValue<mlir::acc::PointerLikeType> varPtr,
                         mlir::Type varType) const;
};

template <typename T>
struct OpenACCMappableModel
    : public mlir::acc::MappableType::ExternalModel<OpenACCMappableModel<T>,
                                                    T> {
  mlir::TypedValue<mlir::acc::PointerLikeType> getVarPtr(::mlir::Type type,
                                                         mlir::Value var) const;

  std::optional<toolchain::TypeSize>
  getSizeInBytes(mlir::Type type, mlir::Value var, mlir::ValueRange accBounds,
                 const mlir::DataLayout &dataLayout) const;

  std::optional<int64_t>
  getOffsetInBytes(mlir::Type type, mlir::Value var, mlir::ValueRange accBounds,
                   const mlir::DataLayout &dataLayout) const;

  toolchain::SmallVector<mlir::Value>
  generateAccBounds(mlir::Type type, mlir::Value var,
                    mlir::OpBuilder &builder) const;

  mlir::acc::VariableTypeCategory getTypeCategory(mlir::Type type,
                                                  mlir::Value var) const;

  mlir::Value generatePrivateInit(mlir::Type type, mlir::OpBuilder &builder,
                                  mlir::Location loc,
                                  mlir::TypedValue<mlir::acc::MappableType> var,
                                  toolchain::StringRef varName,
                                  mlir::ValueRange extents,
                                  mlir::Value initVal) const;
};

} // namespace fir::acc

#endif // FLANG_OPTIMIZER_OPENACC_FIROPENACCTYPEINTERFACES_H_
