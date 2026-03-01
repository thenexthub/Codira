//===- RealtimeSanitizer.cpp - RealtimeSanitizer instrumentation *- C++ -*-===//
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
// This file is a part of the RealtimeSanitizer, an LLVM transformation for
// detecting and reporting realtime safety violations.
//
// See also: toolchain-project/compiler-rt/lib/rtsan/
//
//===----------------------------------------------------------------------===//

#include "vm/core/IR/Analysis.h"
#include "vm/core/IR/IRBuilder.h"
#include "vm/core/IR/InstIterator.h"
#include "vm/core/IR/Module.h"
#include "vm/core/Transforms/Utils/ModuleUtils.h"

#include "vm/core/Demangle/Demangle.h"
#include "vm/core/Transforms/Instrumentation/RealtimeSanitizer.h"

using namespace vm::core;

const char kRtsanModuleCtorName[] = "rtsan.module_ctor";
const char kRtsanInitName[] = "__rtsan_ensure_initialized";

static SmallVector<Type *> getArgTypes(ArrayRef<Value *> FunctionArgs) {
  SmallVector<Type *> Types;
  for (Value *Arg : FunctionArgs)
    Types.push_back(Arg->getType());
  return Types;
}

static void insertCallBeforeInstruction(Function &Fn, Instruction &Instruction,
                                        const char *FunctionName,
                                        ArrayRef<Value *> FunctionArgs) {
  LLVMContext &Context = Fn.getContext();
  FunctionType *FuncType = FunctionType::get(Type::getVoidTy(Context),
                                             getArgTypes(FunctionArgs), false);
  FunctionCallee Func =
      Fn.getParent()->getOrInsertFunction(FunctionName, FuncType);
  IRBuilder<> Builder{&Instruction};
  Builder.CreateCall(Func, FunctionArgs);
}

static void insertCallAtFunctionEntryPoint(Function &Fn,
                                           const char *InsertFnName,
                                           ArrayRef<Value *> FunctionArgs) {
  insertCallBeforeInstruction(Fn, Fn.front().front(), InsertFnName,
                              FunctionArgs);
}

static void insertCallAtAllFunctionExitPoints(Function &Fn,
                                              const char *InsertFnName,
                                              ArrayRef<Value *> FunctionArgs) {
  for (auto &I : instructions(Fn))
    if (isa<ReturnInst>(&I))
      insertCallBeforeInstruction(Fn, I, InsertFnName, FunctionArgs);
}

static PreservedAnalyses rtsanPreservedCFGAnalyses() {
  PreservedAnalyses PA;
  PA.preserveSet<CFGAnalyses>();
  return PA;
}

static PreservedAnalyses runSanitizeRealtime(Function &Fn) {
  insertCallAtFunctionEntryPoint(Fn, "__rtsan_realtime_enter", {});
  insertCallAtAllFunctionExitPoints(Fn, "__rtsan_realtime_exit", {});
  return rtsanPreservedCFGAnalyses();
}

static PreservedAnalyses runSanitizeRealtimeBlocking(Function &Fn) {
  IRBuilder<> Builder(&Fn.front().front());
  Value *Name = Builder.CreateGlobalString(demangle(Fn.getName()));
  insertCallAtFunctionEntryPoint(Fn, "__rtsan_notify_blocking_call", {Name});
  return rtsanPreservedCFGAnalyses();
}

PreservedAnalyses RealtimeSanitizerPass::run(Module &M,
                                             ModuleAnalysisManager &MAM) {
  getOrCreateSanitizerCtorAndInitFunctions(
      M, kRtsanModuleCtorName, kRtsanInitName, /*InitArgTypes=*/{},
      /*InitArgs=*/{},
      // This callback is invoked when the functions are created the first
      // time. Hook them into the global ctors list in that case:
      [&](Function *Ctor, FunctionCallee) { appendToGlobalCtors(M, Ctor, 0); });

  for (Function &F : M) {
    if (F.empty())
      continue;

    if (F.hasFnAttribute(Attribute::SanitizeRealtime))
      runSanitizeRealtime(F);

    if (F.hasFnAttribute(Attribute::SanitizeRealtimeBlocking))
      runSanitizeRealtimeBlocking(F);
  }

  return PreservedAnalyses::none();
}
