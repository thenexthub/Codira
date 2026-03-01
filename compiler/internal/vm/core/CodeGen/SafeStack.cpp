//===- SafeStack.cpp - Safe Stack Insertion -------------------------------===//
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
// This pass splits the stack into the safe stack (kept as-is for LLVM backend)
// and the unsafe stack (explicitly allocated and managed through the runtime
// support library).
//
// http://clang.toolchain.org/docs/SafeStack.html
//
//===----------------------------------------------------------------------===//

#include "vm/core/CodeGen/SafeStack.h"
#include "SafeStackLayout.h"
#include "vm/core/ADT/APInt.h"
#include "vm/core/ADT/ArrayRef.h"
#include "vm/core/ADT/SmallPtrSet.h"
#include "vm/core/ADT/SmallVector.h"
#include "vm/core/ADT/Statistic.h"
#include "vm/core/Analysis/AssumptionCache.h"
#include "vm/core/Analysis/BranchProbabilityInfo.h"
#include "vm/core/Analysis/DomTreeUpdater.h"
#include "vm/core/Analysis/InlineCost.h"
#include "vm/core/Analysis/LoopInfo.h"
#include "vm/core/Analysis/ScalarEvolution.h"
#include "vm/core/Analysis/ScalarEvolutionExpressions.h"
#include "vm/core/Analysis/StackLifetime.h"
#include "vm/core/Analysis/TargetLibraryInfo.h"
#include "vm/core/CodeGen/TargetLowering.h"
#include "vm/core/CodeGen/TargetPassConfig.h"
#include "vm/core/CodeGen/TargetSubtargetInfo.h"
#include "vm/core/IR/Argument.h"
#include "vm/core/IR/Attributes.h"
#include "vm/core/IR/ConstantRange.h"
#include "vm/core/IR/Constants.h"
#include "vm/core/IR/DIBuilder.h"
#include "vm/core/IR/DataLayout.h"
#include "vm/core/IR/DerivedTypes.h"
#include "vm/core/IR/Dominators.h"
#include "vm/core/IR/Function.h"
#include "vm/core/IR/IRBuilder.h"
#include "vm/core/IR/InstIterator.h"
#include "vm/core/IR/Instruction.h"
#include "vm/core/IR/Instructions.h"
#include "vm/core/IR/IntrinsicInst.h"
#include "vm/core/IR/Intrinsics.h"
#include "vm/core/IR/MDBuilder.h"
#include "vm/core/IR/Metadata.h"
#include "vm/core/IR/Module.h"
#include "vm/core/IR/Type.h"
#include "vm/core/IR/Use.h"
#include "vm/core/IR/Value.h"
#include "vm/core/InitializePasses.h"
#include "vm/core/Pass.h"
#include "vm/core/Support/Casting.h"
#include "vm/core/Support/Debug.h"
#include "vm/core/Support/ErrorHandling.h"
#include "vm/core/Support/raw_ostream.h"
#include "vm/core/Target/TargetMachine.h"
#include "vm/core/Transforms/Utils/BasicBlockUtils.h"
#include "vm/core/Transforms/Utils/Cloning.h"
#include "vm/core/Transforms/Utils/Local.h"
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <optional>
#include <string>

using namespace vm::core;
using namespace vm::core::safestack;

#define DEBUG_TYPE "safe-stack"

STATISTIC(NumFunctions, "Total number of functions");
STATISTIC(NumUnsafeStackFunctions, "Number of functions with unsafe stack");
STATISTIC(NumUnsafeStackRestorePointsFunctions,
          "Number of functions that use setjmp or exceptions");

STATISTIC(NumAllocas, "Total number of allocas");
STATISTIC(NumUnsafeStaticAllocas, "Number of unsafe static allocas");
STATISTIC(NumUnsafeDynamicAllocas, "Number of unsafe dynamic allocas");
STATISTIC(NumUnsafeByValArguments, "Number of unsafe byval arguments");
STATISTIC(NumUnsafeStackRestorePoints, "Number of setjmps and landingpads");

/// Use __safestack_pointer_address even if the platform has a faster way of
/// access safe stack pointer.
static cl::opt<bool>
    SafeStackUsePointerAddress("safestack-use-pointer-address",
                                  cl::init(false), cl::Hidden);

static cl::opt<bool> ClColoring("safe-stack-coloring",
                                cl::desc("enable safe stack coloring"),
                                cl::Hidden, cl::init(true));

namespace {

/// The SafeStack pass splits the stack of each function into the safe
/// stack, which is only accessed through memory safe dereferences (as
/// determined statically), and the unsafe stack, which contains all
/// local variables that are accessed in ways that we can't prove to
/// be safe.
class SafeStack {
  Function &F;
  const TargetLoweringBase &TL;
  const DataLayout &DL;
  DomTreeUpdater *DTU;
  ScalarEvolution &SE;

  Type *StackPtrTy;
  Type *IntPtrTy;
  Type *Int32Ty;

  Value *UnsafeStackPtr = nullptr;

  /// Unsafe stack alignment. Each stack frame must ensure that the stack is
  /// aligned to this value. We need to re-align the unsafe stack if the
  /// alignment of any object on the stack exceeds this value.
  ///
  /// 16 seems like a reasonable upper bound on the alignment of objects that we
  /// might expect to appear on the stack on most common targets.
  static constexpr Align StackAlignment = Align::Constant<16>();

