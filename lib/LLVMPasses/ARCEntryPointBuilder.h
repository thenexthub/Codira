//===--- ARCEntryPointBuilder.h ---------------------------------*- C++ -*-===//
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

#ifndef LANGUAGE_LLVMPASSES_ARCENTRYPOINTBUILDER_H
#define LANGUAGE_LLVMPASSES_ARCENTRYPOINTBUILDER_H

#include "language/Basic/Toolchain.h"
#include "language/Basic/NullablePtr.h"
#include "language/Runtime/Config.h"
#include "language/Runtime/RuntimeFnWrappersGen.h"
#include "toolchain/ADT/APInt.h"
#include "toolchain/TargetParser/Triple.h"
#include "toolchain/IR/IRBuilder.h"
#include "toolchain/IR/Module.h"

namespace language {

namespace RuntimeConstants {
  const auto ReadNone = toolchain::Attribute::ReadNone;
  const auto ReadOnly = toolchain::Attribute::ReadOnly;
  const auto NoReturn = toolchain::Attribute::NoReturn;
  const auto NoUnwind = toolchain::Attribute::NoUnwind;
  const auto ZExt = toolchain::Attribute::ZExt;
  const auto FirstParamReturned = toolchain::Attribute::Returned;
}

using namespace RuntimeConstants;

/// A class for building ARC entry points. It is a composition wrapper around an
/// IRBuilder and a constant Cache. It cannot be moved or copied. It is meant
/// to be created once and passed around by reference.
class ARCEntryPointBuilder {
  using IRBuilder = toolchain::IRBuilder<>;
  using Constant = toolchain::Constant;
  using Type = toolchain::Type;
  using Function = toolchain::Function;
  using Instruction = toolchain::Instruction;
  using CallInst = toolchain::CallInst;
  using Value = toolchain::Value;
  using Module = toolchain::Module;
  using AttributeList = toolchain::AttributeList;
  using Attribute = toolchain::Attribute;
  using APInt = toolchain::APInt;
  
  // The builder which we are wrapping.
  IRBuilder B;

  // The constant cache.
  NullablePtr<Constant> Retain;
  NullablePtr<Constant> Release;
  NullablePtr<Constant> CheckUnowned;
  NullablePtr<Constant> RetainN;
  NullablePtr<Constant> ReleaseN;
  NullablePtr<Constant> UnknownObjectRetainN;
  NullablePtr<Constant> UnknownObjectReleaseN;
  NullablePtr<Constant> BridgeRetainN;
  NullablePtr<Constant> BridgeReleaseN;

  // The type cache.
  NullablePtr<Type> ObjectPtrTy;
  NullablePtr<Type> BridgeObjectPtrTy;

  toolchain::CallingConv::ID DefaultCC;

  toolchain::CallInst *CreateCall(Constant *Fn, Value *V) {
    CallInst *CI = B.CreateCall(
        cast<toolchain::FunctionType>(cast<toolchain::Function>(Fn)->getValueType()), Fn,
        V);
    if (auto Fun = toolchain::dyn_cast<toolchain::Function>(Fn))
      CI->setCallingConv(Fun->getCallingConv());
    return CI;
  }

  toolchain::CallInst *CreateCall(Constant *Fn, toolchain::ArrayRef<Value *> Args) {
    CallInst *CI = B.CreateCall(
        cast<toolchain::FunctionType>(cast<toolchain::Function>(Fn)->getValueType()), Fn,
        Args);
    if (auto Fun = toolchain::dyn_cast<toolchain::Function>(Fn))
      CI->setCallingConv(Fun->getCallingConv());
    return CI;
  }

public:
  ARCEntryPointBuilder(Function &F)
      : B(&*F.begin()), Retain(), ObjectPtrTy(),
        DefaultCC(LANGUAGE_DEFAULT_TOOLCHAIN_CC) { }

  ~ARCEntryPointBuilder() = default;
  ARCEntryPointBuilder(ARCEntryPointBuilder &&) = delete;
  ARCEntryPointBuilder(const ARCEntryPointBuilder &) = delete;

  ARCEntryPointBuilder &operator=(const ARCEntryPointBuilder &) = delete;
  void operator=(ARCEntryPointBuilder &&C) = delete;

  void setInsertPoint(Instruction *I) {
    B.SetInsertPoint(I);
  }

