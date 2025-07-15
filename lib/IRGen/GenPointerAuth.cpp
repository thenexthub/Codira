//===--- GenPointerAuth.cpp - IRGen for pointer authentication ------------===//
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
//  This file implements general support for pointer authentication in Codira.
//
//===----------------------------------------------------------------------===//

#include "GenPointerAuth.h"
#include "Callee.h"
#include "ConstantBuilder.h"
#include "GenType.h"
#include "IRGenFunction.h"
#include "IRGenMangler.h"
#include "IRGenModule.h"
#include "language/AST/GenericEnvironment.h"
#include "language/Basic/Assertions.h"
#include "language/SIL/TypeLowering.h"
#include "clang/CodeGen/CodeGenABITypes.h"
#include "toolchain/ADT/APInt.h"
#include "toolchain/Support/raw_ostream.h"

using namespace language;
using namespace irgen;

/**************************** INTRINSIC OPERATIONS ****************************/

/// Return the key and discriminator value for the given auth info,
/// as demanded by the ptrauth intrinsics.
static std::pair<toolchain::Constant*, toolchain::Value*>
getPointerAuthPair(IRGenFunction &IGF, const PointerAuthInfo &authInfo) {
  auto key = toolchain::ConstantInt::get(IGF.IGM.Int32Ty, authInfo.getKey());
  toolchain::Value *discriminator = authInfo.getDiscriminator();
  if (discriminator->getType()->isPointerTy()) {
    discriminator = IGF.Builder.CreatePtrToInt(discriminator, IGF.IGM.Int64Ty);
  }
  return { key, discriminator };
}

toolchain::Value *irgen::emitPointerAuthBlend(IRGenFunction &IGF,
                                         toolchain::Value *address,
                                         toolchain::Value *other) {
  address = IGF.Builder.CreatePtrToInt(address, IGF.IGM.Int64Ty);
  return IGF.Builder.CreateIntrinsicCall(toolchain::Intrinsic::ptrauth_blend,
                                         {address, other});
}

toolchain::Value *irgen::emitPointerAuthStrip(IRGenFunction &IGF,
                                          toolchain::Value *fnPtr,
                                          unsigned Key) {
  auto fnVal = IGF.Builder.CreatePtrToInt(fnPtr, IGF.IGM.Int64Ty);
  auto keyArg = toolchain::ConstantInt::get(IGF.IGM.Int32Ty, Key);
  auto strippedPtr = IGF.Builder.CreateIntrinsicCall(
      toolchain::Intrinsic::ptrauth_strip, {fnVal, keyArg});
  return IGF.Builder.CreateIntToPtr(strippedPtr, fnPtr->getType());
}

FunctionPointer irgen::emitPointerAuthResign(IRGenFunction &IGF,
                                             const FunctionPointer &fn,
                                          const PointerAuthInfo &newAuthInfo) {
  toolchain::Value *fnPtr = emitPointerAuthResign(IGF, fn.getRawPointer(),
                                             fn.getAuthInfo(), newAuthInfo);
  return FunctionPointer::createSigned(fn.getKind(), fnPtr, newAuthInfo,
                                       fn.getSignature());
}

toolchain::Value *irgen::emitPointerAuthResign(IRGenFunction &IGF,
                                          toolchain::Value *fnPtr,
                                          const PointerAuthInfo &oldAuthInfo,
                                          const PointerAuthInfo &newAuthInfo) {
  // If the signatures match, there's nothing to do.
  if (oldAuthInfo == newAuthInfo)
    return fnPtr;

  // If the pointer is not currently signed, sign it.
  if (!oldAuthInfo.isSigned()) {
    return emitPointerAuthSign(IGF, fnPtr, newAuthInfo);
  }

  // If the pointer is not supposed to be signed, auth it.
  if (!newAuthInfo.isSigned()) {
    return emitPointerAuthAuth(IGF, fnPtr, oldAuthInfo);
  }

  // Otherwise, auth and resign it.
  auto origTy = fnPtr->getType();
  fnPtr = IGF.Builder.CreatePtrToInt(fnPtr, IGF.IGM.Int64Ty);

  auto oldPair = getPointerAuthPair(IGF, oldAuthInfo);
  auto newPair = getPointerAuthPair(IGF, newAuthInfo);

  toolchain::Value *args[] = {
    fnPtr, oldPair.first, oldPair.second, newPair.first, newPair.second
  };
  fnPtr =
      IGF.Builder.CreateIntrinsicCall(toolchain::Intrinsic::ptrauth_resign, args);
  return IGF.Builder.CreateIntToPtr(fnPtr, origTy);
}