  /// Return the value of the stack canary.
  Value *getStackGuard(IRBuilder<> &IRB, Function &F);

  /// Load stack guard from the frame and check if it has changed.
  void checkStackGuard(IRBuilder<> &IRB, Function &F, Instruction &RI,
                       AllocaInst *StackGuardSlot, Value *StackGuard);

  /// Find all static allocas, dynamic allocas, return instructions and
  /// stack restore points (exception unwind blocks and setjmp calls) in the
  /// given function and append them to the respective vectors.
  void findInsts(Function &F, SmallVectorImpl<AllocaInst *> &StaticAllocas,
                 SmallVectorImpl<AllocaInst *> &DynamicAllocas,
                 SmallVectorImpl<Argument *> &ByValArguments,
                 SmallVectorImpl<Instruction *> &Returns,
                 SmallVectorImpl<Instruction *> &StackRestorePoints);

  /// Calculate the allocation size of a given alloca. Returns 0 if the
  /// size can not be statically determined.
  uint64_t getStaticAllocaAllocationSize(const AllocaInst* AI);

  /// Allocate space for all static allocas in \p StaticAllocas,
  /// replace allocas with pointers into the unsafe stack.
  ///
  /// \returns A pointer to the top of the unsafe stack after all unsafe static
  /// allocas are allocated.
  Value *moveStaticAllocasToUnsafeStack(IRBuilder<> &IRB, Function &F,
                                        ArrayRef<AllocaInst *> StaticAllocas,
                                        ArrayRef<Argument *> ByValArguments,
                                        Instruction *BasePointer,
                                        AllocaInst *StackGuardSlot);

  /// Generate code to restore the stack after all stack restore points
  /// in \p StackRestorePoints.
  ///
  /// \returns A local variable in which to maintain the dynamic top of the
  /// unsafe stack if needed.
  AllocaInst *
  createStackRestorePoints(IRBuilder<> &IRB, Function &F,
                           ArrayRef<Instruction *> StackRestorePoints,
                           Value *StaticTop, bool NeedDynamicTop);

  /// Replace all allocas in \p DynamicAllocas with code to allocate
  /// space dynamically on the unsafe stack and store the dynamic unsafe stack
  /// top to \p DynamicTop if non-null.
  void moveDynamicAllocasToUnsafeStack(Function &F, Value *UnsafeStackPtr,
                                       AllocaInst *DynamicTop,
                                       ArrayRef<AllocaInst *> DynamicAllocas);

  bool IsSafeStackAlloca(const Value *AllocaPtr, uint64_t AllocaSize);

  bool IsMemIntrinsicSafe(const MemIntrinsic *MI, const Use &U,
                          const Value *AllocaPtr, uint64_t AllocaSize);
  bool IsAccessSafe(Value *Addr, uint64_t Size, const Value *AllocaPtr,
                    uint64_t AllocaSize);

  bool ShouldInlinePointerAddress(CallInst &CI);
  void TryInlinePointerAddress();

public:
  SafeStack(Function &F, const TargetLoweringBase &TL, const DataLayout &DL,
            DomTreeUpdater *DTU, ScalarEvolution &SE)
      : F(F), TL(TL), DL(DL), DTU(DTU), SE(SE),
        StackPtrTy(DL.getAllocaPtrType(F.getContext())),
        IntPtrTy(DL.getIntPtrType(F.getContext())),
        Int32Ty(Type::getInt32Ty(F.getContext())) {}

