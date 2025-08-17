//===--------- CodeGenSYCL.cpp - Code for SYCL kernel generation ----------===//
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
// This contains code required for generation of SYCL kernel caller offload
// entry point functions.
//
//===----------------------------------------------------------------------===//

#include "CodeGenFunction.h"
#include "CodeGenModule.h"

using namespace language::Core;
using namespace CodeGen;

static void SetSYCLKernelAttributes(toolchain::Function *Fn, CodeGenFunction &CGF) {
  // SYCL 2020 device language restrictions require forward progress and
  // disallow recursion.
  Fn->setDoesNotRecurse();
  if (CGF.checkIfFunctionMustProgress())
    Fn->addFnAttr(toolchain::Attribute::MustProgress);
}

void CodeGenModule::EmitSYCLKernelCaller(const FunctionDecl *KernelEntryPointFn,
                                         ASTContext &Ctx) {
  assert(Ctx.getLangOpts().SYCLIsDevice &&
         "SYCL kernel caller offload entry point functions can only be emitted"
         " during device compilation");

  const auto *KernelEntryPointAttr =
      KernelEntryPointFn->getAttr<SYCLKernelEntryPointAttr>();
  assert(KernelEntryPointAttr && "Missing sycl_kernel_entry_point attribute");
  assert(!KernelEntryPointAttr->isInvalidAttr() &&
         "sycl_kernel_entry_point attribute is invalid");

  // Find the SYCLKernelCallStmt.
  SYCLKernelCallStmt *KernelCallStmt =
      cast<SYCLKernelCallStmt>(KernelEntryPointFn->getBody());

  // Retrieve the SYCL kernel caller parameters from the OutlinedFunctionDecl.
  FunctionArgList Args;
  const OutlinedFunctionDecl *OutlinedFnDecl =
      KernelCallStmt->getOutlinedFunctionDecl();
  Args.append(OutlinedFnDecl->param_begin(), OutlinedFnDecl->param_end());

  // Compute the function info and LLVM function type.
  const CGFunctionInfo &FnInfo =
      getTypes().arrangeSYCLKernelCallerDeclaration(Ctx.VoidTy, Args);
  toolchain::FunctionType *FnTy = getTypes().GetFunctionType(FnInfo);

  // Retrieve the generated name for the SYCL kernel caller function.
  CanQualType KernelNameType =
      Ctx.getCanonicalType(KernelEntryPointAttr->getKernelName());
  const SYCLKernelInfo &KernelInfo = Ctx.getSYCLKernelInfo(KernelNameType);
  auto *Fn = toolchain::Function::Create(FnTy, toolchain::Function::ExternalLinkage,
                                    KernelInfo.GetKernelName(), &getModule());

  // Emit the SYCL kernel caller function.
  CodeGenFunction CGF(*this);
  SetLLVMFunctionAttributes(GlobalDecl(), FnInfo, Fn, false);
  SetSYCLKernelAttributes(Fn, CGF);
  CGF.StartFunction(GlobalDecl(), Ctx.VoidTy, Fn, FnInfo, Args,
                    SourceLocation(), SourceLocation());
  CGF.EmitFunctionBody(OutlinedFnDecl->getBody());
  setDSOLocal(Fn);
  SetLLVMFunctionAttributesForDefinition(cast<Decl>(OutlinedFnDecl), Fn);
  CGF.FinishFunction();
}
