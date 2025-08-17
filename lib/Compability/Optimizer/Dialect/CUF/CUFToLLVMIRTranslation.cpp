//===- CUFToLLVMIRTranslation.cpp - Translate CUF dialect to LLVM IR ------===//
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
// This file implements a translation between the MLIR CUF dialect and LLVM IR.
//
//===----------------------------------------------------------------------===//

#include "language/Compability/Optimizer/Dialect/CUF/CUFToLLVMIRTranslation.h"
#include "language/Compability/Optimizer/Dialect/CUF/CUFOps.h"
#include "language/Compability/Runtime/entry-names.h"
#include "mlir/Target/LLVMIR/LLVMTranslationInterface.h"
#include "mlir/Target/LLVMIR/ModuleTranslation.h"
#include "toolchain/ADT/TypeSwitch.h"
#include "toolchain/IR/IRBuilder.h"
#include "toolchain/IR/Module.h"
#include "toolchain/Support/FormatVariadic.h"

using namespace mlir;

namespace {

LogicalResult registerModule(cuf::RegisterModuleOp op,
                             toolchain::IRBuilderBase &builder,
                             LLVM::ModuleTranslation &moduleTranslation) {
  std::string binaryIdentifier =
      op.getName().getLeafReference().str() + "_binary";
  toolchain::Module *module = moduleTranslation.getLLVMModule();
  toolchain::Value *binary = module->getGlobalVariable(binaryIdentifier, true);
  if (!binary)
    return op.emitError() << "Couldn't find the binary: " << binaryIdentifier;

  toolchain::Type *ptrTy = builder.getPtrTy(0);
  toolchain::FunctionCallee fct = module->getOrInsertFunction(
      RTNAME_STRING(CUFRegisterModule),
      toolchain::FunctionType::get(ptrTy, ArrayRef<toolchain::Type *>({ptrTy}), false));
  auto *handle = builder.CreateCall(fct, {binary});
  moduleTranslation.mapValue(op->getResults().front()) = handle;
  return mlir::success();
}

toolchain::Value *getOrCreateFunctionName(toolchain::Module *module,
                                     toolchain::IRBuilderBase &builder,
                                     toolchain::StringRef moduleName,
                                     toolchain::StringRef kernelName) {
  std::string globalName =
      std::string(toolchain::formatv("{0}_{1}_kernel_name", moduleName, kernelName));

  if (toolchain::GlobalVariable *gv = module->getGlobalVariable(globalName))
    return gv;

  return builder.CreateGlobalString(kernelName, globalName);
}

LogicalResult registerKernel(cuf::RegisterKernelOp op,
                             toolchain::IRBuilderBase &builder,
                             LLVM::ModuleTranslation &moduleTranslation) {
  toolchain::Module *module = moduleTranslation.getLLVMModule();
  toolchain::Type *ptrTy = builder.getPtrTy(0);
  toolchain::FunctionCallee fct = module->getOrInsertFunction(
      RTNAME_STRING(CUFRegisterFunction),
      toolchain::FunctionType::get(
          ptrTy, ArrayRef<toolchain::Type *>({ptrTy, ptrTy, ptrTy}), false));
  toolchain::Value *modulePtr = moduleTranslation.lookupValue(op.getModulePtr());
  if (!modulePtr)
    return op.emitError() << "Couldn't find the module ptr";
  toolchain::Function *fctSym =
      moduleTranslation.lookupFunction(op.getKernelName().str());
  if (!fctSym)
    return op.emitError() << "Couldn't find kernel name symbol: "
                          << op.getKernelName().str();
  builder.CreateCall(fct, {modulePtr, fctSym,
                           getOrCreateFunctionName(
                               module, builder, op.getKernelModuleName().str(),
                               op.getKernelName().str())});
  return mlir::success();
}

class CUFDialectLLVMIRTranslationInterface
    : public LLVMTranslationDialectInterface {
public:
  using LLVMTranslationDialectInterface::LLVMTranslationDialectInterface;

  LogicalResult
  convertOperation(Operation *operation, toolchain::IRBuilderBase &builder,
                   LLVM::ModuleTranslation &moduleTranslation) const override {
    return toolchain::TypeSwitch<Operation *, LogicalResult>(operation)
        .Case([&](cuf::RegisterModuleOp op) {
          return registerModule(op, builder, moduleTranslation);
        })
        .Case([&](cuf::RegisterKernelOp op) {
          return registerKernel(op, builder, moduleTranslation);
        })
        .Default([&](Operation *op) {
          return op->emitError("unsupported GPU operation: ") << op->getName();
        });
  }
};

} // namespace

void cuf::registerCUFDialectTranslation(DialectRegistry &registry) {
  registry.insert<cuf::CUFDialect>();
  registry.addExtension(+[](MLIRContext *ctx, cuf::CUFDialect *dialect) {
    dialect->addInterfaces<CUFDialectLLVMIRTranslationInterface>();
  });
}
