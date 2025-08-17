//===-- LLVMInsertChainFolder.cpp -----------------------------------------===//
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

#include "language/Compability/Optimizer/CodeGen/LLVMInsertChainFolder.h"
#include "mlir/Dialect/LLVMIR/LLVMAttrs.h"
#include "mlir/Dialect/LLVMIR/LLVMDialect.h"
#include "mlir/IR/Builders.h"
#include "toolchain/Support/Debug.h"

#define DEBUG_TYPE "flang-insert-folder"

#include <deque>

namespace {
// Helper class to construct the attribute elements of an aggregate value being
// folded without creating a full mlir::Attribute representation for each step
// of the insert value chain, which would both be expensive in terms of
// compilation time and memory (since the intermediate Attribute would survive,
// unused, inside the mlir context).
class InsertChainBackwardFolder {
  // Type for the current value of an element of the aggregate value being
  // constructed by the insert chain.
  // At any point of the insert chain, the value of an element is either:
  //  - nullptr: not yet known, the insert has not yet been seen.
  //  - an mlir::Attribute: the element is fully defined.
  //  - a nested InsertChainBackwardFolder: the element is itself an aggregate
  //    and its sub-elements have been partially defined (insert with mutliple
  //    indices have been seen).

  // The insertion folder assumes backward walk of the insert chain. Once an
  // element or sub-element has been defined, it is not overriden by new
  // insertions (last insert wins).
  using InFlightValue =
      toolchain::PointerUnion<mlir::Attribute, InsertChainBackwardFolder *>;

public:
  InsertChainBackwardFolder(
      mlir::Type type, std::deque<InsertChainBackwardFolder> *folderStorage)
      : values(getNumElements(type), mlir::Attribute{}),
        folderStorage{folderStorage}, type{type} {}

  /// Push
  bool pushValue(mlir::Attribute val, toolchain::ArrayRef<int64_t> at);

  mlir::Attribute finalize(mlir::Attribute defaultFieldValue);

private:
  static int64_t getNumElements(mlir::Type type) {
    if (auto structTy =
            toolchain::dyn_cast_if_present<mlir::LLVM::LLVMStructType>(type))
      return structTy.getBody().size();
    if (auto arrayTy =
            toolchain::dyn_cast_if_present<mlir::LLVM::LLVMArrayType>(type))
      return arrayTy.getNumElements();
    return 0;
  }

  static mlir::Type getSubElementType(mlir::Type type, int64_t field) {
    if (auto arrayTy =
            toolchain::dyn_cast_if_present<mlir::LLVM::LLVMArrayType>(type))
      return arrayTy.getElementType();
    if (auto structTy =
            toolchain::dyn_cast_if_present<mlir::LLVM::LLVMStructType>(type))
      return structTy.getBody()[field];
    return nullptr;
  }

  // Current element value of the aggregate value being built.
  toolchain::SmallVector<InFlightValue> values;
  // std::deque is used to allocate storage for nested list and guarantee the
  // stability of the InsertChainBackwardFolder* used as element value.
  std::deque<InsertChainBackwardFolder> *folderStorage;
  // Type of the aggregate value being built.
  mlir::Type type;
};
} // namespace

// Helper to fold the value being inserted by an toolchain.insert_value.
// This may call tryFoldingLLVMInsertChain if the value is an aggregate and
// was itself constructed by a different insert chain.
// Returns a nullptr Attribute if the value could not be folded.
static mlir::Attribute getAttrIfConstant(mlir::Value val,
                                         mlir::OpBuilder &rewriter) {
  if (auto cst = val.getDefiningOp<mlir::LLVM::ConstantOp>())
    return cst.getValue();
  if (auto insert = val.getDefiningOp<mlir::LLVM::InsertValueOp>()) {
    toolchain::FailureOr<mlir::Attribute> attr =
        fir::tryFoldingLLVMInsertChain(val, rewriter);
    if (succeeded(attr))
      return *attr;
    return nullptr;
  }
  if (val.getDefiningOp<mlir::LLVM::ZeroOp>())
    return mlir::LLVM::ZeroAttr::get(val.getContext());
  if (val.getDefiningOp<mlir::LLVM::UndefOp>())
    return mlir::LLVM::UndefAttr::get(val.getContext());
  if (mlir::Operation *op = val.getDefiningOp()) {
    unsigned resNum = toolchain::cast<mlir::OpResult>(val).getResultNumber();
    toolchain::SmallVector<mlir::Value> results;
    if (mlir::succeeded(rewriter.tryFold(op, results)) &&
        results.size() > resNum) {
      if (auto cst = results[resNum].getDefiningOp<mlir::LLVM::ConstantOp>())
        return cst.getValue();
    }
  }
  if (auto trunc = val.getDefiningOp<mlir::LLVM::TruncOp>())
    if (auto attr = getAttrIfConstant(trunc.getArg(), rewriter))
      if (auto intAttr = toolchain::dyn_cast<mlir::IntegerAttr>(attr))
        return mlir::IntegerAttr::get(trunc.getType(), intAttr.getInt());
  LLVM_DEBUG(toolchain::dbgs() << "cannot fold insert value operand: " << val
                          << "\n");
  return nullptr;
}

