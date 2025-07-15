//===--- IRBuilder.h - Codira IR Builder -------------------------*- C++ -*-===//
//
// Copyright (c) NeXTHub Corporation. All rights reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// This code is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// version 2 for more details (a copy is included in the LICENSE file that
// accompanied this code).
//
// Author(-s): Tunjay Akbarli
//

//===----------------------------------------------------------------------===//
//
// This file defines Codira's specialization of toolchain::IRBuilder.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_IRGEN_IRBUILDER_H
#define LANGUAGE_IRGEN_IRBUILDER_H

#include "toolchain/ADT/PointerUnion.h"
#include "toolchain/IR/IRBuilder.h"
#include "toolchain/IR/InlineAsm.h"
#include "language/Basic/Toolchain.h"
#include "Address.h"
#include "IRGen.h"

namespace language {
namespace irgen {
class FunctionPointer;
class IRGenModule;

using IRBuilderBase = toolchain::IRBuilder<>;

class IRBuilder : public IRBuilderBase {
public:
  // Without this, it keeps resolving to toolchain::IRBuilderBase because
  // of the injected class name.
  using IRBuilderBase = irgen::IRBuilderBase;

private:
  /// The block containing the insertion point when the insertion
  /// point was last cleared.  Used only for preserving block
  /// ordering.
  toolchain::BasicBlock *ClearedIP;
  unsigned NumTrapBarriers = 0;

#ifndef NDEBUG
  /// Whether debug information is requested. Only used in assertions.
  bool DebugInfo;
#endif

  // Set calling convention of the call instruction using
  // the same calling convention as the callee function.
  // This ensures that they are always compatible.
  void setCallingConvUsingCallee(toolchain::CallBase *Call) {
    auto CalleeFn = Call->getCalledFunction();
    if (CalleeFn) {
      auto CC = CalleeFn->getCallingConv();
      Call->setCallingConv(CC);
    }
  }

public:
  IRBuilder(toolchain::LLVMContext &Context, bool DebugInfo)
    : IRBuilderBase(Context), ClearedIP(nullptr)
#ifndef NDEBUG
    , DebugInfo(DebugInfo)
#endif
    {}

  /// Defined below.
  class SavedInsertionPointRAII;

  /// Determines if the current location is apparently reachable.  The
  /// invariant we maintain is that the insertion point of the builder
  /// always points within a block unless the current location is
  /// logically unreachable.  All the low-level routines which emit
  /// branches leave the insertion point in the original block, just
  /// after the branch.  High-level routines may then indicate
  /// unreachability by clearing the insertion point.
  bool hasValidIP() const { return GetInsertBlock() != nullptr; }

  /// Determines whether we're currently inserting after a terminator.
  /// This is really just there for asserts.
  bool hasPostTerminatorIP() const {
    return GetInsertBlock() != nullptr &&
           !GetInsertBlock()->empty() &&
           GetInsertBlock()->back().isTerminator();
  }

  void ClearInsertionPoint() {
    assert(hasValidIP() && "clearing invalid insertion point!");
    assert(ClearedIP == nullptr);

    /// Whenever we clear the insertion point, remember where we were.
    ClearedIP = GetInsertBlock();
    IRBuilderBase::ClearInsertionPoint();
  }

  void SetInsertPoint(toolchain::BasicBlock *BB) {
    ClearedIP = nullptr;
    IRBuilderBase::SetInsertPoint(BB);
  }
  
  void SetInsertPoint(toolchain::BasicBlock *BB, toolchain::BasicBlock::iterator before) {
    ClearedIP = nullptr;
    IRBuilderBase::SetInsertPoint(BB, before);
  }

  void SetInsertPoint(toolchain::Instruction *I) {
    ClearedIP = nullptr;
    IRBuilderBase::SetInsertPoint(I);
  }

  /// Return the LLVM module we're inserting into.
  toolchain::Module *getModule() const {
    if (auto BB = GetInsertBlock())
      return BB->getModule();
    assert(ClearedIP && "IRBuilder has no active or cleared insertion block");
    return ClearedIP->getModule();
  }

