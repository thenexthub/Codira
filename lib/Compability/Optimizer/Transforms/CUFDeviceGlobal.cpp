//===-- CUFDeviceGlobal.cpp -----------------------------------------------===//
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
#include "language/Compability/Optimizer/Dialect/CUF/CUFOps.h"
#include "language/Compability/Optimizer/Dialect/FIRDialect.h"
#include "language/Compability/Optimizer/Dialect/FIROps.h"
#include "language/Compability/Optimizer/HLFIR/HLFIROps.h"
#include "language/Compability/Optimizer/Support/InternalNames.h"
#include "language/Compability/Runtime/CUDA/common.h"
#include "language/Compability/Runtime/allocatable.h"
#include "language/Compability/Support/Fortran.h"
#include "mlir/Dialect/LLVMIR/NVVMDialect.h"
#include "mlir/IR/SymbolTable.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Transforms/DialectConversion.h"
#include "toolchain/ADT/DenseSet.h"

namespace fir {
#define GEN_PASS_DEF_CUFDEVICEGLOBAL
#include "language/Compability/Optimizer/Transforms/Passes.h.inc"
} // namespace fir

namespace {

static void processAddrOfOp(fir::AddrOfOp addrOfOp,
                            mlir::SymbolTable &symbolTable,
                            toolchain::DenseSet<fir::GlobalOp> &candidates,
                            bool recurseInGlobal) {

  // Check if there is a real use of the global.
  if (addrOfOp.getOperation()->hasOneUse()) {
    mlir::OpOperand &addrUse = *addrOfOp.getOperation()->getUses().begin();
    if (mlir::isa<fir::DeclareOp>(addrUse.getOwner()) &&
        addrUse.getOwner()->use_empty())
      return;
  }

  if (auto globalOp = symbolTable.lookup<fir::GlobalOp>(
          addrOfOp.getSymbol().getRootReference().getValue())) {
    // TO DO: limit candidates to non-scalars. Scalars appear to have been
    // folded in already.
    if (recurseInGlobal)
      globalOp.walk([&](fir::AddrOfOp op) {
        processAddrOfOp(op, symbolTable, candidates, recurseInGlobal);
      });
    candidates.insert(globalOp);
  }
}

static void processTypeDescriptor(fir::RecordType recTy,
                                  mlir::SymbolTable &symbolTable,
                                  toolchain::DenseSet<fir::GlobalOp> &candidates) {
  if (auto globalOp = symbolTable.lookup<fir::GlobalOp>(
          fir::NameUniquer::getTypeDescriptorName(recTy.getName()))) {
    if (!candidates.contains(globalOp)) {
      globalOp.walk([&](fir::AddrOfOp op) {
        processAddrOfOp(op, symbolTable, candidates,
                        /*recurseInGlobal=*/true);
      });
      candidates.insert(globalOp);
    }
  }
}

static void processEmboxOp(fir::EmboxOp emboxOp, mlir::SymbolTable &symbolTable,
                           toolchain::DenseSet<fir::GlobalOp> &candidates) {
  if (auto recTy = mlir::dyn_cast<fir::RecordType>(
          fir::unwrapRefType(emboxOp.getMemref().getType())))
    processTypeDescriptor(recTy, symbolTable, candidates);
}

static void
prepareImplicitDeviceGlobals(mlir::func::FuncOp funcOp,
                             mlir::SymbolTable &symbolTable,
                             toolchain::DenseSet<fir::GlobalOp> &candidates) {
  auto cudaProcAttr{
      funcOp->getAttrOfType<cuf::ProcAttributeAttr>(cuf::getProcAttrName())};
  if (cudaProcAttr && cudaProcAttr.getValue() != cuf::ProcAttribute::Host) {
    funcOp.walk([&](fir::AddrOfOp op) {
      processAddrOfOp(op, symbolTable, candidates, /*recurseInGlobal=*/false);
    });
    funcOp.walk(
        [&](fir::EmboxOp op) { processEmboxOp(op, symbolTable, candidates); });
  }
}

static void
processPotentialTypeDescriptor(mlir::Type candidateType,
                               mlir::SymbolTable &symbolTable,
                               toolchain::DenseSet<fir::GlobalOp> &candidates) {
  if (auto boxTy = mlir::dyn_cast<fir::BaseBoxType>(candidateType))
    candidateType = boxTy.getEleTy();
  candidateType = fir::unwrapSequenceType(fir::unwrapRefType(candidateType));
  if (auto recTy = mlir::dyn_cast<fir::RecordType>(candidateType))
    processTypeDescriptor(recTy, symbolTable, candidates);
}

class CUFDeviceGlobal : public fir::impl::CUFDeviceGlobalBase<CUFDeviceGlobal> {
public:
  void runOnOperation() override {
    mlir::Operation *op = getOperation();
    mlir::ModuleOp mod = mlir::dyn_cast<mlir::ModuleOp>(op);
    if (!mod)
      return signalPassFailure();

    toolchain::DenseSet<fir::GlobalOp> candidates;
    mlir::SymbolTable symTable(mod);
    mod.walk([&](mlir::func::FuncOp funcOp) {
      prepareImplicitDeviceGlobals(funcOp, symTable, candidates);
      return mlir::WalkResult::advance();
    });
    mod.walk([&](cuf::KernelOp kernelOp) {
      kernelOp.walk([&](fir::AddrOfOp addrOfOp) {
        processAddrOfOp(addrOfOp, symTable, candidates,
                        /*recurseInGlobal=*/false);
      });
    });

    // Copying the device global variable into the gpu module
    mlir::SymbolTable parentSymTable(mod);
    auto gpuMod = cuf::getOrCreateGPUModule(mod, parentSymTable);
    if (!gpuMod)
      return signalPassFailure();
    mlir::SymbolTable gpuSymTable(gpuMod);
    for (auto globalOp : mod.getOps<fir::GlobalOp>()) {
      if (cuf::isRegisteredDeviceGlobal(globalOp)) {
        candidates.insert(globalOp);
        processPotentialTypeDescriptor(globalOp.getType(), parentSymTable,
                                       candidates);
      } else if (globalOp.getConstant() &&
                 mlir::isa<fir::SequenceType>(
                     fir::unwrapRefType(globalOp.resultType()))) {
        mlir::Attribute initAttr =
            globalOp.getInitVal().value_or(mlir::Attribute());
        if (initAttr && mlir::dyn_cast<mlir::DenseElementsAttr>(initAttr))
          candidates.insert(globalOp);
      }
    }
    for (auto globalOp : candidates) {
      auto globalName{globalOp.getSymbol().getValue()};
      if (gpuSymTable.lookup<fir::GlobalOp>(globalName)) {
        break;
      }
      gpuSymTable.insert(globalOp->clone());
    }
  }
};
} // namespace
