//===- ROCDLToLLVMIRTranslation.cpp - Translate ROCDL to LLVM IR ----------===//
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
// This file implements a translation between the MLIR ROCDL dialect and
// LLVM IR.
//
//===----------------------------------------------------------------------===//

#include "mlir/Target/LLVMIR/Dialect/ROCDL/ROCDLToLLVMIRTranslation.h"
#include "mlir/Dialect/LLVMIR/ROCDLDialect.h"
#include "mlir/IR/BuiltinAttributes.h"
#include "mlir/IR/Operation.h"
#include "mlir/Target/LLVMIR/ModuleTranslation.h"

#include "vm/core/IR/ConstantRange.h"
#include "vm/core/IR/IRBuilder.h"
#include "vm/core/IR/IntrinsicsAMDGPU.h"
#include "vm/core/Support/raw_ostream.h"

using namespace mlir;
using namespace mlir::LLVM;
using mlir::LLVM::detail::createIntrinsicCall;

// Create a call to ROCm-Device-Library function that returns an ID.
// This is intended to specifically call device functions that fetch things like
// block or grid dimensions, and so is limited to functions that take one
// integer parameter.
static toolchain::Value *createDimGetterFunctionCall(toolchain::IRBuilderBase &builder,
                                                Operation *op, StringRef fnName,
                                                int parameter) {
  toolchain::Module *module = builder.GetInsertBlock()->getModule();
  toolchain::FunctionType *functionType = toolchain::FunctionType::get(
      toolchain::Type::getInt64Ty(module->getContext()), // return type.
      toolchain::Type::getInt32Ty(module->getContext()), // parameter type.
      false);                                       // no variadic arguments.
  toolchain::Function *fn = dyn_cast<toolchain::Function>(
      module->getOrInsertFunction(fnName, functionType).getCallee());
  toolchain::Value *fnOp0 = toolchain::ConstantInt::get(
      toolchain::Type::getInt32Ty(module->getContext()), parameter);
  auto *call = builder.CreateCall(fn, ArrayRef<toolchain::Value *>(fnOp0));
  if (auto rangeAttr = op->getAttrOfType<LLVM::ConstantRangeAttr>("range")) {
    // Zero-extend to 64 bits because the GPU dialect uses 32-bit bounds but
    // these ockl functions are defined to be 64-bits
    call->addRangeRetAttr(toolchain::ConstantRange(rangeAttr.getLower().zext(64),
                                              rangeAttr.getUpper().zext(64)));
  }
  return call;
}

