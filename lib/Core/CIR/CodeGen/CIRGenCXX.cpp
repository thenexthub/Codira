//===----------------------------------------------------------------------===//
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
// This contains code dealing with C++ code generation.
//
//===----------------------------------------------------------------------===//

#include "CIRGenFunction.h"
#include "CIRGenModule.h"

#include "language/Core/AST/GlobalDecl.h"
#include "language/Core/CIR/MissingFeatures.h"

using namespace language::Core;
using namespace language::Core::CIRGen;

cir::FuncOp CIRGenModule::codegenCXXStructor(GlobalDecl gd) {
  const CIRGenFunctionInfo &fnInfo =
      getTypes().arrangeCXXStructorDeclaration(gd);
  cir::FuncType funcType = getTypes().getFunctionType(fnInfo);
  cir::FuncOp fn = getAddrOfCXXStructor(gd, &fnInfo, /*FnType=*/nullptr,
                                        /*DontDefer=*/true, ForDefinition);
  setFunctionLinkage(gd, fn);
  CIRGenFunction cgf{*this, builder};
  curCGF = &cgf;
  {
    mlir::OpBuilder::InsertionGuard guard(builder);
    cgf.generateCode(gd, fn, funcType);
  }
  curCGF = nullptr;

  setNonAliasAttributes(gd, fn);
  assert(!cir::MissingFeatures::opFuncAttributesForDefinition());
  return fn;
}
