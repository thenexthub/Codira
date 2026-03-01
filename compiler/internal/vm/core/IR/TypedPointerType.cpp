//===- TypedPointerType.cpp - Typed Pointer Type --------------------------===//
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
//
//===----------------------------------------------------------------------===//

#include "vm/core/IR/TypedPointerType.h"
#include "LLVMContextImpl.h"

using namespace vm::core;

TypedPointerType *TypedPointerType::get(Type *EltTy, unsigned AddressSpace) {
  assert(EltTy && "Can't get a pointer to <null> type!");
  assert(isValidElementType(EltTy) && "Invalid type for pointer element!");

  LLVMContextImpl *CImpl = EltTy->getContext().pImpl;

  TypedPointerType *&Entry =
      CImpl->ASTypedPointerTypes[std::make_pair(EltTy, AddressSpace)];

  if (!Entry)
    Entry = new (CImpl->Alloc) TypedPointerType(EltTy, AddressSpace);
  return Entry;
}

TypedPointerType::TypedPointerType(Type *E, unsigned AddrSpace)
    : Type(E->getContext(), TypedPointerTyID), PointeeTy(E) {
  ContainedTys = &PointeeTy;
  NumContainedTys = 1;
  setSubclassData(AddrSpace);
}

bool TypedPointerType::isValidElementType(Type *ElemTy) {
  return !ElemTy->isVoidTy() && !ElemTy->isLabelTy() &&
         !ElemTy->isMetadataTy() && !ElemTy->isTokenTy() &&
         !ElemTy->isX86_AMXTy();
}