toolchain::Value *irgen::emitPointerAuthAuth(IRGenFunction &IGF, toolchain::Value *fnPtr,
                                        const PointerAuthInfo &oldAuthInfo) {
  auto origTy = fnPtr->getType();
  fnPtr = IGF.Builder.CreatePtrToInt(fnPtr, IGF.IGM.Int64Ty);

  auto oldPair = getPointerAuthPair(IGF, oldAuthInfo);

  toolchain::Value *args[] = {
    fnPtr, oldPair.first, oldPair.second
  };
  fnPtr = IGF.Builder.CreateIntrinsicCall(toolchain::Intrinsic::ptrauth_auth, args);
  return IGF.Builder.CreateIntToPtr(fnPtr, origTy);
}

toolchain::Value *irgen::emitPointerAuthSign(IRGenFunction &IGF, toolchain::Value *fnPtr,
                                        const PointerAuthInfo &newAuthInfo) {
  if (!newAuthInfo.isSigned())
    return fnPtr;

  // Special-case constants.
  if (auto constantFnPtr = dyn_cast<toolchain::Constant>(fnPtr)) {
    if (auto constantDiscriminator =
          dyn_cast<toolchain::Constant>(newAuthInfo.getDiscriminator())) {
      toolchain::Constant *address = nullptr;
      toolchain::ConstantInt *other = nullptr;
      if (constantDiscriminator->getType()->isPointerTy()) {
        address = constantDiscriminator;
      } else if (auto otherDiscriminator =
                     dyn_cast<toolchain::ConstantInt>(constantDiscriminator)) {
        other = otherDiscriminator;
      }
      return IGF.IGM.getConstantSignedPointer(constantFnPtr,
                                              newAuthInfo.getKey(),
                                              address, other);
    }
  }

  auto origTy = fnPtr->getType();
  fnPtr = IGF.Builder.CreatePtrToInt(fnPtr, IGF.IGM.Int64Ty);

  auto newPair = getPointerAuthPair(IGF, newAuthInfo);

  toolchain::Value *args[] = {
    fnPtr, newPair.first, newPair.second
  };
  fnPtr = IGF.Builder.CreateIntrinsicCall(toolchain::Intrinsic::ptrauth_sign, args);
  return IGF.Builder.CreateIntToPtr(fnPtr, origTy);
}

/******************************* DISCRIMINATORS *******************************/

struct IRGenModule::PointerAuthCachesType {
  toolchain::DenseMap<SILDeclRef, toolchain::ConstantInt*> Decls;
  toolchain::DenseMap<CanType, toolchain::ConstantInt*> Types;
  toolchain::DenseMap<CanType, toolchain::ConstantInt*> YieldTypes;
  toolchain::DenseMap<AssociatedTypeDecl *, toolchain::ConstantInt*> AssociatedTypes;
  toolchain::DenseMap<AssociatedConformance, toolchain::ConstantInt*> AssociatedConformances;
};

IRGenModule::PointerAuthCachesType &IRGenModule::getPointerAuthCaches() {
  if (!PointerAuthCaches)
    PointerAuthCaches = new PointerAuthCachesType();
  return *PointerAuthCaches;
}

void IRGenModule::destroyPointerAuthCaches() {
  delete PointerAuthCaches;
}