  using IRBuilderBase::CreateAnd;
  toolchain::Value *CreateAnd(toolchain::Value *LHS, toolchain::Value *RHS,
                         const Twine &Name = "") {
    if (auto *RC = dyn_cast<toolchain::Constant>(RHS))
      if (isa<toolchain::ConstantInt>(RC) &&
          cast<toolchain::ConstantInt>(RC)->isMinusOne())
        return LHS;  // LHS & -1 -> LHS
     return IRBuilderBase::CreateAnd(LHS, RHS, Name);
  }
  toolchain::Value *CreateAnd(toolchain::Value *LHS, const APInt &RHS,
                         const Twine &Name = "") {
    return CreateAnd(LHS, toolchain::ConstantInt::get(LHS->getType(), RHS), Name);
  }
  toolchain::Value *CreateAnd(toolchain::Value *LHS, uint64_t RHS, const Twine &Name = "") {
    return CreateAnd(LHS, toolchain::ConstantInt::get(LHS->getType(), RHS), Name);
  }

  using IRBuilderBase::CreateOr;
  toolchain::Value *CreateOr(toolchain::Value *LHS, toolchain::Value *RHS,
                        const Twine &Name = "") {
    if (auto *RC = dyn_cast<toolchain::Constant>(RHS))
      if (RC->isNullValue())
        return LHS;  // LHS | 0 -> LHS
    return IRBuilderBase::CreateOr(LHS, RHS, Name);
  }
  toolchain::Value *CreateOr(toolchain::Value *LHS, const APInt &RHS,
                        const Twine &Name = "") {
    return CreateOr(LHS, toolchain::ConstantInt::get(LHS->getType(), RHS), Name);
  }
  toolchain::Value *CreateOr(toolchain::Value *LHS, uint64_t RHS,
                        const Twine &Name = "") {
    return CreateOr(LHS, toolchain::ConstantInt::get(LHS->getType(), RHS), Name);
  }

  /// Don't create allocas this way; you'll get a dynamic alloca.
  /// Use IGF::createAlloca or IGF::emitDynamicAlloca.
  toolchain::Value *CreateAlloca(toolchain::Type *type, toolchain::Value *arraySize,
                            const toolchain::Twine &name = "") = delete;

  toolchain::LoadInst *CreateLoad(toolchain::Value *addr, toolchain::Type *elementType,
                             Alignment align, const toolchain::Twine &name = "") {
    toolchain::LoadInst *load = IRBuilderBase::CreateLoad(elementType, addr, name);
    load->setAlignment(toolchain::MaybeAlign(align.getValue()).valueOrOne());
    return load;
  }
  toolchain::LoadInst *CreateLoad(Address addr, const toolchain::Twine &name = "") {
    return CreateLoad(addr.getAddress(), addr.getElementType(),
                      addr.getAlignment(), name);
  }

  toolchain::StoreInst *CreateStore(toolchain::Value *value, toolchain::Value *addr,
                               Alignment align) {
    toolchain::StoreInst *store = IRBuilderBase::CreateStore(value, addr);
    store->setAlignment(toolchain::MaybeAlign(align.getValue()).valueOrOne());
    return store;
  }
  toolchain::StoreInst *CreateStore(toolchain::Value *value, Address addr) {
    return CreateStore(value, addr.getAddress(), addr.getAlignment());
  }

  // These are deleted because we want to force the caller to specify
  // an alignment.
  toolchain::LoadInst *CreateLoad(toolchain::Value *addr,
                             const toolchain::Twine &name = "") = delete;
  toolchain::StoreInst *CreateStore(toolchain::Value *value, toolchain::Value *addr) = delete;
  
  using IRBuilderBase::CreateStructGEP;
  Address CreateStructGEP(Address address, unsigned index, Size offset,
                          const toolchain::Twine &name = "") {
    assert(isa<toolchain::StructType>(address.getElementType()) ||
           isa<toolchain::ArrayType>(address.getElementType()));

    toolchain::Value *addr = CreateStructGEP(address.getElementType(),
                                        address.getAddress(), index, name);
    toolchain::Type *elementType = nullptr;
    if (auto *structTy = dyn_cast<toolchain::StructType>(address.getElementType())) {
      elementType = structTy->getElementType(index);
    } else if (auto *arrTy =
                   dyn_cast<toolchain::ArrayType>(address.getElementType())) {
      elementType = arrTy->getElementType();
    }

    return Address(addr, elementType,
                   address.getAlignment().alignmentAtOffset(offset));
  }
  Address CreateStructGEP(Address address, unsigned index,
                          const toolchain::StructLayout *layout,
                          const toolchain::Twine &name = "") {
    Size offset = Size(layout->getElementOffset(index));
    return CreateStructGEP(address, index, offset, name);
  }

