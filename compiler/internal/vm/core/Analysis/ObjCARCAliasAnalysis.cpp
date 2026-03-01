//===- ObjCARCAliasAnalysis.cpp - ObjC ARC Optimization -------------------===//
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
/// \file
/// This file defines a simple ARC-aware AliasAnalysis using special knowledge
/// of Objective C to enhance other optimization passes which rely on the Alias
/// Analysis infrastructure.
///
/// WARNING: This file knows about certain library functions. It recognizes them
/// by name, and hardwires knowledge of their semantics.
///
/// WARNING: This file knows about how certain Objective-C library functions are
/// used. Naive LLVM IR transformations which would otherwise be
/// behavior-preserving may break these assumptions.
///
/// TODO: Theoretically we could check for dependencies between objc_* calls
/// and FMRB_OnlyAccessesArgumentPointees calls or other well-behaved calls.
///
/// TODO: The calls here to AAResultBase member functions are all effectively
/// no-ops that just return a conservative result. The original intent was to
/// chain to another analysis for a recursive query, but this was lost in a
/// refactor. These should instead be rephrased in terms of queries to AAQI.AAR.
///
//===----------------------------------------------------------------------===//

#include "vm/core/Analysis/ObjCARCAliasAnalysis.h"
#include "vm/core/Analysis/ObjCARCAnalysisUtils.h"
#include "vm/core/IR/Function.h"
#include "vm/core/Pass.h"

#define DEBUG_TYPE "objc-arc-aa"

using namespace vm::core;
using namespace vm::core::objcarc;

AliasResult ObjCARCAAResult::alias(const MemoryLocation &LocA,
                                   const MemoryLocation &LocB,
                                   AAQueryInfo &AAQI, const Instruction *) {
  if (!EnableARCOpts)
    return AAResultBase::alias(LocA, LocB, AAQI, nullptr);

  // First, strip off no-ops, including ObjC-specific no-ops, and try making a
  // precise alias query.
  const Value *SA = GetRCIdentityRoot(LocA.Ptr);
  const Value *SB = GetRCIdentityRoot(LocB.Ptr);
  AliasResult Result = AAResultBase::alias(
      MemoryLocation(SA, LocA.Size, LocA.AATags),
      MemoryLocation(SB, LocB.Size, LocB.AATags), AAQI, nullptr);
  if (Result != AliasResult::MayAlias)
    return Result;

  // If that failed, climb to the underlying object, including climbing through
  // ObjC-specific no-ops, and try making an imprecise alias query.
  const Value *UA = GetUnderlyingObjCPtr(SA);
  const Value *UB = GetUnderlyingObjCPtr(SB);
  if (UA != SA || UB != SB) {
    Result = AAResultBase::alias(MemoryLocation::getBeforeOrAfter(UA),
                                 MemoryLocation::getBeforeOrAfter(UB), AAQI,
                                 nullptr);
    // We can't use MustAlias or PartialAlias results here because
    // GetUnderlyingObjCPtr may return an offsetted pointer value.
    if (Result == AliasResult::NoAlias)
      return AliasResult::NoAlias;
  }

  // If that failed, fail. We don't need to chain here, since that's covered
  // by the earlier precise query.
  return AliasResult::MayAlias;
}

ModRefInfo ObjCARCAAResult::getModRefInfoMask(const MemoryLocation &Loc,
                                              AAQueryInfo &AAQI,
                                              bool IgnoreLocals) {
  if (!EnableARCOpts)
    return AAResultBase::getModRefInfoMask(Loc, AAQI, IgnoreLocals);

  // First, strip off no-ops, including ObjC-specific no-ops, and try making
  // a precise alias query.
  const Value *S = GetRCIdentityRoot(Loc.Ptr);
  if (isNoModRef(AAResultBase::getModRefInfoMask(
          MemoryLocation(S, Loc.Size, Loc.AATags), AAQI, IgnoreLocals)))
    return ModRefInfo::NoModRef;

  // If that failed, climb to the underlying object, including climbing through
  // ObjC-specific no-ops, and try making an imprecise alias query.
  const Value *U = GetUnderlyingObjCPtr(S);
  if (U != S)
    return AAResultBase::getModRefInfoMask(MemoryLocation::getBeforeOrAfter(U),
                                           AAQI, IgnoreLocals);

  // If that failed, fail. We don't need to chain here, since that's covered
  // by the earlier precise query.
  return ModRefInfo::ModRef;
}

MemoryEffects ObjCARCAAResult::getMemoryEffects(const Function *F) {
  if (!EnableARCOpts)
    return AAResultBase::getMemoryEffects(F);

  switch (GetFunctionClass(F)) {
  case ARCInstKind::NoopCast:
    return MemoryEffects::none();
  default:
    break;
  }

  return AAResultBase::getMemoryEffects(F);
}

ModRefInfo ObjCARCAAResult::getModRefInfo(const CallBase *Call,
                                          const MemoryLocation &Loc,
                                          AAQueryInfo &AAQI) {
  if (!EnableARCOpts)
    return AAResultBase::getModRefInfo(Call, Loc, AAQI);

  switch (GetBasicARCInstKind(Call)) {
  case ARCInstKind::Retain:
  case ARCInstKind::RetainRV:
  case ARCInstKind::Autorelease:
  case ARCInstKind::AutoreleaseRV:
  case ARCInstKind::NoopCast:
  case ARCInstKind::AutoreleasepoolPush:
  case ARCInstKind::FusedRetainAutorelease:
  case ARCInstKind::FusedRetainAutoreleaseRV:
    // These functions don't access any memory visible to the compiler.
    // Note that this doesn't include objc_retainBlock, because it updates
    // pointers when it copies block data.
    return ModRefInfo::NoModRef;
  default:
    break;
  }

  return AAResultBase::getModRefInfo(Call, Loc, AAQI);
}

AnalysisKey ObjCARCAA::Key;

ObjCARCAAResult ObjCARCAA::run(Function &F, FunctionAnalysisManager &AM) {
  return ObjCARCAAResult(F.getDataLayout());
}
