//===-- CUFComputeSharedMemoryOffsetsAndSize.cpp --------------------------===//
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
#include "mlir/Dialect/LLVMIR/LLVMDialect.h"
#include "mlir/IR/Value.h"
#include "mlir/Pass/Pass.h"
#include "toolchain/ADT/SmallVector.h"

namespace fir {
#define GEN_PASS_DEF_CUFCOMPUTESHAREDMEMORYOFFSETSANDSIZE
#include "language/Compability/Optimizer/Transforms/Passes.h.inc"
} // namespace fir

using namespace language::Compability::runtime::cuda;

namespace {

struct CUFComputeSharedMemoryOffsetsAndSize
    : public fir::impl::CUFComputeSharedMemoryOffsetsAndSizeBase<
          CUFComputeSharedMemoryOffsetsAndSize> {

  void runOnOperation() override {
    mlir::ModuleOp mod = getOperation();
    mlir::SymbolTable symTab(mod);
    mlir::OpBuilder opBuilder{mod.getBodyRegion()};
    fir::FirOpBuilder builder(opBuilder, mod);
    fir::KindMapping kindMap{fir::getKindMapping(mod)};
    std::optional<mlir::DataLayout> dl =
        fir::support::getOrSetMLIRDataLayout(mod, /*allowDefaultLayout=*/false);
    if (!dl) {
      mlir::emitError(mod.getLoc(),
                      "data layout attribute is required to perform " +
                          getName() + "pass");
    }

    auto gpuMod = cuf::getOrCreateGPUModule(mod, symTab);
    mlir::Type i8Ty = builder.getI8Type();
    mlir::Type i32Ty = builder.getI32Type();
    mlir::Type idxTy = builder.getIndexType();
    for (auto funcOp : gpuMod.getOps<mlir::gpu::GPUFuncOp>()) {
      unsigned nbDynamicSharedVariables = 0;
      unsigned nbStaticSharedVariables = 0;
      uint64_t sharedMemSize = 0;
      unsigned short alignment = 0;
      mlir::Value crtDynOffset;

      // Go over each shared memory operation and compute their start offset and
      // the size and alignment of the global to be generated if all variables
      // are static. If this is dynamic shared memory, then only the alignment
      // is computed.
      for (auto sharedOp : funcOp.getOps<cuf::SharedMemoryOp>()) {
        mlir::Location loc = sharedOp.getLoc();
        builder.setInsertionPoint(sharedOp);
        if (fir::hasDynamicSize(sharedOp.getInType())) {
          mlir::Type ty = sharedOp.getInType();
          if (auto seqTy = mlir::dyn_cast<fir::SequenceType>(ty))
            ty = seqTy.getEleTy();
          unsigned short align = dl->getTypeABIAlignment(ty);
          alignment = std::max(alignment, align);
          uint64_t tySize = dl->getTypeSize(ty);
          ++nbDynamicSharedVariables;
          if (crtDynOffset) {
            sharedOp.getOffsetMutable().assign(
                builder.createConvert(loc, i32Ty, crtDynOffset));
          } else {
            mlir::Value zero = builder.createIntegerConstant(loc, i32Ty, 0);
            sharedOp.getOffsetMutable().assign(zero);
          }

          mlir::Value dynSize =
              builder.createIntegerConstant(loc, idxTy, tySize);
          for (auto extent : sharedOp.getShape())
            dynSize =
                mlir::arith::MulIOp::create(builder, loc, dynSize, extent);
          if (crtDynOffset)
            crtDynOffset = mlir::arith::AddIOp::create(builder, loc,
                                                       crtDynOffset, dynSize);
          else
            crtDynOffset = dynSize;

          continue;
        }
        auto [size, align] = fir::getTypeSizeAndAlignmentOrCrash(
            sharedOp.getLoc(), sharedOp.getInType(), *dl, kindMap);
        ++nbStaticSharedVariables;
        mlir::Value offset = builder.createIntegerConstant(
            loc, i32Ty, toolchain::alignTo(sharedMemSize, align));
        sharedOp.getOffsetMutable().assign(offset);
        sharedMemSize =
            toolchain::alignTo(sharedMemSize, align) + toolchain::alignTo(size, align);
        alignment = std::max(alignment, align);
      }

      if (nbDynamicSharedVariables == 0 && nbStaticSharedVariables == 0)
        continue;

      if (nbDynamicSharedVariables > 0 && nbStaticSharedVariables > 0)
        mlir::emitError(
            funcOp.getLoc(),
            "static and dynamic shared variables in a single kernel");

      mlir::DenseElementsAttr init = {};
      if (sharedMemSize > 0) {
        auto vecTy = mlir::VectorType::get(sharedMemSize, i8Ty);
        mlir::Attribute zero = mlir::IntegerAttr::get(i8Ty, 0);
        init = mlir::DenseElementsAttr::get(vecTy, toolchain::ArrayRef(zero));
      }

      // Create the shared memory global where each shared variable will point
      // to.
      auto sharedMemType = fir::SequenceType::get(sharedMemSize, i8Ty);
      std::string sharedMemGlobalName =
          (funcOp.getName() + toolchain::Twine(cudaSharedMemSuffix)).str();
      mlir::StringAttr linkage = builder.createInternalLinkage();
      builder.setInsertionPointToEnd(gpuMod.getBody());
      toolchain::SmallVector<mlir::NamedAttribute> attrs;
      auto globalOpName = mlir::OperationName(fir::GlobalOp::getOperationName(),
                                              gpuMod.getContext());
      attrs.push_back(mlir::NamedAttribute(
          fir::GlobalOp::getDataAttrAttrName(globalOpName),
          cuf::DataAttributeAttr::get(gpuMod.getContext(),
                                      cuf::DataAttribute::Shared)));
      auto sharedMem = fir::GlobalOp::create(
          builder, funcOp.getLoc(), sharedMemGlobalName, false, false,
          sharedMemType, init, linkage, attrs);
      sharedMem.setAlignment(alignment);
    }
  }
};

} // end anonymous namespace
