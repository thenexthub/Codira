//===------- VectorTypeUtils.cpp - Vector type utility functions ----------===//
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

#include "vm/core/IR/VectorTypeUtils.h"
#include "vm/core/ADT/SmallVectorExtras.h"

using namespace vm::core;

/// A helper for converting structs of scalar types to structs of vector types.
/// Note: Only unpacked literal struct types are supported.
Type *toolchain::toVectorizedStructTy(StructType *StructTy, ElementCount EC) {
  if (EC.isScalar())
    return StructTy;
  assert(isUnpackedStructLiteral(StructTy) &&
         "expected unpacked struct literal");
  assert(all_of(StructTy->elements(), VectorType::isValidElementType) &&
         "expected all element types to be valid vector element types");
  return StructType::get(
      StructTy->getContext(),
      map_to_vector(StructTy->elements(), [&](Type *ElTy) -> Type * {
        return VectorType::get(ElTy, EC);
      }));
}

/// A helper for converting structs of vector types to structs of scalar types.
/// Note: Only unpacked literal struct types are supported.
Type *toolchain::toScalarizedStructTy(StructType *StructTy) {
  assert(isUnpackedStructLiteral(StructTy) &&
         "expected unpacked struct literal");
  return StructType::get(
      StructTy->getContext(),
      map_to_vector(StructTy->elements(), [](Type *ElTy) -> Type * {
        return ElTy->getScalarType();
      }));
}

/// Returns true if `StructTy` is an unpacked literal struct where all elements
/// are vectors of matching element count. This does not include empty structs.
bool toolchain::isVectorizedStructTy(StructType *StructTy) {
  if (!isUnpackedStructLiteral(StructTy))
    return false;
  auto ElemTys = StructTy->elements();
  if (ElemTys.empty() || !ElemTys.front()->isVectorTy())
    return false;
  ElementCount VF = cast<VectorType>(ElemTys.front())->getElementCount();
  return all_of(ElemTys, [&](Type *Ty) {
    return Ty->isVectorTy() && cast<VectorType>(Ty)->getElementCount() == VF;
  });
}

/// Returns true if `StructTy` is an unpacked literal struct where all elements
/// are scalars that can be used as vector element types.
bool toolchain::canVectorizeStructTy(StructType *StructTy) {
  auto ElemTys = StructTy->elements();
  return !ElemTys.empty() && isUnpackedStructLiteral(StructTy) &&
         all_of(ElemTys, VectorType::isValidElementType);
}
