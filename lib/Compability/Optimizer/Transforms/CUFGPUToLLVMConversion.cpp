//===-- CUFGPUToLLVMConversion.cpp ----------------------------------------===//
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

#include "language/Compability/Optimizer/Transforms/CUFGPUToLLVMConversion.h"
#include "language/Compability/Optimizer/Builder/CUFCommon.h"
#include "language/Compability/Optimizer/CodeGen/TypeConverter.h"
#include "language/Compability/Optimizer/Dialect/CUF/CUFOps.h"
#include "language/Compability/Optimizer/Support/DataLayout.h"
#include "language/Compability/Runtime/CUDA/common.h"
#include "language/Compability/Support/Fortran.h"
#include "mlir/Conversion/LLVMCommon/Pattern.h"
#include "mlir/Dialect/DLTI/DLTI.h"
#include "mlir/Dialect/GPU/IR/GPUDialect.h"
#include "mlir/Dialect/LLVMIR/NVVMDialect.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Transforms/DialectConversion.h"
#include "mlir/Transforms/GreedyPatternRewriteDriver.h"
#include "toolchain/Support/FormatVariadic.h"

namespace fir {
#define GEN_PASS_DEF_CUFGPUTOLLVMCONVERSION
#include "language/Compability/Optimizer/Transforms/Passes.h.inc"
} // namespace fir

using namespace fir;
using namespace mlir;
using namespace language::Compability::runtime;

