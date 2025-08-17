//===-- CGBuilder.h - Choose IRBuilder implementation  ----------*- C++ -*-===//
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

#ifndef LANGUAGE_CORE_LIB_CODEGEN_CGBUILDER_H
#define LANGUAGE_CORE_LIB_CODEGEN_CGBUILDER_H

#include "Address.h"
#include "CGValue.h"
#include "CodeGenTypeCache.h"
#include "toolchain/Analysis/Utils/Local.h"
#include "toolchain/IR/DataLayout.h"
#include "toolchain/IR/GEPNoWrapFlags.h"
#include "toolchain/IR/IRBuilder.h"
#include "toolchain/IR/Type.h"

namespace language::Core {
namespace CodeGen {

class CGBuilderTy;
class CodeGenFunction;

/// This is an IRBuilder insertion helper that forwards to
/// CodeGenFunction::InsertHelper, which adds necessary metadata to
/// instructions.
class CGBuilderInserter final : public toolchain::IRBuilderDefaultInserter {
  friend CGBuilderTy;

public:
  CGBuilderInserter() = default;
  explicit CGBuilderInserter(CodeGenFunction *CGF) : CGF(CGF) {}

  /// This forwards to CodeGenFunction::InsertHelper.
  void InsertHelper(toolchain::Instruction *I, const toolchain::Twine &Name,
                    toolchain::BasicBlock::iterator InsertPt) const override;

private:
  CodeGenFunction *CGF = nullptr;
};

typedef CGBuilderInserter CGBuilderInserterTy;

typedef toolchain::IRBuilder<toolchain::ConstantFolder, CGBuilderInserterTy>
    CGBuilderBaseTy;

class CGBuilderTy : public CGBuilderBaseTy {
  friend class Address;

  /// Storing a reference to the type cache here makes it a lot easier
  /// to build natural-feeling, target-specific IR.
  const CodeGenTypeCache &TypeCache;

  CodeGenFunction *getCGF() const { return getInserter().CGF; }

  toolchain::Value *emitRawPointerFromAddress(Address Addr) const {
    return Addr.getBasePointer();
  }

  template <bool IsInBounds>
  Address createConstGEP2_32(Address Addr, unsigned Idx0, unsigned Idx1,
                             const toolchain::Twine &Name) {
    const toolchain::DataLayout &DL = BB->getDataLayout();
    toolchain::Value *V;
    if (IsInBounds)
      V = CreateConstInBoundsGEP2_32(Addr.getElementType(),
                                     emitRawPointerFromAddress(Addr), Idx0,
                                     Idx1, Name);
    else
      V = CreateConstGEP2_32(Addr.getElementType(),
                             emitRawPointerFromAddress(Addr), Idx0, Idx1, Name);
    toolchain::APInt Offset(
        DL.getIndexSizeInBits(Addr.getType()->getPointerAddressSpace()), 0,
        /*isSigned=*/true);
    if (!toolchain::GEPOperator::accumulateConstantOffset(
            Addr.getElementType(), {getInt32(Idx0), getInt32(Idx1)}, DL,
            Offset))
      toolchain_unreachable(
          "accumulateConstantOffset with constant indices should not fail.");
    toolchain::Type *ElementTy = toolchain::GetElementPtrInst::getIndexedType(
        Addr.getElementType(), {Idx0, Idx1});
    return Address(V, ElementTy,
                   Addr.getAlignment().alignmentAtOffset(
                       CharUnits::fromQuantity(Offset.getSExtValue())),
                   IsInBounds ? Addr.isKnownNonNull() : NotKnownNonNull);
  }

public:
  CGBuilderTy(const CodeGenTypeCache &TypeCache, toolchain::LLVMContext &C)
      : CGBuilderBaseTy(C), TypeCache(TypeCache) {}
  CGBuilderTy(const CodeGenTypeCache &TypeCache, toolchain::LLVMContext &C,
              const toolchain::ConstantFolder &F,
              const CGBuilderInserterTy &Inserter)
      : CGBuilderBaseTy(C, F, Inserter), TypeCache(TypeCache) {}
  CGBuilderTy(const CodeGenTypeCache &TypeCache, toolchain::Instruction *I)
      : CGBuilderBaseTy(I), TypeCache(TypeCache) {}
  CGBuilderTy(const CodeGenTypeCache &TypeCache, toolchain::BasicBlock *BB)
      : CGBuilderBaseTy(BB), TypeCache(TypeCache) {}

