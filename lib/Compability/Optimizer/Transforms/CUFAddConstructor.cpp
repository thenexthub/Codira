//===-- CUFAddConstructor.cpp ---------------------------------------------===//
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

#include "language/Compability/Optimizer/Builder/BoxValue.h"
#include "language/Compability/Optimizer/Builder/CUFCommon.h"
#include "language/Compability/Optimizer/Builder/FIRBuilder.h"
#include "language/Compability/Optimizer/Builder/Runtime/RTBuilder.h"
#include "language/Compability/Optimizer/Builder/Todo.h"
#include "language/Compability/Optimizer/CodeGen/Target.h"
#include "language/Compability/Optimizer/CodeGen/TypeConverter.h"
#include "language/Compability/Optimizer/Dialect/CUF/CUFOps.h"
#include "language/Compability/Optimizer/Dialect/FIRAttr.h"
#include "language/Compability/Optimizer/Dialect/FIRDialect.h"
#include "language/Compability/Optimizer/Dialect/FIROps.h"
#include "language/Compability/Optimizer/Dialect/FIROpsSupport.h"
#include "language/Compability/Optimizer/Dialect/FIRType.h"
#include "language/Compability/Optimizer/Support/DataLayout.h"
#include "language/Compability/Runtime/CUDA/registration.h"
#include "language/Compability/Runtime/entry-names.h"
#include "mlir/Dialect/DLTI/DLTI.h"
#include "mlir/Dialect/GPU/IR/GPUDialect.h"
#include "mlir/Dialect/LLVMIR/LLVMAttrs.h"
#include "mlir/Dialect/LLVMIR/LLVMDialect.h"
#include "mlir/IR/Value.h"
#include "mlir/Pass/Pass.h"
#include "toolchain/ADT/SmallVector.h"

namespace fir {
#define GEN_PASS_DEF_CUFADDCONSTRUCTOR
#include "language/Compability/Optimizer/Transforms/Passes.h.inc"
} // namespace fir

using namespace language::Compability::runtime::cuda;

