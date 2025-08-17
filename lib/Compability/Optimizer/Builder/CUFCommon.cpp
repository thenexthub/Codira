//===-- CUFCommon.cpp - Shared functions between passes ---------*- C++ -*-===//
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

#include "language/Compability/Optimizer/Builder/CUFCommon.h"
#include "language/Compability/Optimizer/Builder/FIRBuilder.h"
#include "language/Compability/Optimizer/Dialect/CUF/CUFOps.h"
#include "language/Compability/Optimizer/HLFIR/HLFIROps.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/Dialect/LLVMIR/NVVMDialect.h"
#include "mlir/Dialect/OpenACC/OpenACC.h"

/// Retrieve or create the CUDA Fortran GPU module in the give in \p mod.
mlir::gpu::GPUModuleOp cuf::getOrCreateGPUModule(mlir::ModuleOp mod,
                                                 mlir::SymbolTable &symTab) {
  if (auto gpuMod = symTab.lookup<mlir::gpu::GPUModuleOp>(cudaDeviceModuleName))
    return gpuMod;

  auto *ctx = mod.getContext();
  mod->setAttr(mlir::gpu::GPUDialect::getContainerModuleAttrName(),
               mlir::UnitAttr::get(ctx));

  mlir::OpBuilder builder(ctx);
  auto gpuMod = mlir::gpu::GPUModuleOp::create(builder, mod.getLoc(),
                                               cudaDeviceModuleName);
  mlir::Block::iterator insertPt(mod.getBodyRegion().front().end());
  symTab.insert(gpuMod, insertPt);
  return gpuMod;
}

bool cuf::isCUDADeviceContext(mlir::Operation *op) {
  if (!op || !op->getParentRegion())
    return false;
  return isCUDADeviceContext(*op->getParentRegion());
}

// Check if the insertion point is currently in a device context. HostDevice
// subprogram are not considered fully device context so it will return false
// for it.
// If the insertion point is inside an OpenACC region op, it is considered
// device context.
bool cuf::isCUDADeviceContext(mlir::Region &region,
                              bool isDoConcurrentOffloadEnabled) {
  if (region.getParentOfType<cuf::KernelOp>())
    return true;
  if (region.getParentOfType<mlir::acc::ComputeRegionOpInterface>())
    return true;
  if (auto funcOp = region.getParentOfType<mlir::func::FuncOp>()) {
    if (auto cudaProcAttr =
            funcOp.getOperation()->getAttrOfType<cuf::ProcAttributeAttr>(
                cuf::getProcAttrName())) {
      return cudaProcAttr.getValue() != cuf::ProcAttribute::Host &&
             cudaProcAttr.getValue() != cuf::ProcAttribute::HostDevice;
    }
  }
  if (isDoConcurrentOffloadEnabled &&
      region.getParentOfType<fir::DoConcurrentLoopOp>())
    return true;
  return false;
}

bool cuf::isRegisteredDeviceAttr(std::optional<cuf::DataAttribute> attr) {
  if (attr && (*attr == cuf::DataAttribute::Device ||
               *attr == cuf::DataAttribute::Managed ||
               *attr == cuf::DataAttribute::Constant))
    return true;
  return false;
}

bool cuf::isRegisteredDeviceGlobal(fir::GlobalOp op) {
  if (op.getConstant())
    return false;
  return isRegisteredDeviceAttr(op.getDataAttr());
}

void cuf::genPointerSync(const mlir::Value box, fir::FirOpBuilder &builder) {
  if (auto declareOp = box.getDefiningOp<hlfir::DeclareOp>()) {
    if (auto addrOfOp = declareOp.getMemref().getDefiningOp<fir::AddrOfOp>()) {
      auto mod = addrOfOp->getParentOfType<mlir::ModuleOp>();
      if (auto globalOp =
              mod.lookupSymbol<fir::GlobalOp>(addrOfOp.getSymbol())) {
        if (cuf::isRegisteredDeviceGlobal(globalOp)) {
          cuf::SyncDescriptorOp::create(builder, box.getLoc(),
                                        addrOfOp.getSymbol());
        }
      }
    }
  }
}
