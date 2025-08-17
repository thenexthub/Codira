//===- CSKY.cpp -----------------------------------------------------------===//
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

#include "ABIInfoImpl.h"
#include "TargetInfo.h"

using namespace language::Core;
using namespace language::Core::CodeGen;

//===----------------------------------------------------------------------===//
// CSKY ABI Implementation
//===----------------------------------------------------------------------===//
namespace {
class CSKYABIInfo : public DefaultABIInfo {
  static const int NumArgGPRs = 4;
  static const int NumArgFPRs = 4;

  static const unsigned XLen = 32;
  unsigned FLen;

public:
  CSKYABIInfo(CodeGen::CodeGenTypes &CGT, unsigned FLen)
      : DefaultABIInfo(CGT), FLen(FLen) {}

  void computeInfo(CGFunctionInfo &FI) const override;
  ABIArgInfo classifyArgumentType(QualType Ty, int &ArgGPRsLeft,
                                  int &ArgFPRsLeft,
                                  bool isReturnType = false) const;
  ABIArgInfo classifyReturnType(QualType RetTy) const;

  RValue EmitVAArg(CodeGenFunction &CGF, Address VAListAddr, QualType Ty,
                   AggValueSlot Slot) const override;
};

} // end anonymous namespace

void CSKYABIInfo::computeInfo(CGFunctionInfo &FI) const {
  QualType RetTy = FI.getReturnType();
  if (!getCXXABI().classifyReturnType(FI))
    FI.getReturnInfo() = classifyReturnType(RetTy);

  bool IsRetIndirect = FI.getReturnInfo().getKind() == ABIArgInfo::Indirect;

  // We must track the number of GPRs used in order to conform to the CSKY
  // ABI, as integer scalars passed in registers should have signext/zeroext
  // when promoted.
  int ArgGPRsLeft = IsRetIndirect ? NumArgGPRs - 1 : NumArgGPRs;
  int ArgFPRsLeft = FLen ? NumArgFPRs : 0;

  for (auto &ArgInfo : FI.arguments()) {
    ArgInfo.info = classifyArgumentType(ArgInfo.type, ArgGPRsLeft, ArgFPRsLeft);
  }
}

RValue CSKYABIInfo::EmitVAArg(CodeGenFunction &CGF, Address VAListAddr,
                              QualType Ty, AggValueSlot Slot) const {
  CharUnits SlotSize = CharUnits::fromQuantity(XLen / 8);

  // Empty records are ignored for parameter passing purposes.
  if (isEmptyRecord(getContext(), Ty, true))
    return Slot.asRValue();

  auto TInfo = getContext().getTypeInfoInChars(Ty);

  return emitVoidPtrVAArg(CGF, VAListAddr, Ty, false, TInfo, SlotSize,
                          /*AllowHigherAlign=*/true, Slot);
}

ABIArgInfo CSKYABIInfo::classifyArgumentType(QualType Ty, int &ArgGPRsLeft,
                                             int &ArgFPRsLeft,
                                             bool isReturnType) const {
  assert(ArgGPRsLeft <= NumArgGPRs && "Arg GPR tracking underflow");
  Ty = useFirstFieldIfTransparentUnion(Ty);

  // Structures with either a non-trivial destructor or a non-trivial
  // copy constructor are always passed indirectly.
  if (CGCXXABI::RecordArgABI RAA = getRecordArgABI(Ty, getCXXABI())) {
    if (ArgGPRsLeft)
      ArgGPRsLeft -= 1;
    return getNaturalAlignIndirect(
        Ty, /*AddrSpace=*/getDataLayout().getAllocaAddrSpace(),
        /*ByVal=*/RAA == CGCXXABI::RAA_DirectInMemory);
  }

  // Ignore empty structs/unions.
  if (isEmptyRecord(getContext(), Ty, true))
    return ABIArgInfo::getIgnore();

  if (!Ty->getAsUnionType())
    if (const Type *SeltTy = isSingleElementStruct(Ty, getContext()))
      return ABIArgInfo::getDirect(CGT.ConvertType(QualType(SeltTy, 0)));

  uint64_t Size = getContext().getTypeSize(Ty);
  // Pass floating point values via FPRs if possible.
  if (Ty->isFloatingType() && !Ty->isComplexType() && FLen >= Size &&
      ArgFPRsLeft) {
    ArgFPRsLeft--;
    return ABIArgInfo::getDirect();
  }

  // Complex types for the hard float ABI must be passed direct rather than
  // using CoerceAndExpand.
  if (Ty->isComplexType() && FLen && !isReturnType) {
    QualType EltTy = Ty->castAs<ComplexType>()->getElementType();
    if (getContext().getTypeSize(EltTy) <= FLen) {
      ArgFPRsLeft -= 2;
      return ABIArgInfo::getDirect();
    }
  }

  if (!isAggregateTypeForABI(Ty)) {
    // Treat an enum type as its underlying type.
    if (const EnumType *EnumTy = Ty->getAs<EnumType>())
      Ty = EnumTy->getOriginalDecl()->getDefinitionOrSelf()->getIntegerType();

    // All integral types are promoted to XLen width, unless passed on the
    // stack.
    if (Size < XLen && Ty->isIntegralOrEnumerationType())
      return ABIArgInfo::getExtend(Ty);

    if (const auto *EIT = Ty->getAs<BitIntType>()) {
      if (EIT->getNumBits() < XLen)
        return ABIArgInfo::getExtend(Ty);
    }

    return ABIArgInfo::getDirect();
  }

  // For argument type, the first 4*XLen parts of aggregate will be passed
  // in registers, and the rest will be passed in stack.
  // So we can coerce to integers directly and let backend handle it correctly.
  // For return type, aggregate which <= 2*XLen will be returned in registers.
  // Otherwise, aggregate will be returned indirectly.
  if (!isReturnType || (isReturnType && Size <= 2 * XLen)) {
    if (Size <= XLen) {
      return ABIArgInfo::getDirect(
          toolchain::IntegerType::get(getVMContext(), XLen));
    } else {
      return ABIArgInfo::getDirect(toolchain::ArrayType::get(
          toolchain::IntegerType::get(getVMContext(), XLen), (Size + 31) / XLen));
    }
  }
  return getNaturalAlignIndirect(Ty, getDataLayout().getAllocaAddrSpace(),
                                 /*ByVal=*/false);
}

ABIArgInfo CSKYABIInfo::classifyReturnType(QualType RetTy) const {
  if (RetTy->isVoidType())
    return ABIArgInfo::getIgnore();

  int ArgGPRsLeft = 2;
  int ArgFPRsLeft = FLen ? 1 : 0;

  // The rules for return and argument types are the same, so defer to
  // classifyArgumentType.
  return classifyArgumentType(RetTy, ArgGPRsLeft, ArgFPRsLeft, true);
}

namespace {
class CSKYTargetCodeGenInfo : public TargetCodeGenInfo {
public:
  CSKYTargetCodeGenInfo(CodeGen::CodeGenTypes &CGT, unsigned FLen)
      : TargetCodeGenInfo(std::make_unique<CSKYABIInfo>(CGT, FLen)) {}
};
} // end anonymous namespace

std::unique_ptr<TargetCodeGenInfo>
CodeGen::createCSKYTargetCodeGenInfo(CodeGenModule &CGM, unsigned FLen) {
  return std::make_unique<CSKYTargetCodeGenInfo>(CGM.getTypes(), FLen);
}
