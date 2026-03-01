//===- AttrKindDetail.h - AttrKind conversion details -----------*- C++ -*-===//
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

#ifndef ATTRKINDDETAIL_H_
#define ATTRKINDDETAIL_H_

#include "mlir/Dialect/LLVMIR/LLVMDialect.h"
#include "vm/core/IR/Attributes.h"

namespace mlir {
namespace LLVM {
namespace detail {

/// Returns a list of pairs that each hold a mapping from LLVM attribute kinds
/// to their corresponding string name in LLVM IR dialect.
static toolchain::ArrayRef<std::pair<toolchain::Attribute::AttrKind, toolchain::StringRef>>
getAttrKindToNameMapping() {
  using ElemTy = std::pair<toolchain::Attribute::AttrKind, toolchain::StringRef>;
  // Mapping from toolchain attribute kinds to their corresponding MLIR name.
  static const toolchain::SmallVector<ElemTy> kindNamePairs = {
      {toolchain::Attribute::AttrKind::Alignment, LLVMDialect::getAlignAttrName()},
      {toolchain::Attribute::AttrKind::AllocAlign,
       LLVMDialect::getAllocAlignAttrName()},
      {toolchain::Attribute::AttrKind::AllocatedPointer,
       LLVMDialect::getAllocatedPointerAttrName()},
      {toolchain::Attribute::AttrKind::ByVal, LLVMDialect::getByValAttrName()},
      {toolchain::Attribute::AttrKind::ByRef, LLVMDialect::getByRefAttrName()},
      {toolchain::Attribute::AttrKind::NoUndef, LLVMDialect::getNoUndefAttrName()},
      {toolchain::Attribute::AttrKind::Dereferenceable,
       LLVMDialect::getDereferenceableAttrName()},
      {toolchain::Attribute::AttrKind::DereferenceableOrNull,
       LLVMDialect::getDereferenceableOrNullAttrName()},
      {toolchain::Attribute::AttrKind::ElementType,
       LLVMDialect::getElementTypeAttrName()},
      {toolchain::Attribute::AttrKind::InAlloca, LLVMDialect::getInAllocaAttrName()},
      {toolchain::Attribute::AttrKind::InReg, LLVMDialect::getInRegAttrName()},
      {toolchain::Attribute::AttrKind::Nest, LLVMDialect::getNestAttrName()},
      {toolchain::Attribute::AttrKind::NoAlias, LLVMDialect::getNoAliasAttrName()},
      {toolchain::Attribute::AttrKind::Captures,
       LLVMDialect::getNoCaptureAttrName()},
      {toolchain::Attribute::AttrKind::NoFree, LLVMDialect::getNoFreeAttrName()},
      {toolchain::Attribute::AttrKind::NonNull, LLVMDialect::getNonNullAttrName()},
      {toolchain::Attribute::AttrKind::Preallocated,
       LLVMDialect::getPreallocatedAttrName()},
      {toolchain::Attribute::AttrKind::Range, LLVMDialect::getRangeAttrName()},
      {toolchain::Attribute::AttrKind::ReadOnly, LLVMDialect::getReadonlyAttrName()},
      {toolchain::Attribute::AttrKind::ReadNone, LLVMDialect::getReadnoneAttrName()},
      {toolchain::Attribute::AttrKind::Returned, LLVMDialect::getReturnedAttrName()},
      {toolchain::Attribute::AttrKind::SExt, LLVMDialect::getSExtAttrName()},
      {toolchain::Attribute::AttrKind::StackAlignment,
       LLVMDialect::getStackAlignmentAttrName()},
      {toolchain::Attribute::AttrKind::StructRet,
       LLVMDialect::getStructRetAttrName()},
      {toolchain::Attribute::AttrKind::WriteOnly,
       LLVMDialect::getWriteOnlyAttrName()},
      {toolchain::Attribute::AttrKind::ZExt, LLVMDialect::getZExtAttrName()}};
  return kindNamePairs;
}

/// Returns a dense map from LLVM attribute name to their kind in LLVM IR
/// dialect.
[[maybe_unused]] static toolchain::DenseMap<toolchain::StringRef,
                                       toolchain::Attribute::AttrKind>
getAttrNameToKindMapping() {
  static auto attrNameToKindMapping = []() {
    toolchain::DenseMap<toolchain::StringRef, toolchain::Attribute::AttrKind> nameKindMap;
    for (auto kindNamePair : getAttrKindToNameMapping()) {
      nameKindMap.insert({kindNamePair.second, kindNamePair.first});
    }
    return nameKindMap;
  }();
  return attrNameToKindMapping;
}

} // namespace detail
} // namespace LLVM
} // namespace mlir

#endif // ATTRKINDDETAIL_H_
