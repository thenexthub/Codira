//===- RegisterEverything.cpp - Register all MLIR entities ----------------===//
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

#include "mlir-c/RegisterEverything.h"

#include "mlir/CAPI/IR.h"
#include "mlir/IR/MLIRContext.h"
#include "mlir/InitAllDialects.h"
#include "mlir/InitAllExtensions.h"
#include "mlir/InitAllPasses.h"
#include "mlir/Target/LLVMIR/Dialect/All.h"
#include "mlir/Target/LLVMIR/Dialect/Builtin/BuiltinToLLVMIRTranslation.h"
#include "mlir/Target/LLVMIR/Dialect/LLVMIR/LLVMToLLVMIRTranslation.h"

void mlirRegisterAllDialects(MlirDialectRegistry registry) {
  mlir::registerAllDialects(*unwrap(registry));
  mlir::registerAllExtensions(*unwrap(registry));
}

void mlirRegisterAllLLVMTranslations(MlirContext context) {
  auto &ctx = *unwrap(context);
  mlir::DialectRegistry registry;
  mlir::registerAllToLLVMIRTranslations(registry);
  ctx.appendDialectRegistry(registry);
}

void mlirRegisterAllPasses() { mlir::registerAllPasses(); }