  Value *createInsertValue(Value *V1, Value *V2, unsigned Idx) {
    return B.CreateInsertValue(V1, V2, Idx);
  }

  Value *createExtractValue(Value *V, unsigned Idx) {
    return B.CreateExtractValue(V, Idx);
  }

  Value *createIntToPtr(Value *V, Type *Ty) {
    return B.CreateIntToPtr(V, Ty);
  }

  CallInst *createRetain(Value *V, CallInst *OrigI) {
    // Cast just to make sure that we have the right type.
    V = B.CreatePointerCast(V, getObjectPtrTy());

    // Create the call.
    CallInst *CI = CreateCall(getRetain(OrigI), V);
    return CI;
  }

  CallInst *createRelease(Value *V, CallInst *OrigI) {
    // Cast just to make sure that we have the right type.
    V = B.CreatePointerCast(V, getObjectPtrTy());

    // Create the call.
    CallInst *CI = CreateCall(getRelease(OrigI), V);
    return CI;
  }

  
  CallInst *createCheckUnowned(Value *V, CallInst *OrigI) {
    // Cast just to make sure that we have the right type.
    V = B.CreatePointerCast(V, getObjectPtrTy());
    
    CallInst *CI = CreateCall(getCheckUnowned(OrigI), V);
    return CI;
  }

  CallInst *createRetainN(Value *V, uint32_t n, CallInst *OrigI) {
    // Cast just to make sure that we have the right object type.
    V = B.CreatePointerCast(V, getObjectPtrTy());
    CallInst *CI = CreateCall(getRetainN(OrigI), {V, getIntConstant(n)});
    return CI;
  }

  CallInst *createReleaseN(Value *V, uint32_t n, CallInst *OrigI) {
    // Cast just to make sure we have the right object type.
    V = B.CreatePointerCast(V, getObjectPtrTy());
    CallInst *CI = CreateCall(getReleaseN(OrigI), {V, getIntConstant(n)});
    return CI;
  }

  CallInst *createUnknownObjectRetainN(Value *V, uint32_t n, CallInst *OrigI) {
    // Cast just to make sure that we have the right object type.
    V = B.CreatePointerCast(V, getObjectPtrTy());
    CallInst *CI =
        CreateCall(getUnknownObjectRetainN(OrigI), {V, getIntConstant(n)});
    return CI;
  }

  CallInst *createUnknownObjectReleaseN(Value *V, uint32_t n, CallInst *OrigI) {
    // Cast just to make sure we have the right object type.
    V = B.CreatePointerCast(V, getObjectPtrTy());
    CallInst *CI =
        CreateCall(getUnknownObjectReleaseN(OrigI), {V, getIntConstant(n)});
    return CI;
  }

  CallInst *createBridgeRetainN(Value *V, uint32_t n, CallInst *OrigI) {
    // Cast just to make sure we have the right object type.
    V = B.CreatePointerCast(V, getBridgeObjectPtrTy());
    CallInst *CI = CreateCall(getBridgeRetainN(OrigI), {V, getIntConstant(n)});
    return CI;
  }

  CallInst *createBridgeReleaseN(Value *V, uint32_t n, CallInst *OrigI) {
    // Cast just to make sure we have the right object type.
    V = B.CreatePointerCast(V, getBridgeObjectPtrTy());
    CallInst *CI = CreateCall(getBridgeReleaseN(OrigI), {V, getIntConstant(n)});
    return CI;
  }

  bool isNonAtomic(CallInst *I) {
    // If we have an intrinsic, we know it must be an objc intrinsic. All objc
    // intrinsics are atomic today.
    if (I->getIntrinsicID() != toolchain::Intrinsic::not_intrinsic)
      return false;
    return (I->getCalledFunction()->getName().contains("nonatomic"));
  }

  bool isAtomic(CallInst *I) {
    return !isNonAtomic(I);
  }

  /// Perform a pointer cast of pointer value \p V to \p Ty if \p V has a
  /// different type than \p Ty. If \p V equals \p Ty, just return V.
  toolchain::Value *maybeCast(toolchain::Value *V, toolchain::Type *Ty) {
    if (V->getType() == Ty)
      return V;
    return B.CreatePointerCast(V, Ty);
  }

private:
  Module &getModule() {
    return *B.GetInsertBlock()->getModule();
  }