static const PointerAuthSchema &getFunctionPointerSchema(IRGenModule &IGM,
                                                   CanSILFunctionType fnType) {
  auto &options = IGM.getOptions().PointerAuth;
  switch (fnType->getRepresentation()) {
  case SILFunctionTypeRepresentation::CXXMethod:
  case SILFunctionTypeRepresentation::CFunctionPointer:
    return options.FunctionPointers;

  case SILFunctionTypeRepresentation::Thin:
  case SILFunctionTypeRepresentation::Thick:
  case SILFunctionTypeRepresentation::Method:
  case SILFunctionTypeRepresentation::WitnessMethod:
  case SILFunctionTypeRepresentation::Closure:
  case SILFunctionTypeRepresentation::KeyPathAccessorGetter:
  case SILFunctionTypeRepresentation::KeyPathAccessorSetter:
  case SILFunctionTypeRepresentation::KeyPathAccessorEquals:
  case SILFunctionTypeRepresentation::KeyPathAccessorHash:
    if (fnType->isAsync()) {
      return options.AsyncCodiraFunctionPointers;
    } else if (fnType->isCalleeAllocatedCoroutine()) {
      return options.CoroCodiraFunctionPointers;
    }

    return options.CodiraFunctionPointers;

  case SILFunctionTypeRepresentation::ObjCMethod:
  case SILFunctionTypeRepresentation::Block:
    toolchain_unreachable("not just a function pointer");
  }
  toolchain_unreachable("bad representation");
}

PointerAuthInfo PointerAuthInfo::forFunctionPointer(IRGenModule &IGM,
                                                    CanSILFunctionType fnType) {
  auto &schema = getFunctionPointerSchema(IGM, fnType);

  // If the target doesn't sign function pointers, we're done.
  if (!schema) return PointerAuthInfo();

  assert(!schema.isAddressDiscriminated() &&
         "function pointer cannot be address-discriminated");

  auto discriminator = getOtherDiscriminator(IGM, schema, fnType);
  return PointerAuthInfo(schema.getKey(), discriminator);
}

PointerAuthInfo PointerAuthInfo::emit(IRGenFunction &IGF,
                                      const PointerAuthSchema &schema,
                                      toolchain::Value *storageAddress,
                                      const PointerAuthEntity &entity) {
  if (!schema) return PointerAuthInfo();

  unsigned key = schema.getKey();

  // Produce the 'other' discriminator.
  auto otherDiscriminator = getOtherDiscriminator(IGF.IGM, schema, entity);
  toolchain::Value *discriminator = otherDiscriminator;

  // Factor in the address.
  if (schema.isAddressDiscriminated()) {
    assert(storageAddress &&
           "no storage address for address-discriminated schema");

    if (!otherDiscriminator->isZero()) {
      discriminator = emitPointerAuthBlend(IGF, storageAddress, discriminator);
    } else {
      discriminator =
          IGF.Builder.CreatePtrToInt(storageAddress, IGF.IGM.Int64Ty);
    }
  }

  return PointerAuthInfo(key, discriminator);
}

PointerAuthInfo
PointerAuthInfo::emit(IRGenFunction &IGF,
                      clang::PointerAuthQualifier pointerAuthQual,
                      toolchain::Value *storageAddress) {
  unsigned key = pointerAuthQual.getKey();

  // Produce the 'other' discriminator.
  auto otherDiscriminator = pointerAuthQual.getExtraDiscriminator();
  toolchain::Value *discriminator =
      toolchain::ConstantInt::get(IGF.IGM.Int64Ty, otherDiscriminator);

  // Factor in the address.
  if (pointerAuthQual.isAddressDiscriminated()) {
    assert(storageAddress &&
           "no storage address for address-discriminated schema");

    if (otherDiscriminator != 0) {
      discriminator = emitPointerAuthBlend(IGF, storageAddress, discriminator);
    } else {
      discriminator =
          IGF.Builder.CreatePtrToInt(storageAddress, IGF.IGM.Int64Ty);
    }
  }

  return PointerAuthInfo(key, discriminator);
}

PointerAuthInfo PointerAuthInfo::emit(IRGenFunction &IGF,
                                      const PointerAuthSchema &schema,
                                      toolchain::Value *storageAddress,
                                      toolchain::ConstantInt *otherDiscriminator) {
  if (!schema)
    return PointerAuthInfo();

  unsigned key = schema.getKey();

  toolchain::Value *discriminator = otherDiscriminator;

  // Factor in the address.
  if (schema.isAddressDiscriminated()) {
    assert(storageAddress &&
           "no storage address for address-discriminated schema");

    if (!otherDiscriminator->isZero()) {
      discriminator = emitPointerAuthBlend(IGF, storageAddress, discriminator);
    } else {
      discriminator =
          IGF.Builder.CreatePtrToInt(storageAddress, IGF.IGM.Int64Ty);
    }
  }

  return PointerAuthInfo(key, discriminator);
}

