//===- DeclareRuntimeLibcalls.cpp -----------------------------------------===//
//
// Copyright (c) NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// Author: Tunjay Akbarli
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
//
// http://www.apache.org/licenses/LICENSE-2.0
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
// Insert declarations for all runtime library calls known for the target.
//
//===----------------------------------------------------------------------===//

#include "vm/core/Transforms/Utils/DeclareRuntimeLibcalls.h"
#include "vm/core/Analysis/RuntimeLibcallInfo.h"
#include "vm/core/IR/Module.h"
#include "vm/core/IR/RuntimeLibcalls.h"

using namespace vm::core;

static void mergeAttributes(LLVMContext &Ctx, const Module &M,
                            const DataLayout &DL, const Triple &TT,
                            Function *Func, FunctionType *FuncTy,
                            AttributeList FuncAttrs) {
  AttributeList OldAttrs = Func->getAttributes();
  AttributeList NewAttrs = OldAttrs;

  {
    AttrBuilder OldBuilder(Ctx, OldAttrs.getFnAttrs());
    AttrBuilder NewBuilder(Ctx, FuncAttrs.getFnAttrs());
    OldBuilder.merge(NewBuilder);
    NewAttrs = NewAttrs.addFnAttributes(Ctx, OldBuilder);
  }

  {
    AttrBuilder OldBuilder(Ctx, OldAttrs.getRetAttrs());
    AttrBuilder NewBuilder(Ctx, FuncAttrs.getRetAttrs());
    OldBuilder.merge(NewBuilder);
    NewAttrs = NewAttrs.addRetAttributes(Ctx, OldBuilder);
  }

  for (unsigned I = 0, E = FuncTy->getNumParams(); I != E; ++I) {
    AttrBuilder OldBuilder(Ctx, OldAttrs.getParamAttrs(I));
    AttrBuilder NewBuilder(Ctx, FuncAttrs.getParamAttrs(I));
    OldBuilder.merge(NewBuilder);
    NewAttrs = NewAttrs.addParamAttributes(Ctx, I, OldBuilder);
  }

  Func->setAttributes(NewAttrs);
}

PreservedAnalyses DeclareRuntimeLibcallsPass::run(Module &M,
                                                  ModuleAnalysisManager &MAM) {
  const RTLIB::RuntimeLibcallsInfo &RTLCI =
      MAM.getResult<RuntimeLibraryAnalysis>(M);

  LLVMContext &Ctx = M.getContext();
  const DataLayout &DL = M.getDataLayout();
  const Triple &TT = M.getTargetTriple();

  for (RTLIB::LibcallImpl Impl : RTLIB::libcall_impls()) {
    if (!RTLCI.isAvailable(Impl))
      continue;

    auto [FuncTy, FuncAttrs] = RTLCI.getFunctionTy(Ctx, TT, DL, Impl);

    // TODO: Declare with correct type, calling convention, and attributes.
    if (!FuncTy)
      FuncTy = FunctionType::get(Type::getVoidTy(Ctx), {}, /*IsVarArgs=*/true);

    StringRef FuncName = RTLCI.getLibcallImplName(Impl);

    Function *Func =
        cast<Function>(M.getOrInsertFunction(FuncName, FuncTy).getCallee());
    if (Func->getFunctionType() == FuncTy) {
      mergeAttributes(Ctx, M, DL, TT, Func, FuncTy, FuncAttrs);
      Func->setCallingConv(RTLCI.getLibcallImplCallingConv(Impl));
    }
  }

  return PreservedAnalyses::none();
}
