//===- LLVMIRToNVVMTranslation.cpp - Translate LLVM IR to NVVM dialect ----===//
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
// This file implements a translation between LLVM IR and the MLIR NVVM dialect.
//
//===----------------------------------------------------------------------===//

#include "mlir/Target/LLVMIR/Dialect/NVVM/LLVMIRToNVVMTranslation.h"
#include "mlir/Dialect/LLVMIR/NVVMDialect.h"
#include "mlir/Target/LLVMIR/ModuleImport.h"
#include "vm/core/IR/ConstantRange.h"

using namespace mlir;
using namespace mlir::NVVM;

/// Returns true if the LLVM IR intrinsic is convertible to an MLIR NVVM dialect
/// intrinsic. Returns false otherwise.
static bool isConvertibleIntrinsic(toolchain::Intrinsic::ID id) {
  static const DenseSet<unsigned> convertibleIntrinsics = {
#include "mlir/Dialect/LLVMIR/NVVMConvertibleLLVMIRIntrinsics.inc"
  };
  return convertibleIntrinsics.contains(id);
}

/// Returns the list of LLVM IR intrinsic identifiers that are convertible to
/// MLIR NVVM dialect intrinsics.
static ArrayRef<unsigned> getSupportedIntrinsicsImpl() {
  static const SmallVector<unsigned> convertibleIntrinsics = {
#include "mlir/Dialect/LLVMIR/NVVMConvertibleLLVMIRIntrinsics.inc"
  };
  return convertibleIntrinsics;
}

/// Converts the LLVM intrinsic to an MLIR NVVM dialect operation if a
/// conversion exits. Returns failure otherwise.
static LogicalResult convertIntrinsicImpl(OpBuilder &odsBuilder,
                                          toolchain::CallInst *inst,
                                          LLVM::ModuleImport &moduleImport) {
  toolchain::Intrinsic::ID intrinsicID = inst->getIntrinsicID();

  // Check if the intrinsic is convertible to an MLIR dialect counterpart and
  // copy the arguments to an an LLVM operands array reference for conversion.
  if (isConvertibleIntrinsic(intrinsicID)) {
    SmallVector<toolchain::Value *> args(inst->args());
    ArrayRef<toolchain::Value *> llvmOperands(args);

    SmallVector<toolchain::OperandBundleUse> llvmOpBundles;
    llvmOpBundles.reserve(inst->getNumOperandBundles());
    for (unsigned i = 0; i < inst->getNumOperandBundles(); ++i)
      llvmOpBundles.push_back(inst->getOperandBundleAt(i));

#include "mlir/Dialect/LLVMIR/NVVMFromLLVMIRConversions.inc"
  }

  return failure();
}

namespace {

/// Implementation of the dialect interface that converts operations belonging
/// to the NVVM dialect.
class NVVMDialectLLVMIRImportInterface : public LLVMImportDialectInterface {
public:
  using LLVMImportDialectInterface::LLVMImportDialectInterface;

  /// Converts the LLVM intrinsic to an MLIR NVVM dialect operation if a
  /// conversion exits. Returns failure otherwise.
  LogicalResult convertIntrinsic(OpBuilder &builder, toolchain::CallInst *inst,
                                 LLVM::ModuleImport &moduleImport) const final {
    return convertIntrinsicImpl(builder, inst, moduleImport);
  }

  /// Returns the list of LLVM IR intrinsic identifiers that are convertible to
  /// MLIR NVVM dialect intrinsics.
  ArrayRef<unsigned> getSupportedIntrinsics() const final {
    return getSupportedIntrinsicsImpl();
  }
};

} // namespace

void mlir::registerNVVMDialectImport(DialectRegistry &registry) {
  registry.insert<NVVM::NVVMDialect>();
  registry.addExtension(+[](MLIRContext *ctx, NVVM::NVVMDialect *dialect) {
    dialect->addInterfaces<NVVMDialectLLVMIRImportInterface>();
  });
}

void mlir::registerNVVMDialectImport(MLIRContext &context) {
  DialectRegistry registry;
  registerNVVMDialectImport(registry);
  context.appendDialectRegistry(registry);
}
