//=== CompilerGeneratedNames.cpp - convert special symbols in global names ===//
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

#include "language/Compability/Optimizer/Dialect/FIRDialect.h"
#include "language/Compability/Optimizer/Dialect/FIROps.h"
#include "language/Compability/Optimizer/Dialect/FIROpsSupport.h"
#include "language/Compability/Optimizer/Support/InternalNames.h"
#include "language/Compability/Optimizer/Transforms/Passes.h"
#include "mlir/Dialect/GPU/IR/GPUDialect.h"
#include "mlir/IR/Attributes.h"
#include "mlir/IR/SymbolTable.h"
#include "mlir/Pass/Pass.h"

namespace fir {
#define GEN_PASS_DEF_COMPILERGENERATEDNAMESCONVERSION
#include "language/Compability/Optimizer/Transforms/Passes.h.inc"
} // namespace fir

using namespace mlir;

namespace {

class CompilerGeneratedNamesConversionPass
    : public fir::impl::CompilerGeneratedNamesConversionBase<
          CompilerGeneratedNamesConversionPass> {
public:
  using CompilerGeneratedNamesConversionBase<
      CompilerGeneratedNamesConversionPass>::
      CompilerGeneratedNamesConversionBase;

  mlir::ModuleOp getModule() { return getOperation(); }
  void runOnOperation() override;
};
} // namespace

void CompilerGeneratedNamesConversionPass::runOnOperation() {
  auto op = getOperation();
  auto *context = &getContext();

  toolchain::DenseMap<mlir::StringAttr, mlir::FlatSymbolRefAttr> remappings;

  auto processOp = [&](mlir::Operation &op) {
    auto symName = op.getAttrOfType<mlir::StringAttr>(
        mlir::SymbolTable::getSymbolAttrName());
    auto deconstructedName = fir::NameUniquer::deconstruct(symName);
    if (deconstructedName.first != fir::NameUniquer::NameKind::NOT_UNIQUED &&
        !fir::NameUniquer::isExternalFacingUniquedName(deconstructedName)) {
      std::string newName =
          fir::NameUniquer::replaceSpecialSymbols(symName.getValue().str());
      if (newName != symName) {
        auto newAttr = mlir::StringAttr::get(context, newName);
        mlir::SymbolTable::setSymbolName(&op, newAttr);
        auto newSymRef = mlir::FlatSymbolRefAttr::get(newAttr);
        remappings.try_emplace(symName, newSymRef);
      }
    }
  };
  for (auto &op : op->getRegion(0).front()) {
    if (toolchain::isa<mlir::func::FuncOp>(op) || toolchain::isa<fir::GlobalOp>(op))
      processOp(op);
    else if (auto gpuMod = mlir::dyn_cast<mlir::gpu::GPUModuleOp>(&op))
      for (auto &op : gpuMod->getRegion(0).front())
        if (toolchain::isa<mlir::func::FuncOp>(op) || toolchain::isa<fir::GlobalOp>(op) ||
            toolchain::isa<mlir::gpu::GPUFuncOp>(op))
          processOp(op);
  }

  if (remappings.empty())
    return;

  // Update all uses of the functions and globals that have been renamed.
  op.walk([&remappings](mlir::Operation *nestedOp) {
    toolchain::SmallVector<std::pair<mlir::StringAttr, mlir::SymbolRefAttr>> updates;
    for (const mlir::NamedAttribute &attr : nestedOp->getAttrDictionary())
      if (auto symRef = toolchain::dyn_cast<mlir::SymbolRefAttr>(attr.getValue()))
        if (auto remap = remappings.find(symRef.getRootReference());
            remap != remappings.end())
          updates.emplace_back(std::pair<mlir::StringAttr, mlir::SymbolRefAttr>{
              attr.getName(), mlir::SymbolRefAttr(remap->second)});
    for (auto update : updates)
      nestedOp->setAttr(update.first, update.second);
  });
}