namespace {

static constexpr toolchain::StringRef cudaFortranCtorName{
    "__cudaFortranConstructor"};

struct CUFAddConstructor
    : public fir::impl::CUFAddConstructorBase<CUFAddConstructor> {

  void runOnOperation() override {
    mlir::ModuleOp mod = getOperation();
    mlir::SymbolTable symTab(mod);
    mlir::OpBuilder opBuilder{mod.getBodyRegion()};
    fir::FirOpBuilder builder(opBuilder, mod);
    fir::KindMapping kindMap{fir::getKindMapping(mod)};
    builder.setInsertionPointToEnd(mod.getBody());
    mlir::Location loc = mod.getLoc();
    auto *ctx = mod.getContext();
    auto voidTy = mlir::LLVM::LLVMVoidType::get(ctx);
    auto idxTy = builder.getIndexType();
    auto funcTy =
        mlir::LLVM::LLVMFunctionType::get(voidTy, {}, /*isVarArg=*/false);
    std::optional<mlir::DataLayout> dl =
        fir::support::getOrSetMLIRDataLayout(mod, /*allowDefaultLayout=*/false);
    if (!dl) {
      mlir::emitError(mod.getLoc(),
                      "data layout attribute is required to perform " +
                          getName() + "pass");
    }

    // Symbol reference to CUFRegisterAllocator.
    builder.setInsertionPointToEnd(mod.getBody());
    auto registerFuncOp = mlir::LLVM::LLVMFuncOp::create(
        builder, loc, RTNAME_STRING(CUFRegisterAllocator), funcTy);
    registerFuncOp.setVisibility(mlir::SymbolTable::Visibility::Private);
    auto cufRegisterAllocatorRef = mlir::SymbolRefAttr::get(
        mod.getContext(), RTNAME_STRING(CUFRegisterAllocator));
    builder.setInsertionPointToEnd(mod.getBody());

    // Create the constructor function that call CUFRegisterAllocator.
    auto func = mlir::LLVM::LLVMFuncOp::create(builder, loc,
                                               cudaFortranCtorName, funcTy);
    func.setLinkage(mlir::LLVM::Linkage::Internal);
    builder.setInsertionPointToStart(func.addEntryBlock(builder));
    mlir::LLVM::CallOp::create(builder, loc, funcTy, cufRegisterAllocatorRef);

    auto gpuMod = symTab.lookup<mlir::gpu::GPUModuleOp>(cudaDeviceModuleName);
    if (gpuMod) {
      auto toolchainPtrTy = mlir::LLVM::LLVMPointerType::get(ctx);
      auto registeredMod = cuf::RegisterModuleOp::create(
          builder, loc, toolchainPtrTy,
          mlir::SymbolRefAttr::get(ctx, gpuMod.getName()));

      fir::LLVMTypeConverter typeConverter(mod, /*applyTBAA=*/false,
                                           /*forceUnifiedTBAATree=*/false, *dl);
      // Register kernels
      for (auto func : gpuMod.getOps<mlir::gpu::GPUFuncOp>()) {
        if (func.isKernel()) {
          auto kernelName = mlir::SymbolRefAttr::get(
              builder.getStringAttr(cudaDeviceModuleName),
              {mlir::SymbolRefAttr::get(builder.getContext(), func.getName())});
          cuf::RegisterKernelOp::create(builder, loc, kernelName,
                                        registeredMod);
        }
      }

      // Register variables
      for (fir::GlobalOp globalOp : mod.getOps<fir::GlobalOp>()) {
        auto attr = globalOp.getDataAttrAttr();
        if (!attr)
          continue;

        if (attr.getValue() == cuf::DataAttribute::Managed &&
            !mlir::isa<fir::BaseBoxType>(globalOp.getType()))
          TODO(loc, "registration of non-allocatable managed variables");

        mlir::func::FuncOp func;
        switch (attr.getValue()) {
        case cuf::DataAttribute::Device:
        case cuf::DataAttribute::Constant:
        case cuf::DataAttribute::Managed: {
          func = fir::runtime::getRuntimeFunc<mkRTKey(CUFRegisterVariable)>(
              loc, builder);
          auto fTy = func.getFunctionType();

          // Global variable name
          std::string gblNameStr = globalOp.getSymbol().getValue().str();
          gblNameStr += '\0';
          mlir::Value gblName = fir::getBase(
              fir::factory::createStringLiteral(builder, loc, gblNameStr));

          // Global variable size
          std::optional<uint64_t> size;
          if (auto boxTy =
                  mlir::dyn_cast<fir::BaseBoxType>(globalOp.getType())) {
            mlir::Type structTy = typeConverter.convertBoxTypeAsStruct(boxTy);
            size = dl->getTypeSizeInBits(structTy) / 8;
          }
          if (!size) {
            size = fir::getTypeSizeAndAlignmentOrCrash(loc, globalOp.getType(),
                                                       *dl, kindMap)
                       .first;
          }
          auto sizeVal = builder.createIntegerConstant(loc, idxTy, *size);

          // Global variable address
          mlir::Value addr = fir::AddrOfOp::create(
              builder, loc, globalOp.resultType(), globalOp.getSymbol());

          toolchain::SmallVector<mlir::Value> args{fir::runtime::createArguments(
              builder, loc, fTy, registeredMod, addr, gblName, sizeVal)};
          fir::CallOp::create(builder, loc, func, args);
        } break;
        default:
          break;
        }
      }
    }
    mlir::LLVM::ReturnOp::create(builder, loc, mlir::ValueRange{});

    // Create the toolchain.global_ctor with the function.
    // TODO: We might want to have a utility that retrieve it if already
    // created and adds new functions.
    builder.setInsertionPointToEnd(mod.getBody());
    toolchain::SmallVector<mlir::Attribute> funcs;
    funcs.push_back(
        mlir::FlatSymbolRefAttr::get(mod.getContext(), func.getSymName()));
    toolchain::SmallVector<int> priorities;
    toolchain::SmallVector<mlir::Attribute> data;
    priorities.push_back(0);
    data.push_back(mlir::LLVM::ZeroAttr::get(mod.getContext()));
    mlir::LLVM::GlobalCtorsOp::create(
        builder, mod.getLoc(), builder.getArrayAttr(funcs),
        builder.getI32ArrayAttr(priorities), builder.getArrayAttr(data));
  }
};

} // end anonymous namespace