  /// getRetain - Return a callable function for language_retain.
  Constant *getRetain(CallInst *OrigI) {
    if (Retain)
      return Retain.get();
    auto *ObjectPtrTy = getObjectPtrTy();

    toolchain::Constant *cache = nullptr;
    Retain = getRuntimeFn(
        getModule(), cache, "Codira",
        isNonAtomic(OrigI) ? "language_nonatomic_retain" : "language_retain",
        DefaultCC, RuntimeAvailability::AlwaysAvailable, {ObjectPtrTy},
        {ObjectPtrTy}, {NoUnwind, FirstParamReturned}, {});

    return Retain.get();
  }

  /// getRelease - Return a callable function for language_release.
  Constant *getRelease(CallInst *OrigI) {
    if (Release)
      return Release.get();
    auto *ObjectPtrTy = getObjectPtrTy();
    auto *VoidTy = Type::getVoidTy(getModule().getContext());

    toolchain::Constant *cache = nullptr;
    Release = getRuntimeFn(getModule(), cache, "Codira",
                           isNonAtomic(OrigI) ? "language_nonatomic_release"
                                              : "language_release",
                           DefaultCC, RuntimeAvailability::AlwaysAvailable,
                           {VoidTy}, {ObjectPtrTy}, {NoUnwind}, {});

    return Release.get();
  }

  Constant *getCheckUnowned(CallInst *OrigI) {
    if (CheckUnowned)
      return CheckUnowned.get();
    
    auto *ObjectPtrTy = getObjectPtrTy();
    auto &M = getModule();
    auto AttrList = AttributeList::get(M.getContext(), 1, Attribute::NoCapture);
    AttrList = AttrList.addFnAttribute(M.getContext(), Attribute::NoUnwind);
    CheckUnowned = cast<toolchain::Function>(
        M.getOrInsertFunction("language_checkUnowned", AttrList,
                              Type::getVoidTy(M.getContext()), ObjectPtrTy)
            .getCallee());
    if (toolchain::Triple(M.getTargetTriple()).isOSBinFormatCOFF() &&
        !toolchain::Triple(M.getTargetTriple()).isOSCygMing())
      if (auto *F = toolchain::dyn_cast<toolchain::Function>(CheckUnowned.get()))
        F->setDLLStorageClass(toolchain::GlobalValue::DLLImportStorageClass);
    return CheckUnowned.get();
  }

  /// getRetainN - Return a callable function for language_retain_n.
  Constant *getRetainN(CallInst *OrigI) {
    if (RetainN)
      return RetainN.get();
    auto *ObjectPtrTy = getObjectPtrTy();
    auto *Int32Ty = Type::getInt32Ty(getModule().getContext());

    toolchain::Constant *cache = nullptr;
    RetainN = getRuntimeFn(
        getModule(), cache, "Codira",
        isNonAtomic(OrigI) ? "language_nonatomic_retain_n" : "language_retain_n",
        DefaultCC, RuntimeAvailability::AlwaysAvailable, {ObjectPtrTy},
        {ObjectPtrTy, Int32Ty}, {NoUnwind, FirstParamReturned}, {});

    return RetainN.get();
  }

  /// Return a callable function for language_release_n.
  Constant *getReleaseN(CallInst *OrigI) {
    if (ReleaseN)
      return ReleaseN.get();
    auto *ObjectPtrTy = getObjectPtrTy();
    auto *Int32Ty = Type::getInt32Ty(getModule().getContext());
    auto *VoidTy = Type::getVoidTy(getModule().getContext());

    toolchain::Constant *cache = nullptr;
    ReleaseN = getRuntimeFn(getModule(), cache, "Codira",
                            isNonAtomic(OrigI) ? "language_nonatomic_release_n"
                                               : "language_release_n",
                            DefaultCC, RuntimeAvailability::AlwaysAvailable,
                            {VoidTy}, {ObjectPtrTy, Int32Ty}, {NoUnwind}, {});

    return ReleaseN.get();
  }

  /// getUnknownObjectRetainN - Return a callable function for
  /// language_unknownObjectRetain_n.
  Constant *getUnknownObjectRetainN(CallInst *OrigI) {
    if (UnknownObjectRetainN)
      return UnknownObjectRetainN.get();
    auto *ObjectPtrTy = getObjectPtrTy();
    auto *Int32Ty = Type::getInt32Ty(getModule().getContext());

    toolchain::Constant *cache = nullptr;
    UnknownObjectRetainN = getRuntimeFn(
        getModule(), cache, "Codira",
        isNonAtomic(OrigI) ? "language_nonatomic_unknownObjectRetain_n"
                           : "language_unknownObjectRetain_n",
        DefaultCC, RuntimeAvailability::AlwaysAvailable, {ObjectPtrTy},
        {ObjectPtrTy, Int32Ty}, {NoUnwind, FirstParamReturned}, {});

    return UnknownObjectRetainN.get();
  }

