//===- FunctionCallUtils.cpp - Utilities for C function calls -------------===//
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
// This file implements helper functions to call common simple C functions in
// LLVMIR (e.g. amon others to support printing and debugging).
//
//===----------------------------------------------------------------------===//

#include "mlir/Dialect/LLVMIR/FunctionCallUtils.h"
#include "mlir/Dialect/LLVMIR/LLVMDialect.h"
#include "mlir/IR/Builders.h"
#include "mlir/IR/OpDefinition.h"
#include "mlir/Support/LLVM.h"

using namespace mlir;
using namespace mlir::LLVM;

/// Helper functions to lookup or create the declaration for commonly used
/// external C function calls. The list of functions provided here must be
/// implemented separately (e.g. as  part of a support runtime library or as
/// part of the libc).
static constexpr toolchain::StringRef kPrintI64 = "printI64";
static constexpr toolchain::StringRef kPrintU64 = "printU64";
static constexpr toolchain::StringRef kPrintF16 = "printF16";
static constexpr toolchain::StringRef kPrintBF16 = "printBF16";
static constexpr toolchain::StringRef kPrintF32 = "printF32";
static constexpr toolchain::StringRef kPrintF64 = "printF64";
static constexpr toolchain::StringRef kPrintApFloat = "printApFloat";
static constexpr toolchain::StringRef kPrintString = "printString";
static constexpr toolchain::StringRef kPrintOpen = "printOpen";
static constexpr toolchain::StringRef kPrintClose = "printClose";
static constexpr toolchain::StringRef kPrintComma = "printComma";
static constexpr toolchain::StringRef kPrintNewline = "printNewline";
static constexpr toolchain::StringRef kMalloc = "malloc";
static constexpr toolchain::StringRef kAlignedAlloc = "aligned_alloc";
static constexpr toolchain::StringRef kFree = "free";
static constexpr toolchain::StringRef kGenericAlloc = "_mlir_memref_to_llvm_alloc";
static constexpr toolchain::StringRef kGenericAlignedAlloc =
    "_mlir_memref_to_llvm_aligned_alloc";
static constexpr toolchain::StringRef kGenericFree = "_mlir_memref_to_llvm_free";
static constexpr toolchain::StringRef kMemRefCopy = "memrefCopy";

namespace {
/// Search for an LLVMFuncOp with a given name within an operation with the
/// SymbolTable trait. An optional collection of cached symbol tables can be
/// given to avoid a linear scan of the symbol table operation.
LLVM::LLVMFuncOp lookupFuncOp(StringRef name, Operation *symbolTableOp,
                              SymbolTableCollection *symbolTables = nullptr) {
  if (symbolTables) {
    return symbolTables->lookupSymbolIn<LLVM::LLVMFuncOp>(
        symbolTableOp, StringAttr::get(symbolTableOp->getContext(), name));
  }

  return toolchain::dyn_cast_or_null<LLVM::LLVMFuncOp>(
      SymbolTable::lookupSymbolIn(symbolTableOp, name));
}
} // namespace

/// Generic print function lookupOrCreate helper.
FailureOr<LLVM::LLVMFuncOp>
mlir::LLVM::lookupOrCreateFn(OpBuilder &b, Operation *moduleOp, StringRef name,
                             ArrayRef<Type> paramTypes, Type resultType,
                             bool isVarArg, bool isReserved,
                             SymbolTableCollection *symbolTables) {
  assert(moduleOp->hasTrait<OpTrait::SymbolTable>() &&
         "expected SymbolTable operation");
  auto func = lookupFuncOp(name, moduleOp, symbolTables);
  auto funcT = LLVMFunctionType::get(resultType, paramTypes, isVarArg);
  // Assert the signature of the found function is same as expected
  if (func) {
    if (funcT != func.getFunctionType()) {
      if (isReserved) {
        func.emitError("redefinition of reserved function '")
            << name << "' of different type " << func.getFunctionType()
            << " is prohibited";
      } else {
        func.emitError("redefinition of function '")
            << name << "' of different type " << funcT << " is prohibited";
      }
      return failure();
    }
    return func;
  }

  OpBuilder::InsertionGuard g(b);
  assert(!moduleOp->getRegion(0).empty() && "expected non-empty region");
  b.setInsertionPointToStart(&moduleOp->getRegion(0).front());
  auto funcOp = LLVM::LLVMFuncOp::create(
      b, moduleOp->getLoc(), name,
      LLVM::LLVMFunctionType::get(resultType, paramTypes, isVarArg));

  if (symbolTables) {
    SymbolTable &symbolTable = symbolTables->getSymbolTable(moduleOp);
    symbolTable.insert(funcOp, moduleOp->getRegion(0).front().begin());
  }

  return funcOp;
}

