//===------ CGGPUBuiltin.cpp - Codegen for GPU builtins -------------------===//
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
// Generates code for built-in GPU calls which are not runtime-specific.
// (Runtime-specific codegen lives in programming model specific files.)
//
//===----------------------------------------------------------------------===//

#include "CodeGenFunction.h"
#include "language/Core/Basic/Builtins.h"
#include "toolchain/IR/DataLayout.h"
#include "toolchain/IR/Instruction.h"
#include "toolchain/Transforms/Utils/AMDGPUEmitPrintf.h"

using namespace language::Core;
using namespace CodeGen;

namespace {
toolchain::Function *GetVprintfDeclaration(toolchain::Module &M) {
  toolchain::Type *ArgTypes[] = {toolchain::PointerType::getUnqual(M.getContext()),
                            toolchain::PointerType::getUnqual(M.getContext())};
  toolchain::FunctionType *VprintfFuncType = toolchain::FunctionType::get(
      toolchain::Type::getInt32Ty(M.getContext()), ArgTypes, false);

  if (auto *F = M.getFunction("vprintf")) {
    // Our CUDA system header declares vprintf with the right signature, so
    // nobody else should have been able to declare vprintf with a bogus
    // signature.
    assert(F->getFunctionType() == VprintfFuncType);
    return F;
  }

  // vprintf doesn't already exist; create a declaration and insert it into the
  // module.
  return toolchain::Function::Create(
      VprintfFuncType, toolchain::GlobalVariable::ExternalLinkage, "vprintf", &M);
}

// Transforms a call to printf into a call to the NVPTX vprintf syscall (which
// isn't particularly special; it's invoked just like a regular function).
// vprintf takes two args: A format string, and a pointer to a buffer containing
// the varargs.
//
// For example, the call
//
//   printf("format string", arg1, arg2, arg3);
//
// is converted into something resembling
//
//   struct Tmp {
//     Arg1 a1;
//     Arg2 a2;
//     Arg3 a3;
//   };
//   char* buf = alloca(sizeof(Tmp));
//   *(Tmp*)buf = {a1, a2, a3};
//   vprintf("format string", buf);
//
// buf is aligned to the max of {alignof(Arg1), ...}.  Furthermore, each of the
// args is itself aligned to its preferred alignment.
//
// Note that by the time this function runs, E's args have already undergone the
// standard C vararg promotion (short -> int, float -> double, etc.).

std::pair<toolchain::Value *, toolchain::TypeSize>
packArgsIntoNVPTXFormatBuffer(CodeGenFunction *CGF, const CallArgList &Args) {
  const toolchain::DataLayout &DL = CGF->CGM.getDataLayout();
  toolchain::LLVMContext &Ctx = CGF->CGM.getLLVMContext();
  CGBuilderTy &Builder = CGF->Builder;

  // Construct and fill the args buffer that we'll pass to vprintf.
  if (Args.size() <= 1) {
    // If there are no args, pass a null pointer and size 0
    toolchain::Value *BufferPtr =
        toolchain::ConstantPointerNull::get(toolchain::PointerType::getUnqual(Ctx));
    return {BufferPtr, toolchain::TypeSize::getFixed(0)};
  } else {
    toolchain::SmallVector<toolchain::Type *, 8> ArgTypes;
    for (unsigned I = 1, NumArgs = Args.size(); I < NumArgs; ++I)
      ArgTypes.push_back(Args[I].getRValue(*CGF).getScalarVal()->getType());

    // Using toolchain::StructType is correct only because printf doesn't accept
    // aggregates.  If we had to handle aggregates here, we'd have to manually
    // compute the offsets within the alloca -- we wouldn't be able to assume
    // that the alignment of the toolchain type was the same as the alignment of the
    // clang type.
    toolchain::Type *AllocaTy = toolchain::StructType::create(ArgTypes, "printf_args");
    toolchain::Value *Alloca = CGF->CreateTempAlloca(AllocaTy);

    for (unsigned I = 1, NumArgs = Args.size(); I < NumArgs; ++I) {
      toolchain::Value *P = Builder.CreateStructGEP(AllocaTy, Alloca, I - 1);
      toolchain::Value *Arg = Args[I].getRValue(*CGF).getScalarVal();
      Builder.CreateAlignedStore(Arg, P, DL.getPrefTypeAlign(Arg->getType()));
    }
    toolchain::Value *BufferPtr =
        Builder.CreatePointerCast(Alloca, toolchain::PointerType::getUnqual(Ctx));
    return {BufferPtr, DL.getTypeAllocSize(AllocaTy)};
  }
}

bool containsNonScalarVarargs(CodeGenFunction *CGF, const CallArgList &Args) {
  return toolchain::any_of(toolchain::drop_begin(Args), [&](const CallArg &A) {
    return !A.getRValue(*CGF).isScalar();
  });
}

RValue EmitDevicePrintfCallExpr(const CallExpr *E, CodeGenFunction *CGF,
                                toolchain::Function *Decl, bool WithSizeArg) {
  CodeGenModule &CGM = CGF->CGM;
  CGBuilderTy &Builder = CGF->Builder;
  assert(E->getBuiltinCallee() == Builtin::BIprintf ||
         E->getBuiltinCallee() == Builtin::BI__builtin_printf);
  assert(E->getNumArgs() >= 1); // printf always has at least one arg.

  // Uses the same format as nvptx for the argument packing, but also passes
  // an i32 for the total size of the passed pointer
  CallArgList Args;
  CGF->EmitCallArgs(Args,
                    E->getDirectCallee()->getType()->getAs<FunctionProtoType>(),
                    E->arguments(), E->getDirectCallee(),
                    /* ParamsToSkip = */ 0);

  // We don't know how to emit non-scalar varargs.
  if (containsNonScalarVarargs(CGF, Args)) {
    CGM.ErrorUnsupported(E, "non-scalar arg to printf");
    return RValue::get(toolchain::ConstantInt::get(CGF->IntTy, 0));
  }

  auto r = packArgsIntoNVPTXFormatBuffer(CGF, Args);
  toolchain::Value *BufferPtr = r.first;

  toolchain::SmallVector<toolchain::Value *, 3> Vec = {
      Args[0].getRValue(*CGF).getScalarVal(), BufferPtr};
  if (WithSizeArg) {
    // Passing > 32bit of data as a local alloca doesn't work for nvptx or
    // amdgpu
    toolchain::Constant *Size =
        toolchain::ConstantInt::get(toolchain::Type::getInt32Ty(CGM.getLLVMContext()),
                               static_cast<uint32_t>(r.second.getFixedValue()));

    Vec.push_back(Size);
  }
  return RValue::get(Builder.CreateCall(Decl, Vec));
}
} // namespace

