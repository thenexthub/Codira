//===--- DynamicCasts.h - SIL dynamic-cast utilities ------------*- C++ -*-===//
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
// This file provides basic utilities for working with subtyping
// relationships.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_SIL_DYNAMICCASTS_H
#define LANGUAGE_SIL_DYNAMICCASTS_H

#include "language/AST/Module.h"
#include "language/Basic/ProfileCounter.h"
#include "language/SIL/SILInstruction.h"
#include "language/SIL/SILModule.h"
#include "language/SIL/SILValue.h"

namespace language {

class CanType;
class ModuleDecl;
class SILBuilder;
class SILLocation;
class SILModule;
class SILType;
enum class CastConsumptionKind : uint8_t;
struct SILDynamicCastInst;

/// Returns true if the ownership of all references in this type are preserved
/// (without unbalanced retains or releases) during dynamic casting.
bool doesCastPreserveOwnershipForTypes(SILModule &module, CanType sourceType,
                                       CanType targetType);

enum class DynamicCastFeasibility {
  /// The cast will always succeed.
  WillSucceed,

  /// The cast can succeed for some values.
  MaySucceed,

  /// The cast cannot succeed.
  WillFail,
};

static inline DynamicCastFeasibility
atWorst(DynamicCastFeasibility feasibility, DynamicCastFeasibility worstCase) {
  return (feasibility > worstCase ? worstCase : feasibility);
}

static inline DynamicCastFeasibility
atBest(DynamicCastFeasibility feasibility, DynamicCastFeasibility bestCase) {
  return (feasibility < bestCase ? bestCase : feasibility);
}

/// Classify the feasibility of a dynamic cast.  The source and target
/// types should be unlowered formal types.
DynamicCastFeasibility classifyDynamicCast(
    SILFunction *function,
    CanType sourceType, CanType targetType,
    bool isSourceTypeExact = false,
    bool isWholeModuleOpts = false);

/// Returns true if the conformance is not isolated or if its isolation matches
/// the isolation `inFunction`.
bool matchesActorIsolation(ProtocolConformanceRef conformance, SILFunction *inFunction);

SILValue emitSuccessfulScalarUnconditionalCast(SILBuilder &B, SILLocation loc,
                                               SILDynamicCastInst inst);

SILValue emitSuccessfulScalarUnconditionalCast(
    SILBuilder &B, ModuleDecl *M, SILLocation loc, SILValue value,
    SILType loweredTargetType,
    CanType formalSourceType, CanType formalTargetType,
    SILInstruction *existingCast = nullptr);

bool emitSuccessfulIndirectUnconditionalCast(
    SILBuilder &B, ModuleDecl *M, SILLocation loc,
    SILValue src, CanType sourceType,
    SILValue dest, CanType targetType,
    SILInstruction *existingCast = nullptr);

bool emitSuccessfulIndirectUnconditionalCast(SILBuilder &B, SILLocation loc,
                                             SILDynamicCastInst dynamicCast);

/// Can the given cast be performed by the scalar checked-cast instructions in
/// the current SIL stage, or do we need to use the indirect instructions?
bool canSILUseScalarCheckedCastInstructions(SILModule &M,
                                            CanType sourceType,
                                            CanType targetType);

/// Can the given cast be performed by the scalar checked-cast
/// instructions at IRGen, or do we need to use the indirect instructions?
bool canIRGenUseScalarCheckedCastInstructions(SILModule &M,
                                              CanType sourceType,
                                              CanType targetType);

/// Carry out the operations required for an indirect conditional cast
/// using a scalar cast operation.
void emitIndirectConditionalCastWithScalar(
    SILBuilder &B, ModuleDecl *M, SILLocation loc,
    CheckedCastInstOptions options,
    CastConsumptionKind consumption, SILValue src, CanType sourceType,
    SILValue dest, CanType targetType, SILBasicBlock *trueBB,
    SILBasicBlock *falseBB, ProfileCounter TrueCount = ProfileCounter(),
    ProfileCounter FalseCount = ProfileCounter());

/// Does the type conform to the _ObjectiveCBridgeable protocol.
bool isObjectiveCBridgeable(CanType Ty);

/// Get the bridged NS class of a CF class if it exists. Returns
/// an empty CanType if such class does not exist.
CanType getNSBridgedClassOfCFClass(CanType type);

/// Does the type conform to Error.
bool isError(CanType Ty);

struct SILDynamicCastKind {
  enum innerty : std::underlying_type<SILInstructionKind>::type {
#define DYNAMICCAST_INST(ID, PARENT) ID = unsigned(SILInstructionKind::ID),
#include "language/SIL/SILNodes.def"
  } value;

