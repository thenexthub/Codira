//===- Func.cpp - C Interface for Func dialect ----------------------------===//
//
// Copyright (c) NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// Author: Tunjay Akbarli
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
//
// http://www.apache.org/licenses/LICENSE-2.0
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

#include "mlir-c/Dialect/Func.h"
#include "mlir-c/IR.h"
#include "mlir-c/Support.h"
#include "mlir/CAPI/Registration.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"

MLIR_DEFINE_CAPI_DIALECT_REGISTRATION(Func, func, mlir::func::FuncDialect)

void mlirFuncSetArgAttr(MlirOperation op, intptr_t pos, MlirStringRef name,
                        MlirAttribute attr) {
  toolchain::cast<mlir::func::FuncOp>(unwrap(op))
      .setArgAttr(pos, unwrap(name), unwrap(attr));
}

void mlirFuncSetResultAttr(MlirOperation op, intptr_t pos, MlirStringRef name,
                           MlirAttribute attr) {
  toolchain::cast<mlir::func::FuncOp>(unwrap(op))
      .setResultAttr(pos, unwrap(name), unwrap(attr));
}