toolchain::ConstantInt *
PointerAuthInfo::getOtherDiscriminator(IRGenModule &IGM,
                                       const PointerAuthSchema &schema,
                                       const PointerAuthEntity &entity) {
  assert(schema);
  switch (schema.getOtherDiscrimination()) {
  case PointerAuthSchema::Discrimination::None:
    return toolchain::ConstantInt::get(IGM.Int64Ty, 0);

  case PointerAuthSchema::Discrimination::Decl:
    return entity.getDeclDiscriminator(IGM);

  case PointerAuthSchema::Discrimination::Type:
    return entity.getTypeDiscriminator(IGM);

  case PointerAuthSchema::Discrimination::Constant:
    return toolchain::ConstantInt::get(IGM.Int64Ty,
                                  schema.getConstantDiscrimination());
  }
  toolchain_unreachable("bad kind");
}

static toolchain::ConstantInt *getDiscriminatorForHash(IRGenModule &IGM,
                                                  uint64_t rawHash) {
  uint16_t reducedHash = (rawHash % 0xFFFF) + 1;
  return toolchain::ConstantInt::get(IGM.Int64Ty, reducedHash);
}

static toolchain::ConstantInt *getDiscriminatorForString(IRGenModule &IGM,
                                                    StringRef string) {
  uint64_t rawHash = clang::CodeGen::computeStableStringHash(string);
  return getDiscriminatorForHash(IGM, rawHash);
}

static std::string mangle(AssociatedTypeDecl *assocType) {
  return IRGenMangler(assocType->getASTContext())
    .mangleAssociatedTypeAccessFunctionDiscriminator(assocType);
}

static std::string mangle(const AssociatedConformance &association) {
  return IRGenMangler(association.getAssociatedRequirement()->getASTContext())
    .mangleAssociatedTypeWitnessTableAccessFunctionDiscriminator(association);
}

