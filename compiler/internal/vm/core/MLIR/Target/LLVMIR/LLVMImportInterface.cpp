//===------------------------------------------------------------*- C++ -*-===//
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
// This file implements methods from LLVMImportInterface.
//
//===----------------------------------------------------------------------===//

#include "mlir/Target/LLVMIR/LLVMImportInterface.h"
#include "mlir/Target/LLVMIR/ModuleImport.h"

using namespace mlir;
using namespace mlir::LLVM;
using namespace mlir::LLVM::detail;

LogicalResult mlir::LLVMImportInterface::convertUnregisteredIntrinsic(
    OpBuilder &builder, toolchain::CallInst *inst,
    LLVM::ModuleImport &moduleImport) {
  StringRef intrinName = inst->getCalledFunction()->getName();

  SmallVector<toolchain::Value *> args(inst->args());
  ArrayRef<toolchain::Value *> llvmOperands(args);

  SmallVector<toolchain::OperandBundleUse> llvmOpBundles;
  llvmOpBundles.reserve(inst->getNumOperandBundles());
  for (unsigned i = 0; i < inst->getNumOperandBundles(); ++i)
    llvmOpBundles.push_back(inst->getOperandBundleAt(i));

  SmallVector<Value> mlirOperands;
  SmallVector<NamedAttribute> mlirAttrs;
  if (failed(moduleImport.convertIntrinsicArguments(
          llvmOperands, llvmOpBundles, /*requiresOpBundles=*/false,
          /*immArgPositions=*/{}, /*immArgAttrNames=*/{}, mlirOperands,
          mlirAttrs)))
    return failure();

  Type resultType = moduleImport.convertType(inst->getType());
  auto op = CallIntrinsicOp::create(
      builder, moduleImport.translateLoc(inst->getDebugLoc()),
      isa<LLVMVoidType>(resultType) ? TypeRange{} : TypeRange{resultType},
      StringAttr::get(builder.getContext(), intrinName),
      ValueRange{mlirOperands}, FastmathFlagsAttr{});

  moduleImport.setFastmathFlagsAttr(inst, op);
  moduleImport.convertArgAndResultAttrs(inst, op);

  // Update importer tracking of results.
  unsigned numRes = op.getNumResults();
  if (numRes == 1)
    moduleImport.mapValue(inst) = op.getResult(0);
  else if (numRes == 0)
    moduleImport.mapNoResultOp(inst);
  else
    return op.emitError(
        "expected at most one result from target intrinsic call");

  return success();
}

/// Converts the LLVM intrinsic to an MLIR operation if a conversion exists.
/// Returns failure otherwise.
LogicalResult mlir::LLVMImportInterface::convertIntrinsic(
    OpBuilder &builder, toolchain::CallInst *inst,
    LLVM::ModuleImport &moduleImport) const {
  // Lookup the dialect interface for the given intrinsic.
  // Verify the intrinsic identifier maps to an actual intrinsic.
  toolchain::Intrinsic::ID intrinId = inst->getIntrinsicID();
  assert(intrinId != toolchain::Intrinsic::not_intrinsic);

  // First lookup the intrinsic across different dialects for known
  // supported conversions, examples include arm-neon, nvm-sve, etc.
  Dialect *dialect = nullptr;

  if (!moduleImport.useUnregisteredIntrinsicsOnly())
    dialect = intrinsicToDialect.lookup(intrinId);

  // No specialized (supported) intrinsics, attempt to generate a generic
  // version via toolchain.call_intrinsic (if available).
  if (!dialect)
    return convertUnregisteredIntrinsic(builder, inst, moduleImport);

  // Dispatch the conversion to the dialect interface.
  const LLVMImportDialectInterface *iface = getInterfaceFor(dialect);
  assert(iface && "expected to find a dialect interface");
  return iface->convertIntrinsic(builder, inst, moduleImport);
}