mlir::Attribute
InsertChainBackwardFolder::finalize(mlir::Attribute defaultFieldValue) {
  toolchain::SmallVector<mlir::Attribute> attrs = toolchain::map_to_vector(
      values, [&](InFlightValue inFlight) -> mlir::Attribute {
        if (!inFlight)
          return defaultFieldValue;
        if (auto attr = toolchain::dyn_cast<mlir::Attribute>(inFlight))
          return attr;
        return toolchain::cast<InsertChainBackwardFolder *>(inFlight)->finalize(
            defaultFieldValue);
      });
  return mlir::ArrayAttr::get(type.getContext(), attrs);
}

bool InsertChainBackwardFolder::pushValue(mlir::Attribute val,
                                          toolchain::ArrayRef<int64_t> at) {
  if (at.size() == 0 || at[0] >= static_cast<int64_t>(values.size()))
    return false;
  InFlightValue &inFlight = values[at[0]];
  if (!inFlight) {
    if (at.size() == 1) {
      inFlight = val;
      return true;
    }
    // This is the first insert to a nested field. Create a
    // InsertChainBackwardFolder for the current element value.
    mlir::Type subType = getSubElementType(type, at[0]);
    if (!subType)
      return false;
    InsertChainBackwardFolder &inFlightList =
        folderStorage->emplace_back(subType, folderStorage);
    inFlight = &inFlightList;
    return inFlightList.pushValue(val, at.drop_front());
  }
  // Keep last inserted value if already set.
  if (toolchain::isa<mlir::Attribute>(inFlight))
    return true;
  auto *inFlightList = toolchain::cast<InsertChainBackwardFolder *>(inFlight);
  if (at.size() == 1) {
    if (!toolchain::isa<mlir::LLVM::ZeroAttr, mlir::LLVM::UndefAttr>(val)) {
      LLVM_DEBUG(toolchain::dbgs()
                 << "insert chain sub-element partially overwritten initial "
                    "value is not zero or undef\n");
      return false;
    }
    inFlight = inFlightList->finalize(val);
    return true;
  }
  return inFlightList->pushValue(val, at.drop_front());
}

toolchain::FailureOr<mlir::Attribute>
fir::tryFoldingLLVMInsertChain(mlir::Value val, mlir::OpBuilder &rewriter) {
  if (auto cst = val.getDefiningOp<mlir::LLVM::ConstantOp>())
    return cst.getValue();
  if (auto insert = val.getDefiningOp<mlir::LLVM::InsertValueOp>()) {
    LLVM_DEBUG(toolchain::dbgs() << "trying to fold insert chain:" << val << "\n");
    if (auto structTy =
            toolchain::dyn_cast<mlir::LLVM::LLVMStructType>(insert.getType())) {
      mlir::LLVM::InsertValueOp currentInsert = insert;
      mlir::LLVM::InsertValueOp lastInsert;
      std::deque<InsertChainBackwardFolder> folderStorage;
      InsertChainBackwardFolder inFlightList(structTy, &folderStorage);
      while (currentInsert) {
        mlir::Attribute attr =
            getAttrIfConstant(currentInsert.getValue(), rewriter);
        if (!attr)
          return toolchain::failure();
        if (!inFlightList.pushValue(attr, currentInsert.getPosition()))
          return toolchain::failure();
        lastInsert = currentInsert;
        currentInsert = currentInsert.getContainer()
                            .getDefiningOp<mlir::LLVM::InsertValueOp>();
      }
      mlir::Attribute defaultVal;
      if (lastInsert) {
        if (lastInsert.getContainer().getDefiningOp<mlir::LLVM::ZeroOp>())
          defaultVal = mlir::LLVM::ZeroAttr::get(val.getContext());
        else if (lastInsert.getContainer().getDefiningOp<mlir::LLVM::UndefOp>())
          defaultVal = mlir::LLVM::UndefAttr::get(val.getContext());
      }
      if (!defaultVal) {
        LLVM_DEBUG(toolchain::dbgs()
                   << "insert chain initial value is not Zero or Undef\n");
        return toolchain::failure();
      }
      return inFlightList.finalize(defaultVal);
    }
  }
  return toolchain::failure();
}
