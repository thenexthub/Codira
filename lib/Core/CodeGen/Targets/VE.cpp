//===- VE.cpp -------------------------------------------------------------===//
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
// VE ABI Implementation.
//
namespace {
class VEABIInfo : public DefaultABIInfo {
public:
  VEABIInfo(CodeGenTypes &CGT) : DefaultABIInfo(CGT) {}

private:
  ABIArgInfo classifyReturnType(QualType RetTy) const;
  ABIArgInfo classifyArgumentType(QualType RetTy) const;
  void computeInfo(CGFunctionInfo &FI) const override;
};
} // end anonymous namespace

ABIArgInfo VEABIInfo::classifyReturnType(QualType Ty) const {
  if (Ty->isAnyComplexType())
    return ABIArgInfo::getDirect();
  uint64_t Size = getContext().getTypeSize(Ty);
  if (Size < 64 && Ty->isIntegerType())
    return ABIArgInfo::getExtend(Ty);
  return DefaultABIInfo::classifyReturnType(Ty);
}

ABIArgInfo VEABIInfo::classifyArgumentType(QualType Ty) const {
  if (Ty->isAnyComplexType())
    return ABIArgInfo::getDirect();
  uint64_t Size = getContext().getTypeSize(Ty);
  if (Size < 64 && Ty->isIntegerType())
    return ABIArgInfo::getExtend(Ty);
  return DefaultABIInfo::classifyArgumentType(Ty);
}

void VEABIInfo::computeInfo(CGFunctionInfo &FI) const {
  FI.getReturnInfo() = classifyReturnType(FI.getReturnType());
  for (auto &Arg : FI.arguments())
    Arg.info = classifyArgumentType(Arg.type);
}

namespace {
class VETargetCodeGenInfo : public TargetCodeGenInfo {
public:
  VETargetCodeGenInfo(CodeGenTypes &CGT)
      : TargetCodeGenInfo(std::make_unique<VEABIInfo>(CGT)) {}
  // VE ABI requires the arguments of variadic and prototype-less functions
  // are passed in both registers and memory.
  bool isNoProtoCallVariadic(const CallArgList &args,
                             const FunctionNoProtoType *fnType) const override {
    return true;
  }
};
} // end anonymous namespace

std::unique_ptr<TargetCodeGenInfo>
CodeGen::createVETargetCodeGenInfo(CodeGenModule &CGM) {
  return std::make_unique<VETargetCodeGenInfo>(CGM.getTypes());
}