  /// Given a pointer to an array element, GEP to the array element
  /// N elements past it.  The type is not changed.
  Address CreateConstArrayGEP(Address base, unsigned index, Size eltSize,
                              const toolchain::Twine &name = "") {
    auto addr = CreateConstInBoundsGEP1_32(base.getElementType(),
                                           base.getAddress(), index, name);
    return Address(addr, base.getElementType(),
                   base.getAlignment().alignmentAtOffset(eltSize * index));
  }

  /// Given a pointer to an array element, GEP to the array element
  /// N elements past it.  The type is not changed.
  Address CreateArrayGEP(Address base, toolchain::Value *index, Size eltSize,
                         const toolchain::Twine &name = "") {
    auto addr = CreateInBoundsGEP(base.getElementType(),
                                  base.getAddress(), index, name);
    // Given that Alignment doesn't remember offset alignment,
    // the alignment at index 1 should be conservatively correct for
    // any element in the array.
    return Address(addr, base.getElementType(),
                   base.getAlignment().alignmentAtOffset(eltSize));
  }

  /// Given an i8*, GEP to N bytes past it.
  Address CreateConstByteArrayGEP(Address base, Size offset,
                                  const toolchain::Twine &name = "") {
    auto addr = CreateConstInBoundsGEP1_32(
        base.getElementType(), base.getAddress(), offset.getValue(), name);
    return Address(addr, base.getElementType(),
                   base.getAlignment().alignmentAtOffset(offset));
  }

  using IRBuilderBase::CreateBitCast;

  /// Cast the given address to be a pointer to the given element type,
  /// preserving the original address space.
  Address CreateElementBitCast(Address address, toolchain::Type *type,
                               const toolchain::Twine &name = "") {
    // Do nothing if the type doesn't change.
    if (address.getElementType() == type) {
      return address;
    }

    // Otherwise, cast to a pointer to the correct type.
    auto origPtrType = address.getType();
    return Address(
        CreateBitCast(address.getAddress(),
                      type->getPointerTo(origPtrType->getAddressSpace())),
        type, address.getAlignment());
  }

  /// Insert the given basic block after the IP block and move the
  /// insertion point to it.  Only valid if the IP is valid.
  void emitBlock(toolchain::BasicBlock *BB);

  using IRBuilderBase::CreateMemCpy;
  toolchain::CallInst *CreateMemCpy(Address dest, Address src, Size size) {
    return CreateMemCpy(
        dest.getAddress(), toolchain::MaybeAlign(dest.getAlignment().getValue()),
        src.getAddress(), toolchain::MaybeAlign(src.getAlignment().getValue()),
        size.getValue());
  }

  toolchain::CallInst *CreateMemCpy(Address dest, Address src, toolchain::Value *size) {
    return CreateMemCpy(dest.getAddress(),
                        toolchain::MaybeAlign(dest.getAlignment().getValue()),
                        src.getAddress(),
                        toolchain::MaybeAlign(src.getAlignment().getValue()), size);
  }

  using IRBuilderBase::CreateMemSet;
  toolchain::CallInst *CreateMemSet(Address dest, toolchain::Value *value, Size size) {
    return CreateMemSet(dest.getAddress(), value, size.getValue(),
                        toolchain::MaybeAlign(dest.getAlignment().getValue()));
  }
  toolchain::CallInst *CreateMemSet(Address dest, toolchain::Value *value,
                               toolchain::Value *size) {
    return CreateMemSet(dest.getAddress(), value, size,
                        toolchain::MaybeAlign(dest.getAlignment().getValue()));
  }
  
  using IRBuilderBase::CreateLifetimeStart;
  toolchain::CallInst *CreateLifetimeStart(Address buf, Size size) {
    return CreateLifetimeStart(buf.getAddress(),
                   toolchain::ConstantInt::get(Context, APInt(64, size.getValue())));
  }
  
  using IRBuilderBase::CreateLifetimeEnd;
  toolchain::CallInst *CreateLifetimeEnd(Address buf, Size size) {
    return CreateLifetimeEnd(buf.getAddress(),
                   toolchain::ConstantInt::get(Context, APInt(64, size.getValue())));
  }

  // We're intentionally not allowing direct use of
  // toolchain::IRBuilder::CreateCall in order to push code towards using
  // FunctionPointer.

  bool isTrapIntrinsic(toolchain::Value *Callee) {
    return Callee ==
           toolchain::Intrinsic::getDeclaration(getModule(), toolchain::Intrinsic::trap);
  }
  bool isTrapIntrinsic(toolchain::Intrinsic::ID intrinsicID) {
    return intrinsicID == toolchain::Intrinsic::trap;
  }

