//===- EntryExitInstrumenter.cpp - Function Entry/Exit Instrumentation ----===//
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

#include "vm/core/Transforms/Utils/EntryExitInstrumenter.h"
#include "vm/core/Analysis/GlobalsModRef.h"
#include "vm/core/IR/DebugInfoMetadata.h"
#include "vm/core/IR/Dominators.h"
#include "vm/core/IR/Function.h"
#include "vm/core/IR/Instructions.h"
#include "vm/core/IR/Intrinsics.h"
#include "vm/core/IR/Module.h"
#include "vm/core/IR/Type.h"
#include "vm/core/InitializePasses.h"
#include "vm/core/TargetParser/Triple.h"
#include "vm/core/Pass.h"
#include "vm/core/Transforms/Utils.h"

using namespace vm::core;

static void insertCall(Function &CurFn, StringRef Func,
                       BasicBlock::iterator InsertionPt, DebugLoc DL) {
  Module &M = *InsertionPt->getParent()->getParent()->getParent();
  LLVMContext &C = InsertionPt->getParent()->getContext();

  if (Func == "mcount" ||
      Func == ".mcount" ||
      Func == "toolchain.arm.gnu.eabi.mcount" ||
      Func == "\01_mcount" ||
      Func == "\01mcount" ||
      Func == "__mcount" ||
      Func == "_mcount" ||
      Func == "__cyg_profile_func_enter_bare") {
    Triple TargetTriple(M.getTargetTriple());
    if (TargetTriple.isOSAIX() && Func == "__mcount") {
      Type *SizeTy = M.getDataLayout().getIntPtrType(C);
      Type *SizePtrTy = PointerType::getUnqual(C);
      GlobalVariable *GV = new GlobalVariable(M, SizeTy, /*isConstant=*/false,
                                              GlobalValue::InternalLinkage,
                                              ConstantInt::get(SizeTy, 0));
      CallInst *Call = CallInst::Create(
          M.getOrInsertFunction(Func,
                                FunctionType::get(Type::getVoidTy(C), {SizePtrTy},
                                                  /*isVarArg=*/false)),
          {GV}, "", InsertionPt);
      Call->setDebugLoc(DL);
    } else if (TargetTriple.isRISCV() || TargetTriple.isAArch64() ||
               TargetTriple.isLoongArch()) {
      // On RISC-V, AArch64, and LoongArch, the `_mcount` function takes
      // `__builtin_return_address(0)` as an argument since
      // `__builtin_return_address(1)` is not available on these platforms.
      Instruction *RetAddr = CallInst::Create(
          Intrinsic::getOrInsertDeclaration(&M, Intrinsic::returnaddress),
          ConstantInt::get(Type::getInt32Ty(C), 0), "", InsertionPt);
      RetAddr->setDebugLoc(DL);

      FunctionCallee Fn = M.getOrInsertFunction(
          Func, FunctionType::get(Type::getVoidTy(C), PointerType::getUnqual(C),
                                  false));
      CallInst *Call = CallInst::Create(Fn, RetAddr, "", InsertionPt);
      Call->setDebugLoc(DL);
    } else if (TargetTriple.isSystemZ()) {
      // skip insertion for `mcount` on SystemZ. This will be handled later in
      // `emitPrologue`. Add custom attribute to denote this.
      CurFn.addFnAttr(
          toolchain::Attribute::get(C, "systemz-instrument-function-entry", Func));
    } else {
      FunctionCallee Fn = M.getOrInsertFunction(Func, Type::getVoidTy(C));
      CallInst *Call = CallInst::Create(Fn, "", InsertionPt);
      Call->setDebugLoc(DL);
    }
    return;
  }

  if (Func == "__cyg_profile_func_enter" || Func == "__cyg_profile_func_exit") {
    Type *ArgTypes[] = {PointerType::getUnqual(C), PointerType::getUnqual(C)};

    FunctionCallee Fn = M.getOrInsertFunction(
        Func, FunctionType::get(Type::getVoidTy(C), ArgTypes, false));

    Instruction *RetAddr = CallInst::Create(
        Intrinsic::getOrInsertDeclaration(&M, Intrinsic::returnaddress),
        ArrayRef<Value *>(ConstantInt::get(Type::getInt32Ty(C), 0)), "",
        InsertionPt);
    RetAddr->setDebugLoc(DL);

    Value *Args[] = {&CurFn, RetAddr};
    CallInst *Call =
        CallInst::Create(Fn, ArrayRef<Value *>(Args), "", InsertionPt);
    Call->setDebugLoc(DL);
    return;
  }

  // We only know how to call a fixed set of instrumentation functions, because
  // they all expect different arguments, etc.
  report_fatal_error(Twine("Unknown instrumentation function: '") + Func + "'");
}

