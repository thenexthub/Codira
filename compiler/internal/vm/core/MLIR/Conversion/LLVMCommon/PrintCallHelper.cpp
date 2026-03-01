//===- PrintCallHelper.cpp - Helper to emit runtime print calls -----------===//
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

#include "mlir/Conversion/LLVMCommon/PrintCallHelper.h"
#include "mlir/Conversion/LLVMCommon/TypeConverter.h"
#include "mlir/Dialect/LLVMIR/FunctionCallUtils.h"
#include "mlir/Dialect/LLVMIR/LLVMDialect.h"
#include "mlir/IR/Builders.h"
#include "mlir/IR/BuiltinOps.h"
#include "vm/core/ADT/ArrayRef.h"

using namespace mlir;
using namespace vm::core;

/// Check if a given symbol name is already in use within the module operation.
/// If no symbol with such name is present, then the same identifier is
/// returned. Otherwise, a unique and yet unused identifier is computed starting
/// from the requested one.
static std::string
ensureSymbolNameIsUnique(ModuleOp moduleOp, StringRef symbolName,
                         SymbolTableCollection *symbolTables = nullptr) {
  if (symbolTables) {
    SymbolTable &symbolTable = symbolTables->getSymbolTable(moduleOp);
    unsigned counter = 0;
    SmallString<128> uniqueName = symbolTable.generateSymbolName<128>(
        symbolName,
        [&](const SmallString<128> &tentativeName) {
          return symbolTable.lookupSymbolIn(moduleOp, tentativeName) != nullptr;
        },
        counter);

    return static_cast<std::string>(uniqueName);
  }

  static int counter = 0;
  std::string uniqueName = std::string(symbolName);
  while (moduleOp.lookupSymbol(uniqueName)) {
    uniqueName = std::string(symbolName) + "_" + std::to_string(counter++);
  }
  return uniqueName;
}

LogicalResult mlir::LLVM::createPrintStrCall(
    OpBuilder &builder, Location loc, ModuleOp moduleOp, StringRef symbolName,
    StringRef string, const LLVMTypeConverter &typeConverter, bool addNewline,
    std::optional<StringRef> runtimeFunctionName,
    SymbolTableCollection *symbolTables) {
  auto ip = builder.saveInsertionPoint();
  builder.setInsertionPointToStart(moduleOp.getBody());
  MLIRContext *ctx = builder.getContext();

  // Create a zero-terminated byte representation and allocate global symbol.
  SmallVector<uint8_t> elementVals;
  elementVals.append(string.begin(), string.end());
  if (addNewline)
    elementVals.push_back('\n');
  elementVals.push_back('\0');
  auto dataAttrType = RankedTensorType::get(
      {static_cast<int64_t>(elementVals.size())}, builder.getI8Type());
  auto dataAttr =
      DenseElementsAttr::get(dataAttrType, toolchain::ArrayRef(elementVals));
  auto arrayTy =
      LLVM::LLVMArrayType::get(IntegerType::get(ctx, 8), elementVals.size());
  auto globalOp = LLVM::GlobalOp::create(
      builder, loc, arrayTy, /*isConstant=*/true, LLVM::Linkage::Private,
      ensureSymbolNameIsUnique(moduleOp, symbolName, symbolTables), dataAttr);

  auto ptrTy = LLVM::LLVMPointerType::get(builder.getContext());
  // Emit call to `printStr` in runtime library.
  builder.restoreInsertionPoint(ip);
  auto msgAddr =
      LLVM::AddressOfOp::create(builder, loc, ptrTy, globalOp.getName());
  SmallVector<LLVM::GEPArg> indices(1, 0);
  Value gep =
      LLVM::GEPOp::create(builder, loc, ptrTy, arrayTy, msgAddr, indices);
  FailureOr<LLVM::LLVMFuncOp> printer =
      LLVM::lookupOrCreatePrintStringFn(builder, moduleOp, runtimeFunctionName);
  if (failed(printer))
    return failure();
  LLVM::CallOp::create(builder, loc, TypeRange(),
                       SymbolRefAttr::get(printer.value()), gep);
  return success();
}