  toolchain::ConstantInt *getSize(CharUnits N) {
    return toolchain::ConstantInt::get(TypeCache.SizeTy, N.getQuantity());
  }
  toolchain::ConstantInt *getSize(uint64_t N) {
    return toolchain::ConstantInt::get(TypeCache.SizeTy, N);
  }

  // Note that we intentionally hide the CreateLoad APIs that don't
  // take an alignment.
  toolchain::LoadInst *CreateLoad(Address Addr, const toolchain::Twine &Name = "") {
    return CreateAlignedLoad(Addr.getElementType(),
                             emitRawPointerFromAddress(Addr),
                             Addr.getAlignment().getAsAlign(), Name);
  }
  toolchain::LoadInst *CreateLoad(Address Addr, const char *Name) {
    // This overload is required to prevent string literals from
    // ending up in the IsVolatile overload.
    return CreateAlignedLoad(Addr.getElementType(),
                             emitRawPointerFromAddress(Addr),
                             Addr.getAlignment().getAsAlign(), Name);
  }
  toolchain::LoadInst *CreateLoad(Address Addr, bool IsVolatile,
                             const toolchain::Twine &Name = "") {
    return CreateAlignedLoad(
        Addr.getElementType(), emitRawPointerFromAddress(Addr),
        Addr.getAlignment().getAsAlign(), IsVolatile, Name);
  }

  using CGBuilderBaseTy::CreateAlignedLoad;
  toolchain::LoadInst *CreateAlignedLoad(toolchain::Type *Ty, toolchain::Value *Addr,
                                    CharUnits Align,
                                    const toolchain::Twine &Name = "") {
    return CreateAlignedLoad(Ty, Addr, Align.getAsAlign(), Name);
  }

  // Note that we intentionally hide the CreateStore APIs that don't
  // take an alignment.
  toolchain::StoreInst *CreateStore(toolchain::Value *Val, Address Addr,
                               bool IsVolatile = false) {
    return CreateAlignedStore(Val, emitRawPointerFromAddress(Addr),
                              Addr.getAlignment().getAsAlign(), IsVolatile);
  }

  using CGBuilderBaseTy::CreateAlignedStore;
  toolchain::StoreInst *CreateAlignedStore(toolchain::Value *Val, toolchain::Value *Addr,
                                      CharUnits Align,
                                      bool IsVolatile = false) {
    return CreateAlignedStore(Val, Addr, Align.getAsAlign(), IsVolatile);
  }

  // FIXME: these "default-aligned" APIs should be removed,
  // but I don't feel like fixing all the builtin code right now.
  toolchain::StoreInst *CreateDefaultAlignedStore(toolchain::Value *Val,
                                             toolchain::Value *Addr,
                                             bool IsVolatile = false) {
    return CGBuilderBaseTy::CreateStore(Val, Addr, IsVolatile);
  }

  /// Emit a load from an i1 flag variable.
  toolchain::LoadInst *CreateFlagLoad(toolchain::Value *Addr,
                                 const toolchain::Twine &Name = "") {
    return CreateAlignedLoad(getInt1Ty(), Addr, CharUnits::One(), Name);
  }

  /// Emit a store to an i1 flag variable.
  toolchain::StoreInst *CreateFlagStore(bool Value, toolchain::Value *Addr) {
    return CreateAlignedStore(getInt1(Value), Addr, CharUnits::One());
  }

  toolchain::AtomicCmpXchgInst *
  CreateAtomicCmpXchg(Address Addr, toolchain::Value *Cmp, toolchain::Value *New,
                      toolchain::AtomicOrdering SuccessOrdering,
                      toolchain::AtomicOrdering FailureOrdering,
                      toolchain::SyncScope::ID SSID = toolchain::SyncScope::System) {
    return CGBuilderBaseTy::CreateAtomicCmpXchg(
        Addr.emitRawPointer(*getCGF()), Cmp, New,
        Addr.getAlignment().getAsAlign(), SuccessOrdering, FailureOrdering,
        SSID);
  }

  toolchain::AtomicRMWInst *
  CreateAtomicRMW(toolchain::AtomicRMWInst::BinOp Op, Address Addr, toolchain::Value *Val,
                  toolchain::AtomicOrdering Ordering,
                  toolchain::SyncScope::ID SSID = toolchain::SyncScope::System) {
    return CGBuilderBaseTy::CreateAtomicRMW(
        Op, Addr.emitRawPointer(*getCGF()), Val,
        Addr.getAlignment().getAsAlign(), Ordering, SSID);
  }

