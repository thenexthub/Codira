//===--------- DirectX.cpp - Emit LLVM Code for builtins ------------------===//
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
// This contains code to emit Builtin calls as LLVM code.
//
//===----------------------------------------------------------------------===//

#include "CGHLSLRuntime.h"
#include "CodeGenFunction.h"
#include "language/Core/Basic/TargetBuiltins.h"
#include "toolchain/IR/Intrinsics.h"

using namespace language::Core;
using namespace CodeGen;
using namespace toolchain;

Value *CodeGenFunction::EmitDirectXBuiltinExpr(unsigned BuiltinID,
                                               const CallExpr *E) {
  switch (BuiltinID) {
  case DirectX::BI__builtin_dx_dot2add: {
    Value *A = EmitScalarExpr(E->getArg(0));
    Value *B = EmitScalarExpr(E->getArg(1));
    Value *Acc = EmitScalarExpr(E->getArg(2));

    Value *AX = Builder.CreateExtractElement(A, Builder.getSize(0));
    Value *AY = Builder.CreateExtractElement(A, Builder.getSize(1));
    Value *BX = Builder.CreateExtractElement(B, Builder.getSize(0));
    Value *BY = Builder.CreateExtractElement(B, Builder.getSize(1));

    Intrinsic::ID ID = toolchain ::Intrinsic::dx_dot2add;
    return Builder.CreateIntrinsic(
        /*ReturnType=*/Acc->getType(), ID,
        ArrayRef<Value *>{Acc, AX, AY, BX, BY}, nullptr, "dx.dot2add");
  }
  }
  return nullptr;
}
