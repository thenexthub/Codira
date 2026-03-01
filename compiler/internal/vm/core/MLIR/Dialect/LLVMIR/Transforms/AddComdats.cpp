//===- AddComdats.cpp - Add comdats to linkonce functions -----------------===//
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

#include "mlir/Dialect/LLVMIR/Transforms/AddComdats.h"
#include "mlir/Dialect/LLVMIR/LLVMDialect.h"
#include "mlir/Pass/Pass.h"

namespace mlir {
namespace LLVM {
#define GEN_PASS_DEF_LLVMADDCOMDATS
#include "mlir/Dialect/LLVMIR/Transforms/Passes.h.inc"
} // namespace LLVM
} // namespace mlir

using namespace mlir;

static void addComdat(LLVM::LLVMFuncOp &op, OpBuilder &builder,
                      SymbolTable &symbolTable, ModuleOp &module) {
  const char *comdatName = "__llvm_comdat";
  mlir::LLVM::ComdatOp comdatOp =
      symbolTable.lookup<mlir::LLVM::ComdatOp>(comdatName);
  if (!comdatOp) {
    PatternRewriter::InsertionGuard guard(builder);
    builder.setInsertionPointToStart(module.getBody());
    comdatOp =
        mlir::LLVM::ComdatOp::create(builder, module.getLoc(), comdatName);
    symbolTable.insert(comdatOp);
  }

  PatternRewriter::InsertionGuard guard(builder);
  builder.setInsertionPointToStart(&comdatOp.getBody().back());
  auto selectorOp = mlir::LLVM::ComdatSelectorOp::create(
      builder, comdatOp.getLoc(), op.getSymName(),
      mlir::LLVM::comdat::Comdat::Any);
  op.setComdatAttr(mlir::SymbolRefAttr::get(
      builder.getContext(), comdatName,
      mlir::FlatSymbolRefAttr::get(selectorOp.getSymNameAttr())));
}

namespace {
struct AddComdatsPass : public LLVM::impl::LLVMAddComdatsBase<AddComdatsPass> {
  void runOnOperation() override {
    OpBuilder builder{&getContext()};
    ModuleOp mod = getOperation();

    std::unique_ptr<SymbolTable> symbolTable;
    auto getSymTab = [&]() -> SymbolTable & {
      if (!symbolTable)
        symbolTable = std::make_unique<SymbolTable>(mod);
      return *symbolTable;
    };
    for (auto op : mod.getBody()->getOps<LLVM::LLVMFuncOp>()) {
      if (op.getLinkage() == LLVM::Linkage::Linkonce ||
          op.getLinkage() == LLVM::Linkage::LinkonceODR) {
        addComdat(op, builder, getSymTab(), mod);
      }
    }
  }
};
} // namespace
