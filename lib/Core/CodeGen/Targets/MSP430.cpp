//===- MSP430.cpp ---------------------------------------------------------===//
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
#include "toolchain/ADT/StringExtras.h"

using namespace language::Core;
using namespace language::Core::CodeGen;

//===----------------------------------------------------------------------===//
// MSP430 ABI Implementation
//===----------------------------------------------------------------------===//

namespace {

class MSP430ABIInfo : public DefaultABIInfo {
  static ABIArgInfo complexArgInfo() {
    ABIArgInfo Info = ABIArgInfo::getDirect();
    Info.setCanBeFlattened(false);
    return Info;
  }

public:
  MSP430ABIInfo(CodeGenTypes &CGT) : DefaultABIInfo(CGT) {}

  ABIArgInfo classifyReturnType(QualType RetTy) const {
    if (RetTy->isAnyComplexType())
      return complexArgInfo();

    return DefaultABIInfo::classifyReturnType(RetTy);
  }

  ABIArgInfo classifyArgumentType(QualType RetTy) const {
    if (RetTy->isAnyComplexType())
      return complexArgInfo();

    return DefaultABIInfo::classifyArgumentType(RetTy);
  }

  // Just copy the original implementations because
  // DefaultABIInfo::classify{Return,Argument}Type() are not virtual
  void computeInfo(CGFunctionInfo &FI) const override {
    if (!getCXXABI().classifyReturnType(FI))
      FI.getReturnInfo() = classifyReturnType(FI.getReturnType());
    for (auto &I : FI.arguments())
      I.info = classifyArgumentType(I.type);
  }

  RValue EmitVAArg(CodeGenFunction &CGF, Address VAListAddr, QualType Ty,
                   AggValueSlot Slot) const override {
    return CGF.EmitLoadOfAnyValue(
        CGF.MakeAddrLValue(
            EmitVAArgInstr(CGF, VAListAddr, Ty, classifyArgumentType(Ty)), Ty),
        Slot);
  }
};

class MSP430TargetCodeGenInfo : public TargetCodeGenInfo {
public:
  MSP430TargetCodeGenInfo(CodeGenTypes &CGT)
      : TargetCodeGenInfo(std::make_unique<MSP430ABIInfo>(CGT)) {}
  void setTargetAttributes(const Decl *D, toolchain::GlobalValue *GV,
                           CodeGen::CodeGenModule &M) const override;
};

}

void MSP430TargetCodeGenInfo::setTargetAttributes(
    const Decl *D, toolchain::GlobalValue *GV, CodeGen::CodeGenModule &M) const {
  if (GV->isDeclaration())
    return;
  if (const FunctionDecl *FD = dyn_cast_or_null<FunctionDecl>(D)) {
    const auto *InterruptAttr = FD->getAttr<MSP430InterruptAttr>();
    if (!InterruptAttr)
      return;

    // Handle 'interrupt' attribute:
    toolchain::Function *F = cast<toolchain::Function>(GV);

    // Step 1: Set ISR calling convention.
    F->setCallingConv(toolchain::CallingConv::MSP430_INTR);

    // Step 2: Add attributes goodness.
    F->addFnAttr(toolchain::Attribute::NoInline);
    F->addFnAttr("interrupt", toolchain::utostr(InterruptAttr->getNumber()));
  }
}

std::unique_ptr<TargetCodeGenInfo>
CodeGen::createMSP430TargetCodeGenInfo(CodeGenModule &CGM) {
  return std::make_unique<MSP430TargetCodeGenInfo>(CGM.getTypes());
}
