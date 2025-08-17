//===- BPF.cpp ------------------------------------------------------------===//
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
// BPF ABI Implementation
//===----------------------------------------------------------------------===//

namespace {

class BPFABIInfo : public DefaultABIInfo {
public:
  BPFABIInfo(CodeGenTypes &CGT) : DefaultABIInfo(CGT) {}

  ABIArgInfo classifyArgumentType(QualType Ty) const {
    Ty = useFirstFieldIfTransparentUnion(Ty);

    if (isAggregateTypeForABI(Ty)) {
      uint64_t Bits = getContext().getTypeSize(Ty);
      if (Bits == 0)
        return ABIArgInfo::getIgnore();

      // If the aggregate needs 1 or 2 registers, do not use reference.
      if (Bits <= 128) {
        toolchain::Type *CoerceTy;
        if (Bits <= 64) {
          CoerceTy =
              toolchain::IntegerType::get(getVMContext(), toolchain::alignTo(Bits, 8));
        } else {
          toolchain::Type *RegTy = toolchain::IntegerType::get(getVMContext(), 64);
          CoerceTy = toolchain::ArrayType::get(RegTy, 2);
        }
        return ABIArgInfo::getDirect(CoerceTy);
      } else {
        return getNaturalAlignIndirect(Ty,
                                       getDataLayout().getAllocaAddrSpace());
      }
    }

    if (const EnumType *EnumTy = Ty->getAs<EnumType>())
      Ty = EnumTy->getOriginalDecl()->getDefinitionOrSelf()->getIntegerType();

    ASTContext &Context = getContext();
    if (const auto *EIT = Ty->getAs<BitIntType>())
      if (EIT->getNumBits() > Context.getTypeSize(Context.Int128Ty))
        return getNaturalAlignIndirect(Ty,
                                       getDataLayout().getAllocaAddrSpace());

    return (isPromotableIntegerTypeForABI(Ty) ? ABIArgInfo::getExtend(Ty)
                                              : ABIArgInfo::getDirect());
  }

  ABIArgInfo classifyReturnType(QualType RetTy) const {
    if (RetTy->isVoidType())
      return ABIArgInfo::getIgnore();

    if (isAggregateTypeForABI(RetTy))
      return getNaturalAlignIndirect(RetTy,
                                     getDataLayout().getAllocaAddrSpace());

    // Treat an enum type as its underlying type.
    if (const EnumType *EnumTy = RetTy->getAs<EnumType>())
      RetTy =
          EnumTy->getOriginalDecl()->getDefinitionOrSelf()->getIntegerType();

    ASTContext &Context = getContext();
    if (const auto *EIT = RetTy->getAs<BitIntType>())
      if (EIT->getNumBits() > Context.getTypeSize(Context.Int128Ty))
        return getNaturalAlignIndirect(RetTy,
                                       getDataLayout().getAllocaAddrSpace());

    // Caller will do necessary sign/zero extension.
    return ABIArgInfo::getDirect();
  }

  void computeInfo(CGFunctionInfo &FI) const override {
    FI.getReturnInfo() = classifyReturnType(FI.getReturnType());
    for (auto &I : FI.arguments())
      I.info = classifyArgumentType(I.type);
  }

};

class BPFTargetCodeGenInfo : public TargetCodeGenInfo {
public:
  BPFTargetCodeGenInfo(CodeGenTypes &CGT)
      : TargetCodeGenInfo(std::make_unique<BPFABIInfo>(CGT)) {}
};

}

std::unique_ptr<TargetCodeGenInfo>
CodeGen::createBPFTargetCodeGenInfo(CodeGenModule &CGM) {
  return std::make_unique<BPFTargetCodeGenInfo>(CGM.getTypes());
}