namespace {

static mlir::Value createKernelArgArray(mlir::Location loc,
                                        mlir::ValueRange operands,
                                        mlir::PatternRewriter &rewriter) {

  auto *ctx = rewriter.getContext();
  toolchain::SmallVector<mlir::Type> structTypes(operands.size(), nullptr);

  for (auto [i, arg] : toolchain::enumerate(operands))
    structTypes[i] = arg.getType();

  auto structTy = mlir::LLVM::LLVMStructType::getLiteral(ctx, structTypes);
  auto ptrTy = mlir::LLVM::LLVMPointerType::get(rewriter.getContext());
  mlir::Type i32Ty = rewriter.getI32Type();
  auto zero = mlir::LLVM::ConstantOp::create(rewriter, loc, i32Ty,
                                             rewriter.getIntegerAttr(i32Ty, 0));
  auto one = mlir::LLVM::ConstantOp::create(rewriter, loc, i32Ty,
                                            rewriter.getIntegerAttr(i32Ty, 1));
  mlir::Value argStruct =
      mlir::LLVM::AllocaOp::create(rewriter, loc, ptrTy, structTy, one);
  auto size = mlir::LLVM::ConstantOp::create(
      rewriter, loc, i32Ty, rewriter.getIntegerAttr(i32Ty, structTypes.size()));
  mlir::Value argArray =
      mlir::LLVM::AllocaOp::create(rewriter, loc, ptrTy, ptrTy, size);

  for (auto [i, arg] : toolchain::enumerate(operands)) {
    auto indice = mlir::LLVM::ConstantOp::create(
        rewriter, loc, i32Ty, rewriter.getIntegerAttr(i32Ty, i));
    mlir::Value structMember =
        LLVM::GEPOp::create(rewriter, loc, ptrTy, structTy, argStruct,
                            mlir::ArrayRef<mlir::Value>({zero, indice}));
    LLVM::StoreOp::create(rewriter, loc, arg, structMember);
    mlir::Value arrayMember =
        LLVM::GEPOp::create(rewriter, loc, ptrTy, ptrTy, argArray,
                            mlir::ArrayRef<mlir::Value>({indice}));
    LLVM::StoreOp::create(rewriter, loc, structMember, arrayMember);
  }
  return argArray;
}

struct GPULaunchKernelConversion
    : public mlir::ConvertOpToLLVMPattern<mlir::gpu::LaunchFuncOp> {
  explicit GPULaunchKernelConversion(
      const fir::LLVMTypeConverter &typeConverter, mlir::PatternBenefit benefit)
      : mlir::ConvertOpToLLVMPattern<mlir::gpu::LaunchFuncOp>(typeConverter,
                                                              benefit) {}

  using OpAdaptor = typename mlir::gpu::LaunchFuncOp::Adaptor;

  mlir::LogicalResult
  matchAndRewrite(mlir::gpu::LaunchFuncOp op, OpAdaptor adaptor,
                  mlir::ConversionPatternRewriter &rewriter) const override {
    // Only convert gpu.launch_func for CUDA Fortran.
    if (!op.getOperation()->getAttrOfType<cuf::ProcAttributeAttr>(
            cuf::getProcAttrName()))
      return mlir::failure();

    mlir::Location loc = op.getLoc();
    auto *ctx = rewriter.getContext();
    mlir::ModuleOp mod = op->getParentOfType<mlir::ModuleOp>();
    mlir::Value dynamicMemorySize = op.getDynamicSharedMemorySize();
    mlir::Type i32Ty = rewriter.getI32Type();
    if (!dynamicMemorySize)
      dynamicMemorySize = mlir::LLVM::ConstantOp::create(
          rewriter, loc, i32Ty, rewriter.getIntegerAttr(i32Ty, 0));

    mlir::Value kernelArgs =
        createKernelArgArray(loc, adaptor.getKernelOperands(), rewriter);

    auto ptrTy = mlir::LLVM::LLVMPointerType::get(rewriter.getContext());
    auto kernel = mod.lookupSymbol<mlir::LLVM::LLVMFuncOp>(op.getKernelName());
    mlir::Value kernelPtr;
    if (!kernel) {
      auto funcOp = mod.lookupSymbol<mlir::func::FuncOp>(op.getKernelName());
      if (!funcOp)
        return mlir::failure();
      kernelPtr =
          LLVM::AddressOfOp::create(rewriter, loc, ptrTy, funcOp.getName());
    } else {
      kernelPtr =
          LLVM::AddressOfOp::create(rewriter, loc, ptrTy, kernel.getName());
    }

    auto toolchainIntPtrType = mlir::IntegerType::get(
        ctx, this->getTypeConverter()->getPointerBitwidth(0));
    auto voidTy = mlir::LLVM::LLVMVoidType::get(ctx);

    mlir::Value nullPtr = LLVM::ZeroOp::create(rewriter, loc, ptrTy);

    if (op.hasClusterSize()) {
      auto funcOp = mod.lookupSymbol<mlir::LLVM::LLVMFuncOp>(
          RTNAME_STRING(CUFLaunchClusterKernel));
      auto funcTy = mlir::LLVM::LLVMFunctionType::get(
          voidTy,
          {ptrTy, toolchainIntPtrType, toolchainIntPtrType, toolchainIntPtrType,
           toolchainIntPtrType, toolchainIntPtrType, toolchainIntPtrType, toolchainIntPtrType,
           toolchainIntPtrType, toolchainIntPtrType, ptrTy, i32Ty, ptrTy, ptrTy},
          /*isVarArg=*/false);
      auto cufLaunchClusterKernel = mlir::SymbolRefAttr::get(
          mod.getContext(), RTNAME_STRING(CUFLaunchClusterKernel));
      if (!funcOp) {
        mlir::OpBuilder::InsertionGuard insertGuard(rewriter);
        rewriter.setInsertionPointToStart(mod.getBody());
        auto launchKernelFuncOp = mlir::LLVM::LLVMFuncOp::create(
            rewriter, loc, RTNAME_STRING(CUFLaunchClusterKernel), funcTy);
        launchKernelFuncOp.setVisibility(
            mlir::SymbolTable::Visibility::Private);
      }

      mlir::Value stream = nullPtr;
      if (!adaptor.getAsyncDependencies().empty()) {
        if (adaptor.getAsyncDependencies().size() != 1)
          return rewriter.notifyMatchFailure(
              op, "Can only convert with exactly one stream dependency.");
        stream = adaptor.getAsyncDependencies().front();
      }

      mlir::LLVM::CallOp::create(
          rewriter, loc, funcTy, cufLaunchClusterKernel,
          mlir::ValueRange{kernelPtr, adaptor.getClusterSizeX(),
                           adaptor.getClusterSizeY(), adaptor.getClusterSizeZ(),
                           adaptor.getGridSizeX(), adaptor.getGridSizeY(),
                           adaptor.getGridSizeZ(), adaptor.getBlockSizeX(),
                           adaptor.getBlockSizeY(), adaptor.getBlockSizeZ(),
                           stream, dynamicMemorySize, kernelArgs, nullPtr});
      rewriter.eraseOp(op);
    } else {
      auto procAttr =
          op->getAttrOfType<cuf::ProcAttributeAttr>(cuf::getProcAttrName());
      bool isGridGlobal =
          procAttr && procAttr.getValue() == cuf::ProcAttribute::GridGlobal;
      toolchain::StringRef fctName = isGridGlobal
                                    ? RTNAME_STRING(CUFLaunchCooperativeKernel)
                                    : RTNAME_STRING(CUFLaunchKernel);
      auto funcOp = mod.lookupSymbol<mlir::LLVM::LLVMFuncOp>(fctName);
      auto funcTy = mlir::LLVM::LLVMFunctionType::get(
          voidTy,
          {ptrTy, toolchainIntPtrType, toolchainIntPtrType, toolchainIntPtrType,
           toolchainIntPtrType, toolchainIntPtrType, toolchainIntPtrType, ptrTy, i32Ty, ptrTy,
           ptrTy},
          /*isVarArg=*/false);
      auto cufLaunchKernel =
          mlir::SymbolRefAttr::get(mod.getContext(), fctName);
      if (!funcOp) {
        mlir::OpBuilder::InsertionGuard insertGuard(rewriter);
        rewriter.setInsertionPointToStart(mod.getBody());
        auto launchKernelFuncOp =
            mlir::LLVM::LLVMFuncOp::create(rewriter, loc, fctName, funcTy);
        launchKernelFuncOp.setVisibility(
            mlir::SymbolTable::Visibility::Private);
      }

      mlir::Value stream = nullPtr;
      if (!adaptor.getAsyncDependencies().empty()) {
        if (adaptor.getAsyncDependencies().size() != 1)
          return rewriter.notifyMatchFailure(
              op, "Can only convert with exactly one stream dependency.");
        stream = adaptor.getAsyncDependencies().front();
      }

      mlir::LLVM::CallOp::create(
          rewriter, loc, funcTy, cufLaunchKernel,
          mlir::ValueRange{kernelPtr, adaptor.getGridSizeX(),
                           adaptor.getGridSizeY(), adaptor.getGridSizeZ(),
                           adaptor.getBlockSizeX(), adaptor.getBlockSizeY(),
                           adaptor.getBlockSizeZ(), stream, dynamicMemorySize,
                           kernelArgs, nullPtr});
      rewriter.eraseOp(op);
    }

    return mlir::success();
  }
};

static std::string getFuncName(cuf::SharedMemoryOp op) {
  if (auto gpuFuncOp = op->getParentOfType<mlir::gpu::GPUFuncOp>())
    return gpuFuncOp.getName().str();
  if (auto funcOp = op->getParentOfType<mlir::func::FuncOp>())
    return funcOp.getName().str();
  if (auto toolchainFuncOp = op->getParentOfType<mlir::LLVM::LLVMFuncOp>())
    return toolchainFuncOp.getSymName().str();
  return "";
}

static mlir::Value createAddressOfOp(mlir::ConversionPatternRewriter &rewriter,
                                     mlir::Location loc,
                                     gpu::GPUModuleOp gpuMod,
                                     std::string &sharedGlobalName) {
  auto toolchainPtrTy = mlir::LLVM::LLVMPointerType::get(
      rewriter.getContext(), mlir::NVVM::NVVMMemorySpace::kSharedMemorySpace);
  if (auto g = gpuMod.lookupSymbol<fir::GlobalOp>(sharedGlobalName))
    return mlir::LLVM::AddressOfOp::create(rewriter, loc, toolchainPtrTy,
                                           g.getSymName());
  if (auto g = gpuMod.lookupSymbol<mlir::LLVM::GlobalOp>(sharedGlobalName))
    return mlir::LLVM::AddressOfOp::create(rewriter, loc, toolchainPtrTy,
                                           g.getSymName());
  return {};
}

struct CUFSharedMemoryOpConversion
    : public mlir::ConvertOpToLLVMPattern<cuf::SharedMemoryOp> {
  explicit CUFSharedMemoryOpConversion(
      const fir::LLVMTypeConverter &typeConverter, mlir::PatternBenefit benefit)
      : mlir::ConvertOpToLLVMPattern<cuf::SharedMemoryOp>(typeConverter,
                                                          benefit) {}
  using OpAdaptor = typename cuf::SharedMemoryOp::Adaptor;

  mlir::LogicalResult
  matchAndRewrite(cuf::SharedMemoryOp op, OpAdaptor adaptor,
                  mlir::ConversionPatternRewriter &rewriter) const override {
    mlir::Location loc = op->getLoc();
    if (!op.getOffset())
      mlir::emitError(loc,
                      "cuf.shared_memory must have an offset for code gen");

    auto gpuMod = op->getParentOfType<gpu::GPUModuleOp>();
    std::string sharedGlobalName =
        (getFuncName(op) + toolchain::Twine(cudaSharedMemSuffix)).str();
    mlir::Value sharedGlobalAddr =
        createAddressOfOp(rewriter, loc, gpuMod, sharedGlobalName);

    if (!sharedGlobalAddr)
      mlir::emitError(loc, "Could not find the shared global operation\n");

    auto castPtr = mlir::LLVM::AddrSpaceCastOp::create(
        rewriter, loc, mlir::LLVM::LLVMPointerType::get(rewriter.getContext()),
        sharedGlobalAddr);
    mlir::Type baseType = castPtr->getResultTypes().front();
    toolchain::SmallVector<mlir::LLVM::GEPArg> gepArgs = {op.getOffset()};
    mlir::Value shmemPtr = mlir::LLVM::GEPOp::create(
        rewriter, loc, baseType, rewriter.getI8Type(), castPtr, gepArgs);
    rewriter.replaceOp(op, {shmemPtr});
    return mlir::success();
  }
};

struct CUFStreamCastConversion
    : public mlir::ConvertOpToLLVMPattern<cuf::StreamCastOp> {
  explicit CUFStreamCastConversion(const fir::LLVMTypeConverter &typeConverter,
                                   mlir::PatternBenefit benefit)
      : mlir::ConvertOpToLLVMPattern<cuf::StreamCastOp>(typeConverter,
                                                        benefit) {}
  using OpAdaptor = typename cuf::StreamCastOp::Adaptor;

  mlir::LogicalResult
  matchAndRewrite(cuf::StreamCastOp op, OpAdaptor adaptor,
                  mlir::ConversionPatternRewriter &rewriter) const override {
    rewriter.replaceOp(op, adaptor.getStream());
    return mlir::success();
  }
};

class CUFGPUToLLVMConversion
    : public fir::impl::CUFGPUToLLVMConversionBase<CUFGPUToLLVMConversion> {
public:
  void runOnOperation() override {
    auto *ctx = &getContext();
    mlir::RewritePatternSet patterns(ctx);
    mlir::ConversionTarget target(*ctx);

    mlir::Operation *op = getOperation();
    mlir::ModuleOp module = mlir::dyn_cast<mlir::ModuleOp>(op);
    if (!module)
      return signalPassFailure();

    std::optional<mlir::DataLayout> dl = fir::support::getOrSetMLIRDataLayout(
        module, /*allowDefaultLayout=*/false);
    fir::LLVMTypeConverter typeConverter(module, /*applyTBAA=*/false,
                                         /*forceUnifiedTBAATree=*/false, *dl);
    cuf::populateCUFGPUToLLVMConversionPatterns(typeConverter, patterns);

    target.addDynamicallyLegalOp<mlir::gpu::LaunchFuncOp>(
        [&](mlir::gpu::LaunchFuncOp op) {
          if (op.getOperation()->getAttrOfType<cuf::ProcAttributeAttr>(
                  cuf::getProcAttrName()))
            return false;
          return true;
        });

    target.addIllegalOp<cuf::SharedMemoryOp>();
    target.addLegalDialect<mlir::LLVM::LLVMDialect>();
    if (mlir::failed(mlir::applyPartialConversion(getOperation(), target,
                                                  std::move(patterns)))) {
      mlir::emitError(mlir::UnknownLoc::get(ctx),
                      "error in CUF GPU op conversion\n");
      signalPassFailure();
    }
  }
};
} // namespace

void cuf::populateCUFGPUToLLVMConversionPatterns(
    fir::LLVMTypeConverter &converter, mlir::RewritePatternSet &patterns,
    mlir::PatternBenefit benefit) {
  converter.addConversion([&converter](mlir::gpu::AsyncTokenType) -> Type {
    return mlir::LLVM::LLVMPointerType::get(&converter.getContext());
  });
  patterns.add<CUFSharedMemoryOpConversion, GPULaunchKernelConversion,
               CUFStreamCastConversion>(converter, benefit);
}