toolchain::ConstantInt *
PointerAuthEntity::getDeclDiscriminator(IRGenModule &IGM) const {
  switch (StoredKind) {
  case Kind::None:
  case Kind::CanSILFunctionType:
  case Kind::CoroutineYieldTypes:
    toolchain_unreachable("no declaration for schema using decl discrimination");

  case Kind::Special: {
    auto getSpecialDiscriminator = [](Special special) -> uint16_t {
      switch (special) {
      case Special::HeapDestructor:
        return SpecialPointerAuthDiscriminators::HeapDestructor;
      case Special::TypeDescriptor:
      case Special::TypeDescriptorAsArgument:
        return SpecialPointerAuthDiscriminators::TypeDescriptor;
      case Special::ProtocolConformanceDescriptor:
      case Special::ProtocolConformanceDescriptorAsArgument:
        return SpecialPointerAuthDiscriminators::ProtocolConformanceDescriptor;
      case Special::ProtocolDescriptorAsArgument:
        return SpecialPointerAuthDiscriminators::ProtocolDescriptor;
      case Special::OpaqueTypeDescriptorAsArgument:
        return SpecialPointerAuthDiscriminators::OpaqueTypeDescriptor;
      case Special::ContextDescriptorAsArgument:
        return SpecialPointerAuthDiscriminators::ContextDescriptor;
      case Special::PartialApplyCapture:
        return PointerAuthDiscriminator_PartialApplyCapture;
      case Special::KeyPathDestroy:
        return SpecialPointerAuthDiscriminators::KeyPathDestroy;
      case Special::KeyPathCopy:
        return SpecialPointerAuthDiscriminators::KeyPathCopy;
      case Special::KeyPathEquals:
        return SpecialPointerAuthDiscriminators::KeyPathEquals;
      case Special::KeyPathHash:
        return SpecialPointerAuthDiscriminators::KeyPathHash;
      case Special::KeyPathGetter:
        return SpecialPointerAuthDiscriminators::KeyPathGetter;
      case Special::KeyPathNonmutatingSetter:
        return SpecialPointerAuthDiscriminators::KeyPathNonmutatingSetter;
      case Special::KeyPathMutatingSetter:
        return SpecialPointerAuthDiscriminators::KeyPathMutatingSetter;
      case Special::KeyPathGetLayout:
        return SpecialPointerAuthDiscriminators::KeyPathGetLayout;
      case Special::KeyPathInitializer:
        return SpecialPointerAuthDiscriminators::KeyPathInitializer;
      case Special::KeyPathMetadataAccessor:
        return SpecialPointerAuthDiscriminators::KeyPathMetadataAccessor;
      case Special::DynamicReplacementKey:
        return SpecialPointerAuthDiscriminators::DynamicReplacementKey;
      case Special::TypeLayoutString:
        return SpecialPointerAuthDiscriminators::TypeLayoutString;
      case Special::BlockCopyHelper:
      case Special::BlockDisposeHelper:
        toolchain_unreachable("no known discriminator for these foreign entities");
      }
      toolchain_unreachable("bad kind");
    };
    auto specialKind = Storage.get<Special>(StoredKind);
    return toolchain::ConstantInt::get(IGM.Int64Ty,
                                  getSpecialDiscriminator(specialKind));
  }

  case Kind::ValueWitness: {
    auto getValueWitnessDiscriminator = [](ValueWitness witness) -> uint16_t {
      switch (witness) {
#define WANT_ALL_VALUE_WITNESSES
#define DATA_VALUE_WITNESS(LOWER, UPPER, TYPE)
#define FUNCTION_VALUE_WITNESS(LOWER, ID, RET, PARAMS) \
      case ValueWitness::ID: return SpecialPointerAuthDiscriminators::ID;
#include "language/ABI/ValueWitness.def"
      case ValueWitness::Size:
      case ValueWitness::Flags:
      case ValueWitness::Stride:
      case ValueWitness::ExtraInhabitantCount:
        toolchain_unreachable("not a function value witness");
      }
      toolchain_unreachable("bad kind");
    };
    auto witness = Storage.get<ValueWitness>(StoredKind);
    return toolchain::ConstantInt::get(IGM.Int64Ty,
                                  getValueWitnessDiscriminator(witness));
  }

  case Kind::AssociatedType: {
    auto association = Storage.get<AssociatedTypeDecl *>(StoredKind);
    toolchain::ConstantInt *&cache =
      IGM.getPointerAuthCaches().AssociatedTypes[association];
    if (cache) return cache;

    auto mangling = mangle(association);
    cache = getDiscriminatorForString(IGM, mangling);
    return cache;
  }

  case Kind::AssociatedConformance: {
    auto conformance = Storage.get<AssociatedConformance>(StoredKind);
    toolchain::ConstantInt *&cache =
      IGM.getPointerAuthCaches().AssociatedConformances[conformance];
    if (cache) return cache;

    auto mangling = mangle(conformance);
    cache = getDiscriminatorForString(IGM, mangling);
    return cache;
  }

  case Kind::SILDeclRef: {
    auto constant = Storage.get<SILDeclRef>(StoredKind);
    toolchain::ConstantInt *&cache = IGM.getPointerAuthCaches().Decls[constant];
    if (cache) return cache;

    // Getting the discriminator for a foreign SILDeclRef just means
    // converting it to a foreign declaration and asking Clang IRGen
    // for the corresponding discriminator, but that's not completely
    // trivial.
    assert(!constant.isForeign &&
           "discriminator for foreign declaration not supported yet!");

    auto mangling = constant.mangle();
    cache = getDiscriminatorForString(IGM, mangling);
    return cache;
  }
  case Kind::SILFunction: {
    auto fn = Storage.get<SILFunction*>(StoredKind);
    return getDiscriminatorForString(IGM, fn->getName());
  }
  }
  toolchain_unreachable("bad kind");
}

static void hashStringForFunctionType(IRGenModule &IGM, CanSILFunctionType type,
                                      raw_ostream &Out,
                                      GenericEnvironment *genericEnv);

