//===- BoundsChecking.cpp - Instrumentation for run-time bounds checking --===//
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

#include "vm/core/Transforms/Instrumentation/BoundsChecking.h"
#include "vm/core/ADT/Statistic.h"
#include "vm/core/ADT/StringRef.h"
#include "vm/core/ADT/Twine.h"
#include "vm/core/Analysis/MemoryBuiltins.h"
#include "vm/core/Analysis/ScalarEvolution.h"
#include "vm/core/Analysis/TargetFolder.h"
#include "vm/core/Analysis/TargetLibraryInfo.h"
#include "vm/core/IR/BasicBlock.h"
#include "vm/core/IR/Constants.h"
#include "vm/core/IR/DataLayout.h"
#include "vm/core/IR/Function.h"
#include "vm/core/IR/IRBuilder.h"
#include "vm/core/IR/InstIterator.h"
#include "vm/core/IR/Instruction.h"
#include "vm/core/IR/Instructions.h"
#include "vm/core/IR/Intrinsics.h"
#include "vm/core/IR/Value.h"
#include "vm/core/Support/Casting.h"
#include "vm/core/Support/CommandLine.h"
#include "vm/core/Support/Debug.h"
#include "vm/core/Support/raw_ostream.h"
#include <utility>

using namespace vm::core;

#define DEBUG_TYPE "bounds-checking"

static cl::opt<bool> SingleTrapBB("bounds-checking-single-trap",
                                  cl::desc("Use one trap block per function"));

STATISTIC(ChecksAdded, "Bounds checks added");
STATISTIC(ChecksSkipped, "Bounds checks skipped");
STATISTIC(ChecksUnable, "Bounds checks unable to add");

class BuilderTy : public IRBuilder<TargetFolder> {
public:
  BuilderTy(BasicBlock *TheBB, BasicBlock::iterator IP, TargetFolder Folder)
      : IRBuilder<TargetFolder>(TheBB, IP, Folder) {
    SetNoSanitizeMetadata();
  }
};

/// Gets the conditions under which memory accessing instructions will overflow.
///
/// \p Ptr is the pointer that will be read/written, and \p InstVal is either
/// the result from the load or the value being stored. It is used to determine
/// the size of memory block that is touched.
///
/// Returns the condition under which the access will overflow.
static Value *getBoundsCheckCond(Value *Ptr, Value *InstVal,
                                 const DataLayout &DL, TargetLibraryInfo &TLI,
                                 ObjectSizeOffsetEvaluator &ObjSizeEval,
                                 BuilderTy &IRB, ScalarEvolution &SE) {
  TypeSize NeededSize = DL.getTypeStoreSize(InstVal->getType());
  LLVM_DEBUG(dbgs() << "Instrument " << *Ptr << " for " << Twine(NeededSize)
                    << " bytes\n");

  SizeOffsetValue SizeOffset = ObjSizeEval.compute(Ptr);

  if (!SizeOffset.bothKnown()) {
    ++ChecksUnable;
    return nullptr;
  }

  Value *Size = SizeOffset.Size;
  Value *Offset = SizeOffset.Offset;
  ConstantInt *SizeCI = dyn_cast<ConstantInt>(Size);

  Type *IndexTy = DL.getIndexType(Ptr->getType());
  Value *NeededSizeVal = IRB.CreateTypeSize(IndexTy, NeededSize);

  auto SizeRange = SE.getUnsignedRange(SE.getSCEV(Size));
  auto OffsetRange = SE.getUnsignedRange(SE.getSCEV(Offset));
  auto NeededSizeRange = SE.getUnsignedRange(SE.getSCEV(NeededSizeVal));

  // three checks are required to ensure safety:
  // . Offset >= 0  (since the offset is given from the base ptr)
  // . Size >= Offset  (unsigned)
  // . Size - Offset >= NeededSize  (unsigned)
  //
  // optimization: if Size >= 0 (signed), skip 1st check
  // FIXME: add NSW/NUW here?  -- we dont care if the subtraction overflows
  Value *ObjSize = IRB.CreateSub(Size, Offset);
  Value *Cmp2 = SizeRange.getUnsignedMin().uge(OffsetRange.getUnsignedMax())
                    ? ConstantInt::getFalse(Ptr->getContext())
                    : IRB.CreateICmpULT(Size, Offset);
  Value *Cmp3 = SizeRange.sub(OffsetRange)
                        .getUnsignedMin()
                        .uge(NeededSizeRange.getUnsignedMax())
                    ? ConstantInt::getFalse(Ptr->getContext())
                    : IRB.CreateICmpULT(ObjSize, NeededSizeVal);
  Value *Or = IRB.CreateOr(Cmp2, Cmp3);
  if ((!SizeCI || SizeCI->getValue().slt(0)) &&
      !SizeRange.getSignedMin().isNonNegative()) {
    Value *Cmp1 = IRB.CreateICmpSLT(Offset, ConstantInt::get(IndexTy, 0));
    Or = IRB.CreateOr(Cmp1, Or);
  }

  return Or;
}

