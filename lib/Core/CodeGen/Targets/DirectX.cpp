//===- DirectX.cpp---------------------------------------------------------===//
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
#include "CodeGenModule.h"
#include "HLSLBufferLayoutBuilder.h"
#include "TargetInfo.h"
#include "language/Core/AST/Type.h"
#include "toolchain/ADT/SmallVector.h"
#include "toolchain/IR/DerivedTypes.h"
#include "toolchain/IR/Type.h"

using namespace language::Core;
using namespace language::Core::CodeGen;

//===----------------------------------------------------------------------===//
// Target codegen info implementation for DirectX.
//===----------------------------------------------------------------------===//

namespace {

class DirectXTargetCodeGenInfo : public TargetCodeGenInfo {
public:
  DirectXTargetCodeGenInfo(CodeGen::CodeGenTypes &CGT)
      : TargetCodeGenInfo(std::make_unique<DefaultABIInfo>(CGT)) {}

  toolchain::Type *
  getHLSLType(CodeGenModule &CGM, const Type *T,
              const SmallVector<int32_t> *Packoffsets = nullptr) const override;
};

toolchain::Type *DirectXTargetCodeGenInfo::getHLSLType(
    CodeGenModule &CGM, const Type *Ty,
    const SmallVector<int32_t> *Packoffsets) const {
  auto *ResType = dyn_cast<HLSLAttributedResourceType>(Ty);
  if (!ResType)
    return nullptr;

  toolchain::LLVMContext &Ctx = CGM.getLLVMContext();
  const HLSLAttributedResourceType::Attributes &ResAttrs = ResType->getAttrs();
  switch (ResAttrs.ResourceClass) {
  case toolchain::dxil::ResourceClass::UAV:
  case toolchain::dxil::ResourceClass::SRV: {
    // TypedBuffer and RawBuffer both need element type
    QualType ContainedTy = ResType->getContainedType();
    if (ContainedTy.isNull())
      return nullptr;

    // convert element type
    toolchain::Type *ElemType = CGM.getTypes().ConvertTypeForMem(ContainedTy);

    toolchain::StringRef TypeName =
        ResAttrs.RawBuffer ? "dx.RawBuffer" : "dx.TypedBuffer";
    SmallVector<unsigned, 3> Ints = {/*IsWriteable*/ ResAttrs.ResourceClass ==
                                         toolchain::dxil::ResourceClass::UAV,
                                     /*IsROV*/ ResAttrs.IsROV};
    if (!ResAttrs.RawBuffer) {
      const language::Core::Type *ElemType = ContainedTy->getUnqualifiedDesugaredType();
      if (ElemType->isVectorType())
        ElemType = cast<language::Core::VectorType>(ElemType)
                       ->getElementType()
                       ->getUnqualifiedDesugaredType();
      Ints.push_back(/*IsSigned*/ ElemType->isSignedIntegerType());
    }

    return toolchain::TargetExtType::get(Ctx, TypeName, {ElemType}, Ints);
  }
  case toolchain::dxil::ResourceClass::CBuffer: {
    QualType ContainedTy = ResType->getContainedType();
    if (ContainedTy.isNull() || !ContainedTy->isStructureType())
      return nullptr;

    toolchain::Type *BufferLayoutTy =
        HLSLBufferLayoutBuilder(CGM, "dx.Layout")
            .createLayoutType(ContainedTy->getAsStructureType(), Packoffsets);
    if (!BufferLayoutTy)
      return nullptr;

    return toolchain::TargetExtType::get(Ctx, "dx.CBuffer", {BufferLayoutTy});
  }
  case toolchain::dxil::ResourceClass::Sampler:
    toolchain_unreachable("dx.Sampler handles are not implemented yet");
    break;
  }
  toolchain_unreachable("Unknown toolchain::dxil::ResourceClass enum");
}

} // namespace

std::unique_ptr<TargetCodeGenInfo>
CodeGen::createDirectXTargetCodeGenInfo(CodeGenModule &CGM) {
  return std::make_unique<DirectXTargetCodeGenInfo>(CGM.getTypes());
}
