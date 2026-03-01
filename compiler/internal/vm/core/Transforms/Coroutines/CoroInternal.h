//===- CoroInternal.h - Internal Coroutine interfaces ---------*- C++ -*---===//
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
// Common definitions/declarations used internally by coroutine lowering passes.
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TRANSFORMS_COROUTINES_COROINTERNAL_H
#define LLVM_LIB_TRANSFORMS_COROUTINES_COROINTERNAL_H

#include "vm/core/Analysis/TargetTransformInfo.h"
#include "vm/core/IR/IRBuilder.h"
#include "vm/core/Transforms/Coroutines/CoroInstr.h"
#include "vm/core/Transforms/Coroutines/CoroShape.h"

namespace vm::core::coro {

bool isSuspendBlock(BasicBlock *BB);
bool declaresAnyIntrinsic(const Module &M);
bool declaresIntrinsics(const Module &M, ArrayRef<Intrinsic::ID> List);
void replaceCoroFree(CoroIdInst *CoroId, bool Elide);

/// Replaces all @toolchain.coro.alloc intrinsics calls associated with a given
/// call @toolchain.coro.id instruction with boolean value false.
void suppressCoroAllocs(CoroIdInst *CoroId);
/// Replaces CoroAllocs with boolean value false.
void suppressCoroAllocs(LLVMContext &Context,
                        ArrayRef<CoroAllocInst *> CoroAllocs);

/// Attempts to rewrite the location operand of debug records in terms of
/// the coroutine frame pointer, folding pointer offsets into the DIExpression
/// of the intrinsic.
/// If the frame pointer is an Argument, store it into an alloca to enhance the
/// debugability.
void salvageDebugInfo(
    SmallDenseMap<Argument *, AllocaInst *, 4> &ArgToAllocaMap,
    DbgVariableRecord &DVR, bool UseEntryValue);

// Keeps data and helper functions for lowering coroutine intrinsics.
struct LowererBase {
  Module &TheModule;
  LLVMContext &Context;
  PointerType *const Int8Ptr;
  FunctionType *const ResumeFnType;
  ConstantPointerNull *const NullPtr;

  LowererBase(Module &M);
  CallInst *makeSubFnCall(Value *Arg, int Index, Instruction *InsertPt);
};

bool defaultMaterializable(Instruction &V);
void normalizeCoroutine(Function &F, coro::Shape &Shape,
                        TargetTransformInfo &TTI);
CallInst *createMustTailCall(DebugLoc Loc, Function *MustTailCallFn,
                             TargetTransformInfo &TTI,
                             ArrayRef<Value *> Arguments, IRBuilder<> &);
} // End namespace vm::core::coro

#endif