static void hashStringForType(IRGenModule &IGM, CanType Ty, raw_ostream &Out,
                              GenericEnvironment *genericEnv) {
  if (Ty->isAnyClassReferenceType()) {
    // Any class type has to be hashed opaquely.
    Out << "-class";
  } else if (isa<AnyMetatypeType>(Ty)) {
    // Any metatype has to be hashed opaquely.
    Out << "-metatype";
  } else if (auto UnwrappedTy = Ty->getOptionalObjectType()) {
    if (UnwrappedTy->isBridgeableObjectType()) {
      // Optional<T> is compatible with T when T is class-based.
      hashStringForType(IGM, UnwrappedTy->getCanonicalType(), Out, genericEnv);
    } else if (UnwrappedTy->is<MetatypeType>()) {
      // Optional<T> is compatible with T when T is a metatype.
      hashStringForType(IGM, UnwrappedTy->getCanonicalType(), Out, genericEnv);
    } else {
      // Optional<T> is direct if and only if T is.
      Out << "Optional<";
      hashStringForType(IGM, UnwrappedTy->getCanonicalType(), Out, genericEnv);
      Out << ">";
    }
  } else if (auto ETy = dyn_cast<ExistentialType>(Ty)) {
    // Look through existential types
    hashStringForType(IGM, ETy->getConstraintType()->getCanonicalType(),
                      Out, genericEnv);
  } else if (auto GTy = dyn_cast<AnyGenericType>(Ty)) {
    // For generic and non-generic value types, use the mangled declaration
    // name, and ignore all generic arguments.
    NominalTypeDecl *nominal = cast<NominalTypeDecl>(GTy->getDecl());
    Out << Mangle::ASTMangler(IGM.Context).mangleNominalType(nominal);
  } else if (auto FTy = dyn_cast<SILFunctionType>(Ty)) {
    Out << "(";
    hashStringForFunctionType(IGM, FTy, Out, genericEnv);
    Out << ")";
  } else {
    Out << "-";
  }
}

template <class T>
static void hashStringForList(IRGenModule &IGM, const ArrayRef<T> &list,
                              raw_ostream &Out, GenericEnvironment *genericEnv,
                              const SILFunctionType *fnType) {
  for (auto paramOrRetVal : list) {
    if (paramOrRetVal.isFormalIndirect()) {
      // Indirect params and return values have to be opaque.
      Out << "-indirect";
    } else {
      CanType Ty = paramOrRetVal.getArgumentType(
          IGM.getSILModule(), fnType, IGM.getMaximalTypeExpansionContext());
      if (Ty->hasTypeParameter())
        Ty = genericEnv->mapTypeIntoContext(Ty)->getCanonicalType();
      hashStringForType(IGM, Ty, Out, genericEnv);
    }
    Out << ":";
  }
}

static void hashStringForList(IRGenModule &IGM,
                              const ArrayRef<SILResultInfo> &list,
                              raw_ostream &Out, GenericEnvironment *genericEnv,
                              const SILFunctionType *fnType) {
  for (auto paramOrRetVal : list) {
    if (paramOrRetVal.isFormalIndirect()) {
      // Indirect params and return values have to be opaque.
      Out << "-indirect";
    } else {
      CanType Ty = paramOrRetVal.getReturnValueType(
          IGM.getSILModule(), fnType, IGM.getMaximalTypeExpansionContext());
      if (Ty->hasTypeParameter())
        Ty = genericEnv->mapTypeIntoContext(Ty)->getCanonicalType();
      hashStringForType(IGM, Ty, Out, genericEnv);
    }
    Out << ":";
  }
}

static void hashStringForFunctionType(IRGenModule &IGM, CanSILFunctionType type,
                                      raw_ostream &Out,
                                      GenericEnvironment *genericEnv) {
  Out << (type->isCoroutine() ? "coroutine" : "function") << ":";
  Out << type->getNumParameters() << ":";
  hashStringForList(IGM, type->getParameters(), Out, genericEnv, type);
  Out << type->getNumResults() << ":";
  hashStringForList(IGM, type->getResults(), Out, genericEnv, type);
  if (type->isCoroutine()) {
    Out << type->getNumYields() << ":";
    hashStringForList(IGM, type->getYields(), Out, genericEnv, type);
  }
}