static FailureOr<LLVM::LLVMFuncOp>
lookupOrCreateReservedFn(OpBuilder &b, Operation *moduleOp, StringRef name,
                         ArrayRef<Type> paramTypes, Type resultType,
                         SymbolTableCollection *symbolTables) {
  return lookupOrCreateFn(b, moduleOp, name, paramTypes, resultType,
                          /*isVarArg=*/false, /*isReserved=*/true,
                          symbolTables);
}

FailureOr<LLVM::LLVMFuncOp>
mlir::LLVM::lookupOrCreatePrintI64Fn(OpBuilder &b, Operation *moduleOp,
                                     SymbolTableCollection *symbolTables) {
  return lookupOrCreateReservedFn(
      b, moduleOp, kPrintI64, IntegerType::get(moduleOp->getContext(), 64),
      LLVM::LLVMVoidType::get(moduleOp->getContext()), symbolTables);
}

FailureOr<LLVM::LLVMFuncOp>
mlir::LLVM::lookupOrCreatePrintU64Fn(OpBuilder &b, Operation *moduleOp,
                                     SymbolTableCollection *symbolTables) {
  return lookupOrCreateReservedFn(
      b, moduleOp, kPrintU64, IntegerType::get(moduleOp->getContext(), 64),
      LLVM::LLVMVoidType::get(moduleOp->getContext()), symbolTables);
}

FailureOr<LLVM::LLVMFuncOp>
mlir::LLVM::lookupOrCreatePrintF16Fn(OpBuilder &b, Operation *moduleOp,
                                     SymbolTableCollection *symbolTables) {
  return lookupOrCreateReservedFn(
      b, moduleOp, kPrintF16,
      IntegerType::get(moduleOp->getContext(), 16), // bits!
      LLVM::LLVMVoidType::get(moduleOp->getContext()), symbolTables);
}

FailureOr<LLVM::LLVMFuncOp>
mlir::LLVM::lookupOrCreatePrintBF16Fn(OpBuilder &b, Operation *moduleOp,
                                      SymbolTableCollection *symbolTables) {
  return lookupOrCreateReservedFn(
      b, moduleOp, kPrintBF16,
      IntegerType::get(moduleOp->getContext(), 16), // bits!
      LLVM::LLVMVoidType::get(moduleOp->getContext()), symbolTables);
}

FailureOr<LLVM::LLVMFuncOp>
mlir::LLVM::lookupOrCreatePrintF32Fn(OpBuilder &b, Operation *moduleOp,
                                     SymbolTableCollection *symbolTables) {
  return lookupOrCreateReservedFn(
      b, moduleOp, kPrintF32, Float32Type::get(moduleOp->getContext()),
      LLVM::LLVMVoidType::get(moduleOp->getContext()), symbolTables);
}

FailureOr<LLVM::LLVMFuncOp>
mlir::LLVM::lookupOrCreatePrintF64Fn(OpBuilder &b, Operation *moduleOp,
                                     SymbolTableCollection *symbolTables) {
  return lookupOrCreateReservedFn(
      b, moduleOp, kPrintF64, Float64Type::get(moduleOp->getContext()),
      LLVM::LLVMVoidType::get(moduleOp->getContext()), symbolTables);
}

FailureOr<LLVM::LLVMFuncOp>
mlir::LLVM::lookupOrCreateApFloatPrintFn(OpBuilder &b, Operation *moduleOp,
                                         SymbolTableCollection *symbolTables) {
  return lookupOrCreateReservedFn(
      b, moduleOp, kPrintApFloat,
      {IntegerType::get(moduleOp->getContext(), 32),
       IntegerType::get(moduleOp->getContext(), 64)},
      LLVM::LLVMVoidType::get(moduleOp->getContext()), symbolTables);
}

static LLVM::LLVMPointerType getCharPtr(MLIRContext *context) {
  return LLVM::LLVMPointerType::get(context);
}

static LLVM::LLVMPointerType getVoidPtr(MLIRContext *context) {
  // A char pointer and void ptr are the same in LLVM IR.
  return getCharPtr(context);
}

FailureOr<LLVM::LLVMFuncOp> mlir::LLVM::lookupOrCreatePrintStringFn(
    OpBuilder &b, Operation *moduleOp,
    std::optional<StringRef> runtimeFunctionName,
    SymbolTableCollection *symbolTables) {
  return lookupOrCreateReservedFn(
      b, moduleOp, runtimeFunctionName.value_or(kPrintString),
      getCharPtr(moduleOp->getContext()),
      LLVM::LLVMVoidType::get(moduleOp->getContext()), symbolTables);
}