  // Run the transformation on the associated function.
  // Returns whether the function was changed.
  bool run();
};

uint64_t SafeStack::getStaticAllocaAllocationSize(const AllocaInst* AI) {
  uint64_t Size = DL.getTypeAllocSize(AI->getAllocatedType());
  if (AI->isArrayAllocation()) {
    auto C = dyn_cast<ConstantInt>(AI->getArraySize());
    if (!C)
      return 0;
    Size *= C->getZExtValue();
  }
  return Size;
}

bool SafeStack::IsAccessSafe(Value *Addr, uint64_t AccessSize,
                             const Value *AllocaPtr, uint64_t AllocaSize) {
  const SCEV *AddrExpr = SE.getSCEV(Addr);
  const auto *Base = dyn_cast<SCEVUnknown>(SE.getPointerBase(AddrExpr));
  if (!Base || Base->getValue() != AllocaPtr) {
    LLVM_DEBUG(
        dbgs() << "[SafeStack] "
               << (isa<AllocaInst>(AllocaPtr) ? "Alloca " : "ByValArgument ")
               << *AllocaPtr << "\n"
               << "SCEV " << *AddrExpr << " not directly based on alloca\n");
    return false;
  }

  const SCEV *Expr = SE.removePointerBase(AddrExpr);
  uint64_t BitWidth = SE.getTypeSizeInBits(Expr->getType());
  ConstantRange AccessStartRange = SE.getUnsignedRange(Expr);
  ConstantRange SizeRange =
      ConstantRange(APInt(BitWidth, 0), APInt(BitWidth, AccessSize));
  ConstantRange AccessRange = AccessStartRange.add(SizeRange);
  ConstantRange AllocaRange =
      ConstantRange(APInt(BitWidth, 0), APInt(BitWidth, AllocaSize));
  bool Safe = AllocaRange.contains(AccessRange);

  LLVM_DEBUG(
      dbgs() << "[SafeStack] "
             << (isa<AllocaInst>(AllocaPtr) ? "Alloca " : "ByValArgument ")
             << *AllocaPtr << "\n"
             << "            Access " << *Addr << "\n"
             << "            SCEV " << *Expr
             << " U: " << SE.getUnsignedRange(Expr)
             << ", S: " << SE.getSignedRange(Expr) << "\n"
             << "            Range " << AccessRange << "\n"
             << "            AllocaRange " << AllocaRange << "\n"
             << "            " << (Safe ? "safe" : "unsafe") << "\n");

  return Safe;
}

bool SafeStack::IsMemIntrinsicSafe(const MemIntrinsic *MI, const Use &U,
                                   const Value *AllocaPtr,
                                   uint64_t AllocaSize) {
  if (auto MTI = dyn_cast<MemTransferInst>(MI)) {
    if (MTI->getRawSource() != U && MTI->getRawDest() != U)
      return true;
  } else {
    if (MI->getRawDest() != U)
      return true;
  }

  auto Len = MI->getLengthInBytes();
  // Non-constant size => unsafe. FIXME: try SCEV getRange.
  if (!Len) return false;
  return IsAccessSafe(U, Len->getZExtValue(), AllocaPtr, AllocaSize);
}

/// Check whether a given allocation must be put on the safe
/// stack or not. The function analyzes all uses of AI and checks whether it is
/// only accessed in a memory safe way (as decided statically).
bool SafeStack::IsSafeStackAlloca(const Value *AllocaPtr, uint64_t AllocaSize) {
  // Go through all uses of this alloca and check whether all accesses to the
  // allocated object are statically known to be memory safe and, hence, the
  // object can be placed on the safe stack.
  SmallPtrSet<const Value *, 16> Visited;
  SmallVector<const Value *, 8> WorkList;
  WorkList.push_back(AllocaPtr);

  // A DFS search through all uses of the alloca in bitcasts/PHI/GEPs/etc.
  while (!WorkList.empty()) {
    const Value *V = WorkList.pop_back_val();
    for (const Use &UI : V->uses()) {
      auto I = cast<const Instruction>(UI.getUser());
      assert(V == UI.get());

      switch (I->getOpcode()) {
      case Instruction::Load:
        if (!IsAccessSafe(UI, DL.getTypeStoreSize(I->getType()), AllocaPtr,
                          AllocaSize))
          return false;
        break;

      case Instruction::VAArg:
        // "va-arg" from a pointer is safe.
        break;
      case Instruction::Store:
        if (V == I->getOperand(0)) {
          // Stored the pointer - conservatively assume it may be unsafe.
          LLVM_DEBUG(dbgs()
                     << "[SafeStack] Unsafe alloca: " << *AllocaPtr
                     << "\n            store of address: " << *I << "\n");
          return false;
        }

        if (!IsAccessSafe(UI, DL.getTypeStoreSize(I->getOperand(0)->getType()),
                          AllocaPtr, AllocaSize))
          return false;
        break;

      case Instruction::Ret:
        // Information leak.
        return false;

      case Instruction::Call:
      case Instruction::Invoke: {
        const CallBase &CS = *cast<CallBase>(I);

        if (I->isLifetimeStartOrEnd())
          continue;

        if (const MemIntrinsic *MI = dyn_cast<MemIntrinsic>(I)) {
          if (!IsMemIntrinsicSafe(MI, UI, AllocaPtr, AllocaSize)) {
            LLVM_DEBUG(dbgs()
                       << "[SafeStack] Unsafe alloca: " << *AllocaPtr
                       << "\n            unsafe memintrinsic: " << *I << "\n");
            return false;
          }
          continue;
        }

        // LLVM 'nocapture' attribute is only set for arguments whose address
        // is not stored, passed around, or used in any other non-trivial way.
        // We assume that passing a pointer to an object as a 'nocapture
        // readnone' argument is safe.
        // FIXME: a more precise solution would require an interprocedural
        // analysis here, which would look at all uses of an argument inside
        // the function being called.
        auto B = CS.arg_begin(), E = CS.arg_end();
        for (const auto *A = B; A != E; ++A)
          if (A->get() == V)
            if (!(CS.doesNotCapture(A - B) && (CS.doesNotAccessMemory(A - B) ||
                                               CS.doesNotAccessMemory()))) {
              LLVM_DEBUG(dbgs() << "[SafeStack] Unsafe alloca: " << *AllocaPtr
                                << "\n            unsafe call: " << *I << "\n");
              return false;
            }
        continue;
      }

      default:
        if (Visited.insert(I).second)
          WorkList.push_back(cast<const Instruction>(I));
      }
    }
  }

  // All uses of the alloca are safe, we can place it on the safe stack.
  return true;
}

Value *SafeStack::getStackGuard(IRBuilder<> &IRB, Function &F) {
  Value *StackGuardVar = TL.getIRStackGuard(IRB);
  Module *M = F.getParent();

  if (!StackGuardVar) {
    TL.insertSSPDeclarations(*M);
    return IRB.CreateIntrinsic(Intrinsic::stackguard, {});
  }

  return IRB.CreateLoad(StackPtrTy, StackGuardVar, "StackGuard");
}

void SafeStack::findInsts(Function &F,
                          SmallVectorImpl<AllocaInst *> &StaticAllocas,
                          SmallVectorImpl<AllocaInst *> &DynamicAllocas,
                          SmallVectorImpl<Argument *> &ByValArguments,
                          SmallVectorImpl<Instruction *> &Returns,
                          SmallVectorImpl<Instruction *> &StackRestorePoints) {
  for (Instruction &I : instructions(&F)) {
    if (auto AI = dyn_cast<AllocaInst>(&I)) {
      ++NumAllocas;

      uint64_t Size = getStaticAllocaAllocationSize(AI);
      if (IsSafeStackAlloca(AI, Size))
        continue;

      if (AI->isStaticAlloca()) {
        ++NumUnsafeStaticAllocas;
        StaticAllocas.push_back(AI);
      } else {
        ++NumUnsafeDynamicAllocas;
        DynamicAllocas.push_back(AI);
      }
    } else if (auto RI = dyn_cast<ReturnInst>(&I)) {
      if (CallInst *CI = I.getParent()->getTerminatingMustTailCall())
        Returns.push_back(CI);
      else
        Returns.push_back(RI);
    } else if (auto CI = dyn_cast<CallInst>(&I)) {
      // setjmps require stack restore.
      if (CI->getCalledFunction() && CI->canReturnTwice())
        StackRestorePoints.push_back(CI);
    } else if (auto LP = dyn_cast<LandingPadInst>(&I)) {
      // Exception landing pads require stack restore.
      StackRestorePoints.push_back(LP);
    } else if (auto II = dyn_cast<IntrinsicInst>(&I)) {
      if (II->getIntrinsicID() == Intrinsic::gcroot)
        report_fatal_error(
            "gcroot intrinsic not compatible with safestack attribute");
    }
  }
  for (Argument &Arg : F.args()) {
    if (!Arg.hasByValAttr())
      continue;
    uint64_t Size = DL.getTypeStoreSize(Arg.getParamByValType());
    if (IsSafeStackAlloca(&Arg, Size))
      continue;

    ++NumUnsafeByValArguments;
    ByValArguments.push_back(&Arg);
  }
}

AllocaInst *
SafeStack::createStackRestorePoints(IRBuilder<> &IRB, Function &F,
                                    ArrayRef<Instruction *> StackRestorePoints,
                                    Value *StaticTop, bool NeedDynamicTop) {
  assert(StaticTop && "The stack top isn't set.");

  if (StackRestorePoints.empty())
    return nullptr;

  // We need the current value of the shadow stack pointer to restore
  // after longjmp or exception catching.

  // FIXME: On some platforms this could be handled by the longjmp/exception
  // runtime itself.

  AllocaInst *DynamicTop = nullptr;
  if (NeedDynamicTop) {
    // If we also have dynamic alloca's, the stack pointer value changes
    // throughout the function. For now we store it in an alloca.
    DynamicTop = IRB.CreateAlloca(StackPtrTy, /*ArraySize=*/nullptr,
                                  "unsafe_stack_dynamic_ptr");
    IRB.CreateStore(StaticTop, DynamicTop);
  }

  // Restore current stack pointer after longjmp/exception catch.
  for (Instruction *I : StackRestorePoints) {
    ++NumUnsafeStackRestorePoints;

    IRB.SetInsertPoint(I->getNextNode());
    Value *CurrentTop =
        DynamicTop ? IRB.CreateLoad(StackPtrTy, DynamicTop) : StaticTop;
    IRB.CreateStore(CurrentTop, UnsafeStackPtr);
  }

  return DynamicTop;
}

void SafeStack::checkStackGuard(IRBuilder<> &IRB, Function &F, Instruction &RI,
                                AllocaInst *StackGuardSlot, Value *StackGuard) {
  Value *V = IRB.CreateLoad(StackPtrTy, StackGuardSlot);
  Value *Cmp = IRB.CreateICmpNE(StackGuard, V);

  auto SuccessProb = BranchProbabilityInfo::getBranchProbStackProtector(true);
  auto FailureProb = BranchProbabilityInfo::getBranchProbStackProtector(false);
  MDNode *Weights = MDBuilder(F.getContext())
                        .createBranchWeights(SuccessProb.getNumerator(),
                                             FailureProb.getNumerator());
  Instruction *CheckTerm =
      SplitBlockAndInsertIfThen(Cmp, &RI, /* Unreachable */ true, Weights, DTU);
  IRBuilder<> IRBFail(CheckTerm);
  // FIXME: respect -fsanitize-trap / -ftrap-function here?
  const char *StackChkFailName =
      TL.getLibcallName(RTLIB::STACKPROTECTOR_CHECK_FAIL);
  if (!StackChkFailName) {
    F.getContext().emitError(
        "no libcall available for stackprotector check fail");
    return;
  }

  FunctionCallee StackChkFail =
      F.getParent()->getOrInsertFunction(StackChkFailName, IRB.getVoidTy());
  IRBFail.CreateCall(StackChkFail, {});
}

/// We explicitly compute and set the unsafe stack layout for all unsafe
/// static alloca instructions. We save the unsafe "base pointer" in the
/// prologue into a local variable and restore it in the epilogue.
Value *SafeStack::moveStaticAllocasToUnsafeStack(
    IRBuilder<> &IRB, Function &F, ArrayRef<AllocaInst *> StaticAllocas,
    ArrayRef<Argument *> ByValArguments, Instruction *BasePointer,
    AllocaInst *StackGuardSlot) {
  if (StaticAllocas.empty() && ByValArguments.empty())
    return BasePointer;

  DIBuilder DIB(*F.getParent());

  StackLifetime SSC(F, StaticAllocas, StackLifetime::LivenessType::May);
  static const StackLifetime::LiveRange NoColoringRange(1, true);
  if (ClColoring)
    SSC.run();

  for (const auto *I : SSC.getMarkers()) {
    auto *Op = dyn_cast<Instruction>(I->getOperand(1));
    const_cast<IntrinsicInst *>(I)->eraseFromParent();
    // Remove the operand bitcast, too, if it has no more uses left.
    if (Op && Op->use_empty())
      Op->eraseFromParent();
  }

  // Unsafe stack always grows down.
  StackLayout SSL(StackAlignment);
  if (StackGuardSlot) {
    Type *Ty = StackGuardSlot->getAllocatedType();
    Align Align = std::max(DL.getPrefTypeAlign(Ty), StackGuardSlot->getAlign());
    SSL.addObject(StackGuardSlot, getStaticAllocaAllocationSize(StackGuardSlot),
                  Align, SSC.getFullLiveRange());
  }

  for (Argument *Arg : ByValArguments) {
    Type *Ty = Arg->getParamByValType();
    uint64_t Size = DL.getTypeStoreSize(Ty);
    if (Size == 0)
      Size = 1; // Don't create zero-sized stack objects.

    // Ensure the object is properly aligned.
    Align Align = DL.getPrefTypeAlign(Ty);
    if (auto A = Arg->getParamAlign())
      Align = std::max(Align, *A);
    SSL.addObject(Arg, Size, Align, SSC.getFullLiveRange());
  }

  for (AllocaInst *AI : StaticAllocas) {
    Type *Ty = AI->getAllocatedType();
    uint64_t Size = getStaticAllocaAllocationSize(AI);
    if (Size == 0)
      Size = 1; // Don't create zero-sized stack objects.

    // Ensure the object is properly aligned.
    Align Align = std::max(DL.getPrefTypeAlign(Ty), AI->getAlign());

    SSL.addObject(AI, Size, Align,
                  ClColoring ? SSC.getLiveRange(AI) : NoColoringRange);
  }

  SSL.computeLayout();
  Align FrameAlignment = SSL.getFrameAlignment();

  // FIXME: tell SSL that we start at a less-then-MaxAlignment aligned location
  // (AlignmentSkew).
  if (FrameAlignment > StackAlignment) {
    // Re-align the base pointer according to the max requested alignment.
    IRB.SetInsertPoint(BasePointer->getNextNode());
    BasePointer = cast<Instruction>(IRB.CreateIntToPtr(
        IRB.CreateAnd(
            IRB.CreatePtrToInt(BasePointer, IntPtrTy),
            ConstantInt::get(IntPtrTy, ~(FrameAlignment.value() - 1))),
        StackPtrTy));
  }

  IRB.SetInsertPoint(BasePointer->getNextNode());

  if (StackGuardSlot) {
    unsigned Offset = SSL.getObjectOffset(StackGuardSlot);
    Value *Off =
        IRB.CreatePtrAdd(BasePointer, ConstantInt::get(Int32Ty, -Offset));
    Value *NewAI =
        IRB.CreateBitCast(Off, StackGuardSlot->getType(), "StackGuardSlot");

    // Replace alloc with the new location.
    StackGuardSlot->replaceAllUsesWith(NewAI);
    StackGuardSlot->eraseFromParent();
  }

  for (Argument *Arg : ByValArguments) {
    unsigned Offset = SSL.getObjectOffset(Arg);
    MaybeAlign Align(SSL.getObjectAlignment(Arg));
    Type *Ty = Arg->getParamByValType();

    uint64_t Size = DL.getTypeStoreSize(Ty);
    if (Size == 0)
      Size = 1; // Don't create zero-sized stack objects.

    Value *Off =
        IRB.CreatePtrAdd(BasePointer, ConstantInt::get(Int32Ty, -Offset));
    Value *NewArg = IRB.CreateBitCast(Off, Arg->getType(),
                                      Arg->getName() + ".unsafe-byval");

    // Replace alloc with the new location.
    replaceDbgDeclare(Arg, BasePointer, DIB, DIExpression::ApplyOffset,
                      -Offset);
    Arg->replaceAllUsesWith(NewArg);
    IRB.SetInsertPoint(cast<Instruction>(NewArg)->getNextNode());
    IRB.CreateMemCpy(Off, Align, Arg, Arg->getParamAlign(), Size);
  }

  // Allocate space for every unsafe static AllocaInst on the unsafe stack.
  for (AllocaInst *AI : StaticAllocas) {
    IRB.SetInsertPoint(AI);
    unsigned Offset = SSL.getObjectOffset(AI);

    replaceDbgDeclare(AI, BasePointer, DIB, DIExpression::ApplyOffset, -Offset);
    replaceDbgValueForAlloca(AI, BasePointer, DIB, -Offset);

    // Replace uses of the alloca with the new location.
    // Insert address calculation close to each use to work around PR27844.
    std::string Name = std::string(AI->getName()) + ".unsafe";
    while (!AI->use_empty()) {
      Use &U = *AI->use_begin();
      Instruction *User = cast<Instruction>(U.getUser());

      // Drop lifetime markers now that this is no longer an alloca.
      // SafeStack has already performed its own stack coloring.
      if (User->isLifetimeStartOrEnd()) {
        User->eraseFromParent();
        continue;
      }

      Instruction *InsertBefore;
      if (auto *PHI = dyn_cast<PHINode>(User))
        InsertBefore = PHI->getIncomingBlock(U)->getTerminator();
      else
        InsertBefore = User;

      IRBuilder<> IRBUser(InsertBefore);
      Value *Off =
          IRBUser.CreatePtrAdd(BasePointer, ConstantInt::get(Int32Ty, -Offset));
      Value *Replacement =
          IRBUser.CreateAddrSpaceCast(Off, AI->getType(), Name);

      if (auto *PHI = dyn_cast<PHINode>(User))
        // PHI nodes may have multiple incoming edges from the same BB (why??),
        // all must be updated at once with the same incoming value.
        PHI->setIncomingValueForBlock(PHI->getIncomingBlock(U), Replacement);
      else
        U.set(Replacement);
    }

    AI->eraseFromParent();
  }

  // Re-align BasePointer so that our callees would see it aligned as
  // expected.
  // FIXME: no need to update BasePointer in leaf functions.
  unsigned FrameSize = alignTo(SSL.getFrameSize(), StackAlignment);

  MDBuilder MDB(F.getContext());
  SmallVector<Metadata *, 2> Data;
  Data.push_back(MDB.createString("unsafe-stack-size"));
  Data.push_back(MDB.createConstant(ConstantInt::get(Int32Ty, FrameSize)));
  MDNode *MD = MDTuple::get(F.getContext(), Data);
  F.setMetadata(LLVMContext::MD_annotation, MD);

  // Update shadow stack pointer in the function epilogue.
  IRB.SetInsertPoint(BasePointer->getNextNode());

  Value *StaticTop =
      IRB.CreatePtrAdd(BasePointer, ConstantInt::get(Int32Ty, -FrameSize),
                       "unsafe_stack_static_top");
  IRB.CreateStore(StaticTop, UnsafeStackPtr);
  return StaticTop;
}

void SafeStack::moveDynamicAllocasToUnsafeStack(
    Function &F, Value *UnsafeStackPtr, AllocaInst *DynamicTop,
    ArrayRef<AllocaInst *> DynamicAllocas) {
  DIBuilder DIB(*F.getParent());

  for (AllocaInst *AI : DynamicAllocas) {
    IRBuilder<> IRB(AI);

    // Compute the new SP value (after AI).
    Value *ArraySize = AI->getArraySize();
    if (ArraySize->getType() != IntPtrTy)
      ArraySize = IRB.CreateIntCast(ArraySize, IntPtrTy, false);

    Type *Ty = AI->getAllocatedType();
    uint64_t TySize = DL.getTypeAllocSize(Ty);
    Value *Size = IRB.CreateMul(ArraySize, ConstantInt::get(IntPtrTy, TySize));

    Value *SP = IRB.CreatePtrToInt(IRB.CreateLoad(StackPtrTy, UnsafeStackPtr),
                                   IntPtrTy);
    SP = IRB.CreateSub(SP, Size);

    // Align the SP value to satisfy the AllocaInst, type and stack alignments.
    auto Align = std::max(std::max(DL.getPrefTypeAlign(Ty), AI->getAlign()),
                          StackAlignment);

    Value *NewTop = IRB.CreateIntToPtr(
        IRB.CreateAnd(
            SP, ConstantInt::getSigned(IntPtrTy, ~uint64_t(Align.value() - 1))),
        StackPtrTy);

    // Save the stack pointer.
    IRB.CreateStore(NewTop, UnsafeStackPtr);
    if (DynamicTop)
      IRB.CreateStore(NewTop, DynamicTop);

    Value *NewAI = IRB.CreatePointerCast(NewTop, AI->getType());
    if (AI->hasName() && isa<Instruction>(NewAI))
      NewAI->takeName(AI);

    replaceDbgDeclare(AI, NewAI, DIB, DIExpression::ApplyOffset, 0);
    AI->replaceAllUsesWith(NewAI);
    AI->eraseFromParent();
  }

  if (!DynamicAllocas.empty()) {
    // Now go through the instructions again, replacing stacksave/stackrestore.
    for (Instruction &I : toolchain::make_early_inc_range(instructions(&F))) {
      auto *II = dyn_cast<IntrinsicInst>(&I);
      if (!II)
        continue;

      if (II->getIntrinsicID() == Intrinsic::stacksave) {
        IRBuilder<> IRB(II);
        Instruction *LI = IRB.CreateLoad(StackPtrTy, UnsafeStackPtr);
        LI->takeName(II);
        II->replaceAllUsesWith(LI);
        II->eraseFromParent();
      } else if (II->getIntrinsicID() == Intrinsic::stackrestore) {
        IRBuilder<> IRB(II);
        Instruction *SI = IRB.CreateStore(II->getArgOperand(0), UnsafeStackPtr);
        SI->takeName(II);
        assert(II->use_empty());
        II->eraseFromParent();
      }
    }
  }
}

bool SafeStack::ShouldInlinePointerAddress(CallInst &CI) {
  Function *Callee = CI.getCalledFunction();
  if (CI.hasFnAttr(Attribute::AlwaysInline) &&
      isInlineViable(*Callee).isSuccess())
    return true;
  if (Callee->isInterposable() || Callee->hasFnAttribute(Attribute::NoInline) ||
      CI.isNoInline())
    return false;
  return true;
}

void SafeStack::TryInlinePointerAddress() {
  auto *CI = dyn_cast<CallInst>(UnsafeStackPtr);
  if (!CI)
    return;

  if(F.hasOptNone())
    return;

  Function *Callee = CI->getCalledFunction();
  if (!Callee || Callee->isDeclaration())
    return;

  if (!ShouldInlinePointerAddress(*CI))
    return;

  InlineFunctionInfo IFI;
  InlineFunction(*CI, IFI);
}

bool SafeStack::run() {
  assert(F.hasFnAttribute(Attribute::SafeStack) &&
         "Can't run SafeStack on a function without the attribute");
  assert(!F.isDeclaration() && "Can't run SafeStack on a function declaration");

  ++NumFunctions;

  SmallVector<AllocaInst *, 16> StaticAllocas;
  SmallVector<AllocaInst *, 4> DynamicAllocas;
  SmallVector<Argument *, 4> ByValArguments;
  SmallVector<Instruction *, 4> Returns;

  // Collect all points where stack gets unwound and needs to be restored
  // This is only necessary because the runtime (setjmp and unwind code) is
  // not aware of the unsafe stack and won't unwind/restore it properly.
  // To work around this problem without changing the runtime, we insert
  // instrumentation to restore the unsafe stack pointer when necessary.
  SmallVector<Instruction *, 4> StackRestorePoints;

  // Find all static and dynamic alloca instructions that must be moved to the
  // unsafe stack, all return instructions and stack restore points.
  findInsts(F, StaticAllocas, DynamicAllocas, ByValArguments, Returns,
            StackRestorePoints);

  if (StaticAllocas.empty() && DynamicAllocas.empty() &&
      ByValArguments.empty() && StackRestorePoints.empty())
    return false; // Nothing to do in this function.

  if (!StaticAllocas.empty() || !DynamicAllocas.empty() ||
      !ByValArguments.empty())
    ++NumUnsafeStackFunctions; // This function has the unsafe stack.

  if (!StackRestorePoints.empty())
    ++NumUnsafeStackRestorePointsFunctions;

  IRBuilder<> IRB(&F.front(), F.begin()->getFirstInsertionPt());
  // Calls must always have a debug location, or else inlining breaks. So
  // we explicitly set a artificial debug location here.
  if (DISubprogram *SP = F.getSubprogram())
    IRB.SetCurrentDebugLocation(
        DILocation::get(SP->getContext(), SP->getScopeLine(), 0, SP));
  if (SafeStackUsePointerAddress) {
    const char *SafestackPointerAddressName =
        TL.getLibcallName(RTLIB::SAFESTACK_POINTER_ADDRESS);
    if (!SafestackPointerAddressName) {
      F.getContext().emitError(
          "no libcall available for safestack pointer address");
      return false;
    }

    FunctionCallee Fn = F.getParent()->getOrInsertFunction(
        SafestackPointerAddressName, IRB.getPtrTy(0));
    UnsafeStackPtr = IRB.CreateCall(Fn);
  } else {
    UnsafeStackPtr = TL.getSafeStackPointerLocation(IRB);
  }

  // Load the current stack pointer (we'll also use it as a base pointer).
  // FIXME: use a dedicated register for it ?
  Instruction *BasePointer =
      IRB.CreateLoad(StackPtrTy, UnsafeStackPtr, false, "unsafe_stack_ptr");
  assert(BasePointer->getType() == StackPtrTy);

  AllocaInst *StackGuardSlot = nullptr;
  // FIXME: implement weaker forms of stack protector.
  if (F.hasFnAttribute(Attribute::StackProtect) ||
      F.hasFnAttribute(Attribute::StackProtectStrong) ||
      F.hasFnAttribute(Attribute::StackProtectReq)) {
    Value *StackGuard = getStackGuard(IRB, F);
    StackGuardSlot = IRB.CreateAlloca(StackPtrTy, nullptr);
    IRB.CreateStore(StackGuard, StackGuardSlot);

    for (Instruction *RI : Returns) {
      IRBuilder<> IRBRet(RI);
      checkStackGuard(IRBRet, F, *RI, StackGuardSlot, StackGuard);
    }
  }

  // The top of the unsafe stack after all unsafe static allocas are
  // allocated.
  Value *StaticTop = moveStaticAllocasToUnsafeStack(
      IRB, F, StaticAllocas, ByValArguments, BasePointer, StackGuardSlot);

  // Safe stack object that stores the current unsafe stack top. It is updated
  // as unsafe dynamic (non-constant-sized) allocas are allocated and freed.
  // This is only needed if we need to restore stack pointer after longjmp
  // or exceptions, and we have dynamic allocations.
  // FIXME: a better alternative might be to store the unsafe stack pointer
  // before setjmp / invoke instructions.
  AllocaInst *DynamicTop = createStackRestorePoints(
      IRB, F, StackRestorePoints, StaticTop, !DynamicAllocas.empty());

  // Handle dynamic allocas.
  moveDynamicAllocasToUnsafeStack(F, UnsafeStackPtr, DynamicTop,
                                  DynamicAllocas);

  // Restore the unsafe stack pointer before each return.
  for (Instruction *RI : Returns) {
    IRB.SetInsertPoint(RI);
    IRB.CreateStore(BasePointer, UnsafeStackPtr);
  }

  TryInlinePointerAddress();

  LLVM_DEBUG(dbgs() << "[SafeStack]     safestack applied\n");
  return true;
}

class SafeStackLegacyPass : public FunctionPass {
  const TargetMachine *TM = nullptr;

public:
  static char ID; // Pass identification, replacement for typeid..