RValue CodeGenFunction::EmitNVPTXDevicePrintfCallExpr(const CallExpr *E) {
  assert(getTarget().getTriple().isNVPTX());
  return EmitDevicePrintfCallExpr(
      E, this, GetVprintfDeclaration(CGM.getModule()), false);
}

RValue CodeGenFunction::EmitAMDGPUDevicePrintfCallExpr(const CallExpr *E) {
  assert(getTarget().getTriple().isAMDGCN() ||
         (getTarget().getTriple().isSPIRV() &&
          getTarget().getTriple().getVendor() == toolchain::Triple::AMD));
  assert(E->getBuiltinCallee() == Builtin::BIprintf ||
         E->getBuiltinCallee() == Builtin::BI__builtin_printf);
  assert(E->getNumArgs() >= 1); // printf always has at least one arg.

  CallArgList CallArgs;
  EmitCallArgs(CallArgs,
               E->getDirectCallee()->getType()->getAs<FunctionProtoType>(),
               E->arguments(), E->getDirectCallee(),
               /* ParamsToSkip = */ 0);

  SmallVector<toolchain::Value *, 8> Args;
  for (const auto &A : CallArgs) {
    // We don't know how to emit non-scalar varargs.
    if (!A.getRValue(*this).isScalar()) {
      CGM.ErrorUnsupported(E, "non-scalar arg to printf");
      return RValue::get(toolchain::ConstantInt::get(IntTy, -1));
    }

    toolchain::Value *Arg = A.getRValue(*this).getScalarVal();
    Args.push_back(Arg);
  }

  toolchain::IRBuilder<> IRB(Builder.GetInsertBlock(), Builder.GetInsertPoint());
  IRB.SetCurrentDebugLocation(Builder.getCurrentDebugLocation());

  bool isBuffered = (CGM.getTarget().getTargetOpts().AMDGPUPrintfKindVal ==
                     language::Core::TargetOptions::AMDGPUPrintfKind::Buffered);
  auto Printf = toolchain::emitAMDGPUPrintfCall(IRB, Args, isBuffered);
  Builder.SetInsertPoint(IRB.GetInsertBlock(), IRB.GetInsertPoint());
  return RValue::get(Printf);
}