static CallInst *InsertTrap(BuilderTy &IRB, bool DebugTrapBB,
                            std::optional<int8_t> GuardKind) {
  if (!DebugTrapBB)
    return IRB.CreateIntrinsic(Intrinsic::trap, {});

  uint64_t ImmArg = GuardKind.has_value()
                        ? GuardKind.value()
                        : IRB.GetInsertBlock()->getParent()->size();
  // Ensure we constrain ImmArg to fitting within a 8-but unsigned integer to
  // prevent overflow.
  if (ImmArg > 255)
    ImmArg = 255;

  return IRB.CreateIntrinsic(Intrinsic::ubsantrap,
                             ConstantInt::get(IRB.getInt8Ty(), ImmArg));
}

static CallInst *InsertCall(BuilderTy &IRB, bool MayReturn, StringRef Name) {
  Function *Fn = IRB.GetInsertBlock()->getParent();
  LLVMContext &Ctx = Fn->getContext();
  toolchain::AttrBuilder B(Ctx);
  B.addAttribute(toolchain::Attribute::NoUnwind);
  if (!MayReturn)
    B.addAttribute(toolchain::Attribute::NoReturn);
  FunctionCallee Callee = Fn->getParent()->getOrInsertFunction(
      Name,
      toolchain::AttributeList::get(Ctx, toolchain::AttributeList::FunctionIndex, B),
      Type::getVoidTy(Ctx));
  return IRB.CreateCall(Callee);
}

/// Adds run-time bounds checks to memory accessing instructions.
///
/// \p Or is the condition that should guard the trap.
///
/// \p GetTrapBB is a callable that returns the trap BB to use on failure.
template <typename GetTrapBBT>
static void insertBoundsCheck(Value *Or, BuilderTy &IRB, GetTrapBBT GetTrapBB) {
  // check if the comparison is always false
  ConstantInt *C = dyn_cast_or_null<ConstantInt>(Or);
  if (C) {
    ++ChecksSkipped;
    // If non-zero, nothing to do.
    if (!C->getZExtValue())
      return;
  }
  ++ChecksAdded;

  BasicBlock::iterator SplitI = IRB.GetInsertPoint();
  BasicBlock *OldBB = SplitI->getParent();
  BasicBlock *Cont = OldBB->splitBasicBlock(SplitI);
  OldBB->getTerminator()->eraseFromParent();

  BasicBlock *TrapBB = GetTrapBB(IRB, Cont);

  if (C) {
    // If we have a constant zero, unconditionally branch.
    // FIXME: We should really handle this differently to bypass the splitting
    // the block.
    BranchInst::Create(TrapBB, OldBB);
    return;
  }

  // Create the conditional branch.
  BranchInst::Create(TrapBB, Cont, Or, OldBB);
}

static std::string
getRuntimeCallName(const BoundsCheckingPass::Options::Runtime &Opts) {
  std::string Name = "__ubsan_handle_local_out_of_bounds";
  if (Opts.MinRuntime)
    Name += "_minimal";
  if (!Opts.MayReturn)
    Name += "_abort";
  else if (Opts.HandlerPreserveAllRegs)
    Name += "_preserve";
  return Name;
}