  SafeStackLegacyPass() : FunctionPass(ID) {
    initializeSafeStackLegacyPassPass(*PassRegistry::getPassRegistry());
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<TargetPassConfig>();
    AU.addRequired<TargetLibraryInfoWrapperPass>();
    AU.addRequired<AssumptionCacheTracker>();
    AU.addPreserved<DominatorTreeWrapperPass>();
  }

  bool runOnFunction(Function &F) override {
    LLVM_DEBUG(dbgs() << "[SafeStack] Function: " << F.getName() << "\n");

    if (!F.hasFnAttribute(Attribute::SafeStack)) {
      LLVM_DEBUG(dbgs() << "[SafeStack]     safestack is not requested"
                           " for this function\n");
      return false;
    }

    if (F.isDeclaration()) {
      LLVM_DEBUG(dbgs() << "[SafeStack]     function definition"
                           " is not available\n");
      return false;
    }

    TM = &getAnalysis<TargetPassConfig>().getTM<TargetMachine>();
    auto *TL = TM->getSubtargetImpl(F)->getTargetLowering();
    if (!TL)
      report_fatal_error("TargetLowering instance is required");

    auto *DL = &F.getDataLayout();
    auto &TLI = getAnalysis<TargetLibraryInfoWrapperPass>().getTLI(F);
    auto &ACT = getAnalysis<AssumptionCacheTracker>().getAssumptionCache(F);

    // Compute DT and LI only for functions that have the attribute.
    // This is only useful because the legacy pass manager doesn't let us
    // compute analyzes lazily.

    DominatorTree *DT;
    bool ShouldPreserveDominatorTree;
    std::optional<DominatorTree> LazilyComputedDomTree;

    // Do we already have a DominatorTree available from the previous pass?
    // Note that we should *NOT* require it, to avoid the case where we end up
    // not needing it, but the legacy PM would have computed it for us anyways.
    if (auto *DTWP = getAnalysisIfAvailable<DominatorTreeWrapperPass>()) {
      DT = &DTWP->getDomTree();
      ShouldPreserveDominatorTree = true;
    } else {
      // Otherwise, we need to compute it.
      LazilyComputedDomTree.emplace(F);
      DT = &*LazilyComputedDomTree;
      ShouldPreserveDominatorTree = false;
    }

    // Likewise, lazily compute loop info.
    LoopInfo LI(*DT);

    DomTreeUpdater DTU(DT, DomTreeUpdater::UpdateStrategy::Lazy);

    ScalarEvolution SE(F, TLI, ACT, *DT, LI);

    return SafeStack(F, *TL, *DL, ShouldPreserveDominatorTree ? &DTU : nullptr,
                     SE)
        .run();
  }
};

} // end anonymous namespace

