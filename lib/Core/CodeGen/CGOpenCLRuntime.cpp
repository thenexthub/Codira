//===----- CGOpenCLRuntime.cpp - Interface to OpenCL Runtimes -------------===//
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
// This provides an abstract class for OpenCL code generation.  Concrete
// subclasses of this implement code generation for specific OpenCL
// runtime libraries.
//
//===----------------------------------------------------------------------===//

#include "CGOpenCLRuntime.h"
#include "CodeGenFunction.h"
#include "TargetInfo.h"
#include "language/Core/CodeGen/ConstantInitBuilder.h"
#include "toolchain/IR/DerivedTypes.h"
#include "toolchain/IR/GlobalValue.h"
#include <assert.h>

using namespace language::Core;
using namespace CodeGen;

CGOpenCLRuntime::~CGOpenCLRuntime() {}

void CGOpenCLRuntime::EmitWorkGroupLocalVarDecl(CodeGenFunction &CGF,
                                                const VarDecl &D) {
  return CGF.EmitStaticVarDecl(D, toolchain::GlobalValue::InternalLinkage);
}

toolchain::Type *CGOpenCLRuntime::convertOpenCLSpecificType(const Type *T) {
  assert(T->isOpenCLSpecificType() && "Not an OpenCL specific type!");

  // Check if the target has a specific translation for this type first.
  if (toolchain::Type *TransTy = CGM.getTargetCodeGenInfo().getOpenCLType(CGM, T))
    return TransTy;

  if (T->isSamplerT())
    return getSamplerType(T);

  return getPointerType(T);
}

toolchain::PointerType *CGOpenCLRuntime::getPointerType(const Type *T) {
  uint32_t AddrSpc = CGM.getContext().getTargetAddressSpace(
      CGM.getContext().getOpenCLTypeAddrSpace(T));
  return toolchain::PointerType::get(CGM.getLLVMContext(), AddrSpc);
}

toolchain::Type *CGOpenCLRuntime::getPipeType(const PipeType *T) {
  if (toolchain::Type *PipeTy = CGM.getTargetCodeGenInfo().getOpenCLType(CGM, T))
    return PipeTy;

  if (T->isReadOnly())
    return getPipeType(T, "opencl.pipe_ro_t", PipeROTy);
  else
    return getPipeType(T, "opencl.pipe_wo_t", PipeWOTy);
}

toolchain::Type *CGOpenCLRuntime::getPipeType(const PipeType *T, StringRef Name,
                                         toolchain::Type *&PipeTy) {
  if (!PipeTy)
    PipeTy = getPointerType(T);
  return PipeTy;
}

toolchain::Type *CGOpenCLRuntime::getSamplerType(const Type *T) {
  if (SamplerTy)
    return SamplerTy;

  if (toolchain::Type *TransTy = CGM.getTargetCodeGenInfo().getOpenCLType(
          CGM, CGM.getContext().OCLSamplerTy.getTypePtr()))
    SamplerTy = TransTy;
  else
    SamplerTy = getPointerType(T);
  return SamplerTy;
}

toolchain::Value *CGOpenCLRuntime::getPipeElemSize(const Expr *PipeArg) {
  const PipeType *PipeTy = PipeArg->getType()->castAs<PipeType>();
  // The type of the last (implicit) argument to be passed.
  toolchain::Type *Int32Ty = toolchain::IntegerType::getInt32Ty(CGM.getLLVMContext());
  unsigned TypeSize = CGM.getContext()
                          .getTypeSizeInChars(PipeTy->getElementType())
                          .getQuantity();
  return toolchain::ConstantInt::get(Int32Ty, TypeSize, false);
}

toolchain::Value *CGOpenCLRuntime::getPipeElemAlign(const Expr *PipeArg) {
  const PipeType *PipeTy = PipeArg->getType()->castAs<PipeType>();
  // The type of the last (implicit) argument to be passed.
  toolchain::Type *Int32Ty = toolchain::IntegerType::getInt32Ty(CGM.getLLVMContext());
  unsigned TypeSize = CGM.getContext()
                          .getTypeAlignInChars(PipeTy->getElementType())
                          .getQuantity();
  return toolchain::ConstantInt::get(Int32Ty, TypeSize, false);
}

toolchain::PointerType *CGOpenCLRuntime::getGenericVoidPointerType() {
  assert(CGM.getLangOpts().OpenCL);
  return toolchain::PointerType::get(
      CGM.getLLVMContext(),
      CGM.getContext().getTargetAddressSpace(LangAS::opencl_generic));
}

// Get the block literal from an expression derived from the block expression.
// OpenCL v2.0 s6.12.5:
// Block variable declarations are implicitly qualified with const. Therefore
// all block variables must be initialized at declaration time and may not be
// reassigned.
static const BlockExpr *getBlockExpr(const Expr *E) {
  const Expr *Prev = nullptr; // to make sure we do not stuck in infinite loop.
  while(!isa<BlockExpr>(E) && E != Prev) {
    Prev = E;
    E = E->IgnoreCasts();
    if (auto DR = dyn_cast<DeclRefExpr>(E)) {
      E = cast<VarDecl>(DR->getDecl())->getInit();
    }
  }
  return cast<BlockExpr>(E);
}

/// Record emitted toolchain invoke function and toolchain block literal for the
/// corresponding block expression.
void CGOpenCLRuntime::recordBlockInfo(const BlockExpr *E,
                                      toolchain::Function *InvokeF,
                                      toolchain::Value *Block, toolchain::Type *BlockTy) {
  assert(!EnqueuedBlockMap.contains(E) && "Block expression emitted twice");
  assert(isa<toolchain::Function>(InvokeF) && "Invalid invoke function");
  assert(Block->getType()->isPointerTy() && "Invalid block literal type");
  EnqueuedBlockInfo &BlockInfo = EnqueuedBlockMap[E];
  BlockInfo.InvokeFunc = InvokeF;
  BlockInfo.BlockArg = Block;
  BlockInfo.BlockTy = BlockTy;
  BlockInfo.KernelHandle = nullptr;
}

toolchain::Function *CGOpenCLRuntime::getInvokeFunction(const Expr *E) {
  return EnqueuedBlockMap[getBlockExpr(E)].InvokeFunc;
}

CGOpenCLRuntime::EnqueuedBlockInfo
CGOpenCLRuntime::emitOpenCLEnqueuedBlock(CodeGenFunction &CGF, const Expr *E) {
  CGF.EmitScalarExpr(E);

  // The block literal may be assigned to a const variable. Chasing down
  // to get the block literal.
  const BlockExpr *Block = getBlockExpr(E);

  auto It = EnqueuedBlockMap.find(Block);
  assert(It != EnqueuedBlockMap.end() && "Block expression not emitted");
  EnqueuedBlockInfo &BlockInfo = It->second;

  // Do not emit the block wrapper again if it has been emitted.
  if (BlockInfo.KernelHandle) {
    return BlockInfo;
  }

  auto *F = CGF.getTargetHooks().createEnqueuedBlockKernel(
      CGF, BlockInfo.InvokeFunc, BlockInfo.BlockTy);

  // The common part of the post-processing of the kernel goes here.
  BlockInfo.KernelHandle = F;
  return BlockInfo;
}
