//===- TCE.cpp ------------------------------------------------------------===//
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
// TCE ABI Implementation (see http://tce.cs.tut.fi). Uses mostly the defaults.
// Currently subclassed only to implement custom OpenCL C function attribute
// handling.
//===----------------------------------------------------------------------===//

namespace {

class TCETargetCodeGenInfo : public TargetCodeGenInfo {
public:
  TCETargetCodeGenInfo(CodeGenTypes &CGT)
      : TargetCodeGenInfo(std::make_unique<DefaultABIInfo>(CGT)) {}

  void setTargetAttributes(const Decl *D, toolchain::GlobalValue *GV,
                           CodeGen::CodeGenModule &M) const override;
};

void TCETargetCodeGenInfo::setTargetAttributes(
    const Decl *D, toolchain::GlobalValue *GV, CodeGen::CodeGenModule &M) const {
  if (GV->isDeclaration())
    return;
  const FunctionDecl *FD = dyn_cast_or_null<FunctionDecl>(D);
  if (!FD) return;

  toolchain::Function *F = cast<toolchain::Function>(GV);

  if (M.getLangOpts().OpenCL) {
    if (FD->hasAttr<DeviceKernelAttr>()) {
      // OpenCL C Kernel functions are not subject to inlining
      F->addFnAttr(toolchain::Attribute::NoInline);
      const ReqdWorkGroupSizeAttr *Attr = FD->getAttr<ReqdWorkGroupSizeAttr>();
      if (Attr) {
        // Convert the reqd_work_group_size() attributes to metadata.
        toolchain::LLVMContext &Context = F->getContext();
        toolchain::NamedMDNode *OpenCLMetadata =
            M.getModule().getOrInsertNamedMetadata(
                "opencl.kernel_wg_size_info");

        auto Eval = [&](Expr *E) {
          return E->EvaluateKnownConstInt(FD->getASTContext());
        };
        SmallVector<toolchain::Metadata *, 5> Operands{
            toolchain::ConstantAsMetadata::get(F),
            toolchain::ConstantAsMetadata::get(toolchain::Constant::getIntegerValue(
                M.Int32Ty, Eval(Attr->getXDim()))),
            toolchain::ConstantAsMetadata::get(toolchain::Constant::getIntegerValue(
                M.Int32Ty, Eval(Attr->getYDim()))),
            toolchain::ConstantAsMetadata::get(toolchain::Constant::getIntegerValue(
                M.Int32Ty, Eval(Attr->getZDim()))),
            // Add a boolean constant operand for "required" (true) or "hint"
            // (false) for implementing the work_group_size_hint attr later.
            // Currently always true as the hint is not yet implemented.
            toolchain::ConstantAsMetadata::get(toolchain::ConstantInt::getTrue(Context))};
        OpenCLMetadata->addOperand(toolchain::MDNode::get(Context, Operands));
      }
    }
  }
}

}

std::unique_ptr<TargetCodeGenInfo>
CodeGen::createTCETargetCodeGenInfo(CodeGenModule &CGM) {
  return std::make_unique<TCETargetCodeGenInfo>(CGM.getTypes());
}