static bool addBoundsChecking(Function &F, TargetLibraryInfo &TLI,
                              ScalarEvolution &SE,
                              const BoundsCheckingPass::Options &Opts) {
  if (F.hasFnAttribute(Attribute::NoSanitizeBounds))
    return false;

  const DataLayout &DL = F.getDataLayout();
  ObjectSizeOpts EvalOpts;
  EvalOpts.RoundToAlign = true;
  EvalOpts.EvalMode = ObjectSizeOpts::Mode::ExactUnderlyingSizeAndOffset;
  ObjectSizeOffsetEvaluator ObjSizeEval(DL, &TLI, F.getContext(), EvalOpts);

  // check HANDLE_MEMORY_INST in include/toolchain/Instruction.def for memory
  // touching instructions
  SmallVector<std::pair<Instruction *, Value *>, 4> TrapInfo;
  for (Instruction &I : instructions(F)) {
    Value *Or = nullptr;
    BuilderTy IRB(I.getParent(), BasicBlock::iterator(&I), TargetFolder(DL));
    if (LoadInst *LI = dyn_cast<LoadInst>(&I)) {
      if (!LI->isVolatile())
        Or = getBoundsCheckCond(LI->getPointerOperand(), LI, DL, TLI,
                                ObjSizeEval, IRB, SE);
    } else if (StoreInst *SI = dyn_cast<StoreInst>(&I)) {
      if (!SI->isVolatile())
        Or = getBoundsCheckCond(SI->getPointerOperand(), SI->getValueOperand(),
                                DL, TLI, ObjSizeEval, IRB, SE);
    } else if (AtomicCmpXchgInst *AI = dyn_cast<AtomicCmpXchgInst>(&I)) {
      if (!AI->isVolatile())
        Or =
            getBoundsCheckCond(AI->getPointerOperand(), AI->getCompareOperand(),
                               DL, TLI, ObjSizeEval, IRB, SE);
    } else if (AtomicRMWInst *AI = dyn_cast<AtomicRMWInst>(&I)) {
      if (!AI->isVolatile())
        Or = getBoundsCheckCond(AI->getPointerOperand(), AI->getValOperand(),
                                DL, TLI, ObjSizeEval, IRB, SE);
    }
    if (Or) {
      if (Opts.GuardKind) {
        toolchain::Value *Allow = IRB.CreateIntrinsic(
            IRB.getInt1Ty(), Intrinsic::allow_ubsan_check,
            {toolchain::ConstantInt::getSigned(IRB.getInt8Ty(), *Opts.GuardKind)});
        Or = IRB.CreateAnd(Or, Allow);
      }
      TrapInfo.push_back(std::make_pair(&I, Or));
    }
  }

  std::string Name;
  if (Opts.Rt)
    Name = getRuntimeCallName(*Opts.Rt);

  // Create a trapping basic block on demand using a callback. Depending on
  // flags, this will either create a single block for the entire function or
  // will create a fresh block every time it is called.
  BasicBlock *ReuseTrapBB = nullptr;
  auto GetTrapBB = [&ReuseTrapBB, &Opts, &Name](BuilderTy &IRB,
                                                BasicBlock *Cont) {
    Function *Fn = IRB.GetInsertBlock()->getParent();
    auto DebugLoc = IRB.getCurrentDebugLocation();
    IRBuilder<>::InsertPointGuard Guard(IRB);

    // Create a trapping basic block on demand using a callback. Depending on
    // flags, this will either create a single block for the entire function or
    // will create a fresh block every time it is called.
    if (ReuseTrapBB)
      return ReuseTrapBB;

    BasicBlock *TrapBB = BasicBlock::Create(Fn->getContext(), "trap", Fn);
    IRB.SetInsertPoint(TrapBB);

    bool DebugTrapBB = !Opts.Merge;
    CallInst *TrapCall = Opts.Rt ? InsertCall(IRB, Opts.Rt->MayReturn, Name)
                                 : InsertTrap(IRB, DebugTrapBB, Opts.GuardKind);
    if (DebugTrapBB)
      TrapCall->addFnAttr(toolchain::Attribute::NoMerge);

    TrapCall->setDoesNotThrow();
    TrapCall->setDebugLoc(DebugLoc);

    bool MayReturn = Opts.Rt && Opts.Rt->MayReturn;
    if (MayReturn) {
      IRB.CreateBr(Cont);
    } else {
      TrapCall->setDoesNotReturn();
      IRB.CreateUnreachable();
    }
    // The preserve-all logic is somewhat duplicated in CGExpr.cpp for
    // local-bounds. Make sure to change that too.
    if (Opts.Rt && Opts.Rt->HandlerPreserveAllRegs && MayReturn)
      TrapCall->setCallingConv(CallingConv::PreserveAll);
    if (!MayReturn && SingleTrapBB && !DebugTrapBB)
      ReuseTrapBB = TrapBB;

    return TrapBB;
  };

  for (const auto &Entry : TrapInfo) {
    Instruction *Inst = Entry.first;
    BuilderTy IRB(Inst->getParent(), BasicBlock::iterator(Inst), TargetFolder(DL));
    insertBoundsCheck(Entry.second, IRB, GetTrapBB);
  }

  return !TrapInfo.empty();
}

PreservedAnalyses BoundsCheckingPass::run(Function &F, FunctionAnalysisManager &AM) {
  auto &TLI = AM.getResult<TargetLibraryAnalysis>(F);
  auto &SE = AM.getResult<ScalarEvolutionAnalysis>(F);

  if (!addBoundsChecking(F, TLI, SE, Opts))
    return PreservedAnalyses::all();

  return PreservedAnalyses::none();
}

void BoundsCheckingPass::printPipeline(
    raw_ostream &OS, function_ref<StringRef(StringRef)> MapClassName2PassName) {
  static_cast<PassInfoMixin<BoundsCheckingPass> *>(this)->printPipeline(
      OS, MapClassName2PassName);
  OS << "<";
  if (Opts.Rt) {
    if (Opts.Rt->MinRuntime)
      OS << "min-";
    OS << "rt";
    if (!Opts.Rt->MayReturn)
      OS << "-abort";
  } else {
    OS << "trap";
  }
  if (Opts.Merge)
    OS << ";merge";
  if (Opts.GuardKind)
    OS << ";guard=" << static_cast<int>(*Opts.GuardKind);
  OS << ">";
}