  explicit SILDynamicCastKind(SILInstructionKind kind) {
    auto newValue = SILDynamicCastKind::fromNodeKindHelper(kind);
    assert(newValue && "Non cast passed into SILDynamicCastKind");
    value = newValue.value();
  }

  SILDynamicCastKind(innerty value) : value(value) {}
  operator innerty() const { return value; }

  static std::optional<SILDynamicCastKind>
  fromNodeKind(SILInstructionKind kind) {
    if (auto innerTyOpt = SILDynamicCastKind::fromNodeKindHelper(kind))
      return SILDynamicCastKind(*innerTyOpt);
    return std::nullopt;
  }

private:
  static std::optional<innerty> fromNodeKindHelper(SILInstructionKind kind) {
    switch (kind) {
#define DYNAMICCAST_INST(ID, PARENT)                                           \
  case SILInstructionKind::ID:                                                 \
    return SILDynamicCastKind::ID;
#include "language/SIL/SILNodes.def"
    default:
      return std::nullopt;
    }
  }
};

struct SILDynamicCastInst {
  SILInstruction *inst;

public:
  SILDynamicCastInst() : inst(nullptr) {}

  explicit SILDynamicCastInst(SILInstruction *inst) : inst(inst) {
    assert(classof(inst) && "not a dynamic cast?!");
  }

  static bool classof(const SILInstruction *inst) {
    return bool(SILDynamicCastKind::fromNodeKind(inst->getKind()));
  }

#define DYNAMICCAST_INST(ID, PARENT)                                           \
  SILDynamicCastInst(ID *i) : inst(i) {}
#include "language/SIL/SILNodes.def"

  static SILDynamicCastInst getAs(SILInstruction *inst) {
    auto kind = SILDynamicCastKind::fromNodeKind(inst->getKind());
    if (!kind)
      return SILDynamicCastInst();
    switch (kind.value()) {
#define DYNAMICCAST_INST(ID, PARENT)                                           \
  case SILDynamicCastKind::ID:                                                 \
    return SILDynamicCastInst(cast<ID>(inst));
#include "language/SIL/SILNodes.def"
    }
    toolchain_unreachable("covered switch");
  }

  SILDynamicCastKind getKind() const {
    return SILDynamicCastKind(inst->getKind());
  }

  explicit operator bool() const { return inst != nullptr; }

  SILInstruction *getInstruction() const { return inst; }

  CastConsumptionKind getBridgedConsumptionKind() const {
    switch (getKind()) {
    case SILDynamicCastKind::CheckedCastAddrBranchInst:
      return cast<CheckedCastAddrBranchInst>(inst)->getConsumptionKind();
    // TODO: Bridged casts cannot be expressed by checked_cast_br or
    // checked_cast_value_br yet. Should we ever support it, please
    // review this code.
    case SILDynamicCastKind::CheckedCastBranchInst:
    case SILDynamicCastKind::UnconditionalCheckedCastAddrInst:
      return CastConsumptionKind::TakeAlways;
    case SILDynamicCastKind::UnconditionalCheckedCastInst:
      return CastConsumptionKind::CopyOnSuccess;
    }
    toolchain_unreachable("covered switch");
  }

