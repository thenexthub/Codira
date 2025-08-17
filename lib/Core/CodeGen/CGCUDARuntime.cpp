//===----- CGCUDARuntime.cpp - Interface to CUDA Runtimes -----------------===//
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
//
// This provides an abstract class for CUDA code generation.  Concrete
// subclasses of this implement code generation for specific CUDA
// runtime libraries.
//
//===----------------------------------------------------------------------===//

#include "CGCUDARuntime.h"
#include "CGCall.h"
#include "CodeGenFunction.h"
#include "language/Core/AST/ExprCXX.h"

using namespace language::Core;
using namespace CodeGen;

CGCUDARuntime::~CGCUDARuntime() {}

RValue CGCUDARuntime::EmitCUDAKernelCallExpr(CodeGenFunction &CGF,
                                             const CUDAKernelCallExpr *E,
                                             ReturnValueSlot ReturnValue,
                                             toolchain::CallBase **CallOrInvoke) {
  toolchain::BasicBlock *ConfigOKBlock = CGF.createBasicBlock("kcall.configok");
  toolchain::BasicBlock *ContBlock = CGF.createBasicBlock("kcall.end");

  CodeGenFunction::ConditionalEvaluation eval(CGF);
  CGF.EmitBranchOnBoolExpr(E->getConfig(), ContBlock, ConfigOKBlock,
                           /*TrueCount=*/0);

  eval.begin(CGF);
  CGF.EmitBlock(ConfigOKBlock);
  CGF.EmitSimpleCallExpr(E, ReturnValue, CallOrInvoke);
  CGF.EmitBranch(ContBlock);

  CGF.EmitBlock(ContBlock);
  eval.end(CGF);

  return RValue::get(nullptr);
}
