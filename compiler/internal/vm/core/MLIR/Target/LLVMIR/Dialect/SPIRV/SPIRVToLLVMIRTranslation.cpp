//===- SPIRVToLLVMIRTranslation.cpp - Translate SPIR-V to LLVM IR ---------===//
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
//
// This file implements a translation between the MLIR SPIR-V dialect and
// LLVM IR.
//
//===----------------------------------------------------------------------===//

#include "mlir/Target/LLVMIR/Dialect/SPIRV/SPIRVToLLVMIRTranslation.h"
#include "mlir/Dialect/SPIRV/IR/SPIRVDialect.h"
#include "mlir/Target/LLVMIR/ModuleTranslation.h"

using namespace mlir;
using namespace mlir::LLVM;

void mlir::registerSPIRVDialectTranslation(DialectRegistry &registry) {
  registry.insert<spirv::SPIRVDialect>();
}

void mlir::registerSPIRVDialectTranslation(MLIRContext &context) {
  DialectRegistry registry;
  registerSPIRVDialectTranslation(registry);
  context.appendDialectRegistry(registry);
}
