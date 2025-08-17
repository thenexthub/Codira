//===- M68k.cpp -----------------------------------------------------------===//
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
// M68k ABI Implementation
//===----------------------------------------------------------------------===//

namespace {

class M68kTargetCodeGenInfo : public TargetCodeGenInfo {
public:
  M68kTargetCodeGenInfo(CodeGenTypes &CGT)
      : TargetCodeGenInfo(std::make_unique<DefaultABIInfo>(CGT)) {}
  void setTargetAttributes(const Decl *D, toolchain::GlobalValue *GV,
                           CodeGen::CodeGenModule &M) const override;
};

} // namespace

void M68kTargetCodeGenInfo::setTargetAttributes(
    const Decl *D, toolchain::GlobalValue *GV, CodeGen::CodeGenModule &M) const {
  if (const auto *FD = dyn_cast_or_null<FunctionDecl>(D)) {
    if (const auto *attr = FD->getAttr<M68kInterruptAttr>()) {
      // Handle 'interrupt' attribute:
      toolchain::Function *F = cast<toolchain::Function>(GV);

      // Step 1: Set ISR calling convention.
      F->setCallingConv(toolchain::CallingConv::M68k_INTR);

      // Step 2: Add attributes goodness.
      F->addFnAttr(toolchain::Attribute::NoInline);

      // Step 3: Emit ISR vector alias.
      unsigned Num = attr->getNumber() / 2;
      toolchain::GlobalAlias::create(toolchain::Function::ExternalLinkage,
                                "__isr_" + Twine(Num), F);
    }
  }
}

std::unique_ptr<TargetCodeGenInfo>
CodeGen::createM68kTargetCodeGenInfo(CodeGenModule &CGM) {
  return std::make_unique<M68kTargetCodeGenInfo>(CGM.getTypes());
}