  using CGBuilderBaseTy::CreateAddrSpaceCast;
  Address CreateAddrSpaceCast(Address Addr, toolchain::Type *Ty,
                              toolchain::Type *ElementTy,
                              const toolchain::Twine &Name = "") {
    if (!Addr.hasOffset())
      return Address(CreateAddrSpaceCast(Addr.getBasePointer(), Ty, Name),
                     ElementTy, Addr.getAlignment(), Addr.getPointerAuthInfo(),
                     /*Offset=*/nullptr, Addr.isKnownNonNull());
    // Eagerly force a raw address if these is an offset.
    return RawAddress(
        CreateAddrSpaceCast(Addr.emitRawPointer(*getCGF()), Ty, Name),
        ElementTy, Addr.getAlignment(), Addr.isKnownNonNull());
  }

  using CGBuilderBaseTy::CreatePointerBitCastOrAddrSpaceCast;
  Address CreatePointerBitCastOrAddrSpaceCast(Address Addr, toolchain::Type *Ty,
                                              toolchain::Type *ElementTy,
                                              const toolchain::Twine &Name = "") {
    if (Addr.getType()->getAddressSpace() == Ty->getPointerAddressSpace())
      return Addr.withElementType(ElementTy);
    return CreateAddrSpaceCast(Addr, Ty, ElementTy, Name);
  }

  /// Given
  ///   %addr = {T1, T2...}* ...
  /// produce
  ///   %name = getelementptr inbounds nuw %addr, i32 0, i32 index
  ///
  /// This API assumes that drilling into a struct like this is always an
  /// inbounds and nuw operation.
  using CGBuilderBaseTy::CreateStructGEP;
  Address CreateStructGEP(Address Addr, unsigned Index,
                          const toolchain::Twine &Name = "") {
    toolchain::StructType *ElTy = cast<toolchain::StructType>(Addr.getElementType());
    const toolchain::DataLayout &DL = BB->getDataLayout();
    const toolchain::StructLayout *Layout = DL.getStructLayout(ElTy);
    auto Offset = CharUnits::fromQuantity(Layout->getElementOffset(Index));

    return Address(CreateStructGEP(Addr.getElementType(), Addr.getBasePointer(),
                                   Index, Name),
                   ElTy->getElementType(Index),
                   Addr.getAlignment().alignmentAtOffset(Offset),
                   Addr.isKnownNonNull());
  }

  /// Given
  ///   %addr = [n x T]* ...
  /// produce
  ///   %name = getelementptr inbounds %addr, i64 0, i64 index
  /// where i64 is actually the target word size.
  ///
  /// This API assumes that drilling into an array like this is always
  /// an inbounds operation.
  Address CreateConstArrayGEP(Address Addr, uint64_t Index,
                              const toolchain::Twine &Name = "") {
    toolchain::ArrayType *ElTy = cast<toolchain::ArrayType>(Addr.getElementType());
    const toolchain::DataLayout &DL = BB->getDataLayout();
    CharUnits EltSize =
        CharUnits::fromQuantity(DL.getTypeAllocSize(ElTy->getElementType()));

    return Address(
        CreateInBoundsGEP(Addr.getElementType(), Addr.getBasePointer(),
                          {getSize(CharUnits::Zero()), getSize(Index)}, Name),
        ElTy->getElementType(),
        Addr.getAlignment().alignmentAtOffset(Index * EltSize),
        Addr.isKnownNonNull());
  }

  /// Given
  ///   %addr = T* ...
  /// produce
  ///   %name = getelementptr inbounds %addr, i64 index
  /// where i64 is actually the target word size.
  Address CreateConstInBoundsGEP(Address Addr, uint64_t Index,
                                 const toolchain::Twine &Name = "") {
    toolchain::Type *ElTy = Addr.getElementType();
    const toolchain::DataLayout &DL = BB->getDataLayout();
    CharUnits EltSize = CharUnits::fromQuantity(DL.getTypeAllocSize(ElTy));

    return Address(
        CreateInBoundsGEP(ElTy, Addr.getBasePointer(), getSize(Index), Name),
        ElTy, Addr.getAlignment().alignmentAtOffset(Index * EltSize),
        Addr.isKnownNonNull());
  }