static bool runOnFunction(Function &F, bool PostInlining) {
  // The asm in a naked function may reasonably expect the argument registers
  // and the return address register (if present) to be live. An inserted
  // function call will clobber these registers. Simply skip naked functions for
  // all targets.
  if (F.hasFnAttribute(Attribute::Naked))
    return false;

  // available_externally functions may not have definitions external to the
  // module (e.g. gnu::always_inline). Instrumenting them might lead to linker
  // errors if they are optimized out. Skip them like GCC.
  if (F.hasAvailableExternallyLinkage())
    return false;

  StringRef EntryAttr = PostInlining ? "instrument-function-entry-inlined"
                                     : "instrument-function-entry";

  StringRef ExitAttr = PostInlining ? "instrument-function-exit-inlined"
                                    : "instrument-function-exit";

  StringRef EntryFunc = F.getFnAttribute(EntryAttr).getValueAsString();
  StringRef ExitFunc = F.getFnAttribute(ExitAttr).getValueAsString();

  bool Changed = false;

  // If the attribute is specified, insert instrumentation and then "consume"
  // the attribute so that it's not inserted again if the pass should happen to
  // run later for some reason.

  if (!EntryFunc.empty()) {
    DebugLoc DL;
    if (auto SP = F.getSubprogram())
      DL = DILocation::get(SP->getContext(), SP->getScopeLine(), 0, SP);

    insertCall(F, EntryFunc, F.begin()->getFirstInsertionPt(), DL);
    Changed = true;
    F.removeFnAttr(EntryAttr);
  }

  if (!ExitFunc.empty()) {
    for (BasicBlock &BB : F) {
      Instruction *T = BB.getTerminator();
      if (!isa<ReturnInst>(T))
        continue;

      // If T is preceded by a musttail call, that's the real terminator.
      if (CallInst *CI = BB.getTerminatingMustTailCall())
        T = CI;

      DebugLoc DL;
      if (DebugLoc TerminatorDL = T->getDebugLoc())
        DL = TerminatorDL;
      else if (auto SP = F.getSubprogram())
        DL = DILocation::get(SP->getContext(), 0, 0, SP);

      insertCall(F, ExitFunc, T->getIterator(), DL);
      Changed = true;
    }
    F.removeFnAttr(ExitAttr);
  }

  return Changed;
}

namespace {
struct PostInlineEntryExitInstrumenter : public FunctionPass {
  static char ID;
  PostInlineEntryExitInstrumenter() : FunctionPass(ID) {
    initializePostInlineEntryExitInstrumenterPass(
        *PassRegistry::getPassRegistry());
  }
  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addPreserved<GlobalsAAWrapperPass>();
    AU.addPreserved<DominatorTreeWrapperPass>();
  }
  bool runOnFunction(Function &F) override { return ::runOnFunction(F, true); }
};
char PostInlineEntryExitInstrumenter::ID = 0;
}

INITIALIZE_PASS_BEGIN(
    PostInlineEntryExitInstrumenter, "post-inline-ee-instrument",
    "Instrument function entry/exit with calls to e.g. mcount() "
    "(post inlining)",
    false, false)
INITIALIZE_PASS_DEPENDENCY(DominatorTreeWrapperPass)
INITIALIZE_PASS_END(
    PostInlineEntryExitInstrumenter, "post-inline-ee-instrument",
    "Instrument function entry/exit with calls to e.g. mcount() "
    "(post inlining)",
    false, false)

FunctionPass *toolchain::createPostInlineEntryExitInstrumenterPass() {
  return new PostInlineEntryExitInstrumenter();
}

PreservedAnalyses
toolchain::EntryExitInstrumenterPass::run(Function &F, FunctionAnalysisManager &AM) {
  if (!runOnFunction(F, PostInlining))
    return PreservedAnalyses::all();
  PreservedAnalyses PA;
  PA.preserveSet<CFGAnalyses>();
  return PA;
}

void toolchain::EntryExitInstrumenterPass::printPipeline(
    raw_ostream &OS, function_ref<StringRef(StringRef)> MapClassName2PassName) {
  static_cast<PassInfoMixin<toolchain::EntryExitInstrumenterPass> *>(this)
      ->printPipeline(OS, MapClassName2PassName);
  OS << '<';
  if (PostInlining)
    OS << "post-inline";
  OS << '>';
}