FailureOr<LLVM::LLVMFuncOp>
mlir::LLVM::lookupOrCreatePrintOpenFn(OpBuilder &b, Operation *moduleOp,
                                      SymbolTableCollection *symbolTables) {
  return lookupOrCreateReservedFn(
      b, moduleOp, kPrintOpen, {},
      LLVM::LLVMVoidType::get(moduleOp->getContext()), symbolTables);
}

FailureOr<LLVM::LLVMFuncOp>
mlir::LLVM::lookupOrCreatePrintCloseFn(OpBuilder &b, Operation *moduleOp,
                                       SymbolTableCollection *symbolTables) {
  return lookupOrCreateReservedFn(
      b, moduleOp, kPrintClose, {},
      LLVM::LLVMVoidType::get(moduleOp->getContext()), symbolTables);
}

FailureOr<LLVM::LLVMFuncOp>
mlir::LLVM::lookupOrCreatePrintCommaFn(OpBuilder &b, Operation *moduleOp,
                                       SymbolTableCollection *symbolTables) {
  return lookupOrCreateReservedFn(
      b, moduleOp, kPrintComma, {},
      LLVM::LLVMVoidType::get(moduleOp->getContext()), symbolTables);
}

FailureOr<LLVM::LLVMFuncOp>
mlir::LLVM::lookupOrCreatePrintNewlineFn(OpBuilder &b, Operation *moduleOp,
                                         SymbolTableCollection *symbolTables) {
  return lookupOrCreateReservedFn(
      b, moduleOp, kPrintNewline, {},
      LLVM::LLVMVoidType::get(moduleOp->getContext()), symbolTables);
}

FailureOr<LLVM::LLVMFuncOp>
mlir::LLVM::lookupOrCreateMallocFn(OpBuilder &b, Operation *moduleOp,
                                   Type indexType,
                                   SymbolTableCollection *symbolTables) {
  return lookupOrCreateReservedFn(b, moduleOp, kMalloc, indexType,
                                  getVoidPtr(moduleOp->getContext()),
                                  symbolTables);
}

FailureOr<LLVM::LLVMFuncOp>
mlir::LLVM::lookupOrCreateAlignedAllocFn(OpBuilder &b, Operation *moduleOp,
                                         Type indexType,
                                         SymbolTableCollection *symbolTables) {
  return lookupOrCreateReservedFn(
      b, moduleOp, kAlignedAlloc, {indexType, indexType},
      getVoidPtr(moduleOp->getContext()), symbolTables);
}

FailureOr<LLVM::LLVMFuncOp>
mlir::LLVM::lookupOrCreateFreeFn(OpBuilder &b, Operation *moduleOp,
                                 SymbolTableCollection *symbolTables) {
  return lookupOrCreateReservedFn(
      b, moduleOp, kFree, getVoidPtr(moduleOp->getContext()),
      LLVM::LLVMVoidType::get(moduleOp->getContext()), symbolTables);
}

FailureOr<LLVM::LLVMFuncOp>
mlir::LLVM::lookupOrCreateGenericAllocFn(OpBuilder &b, Operation *moduleOp,
                                         Type indexType,
                                         SymbolTableCollection *symbolTables) {
  return lookupOrCreateReservedFn(b, moduleOp, kGenericAlloc, indexType,
                                  getVoidPtr(moduleOp->getContext()),
                                  symbolTables);
}

FailureOr<LLVM::LLVMFuncOp> mlir::LLVM::lookupOrCreateGenericAlignedAllocFn(
    OpBuilder &b, Operation *moduleOp, Type indexType,
    SymbolTableCollection *symbolTables) {
  return lookupOrCreateReservedFn(
      b, moduleOp, kGenericAlignedAlloc, {indexType, indexType},
      getVoidPtr(moduleOp->getContext()), symbolTables);
}

FailureOr<LLVM::LLVMFuncOp>
mlir::LLVM::lookupOrCreateGenericFreeFn(OpBuilder &b, Operation *moduleOp,
                                        SymbolTableCollection *symbolTables) {
  return lookupOrCreateReservedFn(
      b, moduleOp, kGenericFree, getVoidPtr(moduleOp->getContext()),
      LLVM::LLVMVoidType::get(moduleOp->getContext()), symbolTables);
}

FailureOr<LLVM::LLVMFuncOp> mlir::LLVM::lookupOrCreateMemRefCopyFn(
    OpBuilder &b, Operation *moduleOp, Type indexType,
    Type unrankedDescriptorType, SymbolTableCollection *symbolTables) {
  return lookupOrCreateReservedFn(
      b, moduleOp, kMemRefCopy,
      ArrayRef<Type>{indexType, unrankedDescriptorType, unrankedDescriptorType},
      LLVM::LLVMVoidType::get(moduleOp->getContext()), symbolTables);
}