  CastConsumptionKind getConsumptionKind() const {
    switch (getKind()) {
    case SILDynamicCastKind::CheckedCastAddrBranchInst:
    case SILDynamicCastKind::CheckedCastBranchInst:
    case SILDynamicCastKind::UnconditionalCheckedCastAddrInst:
    case SILDynamicCastKind::UnconditionalCheckedCastInst:
      toolchain_unreachable("unsupported");
    }
  }

  SILBasicBlock *getSuccessBlock() {
    switch (getKind()) {
    case SILDynamicCastKind::CheckedCastAddrBranchInst:
      return cast<CheckedCastAddrBranchInst>(inst)->getSuccessBB();
    case SILDynamicCastKind::CheckedCastBranchInst:
      return cast<CheckedCastBranchInst>(inst)->getSuccessBB();
    case SILDynamicCastKind::UnconditionalCheckedCastAddrInst:
    case SILDynamicCastKind::UnconditionalCheckedCastInst:
      return nullptr;
    }
    toolchain_unreachable("covered switch");
  }

  std::optional<ProfileCounter> getSuccessBlockCount() {
    switch (getKind()) {
    case SILDynamicCastKind::CheckedCastAddrBranchInst:
      toolchain_unreachable("unsupported");
    case SILDynamicCastKind::CheckedCastBranchInst:
      return cast<CheckedCastBranchInst>(inst)->getTrueBBCount();
    case SILDynamicCastKind::UnconditionalCheckedCastAddrInst:
    case SILDynamicCastKind::UnconditionalCheckedCastInst:
      return std::nullopt;
    }
    toolchain_unreachable("covered switch");
  }

  const SILBasicBlock *getSuccessBlock() const {
    return const_cast<SILDynamicCastInst &>(*this).getSuccessBlock();
  }

  SILBasicBlock *getFailureBlock() {
    switch (getKind()) {
    case SILDynamicCastKind::CheckedCastAddrBranchInst:
      return cast<CheckedCastAddrBranchInst>(inst)->getFailureBB();
    case SILDynamicCastKind::CheckedCastBranchInst:
      return cast<CheckedCastBranchInst>(inst)->getFailureBB();
    case SILDynamicCastKind::UnconditionalCheckedCastAddrInst:
    case SILDynamicCastKind::UnconditionalCheckedCastInst:
      return nullptr;
    }
    toolchain_unreachable("covered switch");
  }

  std::optional<ProfileCounter> getFailureBlockCount() {
    switch (getKind()) {
    case SILDynamicCastKind::CheckedCastAddrBranchInst:
      toolchain_unreachable("unsupported");
    case SILDynamicCastKind::CheckedCastBranchInst:
      return cast<CheckedCastBranchInst>(inst)->getFalseBBCount();
    case SILDynamicCastKind::UnconditionalCheckedCastAddrInst:
    case SILDynamicCastKind::UnconditionalCheckedCastInst:
      return std::nullopt;
    }
    toolchain_unreachable("covered switch");
  }

  const SILBasicBlock *getFailureBlock() const {
    return const_cast<SILDynamicCastInst &>(*this).getFailureBlock();
  }

  SILValue getSource() const {
    switch (getKind()) {
    case SILDynamicCastKind::CheckedCastAddrBranchInst:
      return cast<CheckedCastAddrBranchInst>(inst)->getSrc();
    case SILDynamicCastKind::CheckedCastBranchInst:
      return cast<CheckedCastBranchInst>(inst)->getOperand();
    case SILDynamicCastKind::UnconditionalCheckedCastAddrInst:
      return cast<UnconditionalCheckedCastAddrInst>(inst)->getSrc();
    case SILDynamicCastKind::UnconditionalCheckedCastInst:
      return cast<UnconditionalCheckedCastInst>(inst)->getOperand();
    }
    toolchain_unreachable("covered switch");
  }

