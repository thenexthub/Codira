//===- Instruction.cpp - The Instructions of Sandbox IR -------------------===//
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

#include "vm/core/SandboxIR/Instruction.h"
#include "vm/core/SandboxIR/Function.h"

namespace vm::core::sandboxir {

const char *Instruction::getOpcodeName(Opcode Opc) {
  switch (Opc) {
#define OP(OPC)                                                                \
  case Opcode::OPC:                                                            \
    return #OPC;
#define OPCODES(...) __VA_ARGS__
#define DEF_INSTR(ID, OPC, CLASS) OPC
#include "vm/core/SandboxIR/Values.def"
  }
  llvm_unreachable("Unknown Opcode");
}

toolchain::Instruction *Instruction::getTopmostLLVMInstruction() const {
  Instruction *Prev = getPrevNode();
  if (Prev == nullptr) {
    // If at top of the BB, return the first BB instruction.
    return &*cast<toolchain::BasicBlock>(getParent()->Val)->begin();
  }
  // Else get the Previous sandbox IR instruction's bottom IR instruction and
  // return its successor.
  toolchain::Instruction *PrevBotI = cast<toolchain::Instruction>(Prev->Val);
  return PrevBotI->getNextNode();
}

BBIterator Instruction::getIterator() const {
  auto *I = cast<toolchain::Instruction>(Val);
  return BasicBlock::iterator(I->getParent(), I->getIterator(), &Ctx);
}

Instruction *Instruction::getNextNode() const {
  assert(getParent() != nullptr && "Detached!");
  assert(getIterator() != getParent()->end() && "Already at end!");
  // `Val` is the bottom-most LLVM IR instruction. Get the next in the chain,
  // and get the corresponding sandboxir Instruction that maps to it. This works
  // even for SandboxIR Instructions that map to more than one LLVM Instruction.
  auto *LLVMI = cast<toolchain::Instruction>(Val);
  assert(LLVMI->getParent() != nullptr && "LLVM IR instr is detached!");
  auto *NextLLVMI = LLVMI->getNextNode();
  auto *NextI = cast_or_null<Instruction>(Ctx.getValue(NextLLVMI));
  if (NextI == nullptr)
    return nullptr;
  return NextI;
}

Instruction *Instruction::getPrevNode() const {
  assert(getParent() != nullptr && "Detached!");
  auto It = getIterator();
  if (It != getParent()->begin())
    return std::prev(getIterator()).get();
  return nullptr;
}

void Instruction::removeFromParent() {
  Ctx.getTracker().emplaceIfTracking<RemoveFromParent>(this);

  // Detach all the LLVM IR instructions from their parent BB.
  for (toolchain::Instruction *I : getLLVMInstrs())
    I->removeFromParent();
}

void Instruction::eraseFromParent() {
  assert(users().empty() && "Still connected to users, can't erase!");

  Ctx.runEraseInstrCallbacks(this);
  std::unique_ptr<Value> Detached = Ctx.detach(this);
  auto LLVMInstrs = getLLVMInstrs();

  auto &Tracker = Ctx.getTracker();
  if (Tracker.isTracking()) {
    Tracker.track(std::make_unique<EraseFromParent>(std::move(Detached)));
    // We don't actually delete the IR instruction, because then it would be
    // impossible to bring it back from the dead at the same memory location.
    // Instead we remove it from its BB and track its current location.
    for (toolchain::Instruction *I : LLVMInstrs)
      I->removeFromParent();
    // TODO: Multi-instructions need special treatment because some of the
    // references are internal to the instruction.
    for (toolchain::Instruction *I : LLVMInstrs)
      I->dropAllReferences();
  } else {
    // Erase in reverse to avoid erasing nstructions with attached uses.
    for (toolchain::Instruction *I : reverse(LLVMInstrs))
      I->eraseFromParent();
  }
}

void Instruction::moveBefore(BasicBlock &BB, const BBIterator &WhereIt) {
  if (std::next(getIterator()) == WhereIt)
    // Destination is same as origin, nothing to do.
    return;

  Ctx.runMoveInstrCallbacks(this, WhereIt);
  Ctx.getTracker().emplaceIfTracking<MoveInstr>(this);

  auto *LLVMBB = cast<toolchain::BasicBlock>(BB.Val);
  toolchain::BasicBlock::iterator It;
  if (WhereIt == BB.end()) {
    It = LLVMBB->end();
  } else {
    Instruction *WhereI = &*WhereIt;
    It = WhereI->getTopmostLLVMInstruction()->getIterator();
  }
  // TODO: Move this to the verifier of sandboxir::Instruction.
  assert(is_sorted(getLLVMInstrs(),
                   [](auto *I1, auto *I2) { return I1->comesBefore(I2); }) &&
         "Expected program order!");
  // Do the actual move in LLVM IR.
  for (auto *I : getLLVMInstrs())
    I->moveBefore(*LLVMBB, It);
}

void Instruction::insertBefore(Instruction *BeforeI) {
  toolchain::Instruction *BeforeTopI = BeforeI->getTopmostLLVMInstruction();

  Ctx.getTracker().emplaceIfTracking<InsertIntoBB>(this);

  // Insert the LLVM IR Instructions in program order.
  for (toolchain::Instruction *I : getLLVMInstrs())
    I->insertBefore(BeforeTopI->getIterator());
}

void Instruction::insertAfter(Instruction *AfterI) {
  insertInto(AfterI->getParent(), std::next(AfterI->getIterator()));
}

void Instruction::insertInto(BasicBlock *BB, const BBIterator &WhereIt) {
  toolchain::BasicBlock *LLVMBB = cast<toolchain::BasicBlock>(BB->Val);
  toolchain::Instruction *LLVMBeforeI;
  toolchain::BasicBlock::iterator LLVMBeforeIt;
  Instruction *BeforeI;
  if (WhereIt != BB->end()) {
    BeforeI = &*WhereIt;
    LLVMBeforeI = BeforeI->getTopmostLLVMInstruction();
    LLVMBeforeIt = LLVMBeforeI->getIterator();
  } else {
    BeforeI = nullptr;
    LLVMBeforeI = nullptr;
    LLVMBeforeIt = LLVMBB->end();
  }

  Ctx.getTracker().emplaceIfTracking<InsertIntoBB>(this);

  // Insert the LLVM IR Instructions in program order.
  for (toolchain::Instruction *I : getLLVMInstrs())
    I->insertInto(LLVMBB, LLVMBeforeIt);
}

BasicBlock *Instruction::getParent() const {
  // Get the LLVM IR Instruction that this maps to, get its parent, and get the
  // corresponding sandboxir::BasicBlock by looking it up in sandboxir::Context.
  auto *BB = cast<toolchain::Instruction>(Val)->getParent();
  if (BB == nullptr)
    return nullptr;
  return cast<BasicBlock>(Ctx.getValue(BB));
}

bool Instruction::classof(const sandboxir::Value *From) {
  switch (From->getSubclassID()) {
#define DEF_INSTR(ID, OPC, CLASS)                                              \
  case ClassID::ID:                                                            \
    return true;
#include "vm/core/SandboxIR/Values.def"
  default:
    return false;
  }
}

void Instruction::setHasNoUnsignedWrap(bool B) {
  Ctx.getTracker()
      .emplaceIfTracking<GenericSetter<&Instruction::hasNoUnsignedWrap,
                                       &Instruction::setHasNoUnsignedWrap>>(
          this);
  cast<toolchain::Instruction>(Val)->setHasNoUnsignedWrap(B);
}

void Instruction::setHasNoSignedWrap(bool B) {
  Ctx.getTracker()
      .emplaceIfTracking<GenericSetter<&Instruction::hasNoSignedWrap,
                                       &Instruction::setHasNoSignedWrap>>(this);
  cast<toolchain::Instruction>(Val)->setHasNoSignedWrap(B);
}

void Instruction::setFast(bool B) {
  Ctx.getTracker()
      .emplaceIfTracking<
          GenericSetter<&Instruction::isFast, &Instruction::setFast>>(this);
  cast<toolchain::Instruction>(Val)->setFast(B);
}

void Instruction::setIsExact(bool B) {
  Ctx.getTracker()
      .emplaceIfTracking<
          GenericSetter<&Instruction::isExact, &Instruction::setIsExact>>(this);
  cast<toolchain::Instruction>(Val)->setIsExact(B);
}

void Instruction::setHasAllowReassoc(bool B) {
  Ctx.getTracker()
      .emplaceIfTracking<GenericSetter<&Instruction::hasAllowReassoc,
                                       &Instruction::setHasAllowReassoc>>(this);
  cast<toolchain::Instruction>(Val)->setHasAllowReassoc(B);
}

void Instruction::setHasNoNaNs(bool B) {
  Ctx.getTracker()
      .emplaceIfTracking<
          GenericSetter<&Instruction::hasNoNaNs, &Instruction::setHasNoNaNs>>(
          this);
  cast<toolchain::Instruction>(Val)->setHasNoNaNs(B);
}

void Instruction::setHasNoInfs(bool B) {
  Ctx.getTracker()
      .emplaceIfTracking<
          GenericSetter<&Instruction::hasNoInfs, &Instruction::setHasNoInfs>>(
          this);
  cast<toolchain::Instruction>(Val)->setHasNoInfs(B);
}

void Instruction::setHasNoSignedZeros(bool B) {
  Ctx.getTracker()
      .emplaceIfTracking<GenericSetter<&Instruction::hasNoSignedZeros,
                                       &Instruction::setHasNoSignedZeros>>(
          this);
  cast<toolchain::Instruction>(Val)->setHasNoSignedZeros(B);
}

void Instruction::setHasAllowReciprocal(bool B) {
  Ctx.getTracker()
      .emplaceIfTracking<GenericSetter<&Instruction::hasAllowReciprocal,
                                       &Instruction::setHasAllowReciprocal>>(
          this);
  cast<toolchain::Instruction>(Val)->setHasAllowReciprocal(B);
}

void Instruction::setHasAllowContract(bool B) {
  Ctx.getTracker()
      .emplaceIfTracking<GenericSetter<&Instruction::hasAllowContract,
                                       &Instruction::setHasAllowContract>>(
          this);
  cast<toolchain::Instruction>(Val)->setHasAllowContract(B);
}

void Instruction::setFastMathFlags(FastMathFlags FMF) {
  Ctx.getTracker()
      .emplaceIfTracking<GenericSetter<&Instruction::getFastMathFlags,
                                       &Instruction::copyFastMathFlags>>(this);
  cast<toolchain::Instruction>(Val)->setFastMathFlags(FMF);
}

void Instruction::copyFastMathFlags(FastMathFlags FMF) {
  Ctx.getTracker()
      .emplaceIfTracking<GenericSetter<&Instruction::getFastMathFlags,
                                       &Instruction::copyFastMathFlags>>(this);
  cast<toolchain::Instruction>(Val)->copyFastMathFlags(FMF);
}

Type *Instruction::getAccessType() const {
  return Ctx.getType(cast<toolchain::Instruction>(Val)->getAccessType());
}

void Instruction::setHasApproxFunc(bool B) {
  Ctx.getTracker()
      .emplaceIfTracking<GenericSetter<&Instruction::hasApproxFunc,
                                       &Instruction::setHasApproxFunc>>(this);
  cast<toolchain::Instruction>(Val)->setHasApproxFunc(B);
}

#ifndef NDEBUG
void Instruction::dumpOS(raw_ostream &OS) const {
  OS << "Unimplemented! Please override dump().";
}
#endif // NDEBUG

VAArgInst *VAArgInst::create(Value *List, Type *Ty, InsertPosition Pos,
                             Context &Ctx, const Twine &Name) {
  auto &Builder = setInsertPos(Pos);
  auto *LLVMI =
      cast<toolchain::VAArgInst>(Builder.CreateVAArg(List->Val, Ty->LLVMTy, Name));
  return Ctx.createVAArgInst(LLVMI);
}

Value *VAArgInst::getPointerOperand() {
  return Ctx.getValue(cast<toolchain::VAArgInst>(Val)->getPointerOperand());
}

FreezeInst *FreezeInst::create(Value *V, InsertPosition Pos, Context &Ctx,
                               const Twine &Name) {
  auto &Builder = setInsertPos(Pos);
  auto *LLVMI = cast<toolchain::FreezeInst>(Builder.CreateFreeze(V->Val, Name));
  return Ctx.createFreezeInst(LLVMI);
}

FenceInst *FenceInst::create(AtomicOrdering Ordering, InsertPosition Pos,
                             Context &Ctx, SyncScope::ID SSID) {
  auto &Builder = Instruction::setInsertPos(Pos);
  toolchain::FenceInst *LLVMI = Builder.CreateFence(Ordering, SSID);
  return Ctx.createFenceInst(LLVMI);
}

void FenceInst::setOrdering(AtomicOrdering Ordering) {
  Ctx.getTracker()
      .emplaceIfTracking<
          GenericSetter<&FenceInst::getOrdering, &FenceInst::setOrdering>>(
          this);
  cast<toolchain::FenceInst>(Val)->setOrdering(Ordering);
}

void FenceInst::setSyncScopeID(SyncScope::ID SSID) {
  Ctx.getTracker()
      .emplaceIfTracking<GenericSetter<&FenceInst::getSyncScopeID,
                                       &FenceInst::setSyncScopeID>>(this);
  cast<toolchain::FenceInst>(Val)->setSyncScopeID(SSID);
}

Value *SelectInst::create(Value *Cond, Value *True, Value *False,
                          InsertPosition Pos, Context &Ctx, const Twine &Name) {
  auto &Builder = Instruction::setInsertPos(Pos);
  toolchain::Value *NewV =
      Builder.CreateSelect(Cond->Val, True->Val, False->Val, Name);
  if (auto *NewSI = dyn_cast<toolchain::SelectInst>(NewV))
    return Ctx.createSelectInst(NewSI);
  assert(isa<toolchain::Constant>(NewV) && "Expected constant");
  return Ctx.getOrCreateConstant(cast<toolchain::Constant>(NewV));
}

void SelectInst::swapValues() {
  Ctx.getTracker().emplaceIfTracking<UseSwap>(getOperandUse(1),
                                              getOperandUse(2));
  cast<toolchain::SelectInst>(Val)->swapValues();
}

bool SelectInst::classof(const Value *From) {
  return From->getSubclassID() == ClassID::Select;
}

BranchInst *BranchInst::create(BasicBlock *IfTrue, InsertPosition Pos,
                               Context &Ctx) {
  auto &Builder = setInsertPos(Pos);
  toolchain::BranchInst *NewBr =
      Builder.CreateBr(cast<toolchain::BasicBlock>(IfTrue->Val));
  return Ctx.createBranchInst(NewBr);
}

BranchInst *BranchInst::create(BasicBlock *IfTrue, BasicBlock *IfFalse,
                               Value *Cond, InsertPosition Pos, Context &Ctx) {
  auto &Builder = setInsertPos(Pos);
  toolchain::BranchInst *NewBr =
      Builder.CreateCondBr(Cond->Val, cast<toolchain::BasicBlock>(IfTrue->Val),
                           cast<toolchain::BasicBlock>(IfFalse->Val));
  return Ctx.createBranchInst(NewBr);
}

bool BranchInst::classof(const Value *From) {
  return From->getSubclassID() == ClassID::Br;
}

Value *BranchInst::getCondition() const {
  assert(isConditional() && "Cannot get condition of an uncond branch!");
  return Ctx.getValue(cast<toolchain::BranchInst>(Val)->getCondition());
}

BasicBlock *BranchInst::getSuccessor(unsigned SuccIdx) const {
  assert(SuccIdx < getNumSuccessors() &&
         "Successor # out of range for Branch!");
  return cast_or_null<BasicBlock>(
      Ctx.getValue(cast<toolchain::BranchInst>(Val)->getSuccessor(SuccIdx)));
}

void BranchInst::setSuccessor(unsigned Idx, BasicBlock *NewSucc) {
  assert((Idx == 0 || Idx == 1) && "Out of bounds!");
  setOperand(2u - Idx, NewSucc);
}

BasicBlock *BranchInst::LLVMBBToSBBB::operator()(toolchain::BasicBlock *BB) const {
  return cast<BasicBlock>(Ctx.getValue(BB));
}
const BasicBlock *
BranchInst::ConstLLVMBBToSBBB::operator()(const toolchain::BasicBlock *BB) const {
  return cast<BasicBlock>(Ctx.getValue(BB));
}

void LoadInst::setVolatile(bool V) {
  Ctx.getTracker()
      .emplaceIfTracking<
          GenericSetter<&LoadInst::isVolatile, &LoadInst::setVolatile>>(this);
  cast<toolchain::LoadInst>(Val)->setVolatile(V);
}

LoadInst *LoadInst::create(Type *Ty, Value *Ptr, MaybeAlign Align,
                           InsertPosition Pos, bool IsVolatile, Context &Ctx,
                           const Twine &Name) {
  auto &Builder = setInsertPos(Pos);
  auto *NewLI =
      Builder.CreateAlignedLoad(Ty->LLVMTy, Ptr->Val, Align, IsVolatile, Name);
  auto *NewSBI = Ctx.createLoadInst(NewLI);
  return NewSBI;
}

bool LoadInst::classof(const Value *From) {
  return From->getSubclassID() == ClassID::Load;
}

Value *LoadInst::getPointerOperand() const {
  return Ctx.getValue(cast<toolchain::LoadInst>(Val)->getPointerOperand());
}

void StoreInst::setVolatile(bool V) {
  Ctx.getTracker()
      .emplaceIfTracking<
          GenericSetter<&StoreInst::isVolatile, &StoreInst::setVolatile>>(this);
  cast<toolchain::StoreInst>(Val)->setVolatile(V);
}

StoreInst *StoreInst::create(Value *V, Value *Ptr, MaybeAlign Align,
                             InsertPosition Pos, bool IsVolatile,
                             Context &Ctx) {
  auto &Builder = setInsertPos(Pos);
  auto *NewSI = Builder.CreateAlignedStore(V->Val, Ptr->Val, Align, IsVolatile);
  auto *NewSBI = Ctx.createStoreInst(NewSI);
  return NewSBI;
}

bool StoreInst::classof(const Value *From) {
  return From->getSubclassID() == ClassID::Store;
}

Value *StoreInst::getValueOperand() const {
  return Ctx.getValue(cast<toolchain::StoreInst>(Val)->getValueOperand());
}

Value *StoreInst::getPointerOperand() const {
  return Ctx.getValue(cast<toolchain::StoreInst>(Val)->getPointerOperand());
}

UnreachableInst *UnreachableInst::create(InsertPosition Pos, Context &Ctx) {
  auto &Builder = setInsertPos(Pos);
  toolchain::UnreachableInst *NewUI = Builder.CreateUnreachable();
  return Ctx.createUnreachableInst(NewUI);
}

bool UnreachableInst::classof(const Value *From) {
  return From->getSubclassID() == ClassID::Unreachable;
}

ReturnInst *ReturnInst::createCommon(Value *RetVal, IRBuilder<> &Builder,
                                     Context &Ctx) {
  toolchain::ReturnInst *NewRI;
  if (RetVal != nullptr)
    NewRI = Builder.CreateRet(RetVal->Val);
  else
    NewRI = Builder.CreateRetVoid();
  return Ctx.createReturnInst(NewRI);
}

ReturnInst *ReturnInst::create(Value *RetVal, InsertPosition Pos,
                               Context &Ctx) {
  auto &Builder = setInsertPos(Pos);
  return createCommon(RetVal, Builder, Ctx);
}

Value *ReturnInst::getReturnValue() const {
  auto *LLVMRetVal = cast<toolchain::ReturnInst>(Val)->getReturnValue();
  return LLVMRetVal != nullptr ? Ctx.getValue(LLVMRetVal) : nullptr;
}

FunctionType *CallBase::getFunctionType() const {
  return cast<FunctionType>(
      Ctx.getType(cast<toolchain::CallBase>(Val)->getFunctionType()));
}

Value *CallBase::getCalledOperand() const {
  return Ctx.getValue(cast<toolchain::CallBase>(Val)->getCalledOperand());
}

Use CallBase::getCalledOperandUse() const {
  toolchain::Use *LLVMUse = &cast<toolchain::CallBase>(Val)->getCalledOperandUse();
  return Use(LLVMUse, cast<User>(Ctx.getValue(LLVMUse->getUser())), Ctx);
}

Function *CallBase::getCalledFunction() const {
  return cast_or_null<Function>(
      Ctx.getValue(cast<toolchain::CallBase>(Val)->getCalledFunction()));
}
Function *CallBase::getCaller() {
  return cast<Function>(Ctx.getValue(cast<toolchain::CallBase>(Val)->getCaller()));
}

void CallBase::setCalledFunction(Function *F) {
  // F's function type is private, so we rely on `setCalledFunction()` to update
  // it. But even though we are calling `setCalledFunction()` we also need to
  // track this change at the SandboxIR level, which is why we call
  // `setCalledOperand()` here.
  // Note: This may break if `setCalledFunction()` early returns if `F`
  // is already set, but we do have a unit test for it.
  setCalledOperand(F);
  cast<toolchain::CallBase>(Val)->setCalledFunction(
      cast<toolchain::FunctionType>(F->getFunctionType()->LLVMTy),
      cast<toolchain::Function>(F->Val));
}

CallInst *CallInst::create(FunctionType *FTy, Value *Func,
                           ArrayRef<Value *> Args, InsertPosition Pos,
                           Context &Ctx, const Twine &NameStr) {
  auto &Builder = setInsertPos(Pos);
  SmallVector<toolchain::Value *> LLVMArgs;
  LLVMArgs.reserve(Args.size());
  for (Value *Arg : Args)
    LLVMArgs.push_back(Arg->Val);
  toolchain::CallInst *NewCI = Builder.CreateCall(
      cast<toolchain::FunctionType>(FTy->LLVMTy), Func->Val, LLVMArgs, NameStr);
  return Ctx.createCallInst(NewCI);
}

InvokeInst *InvokeInst::create(FunctionType *FTy, Value *Func,
                               BasicBlock *IfNormal, BasicBlock *IfException,
                               ArrayRef<Value *> Args, InsertPosition Pos,
                               Context &Ctx, const Twine &NameStr) {
  auto &Builder = setInsertPos(Pos);
  SmallVector<toolchain::Value *> LLVMArgs;
  LLVMArgs.reserve(Args.size());
  for (Value *Arg : Args)
    LLVMArgs.push_back(Arg->Val);
  toolchain::InvokeInst *Invoke = Builder.CreateInvoke(
      cast<toolchain::FunctionType>(FTy->LLVMTy), Func->Val,
      cast<toolchain::BasicBlock>(IfNormal->Val),
      cast<toolchain::BasicBlock>(IfException->Val), LLVMArgs, NameStr);
  return Ctx.createInvokeInst(Invoke);
}

BasicBlock *InvokeInst::getNormalDest() const {
  return cast<BasicBlock>(
      Ctx.getValue(cast<toolchain::InvokeInst>(Val)->getNormalDest()));
}
BasicBlock *InvokeInst::getUnwindDest() const {
  return cast<BasicBlock>(
      Ctx.getValue(cast<toolchain::InvokeInst>(Val)->getUnwindDest()));
}
void InvokeInst::setNormalDest(BasicBlock *BB) {
  setOperand(1, BB);
  assert(getNormalDest() == BB && "LLVM IR uses a different operan index!");
}
void InvokeInst::setUnwindDest(BasicBlock *BB) {
  setOperand(2, BB);
  assert(getUnwindDest() == BB && "LLVM IR uses a different operan index!");
}
LandingPadInst *InvokeInst::getLandingPadInst() const {
  return cast<LandingPadInst>(
      Ctx.getValue(cast<toolchain::InvokeInst>(Val)->getLandingPadInst()));
  ;
}
BasicBlock *InvokeInst::getSuccessor(unsigned SuccIdx) const {
  return cast<BasicBlock>(
      Ctx.getValue(cast<toolchain::InvokeInst>(Val)->getSuccessor(SuccIdx)));
}

CallBrInst *CallBrInst::create(FunctionType *FTy, Value *Func,
                               BasicBlock *DefaultDest,
                               ArrayRef<BasicBlock *> IndirectDests,
                               ArrayRef<Value *> Args, InsertPosition Pos,
                               Context &Ctx, const Twine &NameStr) {
  auto &Builder = setInsertPos(Pos);
  SmallVector<toolchain::BasicBlock *> LLVMIndirectDests;
  LLVMIndirectDests.reserve(IndirectDests.size());
  for (BasicBlock *IndDest : IndirectDests)
    LLVMIndirectDests.push_back(cast<toolchain::BasicBlock>(IndDest->Val));

  SmallVector<toolchain::Value *> LLVMArgs;
  LLVMArgs.reserve(Args.size());
  for (Value *Arg : Args)
    LLVMArgs.push_back(Arg->Val);

  toolchain::CallBrInst *CallBr =
      Builder.CreateCallBr(cast<toolchain::FunctionType>(FTy->LLVMTy), Func->Val,
                           cast<toolchain::BasicBlock>(DefaultDest->Val),
                           LLVMIndirectDests, LLVMArgs, NameStr);
  return Ctx.createCallBrInst(CallBr);
}

Value *CallBrInst::getIndirectDestLabel(unsigned Idx) const {
  return Ctx.getValue(cast<toolchain::CallBrInst>(Val)->getIndirectDestLabel(Idx));
}
Value *CallBrInst::getIndirectDestLabelUse(unsigned Idx) const {
  return Ctx.getValue(
      cast<toolchain::CallBrInst>(Val)->getIndirectDestLabelUse(Idx));
}
BasicBlock *CallBrInst::getDefaultDest() const {
  return cast<BasicBlock>(
      Ctx.getValue(cast<toolchain::CallBrInst>(Val)->getDefaultDest()));
}
BasicBlock *CallBrInst::getIndirectDest(unsigned Idx) const {
  return cast<BasicBlock>(
      Ctx.getValue(cast<toolchain::CallBrInst>(Val)->getIndirectDest(Idx)));
}
toolchain::SmallVector<BasicBlock *, 16> CallBrInst::getIndirectDests() const {
  SmallVector<BasicBlock *, 16> BBs;
  for (toolchain::BasicBlock *LLVMBB :
       cast<toolchain::CallBrInst>(Val)->getIndirectDests())
    BBs.push_back(cast<BasicBlock>(Ctx.getValue(LLVMBB)));
  return BBs;
}
void CallBrInst::setDefaultDest(BasicBlock *BB) {
  Ctx.getTracker()
      .emplaceIfTracking<GenericSetter<&CallBrInst::getDefaultDest,
                                       &CallBrInst::setDefaultDest>>(this);
  cast<toolchain::CallBrInst>(Val)->setDefaultDest(cast<toolchain::BasicBlock>(BB->Val));
}
void CallBrInst::setIndirectDest(unsigned Idx, BasicBlock *BB) {
  Ctx.getTracker()
      .emplaceIfTracking<GenericSetterWithIdx<&CallBrInst::getIndirectDest,
                                              &CallBrInst::setIndirectDest>>(
          this, Idx);
  cast<toolchain::CallBrInst>(Val)->setIndirectDest(Idx,
                                               cast<toolchain::BasicBlock>(BB->Val));
}
BasicBlock *CallBrInst::getSuccessor(unsigned Idx) const {
  return cast<BasicBlock>(
      Ctx.getValue(cast<toolchain::CallBrInst>(Val)->getSuccessor(Idx)));
}

LandingPadInst *LandingPadInst::create(Type *RetTy, unsigned NumReservedClauses,
                                       InsertPosition Pos, Context &Ctx,
                                       const Twine &Name) {
  auto &Builder = setInsertPos(Pos);
  toolchain::LandingPadInst *LLVMI =
      Builder.CreateLandingPad(RetTy->LLVMTy, NumReservedClauses, Name);
  return Ctx.createLandingPadInst(LLVMI);
}

void LandingPadInst::setCleanup(bool V) {
  Ctx.getTracker()
      .emplaceIfTracking<GenericSetter<&LandingPadInst::isCleanup,
                                       &LandingPadInst::setCleanup>>(this);
  cast<toolchain::LandingPadInst>(Val)->setCleanup(V);
}

Constant *LandingPadInst::getClause(unsigned Idx) const {
  return cast<Constant>(
      Ctx.getValue(cast<toolchain::LandingPadInst>(Val)->getClause(Idx)));
}

Value *FuncletPadInst::getParentPad() const {
  return Ctx.getValue(cast<toolchain::FuncletPadInst>(Val)->getParentPad());
}

void FuncletPadInst::setParentPad(Value *ParentPad) {
  Ctx.getTracker()
      .emplaceIfTracking<GenericSetter<&FuncletPadInst::getParentPad,
                                       &FuncletPadInst::setParentPad>>(this);
  cast<toolchain::FuncletPadInst>(Val)->setParentPad(ParentPad->Val);
}

Value *FuncletPadInst::getArgOperand(unsigned Idx) const {
  return Ctx.getValue(cast<toolchain::FuncletPadInst>(Val)->getArgOperand(Idx));
}

void FuncletPadInst::setArgOperand(unsigned Idx, Value *V) {
  Ctx.getTracker()
      .emplaceIfTracking<GenericSetterWithIdx<&FuncletPadInst::getArgOperand,
                                              &FuncletPadInst::setArgOperand>>(
          this, Idx);
  cast<toolchain::FuncletPadInst>(Val)->setArgOperand(Idx, V->Val);
}

CatchSwitchInst *CatchPadInst::getCatchSwitch() const {
  return cast<CatchSwitchInst>(
      Ctx.getValue(cast<toolchain::CatchPadInst>(Val)->getCatchSwitch()));
}

CatchPadInst *CatchPadInst::create(Value *ParentPad, ArrayRef<Value *> Args,
                                   InsertPosition Pos, Context &Ctx,
                                   const Twine &Name) {
  auto &Builder = setInsertPos(Pos);
  SmallVector<toolchain::Value *> LLVMArgs;
  LLVMArgs.reserve(Args.size());
  for (auto *Arg : Args)
    LLVMArgs.push_back(Arg->Val);
  toolchain::CatchPadInst *LLVMI =
      Builder.CreateCatchPad(ParentPad->Val, LLVMArgs, Name);
  return Ctx.createCatchPadInst(LLVMI);
}

CleanupPadInst *CleanupPadInst::create(Value *ParentPad, ArrayRef<Value *> Args,
                                       InsertPosition Pos, Context &Ctx,
                                       const Twine &Name) {
  auto &Builder = setInsertPos(Pos);
  SmallVector<toolchain::Value *> LLVMArgs;
  LLVMArgs.reserve(Args.size());
  for (auto *Arg : Args)
    LLVMArgs.push_back(Arg->Val);
  toolchain::CleanupPadInst *LLVMI =
      Builder.CreateCleanupPad(ParentPad->Val, LLVMArgs, Name);
  return Ctx.createCleanupPadInst(LLVMI);
}

CatchReturnInst *CatchReturnInst::create(CatchPadInst *CatchPad, BasicBlock *BB,
                                         InsertPosition Pos, Context &Ctx) {
  auto &Builder = setInsertPos(Pos);
  toolchain::CatchReturnInst *LLVMI = Builder.CreateCatchRet(
      cast<toolchain::CatchPadInst>(CatchPad->Val), cast<toolchain::BasicBlock>(BB->Val));
  return Ctx.createCatchReturnInst(LLVMI);
}

CatchPadInst *CatchReturnInst::getCatchPad() const {
  return cast<CatchPadInst>(
      Ctx.getValue(cast<toolchain::CatchReturnInst>(Val)->getCatchPad()));
}

void CatchReturnInst::setCatchPad(CatchPadInst *CatchPad) {
  Ctx.getTracker()
      .emplaceIfTracking<GenericSetter<&CatchReturnInst::getCatchPad,
                                       &CatchReturnInst::setCatchPad>>(this);
  cast<toolchain::CatchReturnInst>(Val)->setCatchPad(
      cast<toolchain::CatchPadInst>(CatchPad->Val));
}

BasicBlock *CatchReturnInst::getSuccessor() const {
  return cast<BasicBlock>(
      Ctx.getValue(cast<toolchain::CatchReturnInst>(Val)->getSuccessor()));
}

void CatchReturnInst::setSuccessor(BasicBlock *NewSucc) {
  Ctx.getTracker()
      .emplaceIfTracking<GenericSetter<&CatchReturnInst::getSuccessor,
                                       &CatchReturnInst::setSuccessor>>(this);
  cast<toolchain::CatchReturnInst>(Val)->setSuccessor(
      cast<toolchain::BasicBlock>(NewSucc->Val));
}

Value *CatchReturnInst::getCatchSwitchParentPad() const {
  return Ctx.getValue(
      cast<toolchain::CatchReturnInst>(Val)->getCatchSwitchParentPad());
}

CleanupReturnInst *CleanupReturnInst::create(CleanupPadInst *CleanupPad,
                                             BasicBlock *UnwindBB,
                                             InsertPosition Pos, Context &Ctx) {
  auto &Builder = setInsertPos(Pos);
  auto *LLVMUnwindBB =
      UnwindBB != nullptr ? cast<toolchain::BasicBlock>(UnwindBB->Val) : nullptr;
  toolchain::CleanupReturnInst *LLVMI = Builder.CreateCleanupRet(
      cast<toolchain::CleanupPadInst>(CleanupPad->Val), LLVMUnwindBB);
  return Ctx.createCleanupReturnInst(LLVMI);
}

CleanupPadInst *CleanupReturnInst::getCleanupPad() const {
  return cast<CleanupPadInst>(
      Ctx.getValue(cast<toolchain::CleanupReturnInst>(Val)->getCleanupPad()));
}

void CleanupReturnInst::setCleanupPad(CleanupPadInst *CleanupPad) {
  Ctx.getTracker()
      .emplaceIfTracking<GenericSetter<&CleanupReturnInst::getCleanupPad,
                                       &CleanupReturnInst::setCleanupPad>>(
          this);
  cast<toolchain::CleanupReturnInst>(Val)->setCleanupPad(
      cast<toolchain::CleanupPadInst>(CleanupPad->Val));
}

BasicBlock *CleanupReturnInst::getUnwindDest() const {
  return cast_or_null<BasicBlock>(
      Ctx.getValue(cast<toolchain::CleanupReturnInst>(Val)->getUnwindDest()));
}

void CleanupReturnInst::setUnwindDest(BasicBlock *NewDest) {
  Ctx.getTracker()
      .emplaceIfTracking<GenericSetter<&CleanupReturnInst::getUnwindDest,
                                       &CleanupReturnInst::setUnwindDest>>(
          this);
  cast<toolchain::CleanupReturnInst>(Val)->setUnwindDest(
      cast<toolchain::BasicBlock>(NewDest->Val));
}

Value *GetElementPtrInst::create(Type *Ty, Value *Ptr,
                                 ArrayRef<Value *> IdxList, InsertPosition Pos,
                                 Context &Ctx, const Twine &NameStr) {
  auto &Builder = setInsertPos(Pos);
  SmallVector<toolchain::Value *> LLVMIdxList;
  LLVMIdxList.reserve(IdxList.size());
  for (Value *Idx : IdxList)
    LLVMIdxList.push_back(Idx->Val);
  toolchain::Value *NewV =
      Builder.CreateGEP(Ty->LLVMTy, Ptr->Val, LLVMIdxList, NameStr);
  if (auto *NewGEP = dyn_cast<toolchain::GetElementPtrInst>(NewV))
    return Ctx.createGetElementPtrInst(NewGEP);
  assert(isa<toolchain::Constant>(NewV) && "Expected constant");
  return Ctx.getOrCreateConstant(cast<toolchain::Constant>(NewV));
}

Type *GetElementPtrInst::getSourceElementType() const {
  return Ctx.getType(
      cast<toolchain::GetElementPtrInst>(Val)->getSourceElementType());
}

Type *GetElementPtrInst::getResultElementType() const {
  return Ctx.getType(
      cast<toolchain::GetElementPtrInst>(Val)->getResultElementType());
}

Value *GetElementPtrInst::getPointerOperand() const {
  return Ctx.getValue(cast<toolchain::GetElementPtrInst>(Val)->getPointerOperand());
}

Type *GetElementPtrInst::getPointerOperandType() const {
  return Ctx.getType(
      cast<toolchain::GetElementPtrInst>(Val)->getPointerOperandType());
}

BasicBlock *PHINode::LLVMBBToBB::operator()(toolchain::BasicBlock *LLVMBB) const {
  return cast<BasicBlock>(Ctx.getValue(LLVMBB));
}

PHINode *PHINode::create(Type *Ty, unsigned NumReservedValues,
                         InsertPosition Pos, Context &Ctx, const Twine &Name) {
  auto &Builder = setInsertPos(Pos);
  toolchain::PHINode *NewPHI =
      Builder.CreatePHI(Ty->LLVMTy, NumReservedValues, Name);
  return Ctx.createPHINode(NewPHI);
}

bool PHINode::classof(const Value *From) {
  return From->getSubclassID() == ClassID::PHI;
}

Value *PHINode::getIncomingValue(unsigned Idx) const {
  return Ctx.getValue(cast<toolchain::PHINode>(Val)->getIncomingValue(Idx));
}
void PHINode::setIncomingValue(unsigned Idx, Value *V) {
  Ctx.getTracker()
      .emplaceIfTracking<GenericSetterWithIdx<&PHINode::getIncomingValue,
                                              &PHINode::setIncomingValue>>(this,
                                                                           Idx);
  cast<toolchain::PHINode>(Val)->setIncomingValue(Idx, V->Val);
}
BasicBlock *PHINode::getIncomingBlock(unsigned Idx) const {
  return cast<BasicBlock>(
      Ctx.getValue(cast<toolchain::PHINode>(Val)->getIncomingBlock(Idx)));
}
BasicBlock *PHINode::getIncomingBlock(const Use &U) const {
  toolchain::Use *LLVMUse = U.LLVMUse;
  toolchain::BasicBlock *BB = cast<toolchain::PHINode>(Val)->getIncomingBlock(*LLVMUse);
  return cast<BasicBlock>(Ctx.getValue(BB));
}
void PHINode::setIncomingBlock(unsigned Idx, BasicBlock *BB) {
  // Helper to disambiguate PHINode::getIncomingBlock(unsigned).
  constexpr BasicBlock *(PHINode::*GetIncomingBlockFn)(unsigned) const =
      &PHINode::getIncomingBlock;
  Ctx.getTracker()
      .emplaceIfTracking<
          GenericSetterWithIdx<GetIncomingBlockFn, &PHINode::setIncomingBlock>>(
          this, Idx);
  cast<toolchain::PHINode>(Val)->setIncomingBlock(Idx,
                                             cast<toolchain::BasicBlock>(BB->Val));
}
void PHINode::addIncoming(Value *V, BasicBlock *BB) {
  auto &Tracker = Ctx.getTracker();
  Tracker.emplaceIfTracking<PHIAddIncoming>(this);

  cast<toolchain::PHINode>(Val)->addIncoming(V->Val,
                                        cast<toolchain::BasicBlock>(BB->Val));
}
Value *PHINode::removeIncomingValue(unsigned Idx) {
  auto &Tracker = Ctx.getTracker();
  Tracker.emplaceIfTracking<PHIRemoveIncoming>(this, Idx);
  toolchain::Value *LLVMV =
      cast<toolchain::PHINode>(Val)->removeIncomingValue(Idx,
                                                    /*DeletePHIIfEmpty=*/false);
  return Ctx.getValue(LLVMV);
}
Value *PHINode::removeIncomingValue(BasicBlock *BB) {
  auto &Tracker = Ctx.getTracker();
  Tracker.emplaceIfTracking<PHIRemoveIncoming>(this, getBasicBlockIndex(BB));

  auto *LLVMBB = cast<toolchain::BasicBlock>(BB->Val);
  toolchain::Value *LLVMV =
      cast<toolchain::PHINode>(Val)->removeIncomingValue(LLVMBB,
                                                    /*DeletePHIIfEmpty=*/false);
  return Ctx.getValue(LLVMV);
}
int PHINode::getBasicBlockIndex(const BasicBlock *BB) const {
  auto *LLVMBB = cast<toolchain::BasicBlock>(BB->Val);
  return cast<toolchain::PHINode>(Val)->getBasicBlockIndex(LLVMBB);
}
Value *PHINode::getIncomingValueForBlock(const BasicBlock *BB) const {
  auto *LLVMBB = cast<toolchain::BasicBlock>(BB->Val);
  toolchain::Value *LLVMV =
      cast<toolchain::PHINode>(Val)->getIncomingValueForBlock(LLVMBB);
  return Ctx.getValue(LLVMV);
}
Value *PHINode::hasConstantValue() const {
  toolchain::Value *LLVMV = cast<toolchain::PHINode>(Val)->hasConstantValue();
  return LLVMV != nullptr ? Ctx.getValue(LLVMV) : nullptr;
}
void PHINode::replaceIncomingBlockWith(const BasicBlock *Old, BasicBlock *New) {
  assert(New && Old && "Sandbox IR PHI node got a null basic block!");
  for (unsigned Idx = 0, NumOps = cast<toolchain::PHINode>(Val)->getNumOperands();
       Idx != NumOps; ++Idx)
    if (getIncomingBlock(Idx) == Old)
      setIncomingBlock(Idx, New);
}
void PHINode::removeIncomingValueIf(function_ref<bool(unsigned)> Predicate) {
  // Avoid duplicate tracking by going through this->removeIncomingValue here at
  // the expense of some performance. Copy PHI::removeIncomingValueIf more
  // directly if performance becomes an issue.

  // Removing the element at index X, moves the element previously at X + 1
  // to X. Working from the end avoids complications from that.
  unsigned Idx = getNumIncomingValues();
  while (Idx > 0) {
    if (Predicate(Idx - 1))
      removeIncomingValue(Idx - 1);
    --Idx;
  }
}

Value *CmpInst::create(Predicate P, Value *S1, Value *S2, InsertPosition Pos,
                       Context &Ctx, const Twine &Name) {
  auto &Builder = setInsertPos(Pos);
  auto *LLVMV = Builder.CreateCmp(P, S1->Val, S2->Val, Name);
  // It may have been folded into a constant.
  if (auto *LLVMC = dyn_cast<toolchain::Constant>(LLVMV))
    return Ctx.getOrCreateConstant(LLVMC);
  if (isa<toolchain::ICmpInst>(LLVMV))
    return Ctx.createICmpInst(cast<toolchain::ICmpInst>(LLVMV));
  return Ctx.createFCmpInst(cast<toolchain::FCmpInst>(LLVMV));
}

Value *CmpInst::createWithCopiedFlags(Predicate P, Value *S1, Value *S2,
                                      const Instruction *F, InsertPosition Pos,
                                      Context &Ctx, const Twine &Name) {
  Value *V = create(P, S1, S2, Pos, Ctx, Name);
  if (auto *C = dyn_cast<Constant>(V))
    return C;
  cast<toolchain::CmpInst>(V->Val)->copyIRFlags(F->Val);
  return V;
}

Type *CmpInst::makeCmpResultType(Type *OpndType) {
  if (auto *VT = dyn_cast<VectorType>(OpndType)) {
    // TODO: Cleanup when we have more complete support for
    // sandboxir::VectorType
    return OpndType->getContext().getType(toolchain::VectorType::get(
        toolchain::Type::getInt1Ty(OpndType->getContext().LLVMCtx),
        cast<toolchain::VectorType>(VT->LLVMTy)->getElementCount()));
  }
  return Type::getInt1Ty(OpndType->getContext());
}

void CmpInst::setPredicate(Predicate P) {
  Ctx.getTracker()
      .emplaceIfTracking<
          GenericSetter<&CmpInst::getPredicate, &CmpInst::setPredicate>>(this);
  cast<toolchain::CmpInst>(Val)->setPredicate(P);
}

void CmpInst::swapOperands() {
  if (ICmpInst *IC = dyn_cast<ICmpInst>(this))
    IC->swapOperands();
  else
    cast<FCmpInst>(this)->swapOperands();
}

void ICmpInst::swapOperands() {
  Ctx.getTracker().emplaceIfTracking<CmpSwapOperands>(this);
  cast<toolchain::ICmpInst>(Val)->swapOperands();
}

void FCmpInst::swapOperands() {
  Ctx.getTracker().emplaceIfTracking<CmpSwapOperands>(this);
  cast<toolchain::FCmpInst>(Val)->swapOperands();
}

#ifndef NDEBUG
void CmpInst::dumpOS(raw_ostream &OS) const {
  dumpCommonPrefix(OS);
  dumpCommonSuffix(OS);
}

void CmpInst::dump() const {
  dumpOS(dbgs());
  dbgs() << "\n";
}
#endif // NDEBUG

static toolchain::Instruction::CastOps getLLVMCastOp(Instruction::Opcode Opc) {
  switch (Opc) {
  case Instruction::Opcode::ZExt:
    return static_cast<toolchain::Instruction::CastOps>(toolchain::Instruction::ZExt);
  case Instruction::Opcode::SExt:
    return static_cast<toolchain::Instruction::CastOps>(toolchain::Instruction::SExt);
  case Instruction::Opcode::FPToUI:
    return static_cast<toolchain::Instruction::CastOps>(toolchain::Instruction::FPToUI);
  case Instruction::Opcode::FPToSI:
    return static_cast<toolchain::Instruction::CastOps>(toolchain::Instruction::FPToSI);
  case Instruction::Opcode::FPExt:
    return static_cast<toolchain::Instruction::CastOps>(toolchain::Instruction::FPExt);
  case Instruction::Opcode::PtrToAddr:
    return static_cast<toolchain::Instruction::CastOps>(
        toolchain::Instruction::PtrToAddr);
  case Instruction::Opcode::PtrToInt:
    return static_cast<toolchain::Instruction::CastOps>(toolchain::Instruction::PtrToInt);
  case Instruction::Opcode::IntToPtr:
    return static_cast<toolchain::Instruction::CastOps>(toolchain::Instruction::IntToPtr);
  case Instruction::Opcode::SIToFP:
    return static_cast<toolchain::Instruction::CastOps>(toolchain::Instruction::SIToFP);
  case Instruction::Opcode::UIToFP:
    return static_cast<toolchain::Instruction::CastOps>(toolchain::Instruction::UIToFP);
  case Instruction::Opcode::Trunc:
    return static_cast<toolchain::Instruction::CastOps>(toolchain::Instruction::Trunc);
  case Instruction::Opcode::FPTrunc:
    return static_cast<toolchain::Instruction::CastOps>(toolchain::Instruction::FPTrunc);
  case Instruction::Opcode::BitCast:
    return static_cast<toolchain::Instruction::CastOps>(toolchain::Instruction::BitCast);
  case Instruction::Opcode::AddrSpaceCast:
    return static_cast<toolchain::Instruction::CastOps>(
        toolchain::Instruction::AddrSpaceCast);
  default:
    llvm_unreachable("Opcode not suitable for CastInst!");
  }
}

/// \Returns the LLVM opcode that corresponds to \p Opc.
static toolchain::Instruction::UnaryOps getLLVMUnaryOp(Instruction::Opcode Opc) {
  switch (Opc) {
  case Instruction::Opcode::FNeg:
    return static_cast<toolchain::Instruction::UnaryOps>(toolchain::Instruction::FNeg);
  default:
    llvm_unreachable("Not a unary op!");
  }
}

CatchSwitchInst *CatchSwitchInst::create(Value *ParentPad, BasicBlock *UnwindBB,
                                         unsigned NumHandlers,
                                         InsertPosition Pos, Context &Ctx,
                                         const Twine &Name) {
  auto &Builder = setInsertPos(Pos);
  toolchain::CatchSwitchInst *LLVMCSI = Builder.CreateCatchSwitch(
      ParentPad->Val, cast<toolchain::BasicBlock>(UnwindBB->Val), NumHandlers, Name);
  return Ctx.createCatchSwitchInst(LLVMCSI);
}

Value *CatchSwitchInst::getParentPad() const {
  return Ctx.getValue(cast<toolchain::CatchSwitchInst>(Val)->getParentPad());
}

void CatchSwitchInst::setParentPad(Value *ParentPad) {
  Ctx.getTracker()
      .emplaceIfTracking<GenericSetter<&CatchSwitchInst::getParentPad,
                                       &CatchSwitchInst::setParentPad>>(this);
  cast<toolchain::CatchSwitchInst>(Val)->setParentPad(ParentPad->Val);
}

BasicBlock *CatchSwitchInst::getUnwindDest() const {
  return cast_or_null<BasicBlock>(
      Ctx.getValue(cast<toolchain::CatchSwitchInst>(Val)->getUnwindDest()));
}

void CatchSwitchInst::setUnwindDest(BasicBlock *UnwindDest) {
  Ctx.getTracker()
      .emplaceIfTracking<GenericSetter<&CatchSwitchInst::getUnwindDest,
                                       &CatchSwitchInst::setUnwindDest>>(this);
  cast<toolchain::CatchSwitchInst>(Val)->setUnwindDest(
      cast<toolchain::BasicBlock>(UnwindDest->Val));
}

void CatchSwitchInst::addHandler(BasicBlock *Dest) {
  Ctx.getTracker().emplaceIfTracking<CatchSwitchAddHandler>(this);
  cast<toolchain::CatchSwitchInst>(Val)->addHandler(
      cast<toolchain::BasicBlock>(Dest->Val));
}

ResumeInst *ResumeInst::create(Value *Exn, InsertPosition Pos, Context &Ctx) {
  auto &Builder = setInsertPos(Pos);
  auto *LLVMI = cast<toolchain::ResumeInst>(Builder.CreateResume(Exn->Val));
  return Ctx.createResumeInst(LLVMI);
}

Value *ResumeInst::getValue() const {
  return Ctx.getValue(cast<toolchain::ResumeInst>(Val)->getValue());
}

SwitchInst *SwitchInst::create(Value *V, BasicBlock *Dest, unsigned NumCases,
                               InsertPosition Pos, Context &Ctx,
                               const Twine &Name) {
  auto &Builder = setInsertPos(Pos);
  toolchain::SwitchInst *LLVMSwitch =
      Builder.CreateSwitch(V->Val, cast<toolchain::BasicBlock>(Dest->Val), NumCases);
  return Ctx.createSwitchInst(LLVMSwitch);
}

Value *SwitchInst::getCondition() const {
  return Ctx.getValue(cast<toolchain::SwitchInst>(Val)->getCondition());
}

void SwitchInst::setCondition(Value *V) {
  Ctx.getTracker()
      .emplaceIfTracking<
          GenericSetter<&SwitchInst::getCondition, &SwitchInst::setCondition>>(
          this);
  cast<toolchain::SwitchInst>(Val)->setCondition(V->Val);
}

BasicBlock *SwitchInst::getDefaultDest() const {
  return cast<BasicBlock>(
      Ctx.getValue(cast<toolchain::SwitchInst>(Val)->getDefaultDest()));
}

void SwitchInst::setDefaultDest(BasicBlock *DefaultCase) {
  Ctx.getTracker()
      .emplaceIfTracking<GenericSetter<&SwitchInst::getDefaultDest,
                                       &SwitchInst::setDefaultDest>>(this);
  cast<toolchain::SwitchInst>(Val)->setDefaultDest(
      cast<toolchain::BasicBlock>(DefaultCase->Val));
}

template <typename LLVMCaseItT, typename BlockT, typename ConstT>
ConstT *
SwitchInst::CaseHandleImpl<LLVMCaseItT, BlockT, ConstT>::getCaseValue() const {
  const auto &LLVMCaseHandle = *LLVMCaseIt;
  auto *LLVMC = Ctx.getValue(LLVMCaseHandle.getCaseValue());
  return cast<ConstT>(LLVMC);
}

template <typename LLVMCaseItT, typename BlockT, typename ConstT>
BlockT *
SwitchInst::CaseHandleImpl<LLVMCaseItT, BlockT, ConstT>::getCaseSuccessor()
    const {
  const auto &LLVMCaseHandle = *LLVMCaseIt;
  auto *LLVMBB = LLVMCaseHandle.getCaseSuccessor();
  return cast<BlockT>(Ctx.getValue(LLVMBB));
}

template class SwitchInst::CaseHandleImpl<toolchain::SwitchInst::CaseIt, BasicBlock,
                                          ConstantInt>;
template class SwitchInst::CaseItImpl<toolchain::SwitchInst::CaseIt, BasicBlock,
                                      ConstantInt>;
template class SwitchInst::CaseHandleImpl<toolchain::SwitchInst::ConstCaseIt,
                                          const BasicBlock, const ConstantInt>;
template class SwitchInst::CaseItImpl<toolchain::SwitchInst::ConstCaseIt,
                                      const BasicBlock, const ConstantInt>;

ConstantInt *SwitchInst::findCaseDest(BasicBlock *BB) {
  auto *LLVMC = cast<toolchain::SwitchInst>(Val)->findCaseDest(
      cast<toolchain::BasicBlock>(BB->Val));
  return LLVMC != nullptr ? cast<ConstantInt>(Ctx.getValue(LLVMC)) : nullptr;
}

void SwitchInst::addCase(ConstantInt *OnVal, BasicBlock *Dest) {
  Ctx.getTracker().emplaceIfTracking<SwitchAddCase>(this, OnVal);
  // TODO: Track this!
  cast<toolchain::SwitchInst>(Val)->addCase(cast<toolchain::ConstantInt>(OnVal->Val),
                                       cast<toolchain::BasicBlock>(Dest->Val));
}

SwitchInst::CaseIt SwitchInst::removeCase(CaseIt It) {
  Ctx.getTracker().emplaceIfTracking<SwitchRemoveCase>(this);

  auto *LLVMSwitch = cast<toolchain::SwitchInst>(Val);
  unsigned CaseNum = It - case_begin();
  toolchain::SwitchInst::CaseIt LLVMIt(LLVMSwitch, CaseNum);
  auto LLVMCaseIt = LLVMSwitch->removeCase(LLVMIt);
  unsigned Num = LLVMCaseIt - LLVMSwitch->case_begin();
  return CaseIt(this, Num);
}

BasicBlock *SwitchInst::getSuccessor(unsigned Idx) const {
  return cast<BasicBlock>(
      Ctx.getValue(cast<toolchain::SwitchInst>(Val)->getSuccessor(Idx)));
}

void SwitchInst::setSuccessor(unsigned Idx, BasicBlock *NewSucc) {
  Ctx.getTracker()
      .emplaceIfTracking<GenericSetterWithIdx<&SwitchInst::getSuccessor,
                                              &SwitchInst::setSuccessor>>(this,
                                                                          Idx);
  cast<toolchain::SwitchInst>(Val)->setSuccessor(
      Idx, cast<toolchain::BasicBlock>(NewSucc->Val));
}

Value *UnaryOperator::create(Instruction::Opcode Op, Value *OpV,
                             InsertPosition Pos, Context &Ctx,
                             const Twine &Name) {
  auto &Builder = setInsertPos(Pos);
  auto *NewLLVMV = Builder.CreateUnOp(getLLVMUnaryOp(Op), OpV->Val, Name);
  if (auto *NewUnOpV = dyn_cast<toolchain::UnaryOperator>(NewLLVMV)) {
    return Ctx.createUnaryOperator(NewUnOpV);
  }
  assert(isa<toolchain::Constant>(NewLLVMV) && "Expected constant");
  return Ctx.getOrCreateConstant(cast<toolchain::Constant>(NewLLVMV));
}

Value *UnaryOperator::createWithCopiedFlags(Instruction::Opcode Op, Value *OpV,
                                            Value *CopyFrom, InsertPosition Pos,
                                            Context &Ctx, const Twine &Name) {
  auto *NewV = create(Op, OpV, Pos, Ctx, Name);
  if (auto *UnI = dyn_cast<toolchain::UnaryOperator>(NewV->Val))
    UnI->copyIRFlags(CopyFrom->Val);
  return NewV;
}

/// \Returns the LLVM opcode that corresponds to \p Opc.
static toolchain::Instruction::BinaryOps getLLVMBinaryOp(Instruction::Opcode Opc) {
  switch (Opc) {
  case Instruction::Opcode::Add:
    return static_cast<toolchain::Instruction::BinaryOps>(toolchain::Instruction::Add);
  case Instruction::Opcode::FAdd:
    return static_cast<toolchain::Instruction::BinaryOps>(toolchain::Instruction::FAdd);
  case Instruction::Opcode::Sub:
    return static_cast<toolchain::Instruction::BinaryOps>(toolchain::Instruction::Sub);
  case Instruction::Opcode::FSub:
    return static_cast<toolchain::Instruction::BinaryOps>(toolchain::Instruction::FSub);
  case Instruction::Opcode::Mul:
    return static_cast<toolchain::Instruction::BinaryOps>(toolchain::Instruction::Mul);
  case Instruction::Opcode::FMul:
    return static_cast<toolchain::Instruction::BinaryOps>(toolchain::Instruction::FMul);
  case Instruction::Opcode::UDiv:
    return static_cast<toolchain::Instruction::BinaryOps>(toolchain::Instruction::UDiv);
  case Instruction::Opcode::SDiv:
    return static_cast<toolchain::Instruction::BinaryOps>(toolchain::Instruction::SDiv);
  case Instruction::Opcode::FDiv:
    return static_cast<toolchain::Instruction::BinaryOps>(toolchain::Instruction::FDiv);
  case Instruction::Opcode::URem:
    return static_cast<toolchain::Instruction::BinaryOps>(toolchain::Instruction::URem);
  case Instruction::Opcode::SRem:
    return static_cast<toolchain::Instruction::BinaryOps>(toolchain::Instruction::SRem);
  case Instruction::Opcode::FRem:
    return static_cast<toolchain::Instruction::BinaryOps>(toolchain::Instruction::FRem);
  case Instruction::Opcode::Shl:
    return static_cast<toolchain::Instruction::BinaryOps>(toolchain::Instruction::Shl);
  case Instruction::Opcode::LShr:
    return static_cast<toolchain::Instruction::BinaryOps>(toolchain::Instruction::LShr);
  case Instruction::Opcode::AShr:
    return static_cast<toolchain::Instruction::BinaryOps>(toolchain::Instruction::AShr);
  case Instruction::Opcode::And:
    return static_cast<toolchain::Instruction::BinaryOps>(toolchain::Instruction::And);
  case Instruction::Opcode::Or:
    return static_cast<toolchain::Instruction::BinaryOps>(toolchain::Instruction::Or);
  case Instruction::Opcode::Xor:
    return static_cast<toolchain::Instruction::BinaryOps>(toolchain::Instruction::Xor);
  default:
    llvm_unreachable("Not a binary op!");
  }
}
Value *BinaryOperator::create(Instruction::Opcode Op, Value *LHS, Value *RHS,
                              InsertPosition Pos, Context &Ctx,
                              const Twine &Name) {
  auto &Builder = setInsertPos(Pos);
  toolchain::Value *NewV =
      Builder.CreateBinOp(getLLVMBinaryOp(Op), LHS->Val, RHS->Val, Name);
  if (auto *NewBinOp = dyn_cast<toolchain::BinaryOperator>(NewV))
    return Ctx.createBinaryOperator(NewBinOp);
  assert(isa<toolchain::Constant>(NewV) && "Expected constant");
  return Ctx.getOrCreateConstant(cast<toolchain::Constant>(NewV));
}

Value *BinaryOperator::createWithCopiedFlags(Instruction::Opcode Op, Value *LHS,
                                             Value *RHS, Value *CopyFrom,
                                             InsertPosition Pos, Context &Ctx,
                                             const Twine &Name) {

  Value *NewV = create(Op, LHS, RHS, Pos, Ctx, Name);
  if (auto *NewBO = dyn_cast<BinaryOperator>(NewV))
    cast<toolchain::BinaryOperator>(NewBO->Val)->copyIRFlags(CopyFrom->Val);
  return NewV;
}

void PossiblyDisjointInst::setIsDisjoint(bool B) {
  Ctx.getTracker()
      .emplaceIfTracking<GenericSetter<&PossiblyDisjointInst::isDisjoint,
                                       &PossiblyDisjointInst::setIsDisjoint>>(
          this);
  cast<toolchain::PossiblyDisjointInst>(Val)->setIsDisjoint(B);
}

void AtomicRMWInst::setAlignment(Align Align) {
  Ctx.getTracker()
      .emplaceIfTracking<GenericSetter<&AtomicRMWInst::getAlign,
                                       &AtomicRMWInst::setAlignment>>(this);
  cast<toolchain::AtomicRMWInst>(Val)->setAlignment(Align);
}

void AtomicRMWInst::setVolatile(bool V) {
  Ctx.getTracker()
      .emplaceIfTracking<GenericSetter<&AtomicRMWInst::isVolatile,
                                       &AtomicRMWInst::setVolatile>>(this);
  cast<toolchain::AtomicRMWInst>(Val)->setVolatile(V);
}

void AtomicRMWInst::setOrdering(AtomicOrdering Ordering) {
  Ctx.getTracker()
      .emplaceIfTracking<GenericSetter<&AtomicRMWInst::getOrdering,
                                       &AtomicRMWInst::setOrdering>>(this);
  cast<toolchain::AtomicRMWInst>(Val)->setOrdering(Ordering);
}

void AtomicRMWInst::setSyncScopeID(SyncScope::ID SSID) {
  Ctx.getTracker()
      .emplaceIfTracking<GenericSetter<&AtomicRMWInst::getSyncScopeID,
                                       &AtomicRMWInst::setSyncScopeID>>(this);
  cast<toolchain::AtomicRMWInst>(Val)->setSyncScopeID(SSID);
}

Value *AtomicRMWInst::getPointerOperand() {
  return Ctx.getValue(cast<toolchain::AtomicRMWInst>(Val)->getPointerOperand());
}

Value *AtomicRMWInst::getValOperand() {
  return Ctx.getValue(cast<toolchain::AtomicRMWInst>(Val)->getValOperand());
}

AtomicRMWInst *AtomicRMWInst::create(BinOp Op, Value *Ptr, Value *Val,
                                     MaybeAlign Align, AtomicOrdering Ordering,
                                     InsertPosition Pos, Context &Ctx,
                                     SyncScope::ID SSID, const Twine &Name) {
  auto &Builder = setInsertPos(Pos);
  auto *LLVMAtomicRMW =
      Builder.CreateAtomicRMW(Op, Ptr->Val, Val->Val, Align, Ordering, SSID);
  LLVMAtomicRMW->setName(Name);
  return Ctx.createAtomicRMWInst(LLVMAtomicRMW);
}

void AtomicCmpXchgInst::setSyncScopeID(SyncScope::ID SSID) {
  Ctx.getTracker()
      .emplaceIfTracking<GenericSetter<&AtomicCmpXchgInst::getSyncScopeID,
                                       &AtomicCmpXchgInst::setSyncScopeID>>(
          this);
  cast<toolchain::AtomicCmpXchgInst>(Val)->setSyncScopeID(SSID);
}

Value *AtomicCmpXchgInst::getPointerOperand() {
  return Ctx.getValue(cast<toolchain::AtomicCmpXchgInst>(Val)->getPointerOperand());
}

Value *AtomicCmpXchgInst::getCompareOperand() {
  return Ctx.getValue(cast<toolchain::AtomicCmpXchgInst>(Val)->getCompareOperand());
}

Value *AtomicCmpXchgInst::getNewValOperand() {
  return Ctx.getValue(cast<toolchain::AtomicCmpXchgInst>(Val)->getNewValOperand());
}

AtomicCmpXchgInst *
AtomicCmpXchgInst::create(Value *Ptr, Value *Cmp, Value *New, MaybeAlign Align,
                          AtomicOrdering SuccessOrdering,
                          AtomicOrdering FailureOrdering, InsertPosition Pos,
                          Context &Ctx, SyncScope::ID SSID, const Twine &Name) {
  auto &Builder = setInsertPos(Pos);
  auto *LLVMAtomicCmpXchg =
      Builder.CreateAtomicCmpXchg(Ptr->Val, Cmp->Val, New->Val, Align,
                                  SuccessOrdering, FailureOrdering, SSID);
  LLVMAtomicCmpXchg->setName(Name);
  return Ctx.createAtomicCmpXchgInst(LLVMAtomicCmpXchg);
}

void AtomicCmpXchgInst::setAlignment(Align Align) {
  Ctx.getTracker()
      .emplaceIfTracking<GenericSetter<&AtomicCmpXchgInst::getAlign,
                                       &AtomicCmpXchgInst::setAlignment>>(this);
  cast<toolchain::AtomicCmpXchgInst>(Val)->setAlignment(Align);
}

void AtomicCmpXchgInst::setVolatile(bool V) {
  Ctx.getTracker()
      .emplaceIfTracking<GenericSetter<&AtomicCmpXchgInst::isVolatile,
                                       &AtomicCmpXchgInst::setVolatile>>(this);
  cast<toolchain::AtomicCmpXchgInst>(Val)->setVolatile(V);
}

void AtomicCmpXchgInst::setWeak(bool IsWeak) {
  Ctx.getTracker()
      .emplaceIfTracking<GenericSetter<&AtomicCmpXchgInst::isWeak,
                                       &AtomicCmpXchgInst::setWeak>>(this);
  cast<toolchain::AtomicCmpXchgInst>(Val)->setWeak(IsWeak);
}

void AtomicCmpXchgInst::setSuccessOrdering(AtomicOrdering Ordering) {
  Ctx.getTracker()
      .emplaceIfTracking<GenericSetter<&AtomicCmpXchgInst::getSuccessOrdering,
                                       &AtomicCmpXchgInst::setSuccessOrdering>>(
          this);
  cast<toolchain::AtomicCmpXchgInst>(Val)->setSuccessOrdering(Ordering);
}

void AtomicCmpXchgInst::setFailureOrdering(AtomicOrdering Ordering) {
  Ctx.getTracker()
      .emplaceIfTracking<GenericSetter<&AtomicCmpXchgInst::getFailureOrdering,
                                       &AtomicCmpXchgInst::setFailureOrdering>>(
          this);
  cast<toolchain::AtomicCmpXchgInst>(Val)->setFailureOrdering(Ordering);
}

AllocaInst *AllocaInst::create(Type *Ty, unsigned AddrSpace, InsertPosition Pos,
                               Context &Ctx, Value *ArraySize,
                               const Twine &Name) {
  auto &Builder = setInsertPos(Pos);
  auto *NewAlloca =
      Builder.CreateAlloca(Ty->LLVMTy, AddrSpace, ArraySize->Val, Name);
  return Ctx.createAllocaInst(NewAlloca);
}

Type *AllocaInst::getAllocatedType() const {
  return Ctx.getType(cast<toolchain::AllocaInst>(Val)->getAllocatedType());
}

void AllocaInst::setAllocatedType(Type *Ty) {
  Ctx.getTracker()
      .emplaceIfTracking<GenericSetter<&AllocaInst::getAllocatedType,
                                       &AllocaInst::setAllocatedType>>(this);
  cast<toolchain::AllocaInst>(Val)->setAllocatedType(Ty->LLVMTy);
}

void AllocaInst::setAlignment(Align Align) {
  Ctx.getTracker()
      .emplaceIfTracking<
          GenericSetter<&AllocaInst::getAlign, &AllocaInst::setAlignment>>(
          this);
  cast<toolchain::AllocaInst>(Val)->setAlignment(Align);
}

void AllocaInst::setUsedWithInAlloca(bool V) {
  Ctx.getTracker()
      .emplaceIfTracking<GenericSetter<&AllocaInst::isUsedWithInAlloca,
                                       &AllocaInst::setUsedWithInAlloca>>(this);
  cast<toolchain::AllocaInst>(Val)->setUsedWithInAlloca(V);
}

Value *AllocaInst::getArraySize() {
  return Ctx.getValue(cast<toolchain::AllocaInst>(Val)->getArraySize());
}

PointerType *AllocaInst::getType() const {
  return cast<PointerType>(Ctx.getType(cast<toolchain::AllocaInst>(Val)->getType()));
}

Value *CastInst::create(Type *DestTy, Opcode Op, Value *Operand,
                        InsertPosition Pos, Context &Ctx, const Twine &Name) {
  assert(getLLVMCastOp(Op) && "Opcode not suitable for CastInst!");
  auto &Builder = setInsertPos(Pos);
  auto *NewV =
      Builder.CreateCast(getLLVMCastOp(Op), Operand->Val, DestTy->LLVMTy, Name);
  if (auto *NewCI = dyn_cast<toolchain::CastInst>(NewV))
    return Ctx.createCastInst(NewCI);
  assert(isa<toolchain::Constant>(NewV) && "Expected constant");
  return Ctx.getOrCreateConstant(cast<toolchain::Constant>(NewV));
}

bool CastInst::classof(const Value *From) {
  return From->getSubclassID() == ClassID::Cast;
}

Type *CastInst::getSrcTy() const {
  return Ctx.getType(cast<toolchain::CastInst>(Val)->getSrcTy());
}

Type *CastInst::getDestTy() const {
  return Ctx.getType(cast<toolchain::CastInst>(Val)->getDestTy());
}

void PossiblyNonNegInst::setNonNeg(bool B) {
  Ctx.getTracker()
      .emplaceIfTracking<GenericSetter<&PossiblyNonNegInst::hasNonNeg,
                                       &PossiblyNonNegInst::setNonNeg>>(this);
  cast<toolchain::PossiblyNonNegInst>(Val)->setNonNeg(B);
}

Value *InsertElementInst::create(Value *Vec, Value *NewElt, Value *Idx,
                                 InsertPosition Pos, Context &Ctx,
                                 const Twine &Name) {
  auto &Builder = Instruction::setInsertPos(Pos);
  toolchain::Value *NewV =
      Builder.CreateInsertElement(Vec->Val, NewElt->Val, Idx->Val, Name);
  if (auto *NewInsert = dyn_cast<toolchain::InsertElementInst>(NewV))
    return Ctx.createInsertElementInst(NewInsert);
  assert(isa<toolchain::Constant>(NewV) && "Expected constant");
  return Ctx.getOrCreateConstant(cast<toolchain::Constant>(NewV));
}

Value *ExtractElementInst::create(Value *Vec, Value *Idx, InsertPosition Pos,
                                  Context &Ctx, const Twine &Name) {
  auto &Builder = setInsertPos(Pos);
  toolchain::Value *NewV = Builder.CreateExtractElement(Vec->Val, Idx->Val, Name);
  if (auto *NewExtract = dyn_cast<toolchain::ExtractElementInst>(NewV))
    return Ctx.createExtractElementInst(NewExtract);
  assert(isa<toolchain::Constant>(NewV) && "Expected constant");
  return Ctx.getOrCreateConstant(cast<toolchain::Constant>(NewV));
}

Value *ShuffleVectorInst::create(Value *V1, Value *V2, Value *Mask,
                                 InsertPosition Pos, Context &Ctx,
                                 const Twine &Name) {
  auto &Builder = setInsertPos(Pos);
  toolchain::Value *NewV =
      Builder.CreateShuffleVector(V1->Val, V2->Val, Mask->Val, Name);
  if (auto *NewShuffle = dyn_cast<toolchain::ShuffleVectorInst>(NewV))
    return Ctx.createShuffleVectorInst(NewShuffle);
  assert(isa<toolchain::Constant>(NewV) && "Expected constant");
  return Ctx.getOrCreateConstant(cast<toolchain::Constant>(NewV));
}

Value *ShuffleVectorInst::create(Value *V1, Value *V2, ArrayRef<int> Mask,
                                 InsertPosition Pos, Context &Ctx,
                                 const Twine &Name) {
  auto &Builder = setInsertPos(Pos);
  toolchain::Value *NewV = Builder.CreateShuffleVector(V1->Val, V2->Val, Mask, Name);
  if (auto *NewShuffle = dyn_cast<toolchain::ShuffleVectorInst>(NewV))
    return Ctx.createShuffleVectorInst(NewShuffle);
  assert(isa<toolchain::Constant>(NewV) && "Expected constant");
  return Ctx.getOrCreateConstant(cast<toolchain::Constant>(NewV));
}

void ShuffleVectorInst::setShuffleMask(ArrayRef<int> Mask) {
  Ctx.getTracker().emplaceIfTracking<ShuffleVectorSetMask>(this);
  cast<toolchain::ShuffleVectorInst>(Val)->setShuffleMask(Mask);
}

VectorType *ShuffleVectorInst::getType() const {
  return cast<VectorType>(
      Ctx.getType(cast<toolchain::ShuffleVectorInst>(Val)->getType()));
}

void ShuffleVectorInst::commute() {
  Ctx.getTracker().emplaceIfTracking<ShuffleVectorSetMask>(this);
  Ctx.getTracker().emplaceIfTracking<UseSwap>(getOperandUse(0),
                                              getOperandUse(1));
  cast<toolchain::ShuffleVectorInst>(Val)->commute();
}

Constant *ShuffleVectorInst::getShuffleMaskForBitcode() const {
  return Ctx.getOrCreateConstant(
      cast<toolchain::ShuffleVectorInst>(Val)->getShuffleMaskForBitcode());
}

Constant *ShuffleVectorInst::convertShuffleMaskForBitcode(ArrayRef<int> Mask,
                                                          Type *ResultTy) {
  return ResultTy->getContext().getOrCreateConstant(
      toolchain::ShuffleVectorInst::convertShuffleMaskForBitcode(Mask,
                                                            ResultTy->LLVMTy));
}

VectorType *ExtractElementInst::getVectorOperandType() const {
  return cast<VectorType>(Ctx.getType(getVectorOperand()->getType()->LLVMTy));
}

Value *ExtractValueInst::create(Value *Agg, ArrayRef<unsigned> Idxs,
                                InsertPosition Pos, Context &Ctx,
                                const Twine &Name) {
  auto &Builder = setInsertPos(Pos);
  toolchain::Value *NewV = Builder.CreateExtractValue(Agg->Val, Idxs, Name);
  if (auto *NewExtractValueInst = dyn_cast<toolchain::ExtractValueInst>(NewV))
    return Ctx.createExtractValueInst(NewExtractValueInst);
  assert(isa<toolchain::Constant>(NewV) && "Expected constant");
  return Ctx.getOrCreateConstant(cast<toolchain::Constant>(NewV));
}

Type *ExtractValueInst::getIndexedType(Type *Agg, ArrayRef<unsigned> Idxs) {
  auto *LLVMTy = toolchain::ExtractValueInst::getIndexedType(Agg->LLVMTy, Idxs);
  return Agg->getContext().getType(LLVMTy);
}

Value *InsertValueInst::create(Value *Agg, Value *Val, ArrayRef<unsigned> Idxs,
                               InsertPosition Pos, Context &Ctx,
                               const Twine &Name) {
  auto &Builder = setInsertPos(Pos);
  toolchain::Value *NewV = Builder.CreateInsertValue(Agg->Val, Val->Val, Idxs, Name);
  if (auto *NewInsertValueInst = dyn_cast<toolchain::InsertValueInst>(NewV))
    return Ctx.createInsertValueInst(NewInsertValueInst);
  assert(isa<toolchain::Constant>(NewV) && "Expected constant");
  return Ctx.getOrCreateConstant(cast<toolchain::Constant>(NewV));
}

ConstantTokenNone *ConstantTokenNone::get(Context &Ctx) {
  auto *LLVMC = toolchain::ConstantTokenNone::get(Ctx.LLVMCtx);
  return cast<ConstantTokenNone>(Ctx.getOrCreateConstant(LLVMC));
}

} // namespace vm::core::sandboxir
