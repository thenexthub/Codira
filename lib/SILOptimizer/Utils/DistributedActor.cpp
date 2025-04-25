//===-- DistributedActor.cpp - SIL utils for distributed actors -*- C++ -*-===//
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

#include "language/SILOptimizer/Utils/DistributedActor.h"

#include "language/AST/ConformanceLookup.h"
#include "language/AST/Decl.h"
#include "language/Basic/Assertions.h"
#include "language/SIL/SILBuilder.h"
#include "language/SIL/SILLocation.h"

namespace language {

void emitDistributedActorSystemWitnessCall(
    SILBuilder &B, SILLocation loc, DeclName methodName, SILValue base,
    // types to be passed through to SubstitutionMap:
    SILType actorType,
    // call arguments, except the base which will be passed last
    ArrayRef<SILValue> args,
    std::optional<std::pair<SILBasicBlock *, SILBasicBlock *>> tryTargets) {
  auto &F = B.getFunction();
  auto &M = B.getModule();
  auto &C = F.getASTContext();

  // Dig out the conformance to DistributedActorSystem.
  ProtocolDecl *DAS = C.getDistributedActorSystemDecl();
  assert(DAS);
  auto systemASTType = base->getType().getASTType();
  ProtocolConformanceRef systemConfRef;

  // If the base is an existential open it.
  if (systemASTType->isAnyExistentialType()) {
    systemASTType = ExistentialArchetypeType::getAny(systemASTType)
        ->getCanonicalType();
    base = B.createOpenExistentialAddr(
        loc, base, F.getLoweredType(systemASTType),
        OpenedExistentialAccess::Immutable);
  }

  systemConfRef = lookupConformance(systemASTType, DAS);
  assert(!systemConfRef.isInvalid() &&
         "Missing conformance to `DistributedActorSystem`");

  // Dig out the method.
  auto method = cast<FuncDecl>(DAS->getSingleRequirement(methodName));
  auto methodRef = SILDeclRef(method, SILDeclRef::Kind::Func);
  auto methodSILTy =
      M.Types.getConstantInfo(B.getTypeExpansionContext(), methodRef)
          .getSILType();

  auto witnessMethod = B.createWitnessMethod(
      loc, systemASTType, systemConfRef, methodRef, methodSILTy);

  // prepare conformance substitutions
  SubstitutionMap subs;
  {
    auto genericSig = method->getGenericSignature();
    SmallVector<Type, 2> subTypes;
    SmallVector<ProtocolConformanceRef, 2> subConformances;
    subTypes.push_back(systemASTType);
    subConformances.push_back(systemConfRef);
    if (actorType) {
      ProtocolDecl *actorProto = C.getProtocol(
          KnownProtocolKind::DistributedActor);
      assert(actorProto);

      ProtocolConformanceRef conformance;
      auto distributedActorConfRef = lookupConformance(
          actorType.getASTType(), actorProto);
      assert(!distributedActorConfRef.isInvalid() &&
             "Missing conformance to `DistributedActor`");
      subTypes.push_back(actorType.getASTType());
      subConformances.push_back(distributedActorConfRef);
    }

    subs = SubstitutionMap::get(genericSig, subTypes, subConformances);
  }

  std::optional<SILValue> temporaryArgumentBuffer;

  // If the self parameter is indirect but the base is a value, put it
  // into a temporary allocation.
  auto methodSILFnTy = methodSILTy.castTo<SILFunctionType>();
  std::optional<SILValue> temporaryActorSystemBuffer;
  if (methodSILFnTy->getSelfParameter().isFormalIndirect() &&
      !base->getType().isAddress()) {
    auto buf = B.createAllocStack(loc, base->getType(), std::nullopt);
    base = B.emitCopyValueOperation(loc, base);
    B.emitStoreValueOperation(
        loc, base, buf, StoreOwnershipQualifier::Init);
    temporaryActorSystemBuffer = SILValue(buf);
  }

  // === Call the method.
  // --- Push the arguments
  SmallVector<SILValue, 2> allArgs;
  auto params = methodSILFnTy->getParameters();
  for (size_t i = 0; i < args.size(); ++i) {
    auto arg = args[i];
    if (params[i].isFormalIndirect() &&
        !arg->getType().isAddress() &&
        !dyn_cast<AnyMetatypeType>(arg->getType().getASTType())) {
      auto buf = B.createAllocStack(loc, arg->getType(), std::nullopt);
      auto argCopy = B.emitCopyValueOperation(loc, arg);
      B.emitStoreValueOperation(
          loc, argCopy, buf, StoreOwnershipQualifier::Init);
      temporaryArgumentBuffer = SILValue(buf);
      allArgs.push_back(*temporaryArgumentBuffer);
    } else {
      allArgs.push_back(arg);
    }
  }
  // Push the self argument
  auto selfArg = temporaryActorSystemBuffer ? *temporaryActorSystemBuffer : base;
  allArgs.push_back(selfArg);

  SILInstruction *apply;
  if (tryTargets) {
    apply = B.createTryApply(
        loc, witnessMethod, subs, allArgs, tryTargets->first,
        tryTargets->second);
  } else {
    apply = B.createApply(loc, witnessMethod, subs, allArgs);
  }

  // Local function to emit a cleanup after the call.
  auto emitCleanup = [&](llvm::function_ref<void(SILBuilder &builder)> fn) {
    if (tryTargets) {
      {
        SILBuilderWithScope normalBuilder(tryTargets->first, apply);
        fn(normalBuilder);
      }
      {
        SILBuilderWithScope errorBuilder(tryTargets->second, apply);
        fn(errorBuilder);
      }
    } else {
      fn(B);
    }
  };

  // ==== If we had to create a buffers we need to clean them up
  // --- Cleanup id buffer
  if (temporaryArgumentBuffer) {
    emitCleanup([&](SILBuilder & builder) {
      auto value = builder.emitLoadValueOperation(
          loc, *temporaryArgumentBuffer, LoadOwnershipQualifier::Take);
      builder.emitDestroyValueOperation(loc, value);
      builder.createDeallocStack(loc, *temporaryArgumentBuffer);
    });
  }
  // --- Cleanup base buffer
  if (temporaryActorSystemBuffer) {
    emitCleanup([&](SILBuilder & builder) {
      auto value = builder.emitLoadValueOperation(
          loc, *temporaryActorSystemBuffer, LoadOwnershipQualifier::Take);
      builder.emitDestroyValueOperation(loc, value);
      builder.createDeallocStack(loc, *temporaryActorSystemBuffer);
    });
  }
}

void emitActorReadyCall(SILBuilder &B, SILLocation loc, SILValue actor,
                        SILValue actorSystem) {

  auto &F = B.getFunction();
  auto &C = F.getASTContext();
  emitDistributedActorSystemWitnessCall(
      B, loc, C.Id_actorReady, actorSystem,
      actor->getType(), { actor });
}

void emitResignIdentityCall(SILBuilder &B, SILLocation loc,
                            ClassDecl* actorDecl,
                            SILValue actor, SILValue idRef) {
  auto &F = B.getFunction();
  auto &C = F.getASTContext();

  SILValue systemRef = refDistributedActorSystem(B, loc, actorDecl, actor);

  emitDistributedActorSystemWitnessCall(
      B, loc, C.Id_resignID,
      systemRef,
      SILType(),
      { idRef });
}

/// Creates a reference to the distributed actor's \p actorSystem
/// stored property.
SILValue refDistributedActorSystem(SILBuilder &b,
                                   SILLocation loc,
                                   ClassDecl *actDecl,
                                   SILValue actorInstance) {
  assert(actDecl);
  assert(actDecl->isDistributedActor());

  // get the VarDecl corresponding to the actorSystem.
  auto refs = actDecl->lookupDirect(actDecl->getASTContext().Id_actorSystem);
  assert(refs.size() == 1);
  VarDecl *actorSystemVar = dyn_cast<VarDecl>(refs.front());

  return b.createRefElementAddr(loc, actorInstance, actorSystemVar);
}

} // namespace language