  toolchain::CallInst *CreateCall(toolchain::Value *Callee, ArrayRef<toolchain::Value *> Args,
                             const Twine &Name = "",
                             toolchain::MDNode *FPMathTag = nullptr) = delete;

  toolchain::CallInst *CreateCall(toolchain::FunctionType *FTy, toolchain::Constant *Callee,
                             ArrayRef<toolchain::Value *> Args,
                             const Twine &Name = "",
                             toolchain::MDNode *FPMathTag = nullptr) {
    assert((!DebugInfo || getCurrentDebugLocation()) && "no debugloc on call");
    assert(!isTrapIntrinsic(Callee) && "Use CreateNonMergeableTrap");
    auto Call = IRBuilderBase::CreateCall(FTy, Callee, Args, Name, FPMathTag);
    setCallingConvUsingCallee(Call);
    return Call;
  }
  toolchain::CallInst *CreateCallWithoutDbgLoc(toolchain::FunctionType *FTy,
                                          toolchain::Constant *Callee,
                                          ArrayRef<toolchain::Value *> Args,
                                          const Twine &Name = "",
                                          toolchain::MDNode *FPMathTag = nullptr) {
    // assert((!DebugInfo || getCurrentDebugLocation()) && "no debugloc on
    // call");
    assert(!isTrapIntrinsic(Callee) && "Use CreateNonMergeableTrap");
    auto Call = IRBuilderBase::CreateCall(FTy, Callee, Args, Name, FPMathTag);
    setCallingConvUsingCallee(Call);
    return Call;
  }

  toolchain::InvokeInst *
  createInvoke(toolchain::FunctionType *fTy, toolchain::Constant *callee,
               ArrayRef<toolchain::Value *> args, toolchain::BasicBlock *invokeNormalDest,
               toolchain::BasicBlock *invokeUnwindDest, const Twine &name = "") {
    assert((!DebugInfo || getCurrentDebugLocation()) && "no debugloc on call");
    auto call = IRBuilderBase::CreateInvoke(fTy, callee, invokeNormalDest,
                                            invokeUnwindDest, args, name);
    setCallingConvUsingCallee(call);
    return call;
  }

  toolchain::CallBase *CreateCallOrInvoke(const FunctionPointer &fn,
                                     ArrayRef<toolchain::Value *> args,
                                     toolchain::BasicBlock *invokeNormalDest,
                                     toolchain::BasicBlock *invokeUnwindDest);

  toolchain::CallInst *CreateCall(const FunctionPointer &fn,
                             ArrayRef<toolchain::Value *> args);

  toolchain::CallInst *CreateCall(const FunctionPointer &fn,
                             ArrayRef<toolchain::Value *> args, const Twine &Name) {
    auto c = CreateCall(fn, args);
    c->setName(Name);
    return c;
  }

  toolchain::CallInst *CreateAsmCall(toolchain::InlineAsm *asmBlock,
                                ArrayRef<toolchain::Value *> args) {
    return IRBuilderBase::CreateCall(asmBlock, args);
  }

  /// Call an intrinsic with no type arguments.
  toolchain::CallInst *CreateIntrinsicCall(toolchain::Intrinsic::ID intrinsicID,
                                      ArrayRef<toolchain::Value *> args,
                                      const Twine &name = "") {
    assert(!isTrapIntrinsic(intrinsicID) && "Use CreateNonMergeableTrap");
    auto intrinsicFn =
      toolchain::Intrinsic::getDeclaration(getModule(), intrinsicID);
    return CreateCallWithoutDbgLoc(
        cast<toolchain::FunctionType>(intrinsicFn->getValueType()), intrinsicFn,
        args, name);
  }

  /// Call an intrinsic with type arguments.
  toolchain::CallInst *CreateIntrinsicCall(toolchain::Intrinsic::ID intrinsicID,
                                      ArrayRef<toolchain::Type*> typeArgs,
                                      ArrayRef<toolchain::Value *> args,
                                      const Twine &name = "") {
    assert(!isTrapIntrinsic(intrinsicID) && "Use CreateNonMergeableTrap");
    auto intrinsicFn =
      toolchain::Intrinsic::getDeclaration(getModule(), intrinsicID, typeArgs);
    return CreateCallWithoutDbgLoc(
        cast<toolchain::FunctionType>(intrinsicFn->getValueType()), intrinsicFn,
        args, name);
  }
  
  /// Create an expect intrinsic call.
  toolchain::CallInst *CreateExpect(toolchain::Value *value,
                               toolchain::Value *expected,
                               const Twine &name = "") {
    return CreateIntrinsicCall(toolchain::Intrinsic::expect,
                               {value->getType()},
                               {value, expected},
                               name);
  }

