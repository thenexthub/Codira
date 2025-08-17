//===- FunctionAttr.cpp ---------------------------------------------------===//
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

//===----------------------------------------------------------------------===//
/// \file
/// This is a generic pass for adding attributes to functions.
//===----------------------------------------------------------------------===//
#include "language/Compability/Optimizer/Dialect/FIROpsSupport.h"
#include "language/Compability/Optimizer/Support/InternalNames.h"
#include "language/Compability/Optimizer/Transforms/Passes.h"
#include "mlir/Dialect/LLVMIR/LLVMAttrs.h"
#include "mlir/Dialect/LLVMIR/LLVMDialect.h"

namespace fir {
#define GEN_PASS_DEF_FUNCTIONATTR
#include "language/Compability/Optimizer/Transforms/Passes.h.inc"
} // namespace fir

#define DEBUG_TYPE "func-attr"

namespace {

class FunctionAttrPass : public fir::impl::FunctionAttrBase<FunctionAttrPass> {
public:
  FunctionAttrPass(const fir::FunctionAttrOptions &options) : Base{options} {}
  FunctionAttrPass() = default;
  void runOnOperation() override;
};

} // namespace

void FunctionAttrPass::runOnOperation() {
  LLVM_DEBUG(toolchain::dbgs() << "=== Begin " DEBUG_TYPE " ===\n");
  mlir::func::FuncOp func = getOperation();

  LLVM_DEBUG(toolchain::dbgs() << "Func-name:" << func.getSymName() << "\n");

  toolchain::StringRef name = func.getSymName();
  auto deconstructed = fir::NameUniquer::deconstruct(name);
  bool isFromModule = !deconstructed.second.modules.empty();

  if ((isFromModule || !func.isDeclaration()) &&
      !fir::hasBindcAttr(func.getOperation())) {
    toolchain::StringRef nocapture = mlir::LLVM::LLVMDialect::getNoCaptureAttrName();
    toolchain::StringRef noalias = mlir::LLVM::LLVMDialect::getNoAliasAttrName();
    mlir::UnitAttr unitAttr = mlir::UnitAttr::get(func.getContext());

    for (auto [index, argType] : toolchain::enumerate(func.getArgumentTypes())) {
      bool isNoCapture = false;
      bool isNoAlias = false;
      if (mlir::isa<fir::ReferenceType>(argType) &&
          !func.getArgAttr(index, fir::getTargetAttrName()) &&
          !func.getArgAttr(index, fir::getAsynchronousAttrName()) &&
          !func.getArgAttr(index, fir::getVolatileAttrName())) {
        isNoCapture = true;
        isNoAlias = !fir::isPointerType(argType);
      } else if (mlir::isa<fir::BaseBoxType>(argType)) {
        // !fir.box arguments will be passed as descriptor pointers
        // at LLVM IR dialect level - they cannot be captured,
        // and cannot alias with anything within the function.
        isNoCapture = isNoAlias = true;
      }
      if (isNoCapture && setNoCapture)
        func.setArgAttr(index, nocapture, unitAttr);
      if (isNoAlias && setNoAlias)
        func.setArgAttr(index, noalias, unitAttr);
    }
  }

  mlir::MLIRContext *context = &getContext();
  if (framePointerKind != mlir::LLVM::framePointerKind::FramePointerKind::None)
    func->setAttr("frame_pointer", mlir::LLVM::FramePointerKindAttr::get(
                                       context, framePointerKind));

  auto toolchainFuncOpName =
      mlir::OperationName(mlir::LLVM::LLVMFuncOp::getOperationName(), context);
  if (!instrumentFunctionEntry.empty())
    func->setAttr(mlir::LLVM::LLVMFuncOp::getInstrumentFunctionEntryAttrName(
                      toolchainFuncOpName),
                  mlir::StringAttr::get(context, instrumentFunctionEntry));
  if (!instrumentFunctionExit.empty())
    func->setAttr(mlir::LLVM::LLVMFuncOp::getInstrumentFunctionExitAttrName(
                      toolchainFuncOpName),
                  mlir::StringAttr::get(context, instrumentFunctionExit));
  if (noInfsFPMath)
    func->setAttr(
        mlir::LLVM::LLVMFuncOp::getNoInfsFpMathAttrName(toolchainFuncOpName),
        mlir::BoolAttr::get(context, true));
  if (noNaNsFPMath)
    func->setAttr(
        mlir::LLVM::LLVMFuncOp::getNoNansFpMathAttrName(toolchainFuncOpName),
        mlir::BoolAttr::get(context, true));
  if (approxFuncFPMath)
    func->setAttr(
        mlir::LLVM::LLVMFuncOp::getApproxFuncFpMathAttrName(toolchainFuncOpName),
        mlir::BoolAttr::get(context, true));
  if (noSignedZerosFPMath)
    func->setAttr(
        mlir::LLVM::LLVMFuncOp::getNoSignedZerosFpMathAttrName(toolchainFuncOpName),
        mlir::BoolAttr::get(context, true));
  if (unsafeFPMath)
    func->setAttr(
        mlir::LLVM::LLVMFuncOp::getUnsafeFpMathAttrName(toolchainFuncOpName),
        mlir::BoolAttr::get(context, true));
  if (!reciprocals.empty())
    func->setAttr(
        mlir::LLVM::LLVMFuncOp::getReciprocalEstimatesAttrName(toolchainFuncOpName),
        mlir::StringAttr::get(context, reciprocals));
  if (!preferVectorWidth.empty())
    func->setAttr(
        mlir::LLVM::LLVMFuncOp::getPreferVectorWidthAttrName(toolchainFuncOpName),
        mlir::StringAttr::get(context, preferVectorWidth));

  LLVM_DEBUG(toolchain::dbgs() << "=== End " DEBUG_TYPE " ===\n");
}
