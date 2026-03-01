//===- Type.cpp - Sandbox IR Type -----------------------------------------===//
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

#include "vm/core/SandboxIR/Type.h"
#include "vm/core/SandboxIR/Context.h"

using namespace vm::core::sandboxir;

Type *Type::getScalarType() const {
  return Ctx.getType(LLVMTy->getScalarType());
}

IntegerType *Type::getInt64Ty(Context &Ctx) {
  return cast<IntegerType>(Ctx.getType(toolchain::Type::getInt64Ty(Ctx.LLVMCtx)));
}
IntegerType *Type::getInt32Ty(Context &Ctx) {
  return cast<IntegerType>(Ctx.getType(toolchain::Type::getInt32Ty(Ctx.LLVMCtx)));
}
IntegerType *Type::getInt16Ty(Context &Ctx) {
  return cast<IntegerType>(Ctx.getType(toolchain::Type::getInt16Ty(Ctx.LLVMCtx)));
}
IntegerType *Type::getInt8Ty(Context &Ctx) {
  return cast<IntegerType>(Ctx.getType(toolchain::Type::getInt8Ty(Ctx.LLVMCtx)));
}
IntegerType *Type::getInt1Ty(Context &Ctx) {
  return cast<IntegerType>(Ctx.getType(toolchain::Type::getInt1Ty(Ctx.LLVMCtx)));
}
Type *Type::getDoubleTy(Context &Ctx) {
  return Ctx.getType(toolchain::Type::getDoubleTy(Ctx.LLVMCtx));
}
Type *Type::getFloatTy(Context &Ctx) {
  return Ctx.getType(toolchain::Type::getFloatTy(Ctx.LLVMCtx));
}
Type *Type::getHalfTy(Context &Ctx) {
  return Ctx.getType(toolchain::Type::getHalfTy(Ctx.LLVMCtx));
}

#ifndef NDEBUG
void Type::dumpOS(raw_ostream &OS) { LLVMTy->print(OS); }
void Type::dump() {
  dumpOS(dbgs());
  dbgs() << "\n";
}
#endif

PointerType *PointerType::get(Context &Ctx, unsigned AddressSpace) {
  return cast<PointerType>(
      Ctx.getType(toolchain::PointerType::get(Ctx.LLVMCtx, AddressSpace)));
}

ArrayType *ArrayType::get(Type *ElementType, uint64_t NumElements) {
  return cast<ArrayType>(ElementType->getContext().getType(
      toolchain::ArrayType::get(ElementType->LLVMTy, NumElements)));
}

StructType *StructType::get(Context &Ctx, ArrayRef<Type *> Elements,
                            bool IsPacked) {
  SmallVector<toolchain::Type *> LLVMElements;
  LLVMElements.reserve(Elements.size());
  for (Type *Elm : Elements)
    LLVMElements.push_back(Elm->LLVMTy);
  return cast<StructType>(
      Ctx.getType(toolchain::StructType::get(Ctx.LLVMCtx, LLVMElements, IsPacked)));
}

VectorType *VectorType::get(Type *ElementType, ElementCount EC) {
  return cast<VectorType>(ElementType->getContext().getType(
      toolchain::VectorType::get(ElementType->LLVMTy, EC)));
}

Type *VectorType::getElementType() const {
  return Ctx.getType(cast<toolchain::VectorType>(LLVMTy)->getElementType());
}
VectorType *VectorType::getInteger(VectorType *VTy) {
  return cast<VectorType>(VTy->getContext().getType(
      toolchain::VectorType::getInteger(cast<toolchain::VectorType>(VTy->LLVMTy))));
}
VectorType *VectorType::getExtendedElementVectorType(VectorType *VTy) {
  return cast<VectorType>(
      VTy->getContext().getType(toolchain::VectorType::getExtendedElementVectorType(
          cast<toolchain::VectorType>(VTy->LLVMTy))));
}
VectorType *VectorType::getTruncatedElementVectorType(VectorType *VTy) {
  return cast<VectorType>(
      VTy->getContext().getType(toolchain::VectorType::getTruncatedElementVectorType(
          cast<toolchain::VectorType>(VTy->LLVMTy))));
}
VectorType *VectorType::getSubdividedVectorType(VectorType *VTy,
                                                int NumSubdivs) {
  return cast<VectorType>(
      VTy->getContext().getType(toolchain::VectorType::getSubdividedVectorType(
          cast<toolchain::VectorType>(VTy->LLVMTy), NumSubdivs)));
}
VectorType *VectorType::getHalfElementsVectorType(VectorType *VTy) {
  return cast<VectorType>(
      VTy->getContext().getType(toolchain::VectorType::getHalfElementsVectorType(
          cast<toolchain::VectorType>(VTy->LLVMTy))));
}
VectorType *VectorType::getDoubleElementsVectorType(VectorType *VTy) {
  return cast<VectorType>(
      VTy->getContext().getType(toolchain::VectorType::getDoubleElementsVectorType(
          cast<toolchain::VectorType>(VTy->LLVMTy))));
}
bool VectorType::isValidElementType(Type *ElemTy) {
  return toolchain::VectorType::isValidElementType(ElemTy->LLVMTy);
}

FixedVectorType *FixedVectorType::get(Type *ElementType, unsigned NumElts) {
  return cast<FixedVectorType>(ElementType->getContext().getType(
      toolchain::FixedVectorType::get(ElementType->LLVMTy, NumElts)));
}

ScalableVectorType *ScalableVectorType::get(Type *ElementType,
                                            unsigned NumElts) {
  return cast<ScalableVectorType>(ElementType->getContext().getType(
      toolchain::ScalableVectorType::get(ElementType->LLVMTy, NumElts)));
}

IntegerType *IntegerType::get(Context &Ctx, unsigned NumBits) {
  return cast<IntegerType>(
      Ctx.getType(toolchain::IntegerType::get(Ctx.LLVMCtx, NumBits)));
}