  // Returns the success value.
  SILValue getDest() const {
    switch (getKind()) {
    case SILDynamicCastKind::CheckedCastAddrBranchInst:
      return cast<CheckedCastAddrBranchInst>(inst)->getDest();
    case SILDynamicCastKind::CheckedCastBranchInst:
      // TODO: Shouldn't this return getSuccessBlock()->getArgument(0)?
      return SILValue();
    case SILDynamicCastKind::UnconditionalCheckedCastAddrInst:
      return cast<UnconditionalCheckedCastAddrInst>(inst)->getDest();
    case SILDynamicCastKind::UnconditionalCheckedCastInst:
      // TODO: Why isn't this:
      //
      // return cast<UnconditionalCheckedCastInst>(inst);
      return SILValue();
    }
    toolchain_unreachable("covered switch");
  }

  CanType getSourceFormalType() const {
    switch (getKind()) {
    case SILDynamicCastKind::CheckedCastAddrBranchInst:
      return cast<CheckedCastAddrBranchInst>(inst)->getSourceFormalType();
    case SILDynamicCastKind::CheckedCastBranchInst:
      return cast<CheckedCastBranchInst>(inst)->getSourceFormalType();
    case SILDynamicCastKind::UnconditionalCheckedCastAddrInst:
      return cast<UnconditionalCheckedCastAddrInst>(inst)->getSourceFormalType();
    case SILDynamicCastKind::UnconditionalCheckedCastInst:
      return cast<UnconditionalCheckedCastInst>(inst)->getSourceFormalType();
    }
    toolchain_unreachable("covered switch");
  }

  SILType getSourceLoweredType() const {
    switch (getKind()) {
    case SILDynamicCastKind::CheckedCastAddrBranchInst:
      return cast<CheckedCastAddrBranchInst>(inst)->getSourceLoweredType();
    case SILDynamicCastKind::CheckedCastBranchInst:
      return cast<CheckedCastBranchInst>(inst)->getSourceLoweredType();
    case SILDynamicCastKind::UnconditionalCheckedCastAddrInst:
      return cast<UnconditionalCheckedCastAddrInst>(inst)->getSourceLoweredType();
    case SILDynamicCastKind::UnconditionalCheckedCastInst:
      return cast<UnconditionalCheckedCastInst>(inst)->getSourceLoweredType();
    }
    toolchain_unreachable("covered switch");
  }

  CanType getTargetFormalType() const {
    switch (getKind()) {
    case SILDynamicCastKind::CheckedCastAddrBranchInst:
      return cast<CheckedCastAddrBranchInst>(inst)->getTargetFormalType();
    case SILDynamicCastKind::CheckedCastBranchInst:
      return cast<CheckedCastBranchInst>(inst)->getTargetFormalType();
    case SILDynamicCastKind::UnconditionalCheckedCastAddrInst:
      return cast<UnconditionalCheckedCastAddrInst>(inst)->getTargetFormalType();
    case SILDynamicCastKind::UnconditionalCheckedCastInst:
      return cast<UnconditionalCheckedCastInst>(inst)->getTargetFormalType();
    }
    toolchain_unreachable("covered switch");
  }

  SILType getTargetLoweredType() const {
    switch (getKind()) {
    case SILDynamicCastKind::CheckedCastAddrBranchInst:
      return cast<CheckedCastAddrBranchInst>(inst)->getDest()->getType();
    case SILDynamicCastKind::CheckedCastBranchInst:
      return cast<CheckedCastBranchInst>(inst)->getTargetLoweredType();
    case SILDynamicCastKind::UnconditionalCheckedCastAddrInst:
      return cast<UnconditionalCheckedCastAddrInst>(inst)->getDest()->getType();
    case SILDynamicCastKind::UnconditionalCheckedCastInst:
      return cast<UnconditionalCheckedCastInst>(inst)->getTargetLoweredType();
    }
    toolchain_unreachable("covered switch");
  }

  bool isSourceTypeExact() const {
    switch (getKind()) {
    case SILDynamicCastKind::CheckedCastBranchInst:
    case SILDynamicCastKind::CheckedCastAddrBranchInst:
    case SILDynamicCastKind::UnconditionalCheckedCastAddrInst:
    case SILDynamicCastKind::UnconditionalCheckedCastInst:
      return isa<MetatypeInst>(getSource());
    }
    toolchain_unreachable("covered switch");
  }

