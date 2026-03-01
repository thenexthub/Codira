//===- Target/DirectX/CBufferDataLayout.cpp - Cbuffer layout helper -------===//
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
// Utils to help cbuffer layout.
//
//===----------------------------------------------------------------------===//

#include "CBufferDataLayout.h"

#include "vm/core/IR/DerivedTypes.h"
#include "vm/core/IR/IRBuilder.h"

namespace vm::core {
namespace dxil {

// Implement cbuffer layout in
// https://learn.microsoft.com/en-us/windows/win32/direct3dhlsl/dx-graphics-hlsl-packing-rules
class LegacyCBufferLayout {
  struct LegacyStructLayout {
    StructType *ST;
    SmallVector<uint32_t> Offsets;
    TypeSize Size = {0, false};
    std::pair<uint32_t, uint32_t> getElementLegacyOffset(unsigned Idx) const {
      assert(Idx < Offsets.size() && "Invalid element idx!");
      uint32_t Offset = Offsets[Idx];
      uint32_t Ch = Offset & (RowAlign - 1);
      return std::make_pair((Offset - Ch) / RowAlign, Ch);
    }
  };

public:
  LegacyCBufferLayout(const DataLayout &DL) : DL(DL) {}
  TypeSize getTypeAllocSizeInBytes(Type *Ty);

private:
  TypeSize applyRowAlign(TypeSize Offset, Type *EltTy);
  TypeSize getTypeAllocSize(Type *Ty);
  LegacyStructLayout &getStructLayout(StructType *ST);
  const DataLayout &DL;
  SmallDenseMap<StructType *, LegacyStructLayout> StructLayouts;
  // 4 Dwords align.
  static const uint32_t RowAlign = 16;
  static TypeSize alignTo4Dwords(TypeSize Offset) {
    return alignTo(Offset, RowAlign);
  }
};

TypeSize LegacyCBufferLayout::getTypeAllocSizeInBytes(Type *Ty) {
  return getTypeAllocSize(Ty);
}

TypeSize LegacyCBufferLayout::applyRowAlign(TypeSize Offset, Type *EltTy) {
  TypeSize AlignedOffset = alignTo4Dwords(Offset);

  if (AlignedOffset == Offset)
    return Offset;

  if (isa<StructType>(EltTy) || isa<ArrayType>(EltTy))
    return AlignedOffset;
  TypeSize Size = DL.getTypeStoreSize(EltTy);
  if ((Offset + Size) > AlignedOffset)
    return AlignedOffset;
  else
    return Offset;
}

TypeSize LegacyCBufferLayout::getTypeAllocSize(Type *Ty) {
  if (auto *ST = dyn_cast<StructType>(Ty)) {
    LegacyStructLayout &Layout = getStructLayout(ST);
    return Layout.Size;
  } else if (auto *AT = dyn_cast<ArrayType>(Ty)) {
    unsigned NumElts = AT->getNumElements();
    if (NumElts == 0)
      return TypeSize::getFixed(0);

    TypeSize EltSize = getTypeAllocSize(AT->getElementType());
    TypeSize AlignedEltSize = alignTo4Dwords(EltSize);
    // Each new element start 4 dwords aligned.
    return TypeSize::getFixed(AlignedEltSize * (NumElts - 1) + EltSize);
  } else {
    // NOTE: Use type store size, not align to ABI on basic types for legacy
    // layout.
    return DL.getTypeStoreSize(Ty);
  }
}

LegacyCBufferLayout::LegacyStructLayout &
LegacyCBufferLayout::getStructLayout(StructType *ST) {
  auto it = StructLayouts.find(ST);
  if (it != StructLayouts.end())
    return it->second;

  TypeSize Offset = TypeSize::getFixed(0);
  LegacyStructLayout Layout;
  Layout.ST = ST;
  for (Type *EltTy : ST->elements()) {
    TypeSize EltSize = getTypeAllocSize(EltTy);
    if (TypeSize ScalarSize = EltTy->getScalarType()->getPrimitiveSizeInBits())
      Offset = alignTo(Offset, ScalarSize >> 3);
    Offset = applyRowAlign(Offset, EltTy);
    Layout.Offsets.emplace_back(Offset);
    Offset = Offset.getWithIncrement(EltSize);
  }
  Layout.Size = Offset;
  StructLayouts[ST] = Layout;
  return StructLayouts[ST];
}

CBufferDataLayout::CBufferDataLayout(const DataLayout &DL, const bool IsLegacy)
    : DL(DL), IsLegacyLayout(IsLegacy),
      LegacyDL(IsLegacy ? std::make_unique<LegacyCBufferLayout>(DL) : nullptr) {
}

CBufferDataLayout::~CBufferDataLayout() = default;

toolchain::TypeSize CBufferDataLayout::getTypeAllocSizeInBytes(Type *Ty) {
  if (IsLegacyLayout)
    return LegacyDL->getTypeAllocSizeInBytes(Ty);
  else
    return DL.getTypeAllocSize(Ty);
}

} // namespace dxil
} // namespace vm::core
