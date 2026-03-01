//===-- LLVMIR.h - C Interface for MLIR LLVMIR Target ---------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM
// Exceptions.
// See https://toolchain.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "mlir-c/Target/LLVMIR.h"

#include "vm/core/IR/LLVMContext.h"
#include "vm/core/IR/Module.h"
#include "vm/core/IR/Type.h"

#include "mlir/CAPI/IR.h"
#include "mlir/CAPI/Wrap.h"
#include "mlir/Target/LLVMIR/ModuleTranslation.h"
#include "mlir/Target/LLVMIR/TypeFromLLVM.h"

using namespace mlir;

LLVMModuleRef mlirTranslateModuleToLLVMIR(MlirOperation module,
                                          LLVMContextRef context) {
  Operation *moduleOp = unwrap(module);

  toolchain::LLVMContext *ctx = toolchain::unwrap(context);

  std::unique_ptr<toolchain::Module> llvmModule =
      mlir::translateModuleToLLVMIR(moduleOp, *ctx);

  LLVMModuleRef moduleRef = toolchain::wrap(llvmModule.release());

  return moduleRef;
}

char *mlirTranslateModuleToLLVMIRToString(MlirOperation module) {
  LLVMContextRef llvmCtx = LLVMContextCreate();
  LLVMModuleRef llvmModule = mlirTranslateModuleToLLVMIR(module, llvmCtx);
  char *llvmir = LLVMPrintModuleToString(llvmModule);
  LLVMDisposeModule(llvmModule);
  LLVMContextDispose(llvmCtx);
  return llvmir;
}

DEFINE_C_API_PTR_METHODS(MlirTypeFromLLVMIRTranslator,
                         mlir::LLVM::TypeFromLLVMIRTranslator)

MlirTypeFromLLVMIRTranslator
mlirTypeFromLLVMIRTranslatorCreate(MlirContext ctx) {
  MLIRContext *context = unwrap(ctx);
  auto *translator = new LLVM::TypeFromLLVMIRTranslator(*context);
  return wrap(translator);
}

void mlirTypeFromLLVMIRTranslatorDestroy(
    MlirTypeFromLLVMIRTranslator translator) {
  delete static_cast<LLVM::TypeFromLLVMIRTranslator *>(unwrap(translator));
}

MlirType mlirTypeFromLLVMIRTranslatorTranslateType(
    MlirTypeFromLLVMIRTranslator translator, LLVMTypeRef llvmType) {
  LLVM::TypeFromLLVMIRTranslator *translator_ = unwrap(translator);
  mlir::Type type = translator_->translateType(toolchain::unwrap(llvmType));
  return wrap(type);
}

DEFINE_C_API_PTR_METHODS(MlirTypeToLLVMIRTranslator,
                         mlir::LLVM::TypeToLLVMIRTranslator)

MlirTypeToLLVMIRTranslator
mlirTypeToLLVMIRTranslatorCreate(LLVMContextRef ctx) {
  toolchain::LLVMContext *context = toolchain::unwrap(ctx);
  auto *translator = new LLVM::TypeToLLVMIRTranslator(*context);
  return wrap(translator);
}

void mlirTypeToLLVMIRTranslatorDestroy(MlirTypeToLLVMIRTranslator translator) {
  delete static_cast<LLVM::TypeToLLVMIRTranslator *>(unwrap(translator));
}

LLVMTypeRef
mlirTypeToLLVMIRTranslatorTranslateType(MlirTypeToLLVMIRTranslator translator,
                                        MlirType mlirType) {
  LLVM::TypeToLLVMIRTranslator *translator_ = unwrap(translator);
  toolchain::Type *type = translator_->translateType(unwrap(mlirType));
  return toolchain::wrap(type);
}