  SILLocation getLocation() const { return inst->getLoc(); }

  SILModule &getModule() const { return inst->getModule(); }
  SILFunction *getFunction() const { return inst->getFunction(); }

  DynamicCastFeasibility classifyFeasibility(bool allowWholeModule) const {
    return language::classifyDynamicCast(
        getFunction(),
        getSourceFormalType(), getTargetFormalType(),
        isSourceTypeExact(), allowWholeModule && getModule().isWholeModule());
  }

  bool isBridgingCast() const {
    // Bridging casts cannot be further simplified.
    auto TargetIsBridgeable = getTargetFormalType()->isBridgeableObjectType();
    auto SourceIsBridgeable = getSourceFormalType()->isBridgeableObjectType();
    return TargetIsBridgeable != SourceIsBridgeable;
  }

  /// Returns true if this dynamic cast can release its source operand.
  bool isRCIdentityPreserving() const;

  /// If getSourceType() is a Codira type that can bridge to an ObjC type, return
  /// the ObjC type it bridges to. If the source type is an objc type, an empty
  /// CanType() is returned.
  CanType getBridgedSourceType() const {
    SILModule &mod = getModule();
    Type t = mod.getASTContext().getBridgedToObjC(mod.getCodiraModule(),
                                                  getSourceFormalType());
    if (!t)
      return CanType();
    return t->getCanonicalType();
  }

  /// If getTargetType() is a Codira type that can bridge to an ObjC type, return
  /// the ObjC type it bridges to. If the target type is an objc type, an empty
  /// CanType() is returned.
  CanType getBridgedTargetType() const {
    SILModule &mod = getModule();
    Type t = mod.getASTContext().getBridgedToObjC(mod.getCodiraModule(),
                                                  getTargetFormalType());
    if (!t)
      return CanType();
    return t->getCanonicalType();
  }

  std::optional<SILType> getLoweredBridgedTargetObjectType() const {
    CanType t = getBridgedTargetType();
    if (!t)
      return std::nullopt;
    return SILType::getPrimitiveObjectType(t);
  }

  bool isConditional() const {
    switch (getKind()) {
    case SILDynamicCastKind::CheckedCastAddrBranchInst: {
      auto f = classifyFeasibility(true /*allow wmo*/);
      return f == DynamicCastFeasibility::MaySucceed;
    }
    case SILDynamicCastKind::CheckedCastBranchInst: {
      auto f = classifyFeasibility(false /*allow wmo*/);
      return f == DynamicCastFeasibility::MaySucceed;
    }
    case SILDynamicCastKind::UnconditionalCheckedCastAddrInst:
    case SILDynamicCastKind::UnconditionalCheckedCastInst:
      return false;
    }
    toolchain_unreachable("covered switch");
  }

  bool canSILUseScalarCheckedCastInstructions() const {
    return language::canSILUseScalarCheckedCastInstructions(
        getModule(), getSourceFormalType(), getTargetFormalType());
  }

  CheckedCastInstOptions getCheckedCastOptions() const {
    switch (getKind()) {
    case SILDynamicCastKind::CheckedCastAddrBranchInst:
      return cast<CheckedCastAddrBranchInst>(inst)->getCheckedCastOptions();
    case SILDynamicCastKind::CheckedCastBranchInst:
      return cast<CheckedCastBranchInst>(inst)->getCheckedCastOptions();
    case SILDynamicCastKind::UnconditionalCheckedCastAddrInst:
      return cast<UnconditionalCheckedCastAddrInst>(inst)
          ->getCheckedCastOptions();
    case SILDynamicCastKind::UnconditionalCheckedCastInst:
      return cast<UnconditionalCheckedCastInst>(inst)
          ->getCheckedCastOptions();
    }
  }
};

} // end namespace language

#endif