namespace {
/// Implementation of the dialect interface that converts operations belonging
/// to the ROCDL dialect to LLVM IR.
class ROCDLDialectLLVMIRTranslationInterface
    : public LLVMTranslationDialectInterface {
public:
  using LLVMTranslationDialectInterface::LLVMTranslationDialectInterface;

  /// Translates the given operation to LLVM IR using the provided IR builder
  /// and saving the state in `moduleTranslation`.
  LogicalResult
  convertOperation(Operation *op, toolchain::IRBuilderBase &builder,
                   LLVM::ModuleTranslation &moduleTranslation) const final {
    Operation &opInst = *op;
#include "mlir/Dialect/LLVMIR/ROCDLConversions.inc"

    return failure();
  }

  /// Attaches module-level metadata for functions marked as kernels.
  LogicalResult
  amendOperation(Operation *op, ArrayRef<toolchain::Instruction *> instructions,
                 NamedAttribute attribute,
                 LLVM::ModuleTranslation &moduleTranslation) const final {
    auto *dialect = dyn_cast<ROCDL::ROCDLDialect>(attribute.getNameDialect());
    toolchain::LLVMContext &llvmContext = moduleTranslation.getLLVMContext();
    if (dialect->getKernelAttrHelper().getName() == attribute.getName()) {
      auto func = dyn_cast<LLVM::LLVMFuncOp>(op);
      if (!func)
        return op->emitOpError(Twine(attribute.getName()) +
                               " is only supported on `toolchain.func` operations");
      ;

      // For GPU kernels,
      // 1. Insert AMDGPU_KERNEL calling convention.
      // 2. Insert amdgpu-flat-work-group-size(1, 256) attribute unless the user
      // has overriden this value - 256 is the default in clang
      toolchain::Function *llvmFunc =
          moduleTranslation.lookupFunction(func.getName());
      llvmFunc->setCallingConv(toolchain::CallingConv::AMDGPU_KERNEL);
      if (!llvmFunc->hasFnAttribute("amdgpu-flat-work-group-size")) {
        llvmFunc->addFnAttr("amdgpu-flat-work-group-size", "1,256");
      }

      // MLIR's GPU kernel APIs all assume and produce uniformly-sized
      // workgroups, so the lowering of the `rocdl.kernel` marker encodes this
      // assumption. This assumption may be overridden by setting
      // `rocdl.uniform_work_group_size` on a given function.
      if (!llvmFunc->hasFnAttribute("uniform-work-group-size"))
        llvmFunc->addFnAttr("uniform-work-group-size", "true");
    }
    // Override flat-work-group-size
    // TODO: update clients to rocdl.flat_work_group_size instead,
    // then remove this half of the branch
    if (dialect->getMaxFlatWorkGroupSizeAttrHelper().getName() ==
        attribute.getName()) {
      auto func = dyn_cast<LLVM::LLVMFuncOp>(op);
      if (!func)
        return op->emitOpError(Twine(attribute.getName()) +
                               " is only supported on `toolchain.func` operations");
      auto value = dyn_cast<IntegerAttr>(attribute.getValue());
      if (!value)
        return op->emitOpError(Twine(attribute.getName()) +
                               " must be an integer");

      toolchain::Function *llvmFunc =
          moduleTranslation.lookupFunction(func.getName());
      toolchain::SmallString<8> llvmAttrValue;
      toolchain::raw_svector_ostream attrValueStream(llvmAttrValue);
      attrValueStream << "1," << value.getInt();
      llvmFunc->addFnAttr("amdgpu-flat-work-group-size", llvmAttrValue);
    }
    if (dialect->getWavesPerEuAttrHelper().getName() == attribute.getName()) {
      auto func = dyn_cast<LLVM::LLVMFuncOp>(op);
      if (!func)
        return op->emitOpError(Twine(attribute.getName()) +
                               " is only supported on `toolchain.func` operations");
      auto value = dyn_cast<IntegerAttr>(attribute.getValue());
      if (!value)
        return op->emitOpError(Twine(attribute.getName()) +
                               " must be an integer");

      toolchain::Function *llvmFunc =
          moduleTranslation.lookupFunction(func.getName());
      toolchain::SmallString<8> llvmAttrValue;
      toolchain::raw_svector_ostream attrValueStream(llvmAttrValue);
      attrValueStream << value.getInt();
      llvmFunc->addFnAttr("amdgpu-waves-per-eu", llvmAttrValue);
    }
    if (dialect->getFlatWorkGroupSizeAttrHelper().getName() ==
        attribute.getName()) {
      auto func = dyn_cast<LLVM::LLVMFuncOp>(op);
      if (!func)
        return op->emitOpError(Twine(attribute.getName()) +
                               " is only supported on `toolchain.func` operations");
      auto value = dyn_cast<StringAttr>(attribute.getValue());
      if (!value)
        return op->emitOpError(Twine(attribute.getName()) +
                               " must be a string");

      toolchain::Function *llvmFunc =
          moduleTranslation.lookupFunction(func.getName());
      toolchain::SmallString<8> llvmAttrValue;
      llvmAttrValue.append(value.getValue());
      llvmFunc->addFnAttr("amdgpu-flat-work-group-size", llvmAttrValue);
    }
    if (ROCDL::ROCDLDialect::getUniformWorkGroupSizeAttrName() ==
        attribute.getName()) {
      auto func = dyn_cast<LLVM::LLVMFuncOp>(op);
      if (!func)
        return op->emitOpError(Twine(attribute.getName()) +
                               " is only supported on `toolchain.func` operations");
      auto value = dyn_cast<BoolAttr>(attribute.getValue());
      if (!value)
        return op->emitOpError(Twine(attribute.getName()) +
                               " must be a boolean");
      toolchain::Function *llvmFunc =
          moduleTranslation.lookupFunction(func.getName());
      llvmFunc->addFnAttr("uniform-work-group-size",
                          value.getValue() ? "true" : "false");
    }
    if (dialect->getUnsafeFpAtomicsAttrHelper().getName() ==
        attribute.getName()) {
      auto func = dyn_cast<LLVM::LLVMFuncOp>(op);
      if (!func)
        return op->emitOpError(Twine(attribute.getName()) +
                               " is only supported on `toolchain.func` operations");
      auto value = dyn_cast<BoolAttr>(attribute.getValue());
      if (!value)
        return op->emitOpError(Twine(attribute.getName()) +
                               " must be a boolean");
      toolchain::Function *llvmFunc =
          moduleTranslation.lookupFunction(func.getName());
      llvmFunc->addFnAttr("amdgpu-unsafe-fp-atomics",
                          value.getValue() ? "true" : "false");
    }
    // Set reqd_work_group_size metadata
    if (dialect->getReqdWorkGroupSizeAttrHelper().getName() ==
        attribute.getName()) {
      auto func = dyn_cast<LLVM::LLVMFuncOp>(op);
      if (!func)
        return op->emitOpError(Twine(attribute.getName()) +
                               " is only supported on `toolchain.func` operations");
      auto value = dyn_cast<DenseI32ArrayAttr>(attribute.getValue());
      if (!value)
        return op->emitOpError(Twine(attribute.getName()) +
                               " must be a dense i32 array attribute");
      SmallVector<toolchain::Metadata *, 3> metadata;
      toolchain::Type *i32 = toolchain::IntegerType::get(llvmContext, 32);
      for (int32_t i : value.asArrayRef()) {
        toolchain::Constant *constant = toolchain::ConstantInt::get(i32, i);
        metadata.push_back(toolchain::ConstantAsMetadata::get(constant));
      }
      toolchain::Function *llvmFunc =
          moduleTranslation.lookupFunction(func.getName());
      toolchain::MDNode *node = toolchain::MDNode::get(llvmContext, metadata);
      llvmFunc->setMetadata("reqd_work_group_size", node);
    }

    // Atomic and nontemporal metadata
    if (dialect->getLastUseAttrHelper().getName() == attribute.getName()) {
      for (toolchain::Instruction *i : instructions)
        i->setMetadata("amdgpu.last.use", toolchain::MDNode::get(llvmContext, {}));
    }
    if (dialect->getNoRemoteMemoryAttrHelper().getName() ==
        attribute.getName()) {
      for (toolchain::Instruction *i : instructions)
        i->setMetadata("amdgpu.no.remote.memory",
                       toolchain::MDNode::get(llvmContext, {}));
    }
    if (dialect->getNoFineGrainedMemoryAttrHelper().getName() ==
        attribute.getName()) {
      for (toolchain::Instruction *i : instructions)
        i->setMetadata("amdgpu.no.fine.grained.memory",
                       toolchain::MDNode::get(llvmContext, {}));
    }
    if (dialect->getIgnoreDenormalModeAttrHelper().getName() ==
        attribute.getName()) {
      for (toolchain::Instruction *i : instructions)
        i->setMetadata("amdgpu.ignore.denormal.mode",
                       toolchain::MDNode::get(llvmContext, {}));
    }

    return success();
  }
};
} // namespace

void mlir::registerROCDLDialectTranslation(DialectRegistry &registry) {
  registry.insert<ROCDL::ROCDLDialect>();
  registry.addExtension(+[](MLIRContext *ctx, ROCDL::ROCDLDialect *dialect) {
    dialect->addInterfaces<ROCDLDialectLLVMIRTranslationInterface>();
  });
}

void mlir::registerROCDLDialectTranslation(MLIRContext &context) {
  DialectRegistry registry;
  registerROCDLDialectTranslation(registry);
  context.appendDialectRegistry(registry);
}