  /// Return a callable function for language_unknownObjectRelease_n.
  Constant *getUnknownObjectReleaseN(CallInst *OrigI) {
    if (UnknownObjectReleaseN)
      return UnknownObjectReleaseN.get();
    auto *ObjectPtrTy = getObjectPtrTy();
    auto *Int32Ty = Type::getInt32Ty(getModule().getContext());
    auto *VoidTy = Type::getVoidTy(getModule().getContext());

    toolchain::Constant *cache = nullptr;
    UnknownObjectReleaseN = getRuntimeFn(
        getModule(), cache, "Codira",
        isNonAtomic(OrigI) ? "language_nonatomic_unknownObjectRelease_n"
                           : "language_unknownObjectRelease_n",
        DefaultCC, RuntimeAvailability::AlwaysAvailable, {VoidTy},
        {ObjectPtrTy, Int32Ty}, {NoUnwind}, {});

    return UnknownObjectReleaseN.get();
  }

  /// Return a callable function for language_bridgeRetain_n.
  Constant *getBridgeRetainN(CallInst *OrigI) {
    if (BridgeRetainN)
      return BridgeRetainN.get();
    auto *BridgeObjectPtrTy = getBridgeObjectPtrTy();
    auto *Int32Ty = Type::getInt32Ty(getModule().getContext());

    toolchain::Constant *cache = nullptr;
    BridgeRetainN = getRuntimeFn(
        getModule(), cache, "Codira",
        isNonAtomic(OrigI) ? "language_nonatomic_bridgeObjectRetain_n"
                           : "language_bridgeObjectRetain_n",
        DefaultCC, RuntimeAvailability::AlwaysAvailable, {BridgeObjectPtrTy},
        {BridgeObjectPtrTy, Int32Ty}, {NoUnwind}, {});
    return BridgeRetainN.get();
  }

  /// Return a callable function for language_bridgeRelease_n.
  Constant *getBridgeReleaseN(CallInst *OrigI) {
    if (BridgeReleaseN)
      return BridgeReleaseN.get();

    auto *BridgeObjectPtrTy = getBridgeObjectPtrTy();
    auto *Int32Ty = Type::getInt32Ty(getModule().getContext());
    auto *VoidTy = Type::getVoidTy(getModule().getContext());

    toolchain::Constant *cache = nullptr;
    BridgeReleaseN = getRuntimeFn(
        getModule(), cache, "Codira",
        isNonAtomic(OrigI) ? "language_nonatomic_bridgeObjectRelease_n"
                           : "language_bridgeObjectRelease_n",
        DefaultCC, RuntimeAvailability::AlwaysAvailable, {VoidTy},
        {BridgeObjectPtrTy, Int32Ty}, {NoUnwind}, {});
    return BridgeReleaseN.get();
  }

  Type *getNamedOpaquePtrTy(StringRef name) {
    auto &M = getModule();
    if (auto *ty = toolchain::StructType::getTypeByName(M.getContext(), name)) {
      return ty->getPointerTo();
    }

    // Otherwise, create an anonymous struct type.
    auto *ty = toolchain::StructType::create(M.getContext(), name);
    return ty->getPointerTo();
  }

  Type *getObjectPtrTy() {
    if (ObjectPtrTy)
      return ObjectPtrTy.get();
    ObjectPtrTy = getNamedOpaquePtrTy("language.refcounted");
    return ObjectPtrTy.get();
  }

  Type *getBridgeObjectPtrTy() {
    if (BridgeObjectPtrTy)
      return BridgeObjectPtrTy.get();
    BridgeObjectPtrTy = getNamedOpaquePtrTy("language.bridge");
    return BridgeObjectPtrTy.get();
  }

  Constant *getIntConstant(uint32_t constant) {
    auto &M = getModule();
    auto *Int32Ty = Type::getInt32Ty(M.getContext());
    return Constant::getIntegerValue(Int32Ty, APInt(32, constant));
  }
};

} // end language namespace

#endif