  // Creates an @toolchain.expect.i1 call, where the value should be an i1 type.
  toolchain::CallInst *CreateExpectCond(IRGenModule &IGM,
                                   toolchain::Value *value,
                                   bool expectedValue, const Twine &name = "");

  /// Call the trap intrinsic. If optimizations are enabled, an inline asm
  /// gadget is emitted before the trap. The gadget inhibits transforms which
  /// merge trap calls together, which makes debugging crashes easier.
  toolchain::CallInst *CreateNonMergeableTrap(IRGenModule &IGM, StringRef failureMsg);

  /// Split a first-class aggregate value into its component pieces.
  template <unsigned N>
  std::array<toolchain::Value *, N> CreateSplit(toolchain::Value *aggregate) {
    assert(isa<toolchain::StructType>(aggregate->getType()));
    assert(cast<toolchain::StructType>(aggregate->getType())->getNumElements() == N);
    std::array<toolchain::Value *, N> results;
    for (unsigned i = 0; i != N; ++i) {
      results[i] = CreateExtractValue(aggregate, i);
    }
    return results;
  }

  /// Combine the given values into a first-class aggregate.
  toolchain::Value *CreateCombine(toolchain::StructType *aggregateType,
                             ArrayRef<toolchain::Value*> values) {
    assert(aggregateType->getNumElements() == values.size());
    toolchain::Value *result = toolchain::UndefValue::get(aggregateType);
    for (unsigned i = 0, e = values.size(); i != e; ++i) {
      result = CreateInsertValue(result, values[i], i);
    }
    return result;
  }

  bool insertingAtEndOfBlock() const {
    assert(hasValidIP() && "Must have insertion point to ask about it");
    return InsertPt == BB->end();
  }
};

/// Given a Builder as input to its constructor, this class resets the Builder
/// so it has the same insertion point at end of scope.
class IRBuilder::SavedInsertionPointRAII {
  IRBuilder &builder;
  PointerUnion<toolchain::Instruction *, toolchain::BasicBlock *> savedInsertionPoint;

public:
  /// Constructor that saves a Builder's insertion point without changing the
  /// builder's underlying insertion point.
  SavedInsertionPointRAII(IRBuilder &inputBuilder)
      : builder(inputBuilder), savedInsertionPoint() {
    // If our builder does not have a valid insertion point, just put nullptr
    // into SavedIP.
    if (!builder.hasValidIP()) {
      savedInsertionPoint = static_cast<toolchain::BasicBlock *>(nullptr);
      return;
    }

    // If we are inserting into the end of the block, stash the insertion block.
    if (builder.insertingAtEndOfBlock()) {
      savedInsertionPoint = builder.GetInsertBlock();
      return;
    }

    // Otherwise, stash the instruction.
    auto *i = &*builder.GetInsertPoint();
    savedInsertionPoint = i;
  }

  SavedInsertionPointRAII(IRBuilder &b, toolchain::Instruction *newInsertionPoint)
      : SavedInsertionPointRAII(b) {
    builder.SetInsertPoint(newInsertionPoint);
  }

  SavedInsertionPointRAII(IRBuilder &b, toolchain::BasicBlock *block,
                          toolchain::BasicBlock::iterator iter)
      : SavedInsertionPointRAII(b) {
    builder.SetInsertPoint(block, iter);
  }

  SavedInsertionPointRAII(IRBuilder &b, toolchain::BasicBlock *insertionBlock)
      : SavedInsertionPointRAII(b) {
    builder.SetInsertPoint(insertionBlock);
  }

  SavedInsertionPointRAII(const SavedInsertionPointRAII &) = delete;
  SavedInsertionPointRAII &operator=(const SavedInsertionPointRAII &) = delete;
  SavedInsertionPointRAII(SavedInsertionPointRAII &&) = delete;
  SavedInsertionPointRAII &operator=(SavedInsertionPointRAII &&) = delete;

  ~SavedInsertionPointRAII() {
    if (savedInsertionPoint.isNull()) {
      builder.ClearInsertionPoint();
    } else if (savedInsertionPoint.is<toolchain::Instruction *>()) {
      builder.SetInsertPoint(savedInsertionPoint.get<toolchain::Instruction *>());
    } else {
      builder.SetInsertPoint(savedInsertionPoint.get<toolchain::BasicBlock *>());
    }
  }
};

} // end namespace irgen
} // end namespace language

#endif
