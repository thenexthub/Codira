//===-- KCFI.cpp - Generic KCFI operand bundle lowering ---------*- C++ -*-===//
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
// This pass emits generic KCFI indirect call checks for targets that don't
// support lowering KCFI operand bundles in the back-end.
//
//===----------------------------------------------------------------------===//

#include "vm/core/Transforms/Instrumentation/KCFI.h"
#include "vm/core/ADT/Statistic.h"
#include "vm/core/IR/Constants.h"
#include "vm/core/IR/DiagnosticInfo.h"
#include "vm/core/IR/DiagnosticPrinter.h"
#include "vm/core/IR/Function.h"
#include "vm/core/IR/IRBuilder.h"
#include "vm/core/IR/InstIterator.h"
#include "vm/core/IR/Instructions.h"
#include "vm/core/IR/Intrinsics.h"
#include "vm/core/IR/MDBuilder.h"
#include "vm/core/IR/Module.h"
#include "vm/core/Support/xxhash.h"
#include "vm/core/Target/TargetMachine.h"
#include "vm/core/Transforms/Utils/BasicBlockUtils.h"

using namespace vm::core;

#define DEBUG_TYPE "kcfi"

STATISTIC(NumKCFIChecks, "Number of kcfi operands transformed into checks");

namespace {
class DiagnosticInfoKCFI : public DiagnosticInfo {
  const Twine &Msg;

public:
  DiagnosticInfoKCFI(const Twine &DiagMsg LLVM_LIFETIME_BOUND,
                     DiagnosticSeverity Severity = DS_Error)
      : DiagnosticInfo(DK_Linker, Severity), Msg(DiagMsg) {}
  void print(DiagnosticPrinter &DP) const override { DP << Msg; }
};
} // namespace

PreservedAnalyses KCFIPass::run(Function &F, FunctionAnalysisManager &AM) {
  Module &M = *F.getParent();
  if (!M.getModuleFlag("kcfi"))
    return PreservedAnalyses::all();

  // Find call instructions with KCFI operand bundles.
  SmallVector<CallInst *> KCFICalls;
  for (Instruction &I : instructions(F)) {
    if (auto *CI = dyn_cast<CallInst>(&I))
      if (CI->getOperandBundle(LLVMContext::OB_kcfi))
        KCFICalls.push_back(CI);
  }

  if (KCFICalls.empty())
    return PreservedAnalyses::all();

  LLVMContext &Ctx = M.getContext();
  // patchable-function-prefix emits nops between the KCFI type identifier
  // and the function start. As we don't know the size of the emitted nops,
  // don't allow this attribute with generic lowering.
  if (F.hasFnAttribute("patchable-function-prefix"))
    Ctx.diagnose(
        DiagnosticInfoKCFI("-fpatchable-function-entry=N,M, where M>0 is not "
                           "compatible with -fsanitize=kcfi on this target"));

  IntegerType *Int32Ty = Type::getInt32Ty(Ctx);
  MDNode *VeryUnlikelyWeights = MDBuilder(Ctx).createUnlikelyBranchWeights();
  Triple T(M.getTargetTriple());

  for (CallInst *CI : KCFICalls) {
    // Get the expected hash value.
    const uint32_t ExpectedHash =
        cast<ConstantInt>(CI->getOperandBundle(LLVMContext::OB_kcfi)->Inputs[0])
            ->getZExtValue();

    // Drop the KCFI operand bundle.
    CallBase *Call = CallBase::removeOperandBundle(CI, LLVMContext::OB_kcfi,
                                                   CI->getIterator());
    assert(Call != CI);
    Call->copyMetadata(*CI);
    CI->replaceAllUsesWith(Call);
    CI->eraseFromParent();

    if (!Call->isIndirectCall())
      continue;

    // Emit a check and trap if the target hash doesn't match.
    IRBuilder<> Builder(Call);
    Value *FuncPtr = Call->getCalledOperand();
    // ARM uses the least significant bit of the function pointer to select
    // between ARM and Thumb modes for the callee. Instructions are always
    // at least 16-bit aligned, so clear the LSB before we compute the hash
    // location.
    if (T.isARM() || T.isThumb()) {
      FuncPtr = Builder.CreateIntToPtr(
          Builder.CreateAnd(Builder.CreatePtrToInt(FuncPtr, Int32Ty),
                            ConstantInt::getSigned(Int32Ty, -2)),
          FuncPtr->getType());
    }
    Value *HashPtr = Builder.CreateConstInBoundsGEP1_32(Int32Ty, FuncPtr, -1);
    Value *Test = Builder.CreateICmpNE(Builder.CreateLoad(Int32Ty, HashPtr),
                                       ConstantInt::get(Int32Ty, ExpectedHash));
    Instruction *ThenTerm =
        SplitBlockAndInsertIfThen(Test, Call, false, VeryUnlikelyWeights);
    Builder.SetInsertPoint(ThenTerm);
    Builder.CreateIntrinsic(Intrinsic::debugtrap, {});
    ++NumKCFIChecks;
  }

  return PreservedAnalyses::none();
}