  /// Given
  ///   %addr = T* ...
  /// produce
  ///   %name = getelementptr inbounds %addr, i64 index
  /// where i64 is actually the target word size.
  Address CreateConstGEP(Address Addr, uint64_t Index,
                         const toolchain::Twine &Name = "") {
    toolchain::Type *ElTy = Addr.getElementType();
    const toolchain::DataLayout &DL = BB->getDataLayout();
    CharUnits EltSize = CharUnits::fromQuantity(DL.getTypeAllocSize(ElTy));

    return Address(CreateGEP(ElTy, Addr.getBasePointer(), getSize(Index), Name),
                   Addr.getElementType(),
                   Addr.getAlignment().alignmentAtOffset(Index * EltSize));
  }

  /// Create GEP with single dynamic index. The address alignment is reduced
  /// according to the element size.
  using CGBuilderBaseTy::CreateGEP;
  Address CreateGEP(CodeGenFunction &CGF, Address Addr, toolchain::Value *Index,
                    const toolchain::Twine &Name = "") {
    const toolchain::DataLayout &DL = BB->getDataLayout();
    CharUnits EltSize =
        CharUnits::fromQuantity(DL.getTypeAllocSize(Addr.getElementType()));

    return Address(
        CreateGEP(Addr.getElementType(), Addr.emitRawPointer(CGF), Index, Name),
        Addr.getElementType(),
        Addr.getAlignment().alignmentOfArrayElement(EltSize));
  }

  /// Given a pointer to i8, adjust it by a given constant offset.
  Address CreateConstInBoundsByteGEP(Address Addr, CharUnits Offset,
                                     const toolchain::Twine &Name = "") {
    assert(Addr.getElementType() == TypeCache.Int8Ty);
    return Address(
        CreateInBoundsGEP(Addr.getElementType(), Addr.getBasePointer(),
                          getSize(Offset), Name),
        Addr.getElementType(), Addr.getAlignment().alignmentAtOffset(Offset),
        Addr.isKnownNonNull());
  }

  Address CreateConstByteGEP(Address Addr, CharUnits Offset,
                             const toolchain::Twine &Name = "") {
    assert(Addr.getElementType() == TypeCache.Int8Ty);
    return Address(CreateGEP(Addr.getElementType(), Addr.getBasePointer(),
                             getSize(Offset), Name),
                   Addr.getElementType(),
                   Addr.getAlignment().alignmentAtOffset(Offset));
  }

  using CGBuilderBaseTy::CreateConstInBoundsGEP2_32;
  Address CreateConstInBoundsGEP2_32(Address Addr, unsigned Idx0, unsigned Idx1,
                                     const toolchain::Twine &Name = "") {
    return createConstGEP2_32<true>(Addr, Idx0, Idx1, Name);
  }

  using CGBuilderBaseTy::CreateConstGEP2_32;
  Address CreateConstGEP2_32(Address Addr, unsigned Idx0, unsigned Idx1,
                             const toolchain::Twine &Name = "") {
    return createConstGEP2_32<false>(Addr, Idx0, Idx1, Name);
  }

  Address CreateGEP(Address Addr, ArrayRef<toolchain::Value *> IdxList,
                    toolchain::Type *ElementType, CharUnits Align,
                    const Twine &Name = "",
                    toolchain::GEPNoWrapFlags NW = toolchain::GEPNoWrapFlags::none()) {
    toolchain::Value *Ptr = emitRawPointerFromAddress(Addr);
    return RawAddress(CreateGEP(Addr.getElementType(), Ptr, IdxList, Name, NW),
                      ElementType, Align);
  }

  using CGBuilderBaseTy::CreateInBoundsGEP;
  Address CreateInBoundsGEP(Address Addr, ArrayRef<toolchain::Value *> IdxList,
                            toolchain::Type *ElementType, CharUnits Align,
                            const Twine &Name = "") {
    return RawAddress(CreateInBoundsGEP(Addr.getElementType(),
                                        emitRawPointerFromAddress(Addr),
                                        IdxList, Name),
                      ElementType, Align, Addr.isKnownNonNull());
  }

  using CGBuilderBaseTy::CreateIsNull;
  toolchain::Value *CreateIsNull(Address Addr, const Twine &Name = "") {
    if (!Addr.hasOffset())
      return CreateIsNull(Addr.getBasePointer(), Name);
    // The pointer isn't null if Addr has an offset since offsets can always
    // be applied inbound.
    return toolchain::ConstantInt::getFalse(Context);
  }