static uint64_t getTypeHash(IRGenModule &IGM, CanSILFunctionType type) {
  // The hash we need to do here ignores:
  //   - thickness, so that we can promote thin-to-thick without rehashing;
  //   - error results, so that we can promote nonthrowing-to-throwing
  //     without rehashing;
  //   - isolation, so that global actor annotations can change in the SDK
  //     without breaking compatibility and so that we can erase it to
  //     nonisolated without rehashing;
  //   - types of indirect arguments/retvals, so they can be substituted freely;
  //   - types of class arguments/retvals
  //   - types of metatype arguments/retvals
  // See isABICompatibleWith and areABICompatibleParamsOrReturns in
  // SILFunctionType.cpp.

  SmallString<32> Buffer;
  toolchain::raw_svector_ostream Out(Buffer);
  auto genericSig = type->getInvocationGenericSignature();
  hashStringForFunctionType(
      IGM, type, Out,
      genericSig.getCanonicalSignature().getGenericEnvironment());
  return clang::CodeGen::computeStableStringHash(Out.str());
}

static uint64_t getYieldTypesHash(IRGenModule &IGM, CanSILFunctionType type) {
  SmallString<32> buffer;
  toolchain::raw_svector_ostream out(buffer);
  auto genericSig = type->getInvocationGenericSignature();
  auto *genericEnv =  genericSig.getCanonicalSignature().getGenericEnvironment();

  out << [&]() -> StringRef {
    switch (type->getCoroutineKind()) {
    case SILCoroutineKind::YieldMany: return "yield_many:";
    case SILCoroutineKind::YieldOnce: return "yield_once:";
    case SILCoroutineKind::YieldOnce2:
      return "yield_once_2:";
    case SILCoroutineKind::None: toolchain_unreachable("not a coroutine");
    }
    toolchain_unreachable("bad coroutine kind");
  }();

  out << type->getNumYields() << ":";

  for (auto yield: type->getYields()) {
    // We can't mangle types on inout and indirect yields because they're
    // abstractable.
    if (yield.isIndirectInOut()) {
      out << "inout";
    } else if (yield.isFormalIndirect()) {
      out << "indirect";
    } else {
      CanType Ty = yield.getArgumentType(IGM.getSILModule(), type,
                                         IGM.getMaximalTypeExpansionContext());
      if (Ty->hasTypeParameter())
        Ty = genericEnv->mapTypeIntoContext(Ty)->getCanonicalType();
      hashStringForType(IGM, Ty, out, genericEnv);
    }
    out << ":";
  }

  return clang::CodeGen::computeStableStringHash(out.str());  
}

toolchain::ConstantInt *
PointerAuthEntity::getTypeDiscriminator(IRGenModule &IGM) const {
  auto getTypeDiscriminator = [&](CanSILFunctionType fnType) {
    switch (fnType->getRepresentation()) {
    // Codira function types are type-discriminated.
    case SILFunctionTypeRepresentation::Thick:
    case SILFunctionTypeRepresentation::Thin:
    case SILFunctionTypeRepresentation::Method:
    case SILFunctionTypeRepresentation::WitnessMethod:
    case SILFunctionTypeRepresentation::Closure:
    case SILFunctionTypeRepresentation::KeyPathAccessorGetter:
    case SILFunctionTypeRepresentation::KeyPathAccessorSetter:
    case SILFunctionTypeRepresentation::KeyPathAccessorEquals:
    case SILFunctionTypeRepresentation::KeyPathAccessorHash: {
      toolchain::ConstantInt *&cache = IGM.getPointerAuthCaches().Types[fnType];
      if (cache) return cache;

      auto hash = getTypeHash(IGM, fnType);
      cache = getDiscriminatorForHash(IGM, hash);
      return cache;
    }
    
    // C function pointers are undiscriminated.
    case SILFunctionTypeRepresentation::CXXMethod:
    case SILFunctionTypeRepresentation::CFunctionPointer:
      return toolchain::ConstantInt::get(IGM.Int64Ty, 0);
      
    case SILFunctionTypeRepresentation::ObjCMethod:
    case SILFunctionTypeRepresentation::Block: {
      toolchain_unreachable("not type discriminated");
    }
    }
    toolchain_unreachable("invalid representation");
  };

  auto getCoroutineYieldTypesDiscriminator = [&](CanSILFunctionType fnType) {
    toolchain::ConstantInt *&cache = IGM.getPointerAuthCaches().Types[fnType];
    if (cache) return cache;

    auto hash = getYieldTypesHash(IGM, fnType);
    cache = getDiscriminatorForHash(IGM, hash);
    return cache;
  };

  switch (StoredKind) {
  case Kind::None:
  case Kind::Special:
  case Kind::ValueWitness:
  case Kind::AssociatedType:
  case Kind::AssociatedConformance:
  case Kind::SILFunction:
    toolchain_unreachable("no type for schema using type discrimination");

  case Kind::CoroutineYieldTypes: {
    auto fnType = Storage.get<CanSILFunctionType>(StoredKind);
    return getCoroutineYieldTypesDiscriminator(fnType);
  }

  case Kind::CanSILFunctionType: {
    auto fnType = Storage.get<CanSILFunctionType>(StoredKind);
    return getTypeDiscriminator(fnType);
  }

  case Kind::SILDeclRef: {
    SILDeclRef decl = Storage.get<SILDeclRef>(StoredKind);
    auto fnType = IGM.getSILTypes().getConstantFunctionType(
        TypeExpansionContext::minimal(), decl);
    return getTypeDiscriminator(fnType);
  }
  }
  toolchain_unreachable("bad kind");
}