PreservedAnalyses SafeStackPass::run(Function &F,
                                     FunctionAnalysisManager &FAM) {
  LLVM_DEBUG(dbgs() << "[SafeStack] Function: " << F.getName() << "\n");

  if (!F.hasFnAttribute(Attribute::SafeStack)) {
    LLVM_DEBUG(dbgs() << "[SafeStack]     safestack is not requested"
                         " for this function\n");
    return PreservedAnalyses::all();
  }

  if (F.isDeclaration()) {
    LLVM_DEBUG(dbgs() << "[SafeStack]     function definition"
                         " is not available\n");
    return PreservedAnalyses::all();
  }

  auto *TL = TM->getSubtargetImpl(F)->getTargetLowering();
  if (!TL)
    report_fatal_error("TargetLowering instance is required");

  auto &DL = F.getDataLayout();

  // preserve DominatorTree
  auto &DT = FAM.getResult<DominatorTreeAnalysis>(F);
  auto &SE = FAM.getResult<ScalarEvolutionAnalysis>(F);
  DomTreeUpdater DTU(DT, DomTreeUpdater::UpdateStrategy::Lazy);

  bool Changed = SafeStack(F, *TL, DL, &DTU, SE).run();

  if (!Changed)
    return PreservedAnalyses::all();
  PreservedAnalyses PA;
  PA.preserve<DominatorTreeAnalysis>();
  return PA;
}

char SafeStackLegacyPass::ID = 0;

INITIALIZE_PASS_BEGIN(SafeStackLegacyPass, DEBUG_TYPE,
                      "Safe Stack instrumentation pass", false, false)
INITIALIZE_PASS_DEPENDENCY(TargetPassConfig)
INITIALIZE_PASS_DEPENDENCY(DominatorTreeWrapperPass)
INITIALIZE_PASS_END(SafeStackLegacyPass, DEBUG_TYPE,
                    "Safe Stack instrumentation pass", false, false)

FunctionPass *toolchain::createSafeStackPass() { return new SafeStackLegacyPass(); }