  using CGBuilderBaseTy::CreateMemCpy;
  toolchain::CallInst *CreateMemCpy(Address Dest, Address Src, toolchain::Value *Size,
                               bool IsVolatile = false) {
    toolchain::Value *DestPtr = emitRawPointerFromAddress(Dest);
    toolchain::Value *SrcPtr = emitRawPointerFromAddress(Src);
    return CreateMemCpy(DestPtr, Dest.getAlignment().getAsAlign(), SrcPtr,
                        Src.getAlignment().getAsAlign(), Size, IsVolatile);
  }
  toolchain::CallInst *CreateMemCpy(Address Dest, Address Src, uint64_t Size,
                               bool IsVolatile = false) {
    toolchain::Value *DestPtr = emitRawPointerFromAddress(Dest);
    toolchain::Value *SrcPtr = emitRawPointerFromAddress(Src);
    return CreateMemCpy(DestPtr, Dest.getAlignment().getAsAlign(), SrcPtr,
                        Src.getAlignment().getAsAlign(), Size, IsVolatile);
  }

  using CGBuilderBaseTy::CreateMemCpyInline;
  toolchain::CallInst *CreateMemCpyInline(Address Dest, Address Src, uint64_t Size) {
    toolchain::Value *DestPtr = emitRawPointerFromAddress(Dest);
    toolchain::Value *SrcPtr = emitRawPointerFromAddress(Src);
    return CreateMemCpyInline(DestPtr, Dest.getAlignment().getAsAlign(), SrcPtr,
                              Src.getAlignment().getAsAlign(), getInt64(Size));
  }

  using CGBuilderBaseTy::CreateMemMove;
  toolchain::CallInst *CreateMemMove(Address Dest, Address Src, toolchain::Value *Size,
                                bool IsVolatile = false) {
    toolchain::Value *DestPtr = emitRawPointerFromAddress(Dest);
    toolchain::Value *SrcPtr = emitRawPointerFromAddress(Src);
    return CreateMemMove(DestPtr, Dest.getAlignment().getAsAlign(), SrcPtr,
                         Src.getAlignment().getAsAlign(), Size, IsVolatile);
  }

  using CGBuilderBaseTy::CreateMemSet;
  toolchain::CallInst *CreateMemSet(Address Dest, toolchain::Value *Value,
                               toolchain::Value *Size, bool IsVolatile = false) {
    return CreateMemSet(emitRawPointerFromAddress(Dest), Value, Size,
                        Dest.getAlignment().getAsAlign(), IsVolatile);
  }

  using CGBuilderBaseTy::CreateMemSetInline;
  toolchain::CallInst *CreateMemSetInline(Address Dest, toolchain::Value *Value,
                                     uint64_t Size) {
    return CreateMemSetInline(emitRawPointerFromAddress(Dest),
                              Dest.getAlignment().getAsAlign(), Value,
                              getInt64(Size));
  }

  using CGBuilderBaseTy::CreatePreserveStructAccessIndex;
  Address CreatePreserveStructAccessIndex(Address Addr, unsigned Index,
                                          unsigned FieldIndex,
                                          toolchain::MDNode *DbgInfo) {
    toolchain::StructType *ElTy = cast<toolchain::StructType>(Addr.getElementType());
    const toolchain::DataLayout &DL = BB->getDataLayout();
    const toolchain::StructLayout *Layout = DL.getStructLayout(ElTy);
    auto Offset = CharUnits::fromQuantity(Layout->getElementOffset(Index));

    return Address(
        CreatePreserveStructAccessIndex(ElTy, emitRawPointerFromAddress(Addr),
                                        Index, FieldIndex, DbgInfo),
        ElTy->getElementType(Index),
        Addr.getAlignment().alignmentAtOffset(Offset));
  }

  using CGBuilderBaseTy::CreatePreserveUnionAccessIndex;
  Address CreatePreserveUnionAccessIndex(Address Addr, unsigned FieldIndex,
                                         toolchain::MDNode *DbgInfo) {
    Addr.replaceBasePointer(CreatePreserveUnionAccessIndex(
        Addr.getBasePointer(), FieldIndex, DbgInfo));
    return Addr;
  }

  using CGBuilderBaseTy::CreateLaunderInvariantGroup;
  Address CreateLaunderInvariantGroup(Address Addr) {
    Addr.replaceBasePointer(CreateLaunderInvariantGroup(Addr.getBasePointer()));
    return Addr;
  }

  using CGBuilderBaseTy::CreateStripInvariantGroup;
  Address CreateStripInvariantGroup(Address Addr) {
    Addr.replaceBasePointer(CreateStripInvariantGroup(Addr.getBasePointer()));
    return Addr;
  }
};

} // end namespace CodeGen
} // end namespace language::Core

#endif