toolchain::Constant *
IRGenModule::getConstantSignedFunctionPointer(toolchain::Constant *fn,
                                              CanSILFunctionType fnType) {
  if (auto &schema = getFunctionPointerSchema(*this, fnType)) {
    return getConstantSignedPointer(fn, schema, fnType, nullptr);
  }
  return fn;
}

toolchain::Constant *
IRGenModule::getConstantSignedCFunctionPointer(toolchain::Constant *fn) {
  if (auto &schema = getOptions().PointerAuth.FunctionPointers) {
    assert(!schema.hasOtherDiscrimination());
    return getConstantSignedPointer(fn, schema, PointerAuthEntity(), nullptr);
  }
  return fn;
}

toolchain::Constant *
IRGenModule::getConstantSignedPointer(toolchain::Constant *pointer, unsigned key,
                                      toolchain::Constant *storageAddress,
                                      toolchain::ConstantInt *otherDiscriminator) {
  return clang::CodeGen::getConstantSignedPointer(getClangCGM(), pointer, key,
                                                  storageAddress,
                                                  otherDiscriminator);
}

toolchain::Constant *IRGenModule::getConstantSignedPointer(toolchain::Constant *pointer,
                                          const PointerAuthSchema &schema,
                                          const PointerAuthEntity &entity,
                                          toolchain::Constant *storageAddress) {
  // If the schema doesn't sign pointers, do nothing.
  if (!schema)
    return pointer;

  auto otherDiscriminator =
    PointerAuthInfo::getOtherDiscriminator(*this, schema, entity);
  return getConstantSignedPointer(pointer, schema.getKey(), storageAddress,
                   otherDiscriminator);
}

void ConstantAggregateBuilderBase::addSignedPointer(toolchain::Constant *pointer,
                                              const PointerAuthSchema &schema,
                                              const PointerAuthEntity &entity) {
  // If the schema doesn't sign pointers, do nothing.
  if (!schema)
    return add(pointer);

  auto otherDiscriminator =
    PointerAuthInfo::getOtherDiscriminator(IGM(), schema, entity);
  addSignedPointer(pointer, schema.getKey(), schema.isAddressDiscriminated(),
                   otherDiscriminator);
}

void ConstantAggregateBuilderBase::addSignedPointer(toolchain::Constant *pointer,
                                              const PointerAuthSchema &schema,
                                              uint16_t otherDiscriminator) {
  // If the schema doesn't sign pointers, do nothing.
  if (!schema)
    return add(pointer);

  addSignedPointer(pointer, schema.getKey(), schema.isAddressDiscriminated(),
                   toolchain::ConstantInt::get(IGM().Int64Ty, otherDiscriminator));
}

toolchain::ConstantInt* IRGenFunction::getMallocTypeId() {
  return getDiscriminatorForString(IGM, CurFn->getName());
}
