//===--- SILBridgingImpl.h ------------------------------------------------===//
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
// This file contains the implementation of bridging functions, which are either
// - depending on if PURE_BRIDGING_MODE is set - included in the cpp file or
// in the header file.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_SIL_SILBRIDGING_IMPL_H
#define LANGUAGE_SIL_SILBRIDGING_IMPL_H

#include "SILBridging.h"
#include "language/AST/Builtins.h"
#include "language/AST/Decl.h"
#include "language/AST/SourceFile.h"
#include "language/AST/StorageImpl.h"
#include "language/AST/SubstitutionMap.h"
#include "language/AST/Types.h"
#include "language/Basic/BasicBridging.h"
#include "language/Basic/Nullability.h"
#include "language/SIL/ApplySite.h"
#include "language/SIL/DynamicCasts.h"
#include "language/SIL/InstWrappers.h"
#include "language/SIL/SILBuilder.h"
#include "language/SIL/SILDefaultWitnessTable.h"
#include "language/SIL/SILFunctionConventions.h"
#include "language/SIL/SILInstruction.h"
#include "language/SIL/SILLocation.h"
#include "language/SIL/SILModule.h"
#include "language/SIL/SILVTable.h"
#include "language/SIL/SILWitnessTable.h"
#include "language/SILOptimizer/Utils/ConstExpr.h"
#include "language/SIL/SILConstants.h"
#include <stdbool.h>
#include <stddef.h>
#include <string>

LANGUAGE_BEGIN_NULLABILITY_ANNOTATIONS

//===----------------------------------------------------------------------===//
//                             BridgedResultInfo
//===----------------------------------------------------------------------===//

BridgedResultConvention BridgedResultInfo::castToResultConvention(language::ResultConvention convention) {
  return static_cast<BridgedResultConvention>(convention);
}

BridgedResultInfo::BridgedResultInfo(language::SILResultInfo resultInfo):
  type(resultInfo.getInterfaceType().getPointer()),
  convention(castToResultConvention(resultInfo.getConvention()))
{}

CodiraInt BridgedResultInfoArray::count() const {
  return resultInfoArray.unbridged<language::SILResultInfo>().size();
}

BridgedResultInfo BridgedResultInfoArray::at(CodiraInt resultIndex) const {
  return BridgedResultInfo(resultInfoArray.unbridged<language::SILResultInfo>()[resultIndex]);
}

//===----------------------------------------------------------------------===//
//                             BridgedYieldInfo
//===----------------------------------------------------------------------===//

CodiraInt BridgedYieldInfoArray::count() const {
  return yieldInfoArray.unbridged<language::SILYieldInfo>().size();
}

BridgedParameterInfo BridgedYieldInfoArray::at(CodiraInt resultIndex) const {
  return BridgedParameterInfo(yieldInfoArray.unbridged<language::SILYieldInfo>()[resultIndex]);
}

//===----------------------------------------------------------------------===//
//                            BridgedParameterInfo
//===----------------------------------------------------------------------===//

inline language::ParameterConvention getParameterConvention(BridgedArgumentConvention convention) {
  switch (convention) {
    case BridgedArgumentConvention::Indirect_In:             return language::ParameterConvention::Indirect_In;
    case BridgedArgumentConvention::Indirect_In_Guaranteed:  return language::ParameterConvention::Indirect_In_Guaranteed;
    case BridgedArgumentConvention::Indirect_Inout:          return language::ParameterConvention::Indirect_Inout;
    case BridgedArgumentConvention::Indirect_InoutAliasable: return language::ParameterConvention::Indirect_InoutAliasable;
    case BridgedArgumentConvention::Indirect_In_CXX:         return language::ParameterConvention::Indirect_In_CXX;
    case BridgedArgumentConvention::Indirect_Out:            break;
    case BridgedArgumentConvention::Direct_Owned:            return language::ParameterConvention::Direct_Owned;
    case BridgedArgumentConvention::Direct_Unowned:          return language::ParameterConvention::Direct_Unowned;
    case BridgedArgumentConvention::Direct_Guaranteed:       return language::ParameterConvention::Direct_Guaranteed;
    case BridgedArgumentConvention::Pack_Owned:              return language::ParameterConvention::Pack_Owned;
    case BridgedArgumentConvention::Pack_Inout:              return language::ParameterConvention::Pack_Inout;
    case BridgedArgumentConvention::Pack_Guaranteed:         return language::ParameterConvention::Pack_Guaranteed;
    case BridgedArgumentConvention::Pack_Out:                break;
  }
  toolchain_unreachable("invalid parameter convention");
}

inline BridgedArgumentConvention getArgumentConvention(language::ParameterConvention convention) {
  switch (convention) {
    case language::ParameterConvention::Indirect_In:              return BridgedArgumentConvention::Indirect_In;
    case language::ParameterConvention::Indirect_In_Guaranteed:   return BridgedArgumentConvention::Indirect_In_Guaranteed;
    case language::ParameterConvention::Indirect_Inout:           return BridgedArgumentConvention::Indirect_Inout;
    case language::ParameterConvention::Indirect_InoutAliasable:  return BridgedArgumentConvention::Indirect_InoutAliasable;
    case language::ParameterConvention::Indirect_In_CXX:          return BridgedArgumentConvention::Indirect_In_CXX;
    case language::ParameterConvention::Direct_Owned:             return BridgedArgumentConvention::Direct_Owned;
    case language::ParameterConvention::Direct_Unowned:           return BridgedArgumentConvention::Direct_Unowned;
    case language::ParameterConvention::Direct_Guaranteed:        return BridgedArgumentConvention::Direct_Guaranteed;
    case language::ParameterConvention::Pack_Owned:               return BridgedArgumentConvention::Pack_Owned;
    case language::ParameterConvention::Pack_Inout:               return BridgedArgumentConvention::Pack_Inout;
    case language::ParameterConvention::Pack_Guaranteed:          return BridgedArgumentConvention::Pack_Guaranteed;
  }
  toolchain_unreachable("invalid parameter convention");
}

inline BridgedArgumentConvention castToArgumentConvention(language::SILArgumentConvention convention) {
  return static_cast<BridgedArgumentConvention>(convention.Value);
}

BridgedParameterInfo::BridgedParameterInfo(language::SILParameterInfo parameterInfo):
  type(parameterInfo.getInterfaceType()),
  convention(getArgumentConvention(parameterInfo.getConvention())),
  options(parameterInfo.getOptions().toRaw())
{}

language::SILParameterInfo BridgedParameterInfo::unbridged() const {
  return language::SILParameterInfo(type.unbridged(), getParameterConvention(convention),
                                 language::SILParameterInfo::Options(options));
}

CodiraInt BridgedParameterInfoArray::count() const {
  return parameterInfoArray.unbridged<language::SILParameterInfo>().size();
}

BridgedParameterInfo BridgedParameterInfoArray::at(CodiraInt parameterIndex) const {
  return BridgedParameterInfo(parameterInfoArray.unbridged<language::SILParameterInfo>()[parameterIndex]);
}

//===----------------------------------------------------------------------===//
//                       BridgedLifetimeDependenceInfo
//===----------------------------------------------------------------------===//

BridgedLifetimeDependenceInfo::BridgedLifetimeDependenceInfo(language::LifetimeDependenceInfo info)
    : inheritLifetimeParamIndices(info.getInheritIndices()),
      scopeLifetimeParamIndices(info.getScopeIndices()),
      addressableParamIndices(info.getAddressableIndices()),
      conditionallyAddressableParamIndices(
        info.getConditionallyAddressableIndices()),
      targetIndex(info.getTargetIndex()), immortal(info.isImmortal()) {}

CodiraInt BridgedLifetimeDependenceInfoArray::count() const {
  return lifetimeDependenceInfoArray.unbridged<language::LifetimeDependenceInfo>().size();
}

BridgedLifetimeDependenceInfo
BridgedLifetimeDependenceInfoArray::at(CodiraInt index) const {
  return BridgedLifetimeDependenceInfo(lifetimeDependenceInfoArray.unbridged<language::LifetimeDependenceInfo>()[index]);
}

bool BridgedLifetimeDependenceInfo::empty() const {
  return !immortal && inheritLifetimeParamIndices == nullptr &&
         scopeLifetimeParamIndices == nullptr;
}

bool BridgedLifetimeDependenceInfo::checkInherit(CodiraInt index) const {
  return inheritLifetimeParamIndices &&
         inheritLifetimeParamIndices->contains(index);
}

bool BridgedLifetimeDependenceInfo::checkScope(CodiraInt index) const {
  return scopeLifetimeParamIndices &&
         scopeLifetimeParamIndices->contains(index);
}

bool BridgedLifetimeDependenceInfo::checkAddressable(CodiraInt index) const {
  return addressableParamIndices && addressableParamIndices->contains(index);
}

bool BridgedLifetimeDependenceInfo::
checkConditionallyAddressable(CodiraInt index) const {
  return conditionallyAddressableParamIndices
    && conditionallyAddressableParamIndices->contains(index);
}

CodiraInt BridgedLifetimeDependenceInfo::getTargetIndex() const {
  return targetIndex;
}

BridgedOwnedString BridgedLifetimeDependenceInfo::getDebugDescription() const {
  std::string lifetimeDependenceString;
  auto getOnIndices = [](language::IndexSubset *bitvector) {
    std::string result;
    bool isFirstSetBit = true;
    for (unsigned i = 0; i < bitvector->getCapacity(); i++) {
      if (bitvector->contains(i)) {
        if (!isFirstSetBit) {
          result += ", ";
        }
        result += std::to_string(i);
        isFirstSetBit = false;
      }
    }
    return result;
  };
  if (inheritLifetimeParamIndices && !inheritLifetimeParamIndices->isEmpty()) {
    lifetimeDependenceString =
        "_inherit(" + getOnIndices(inheritLifetimeParamIndices) + ") ";
  }
  if (scopeLifetimeParamIndices && !scopeLifetimeParamIndices->isEmpty()) {
    lifetimeDependenceString +=
        "_scope(" + getOnIndices(scopeLifetimeParamIndices) + ") ";
  }
  return BridgedOwnedString(lifetimeDependenceString);
}

//===----------------------------------------------------------------------===//
//                               SILFunctionType
//===----------------------------------------------------------------------===//

BridgedResultInfoArray
SILFunctionType_getResultsWithError(BridgedCanType funcTy) {
  return {funcTy.unbridged()->castTo<language::SILFunctionType>()->getResultsWithError()};
}

CodiraInt SILFunctionType_getNumIndirectFormalResultsWithError(BridgedCanType funcTy) {
  auto fnTy = funcTy.unbridged()->castTo<language::SILFunctionType>();
  return fnTy->getNumIndirectFormalResults()
    + (fnTy->hasIndirectErrorResult() ? 1 : 0);
}

CodiraInt SILFunctionType_getNumPackResults(BridgedCanType funcTy) {
  return funcTy.unbridged()->castTo<language::SILFunctionType>()
    ->getNumPackResults();
}

OptionalBridgedResultInfo SILFunctionType_getErrorResult(BridgedCanType funcTy) {
  auto fnTy = funcTy.unbridged()->castTo<language::SILFunctionType>();
  auto resultInfo = fnTy->getOptionalErrorResult();
  if (resultInfo) {
    return {resultInfo->getInterfaceType().getPointer(),
            BridgedResultInfo::castToResultConvention(resultInfo->getConvention())};
  }
  return {nullptr, BridgedResultConvention::Indirect};

}

BridgedParameterInfoArray SILFunctionType_getParameters(BridgedCanType funcTy) {
  return {funcTy.unbridged()->castTo<language::SILFunctionType>()->getParameters()};
}

bool SILFunctionType_hasSelfParam(BridgedCanType funcTy) {
  return funcTy.unbridged()->castTo<language::SILFunctionType>()->hasSelfParam();
}

bool SILFunctionType_isTrivialNoescape(BridgedCanType funcTy) {
  return funcTy.unbridged()->castTo<language::SILFunctionType>()->isTrivialNoEscape();
}

BridgedYieldInfoArray SILFunctionType_getYields(BridgedCanType funcTy) {
  return {funcTy.unbridged()->castTo<language::SILFunctionType>()->getYields()};
}

BridgedLifetimeDependenceInfoArray
SILFunctionType_getLifetimeDependencies(BridgedCanType funcTy) {
  auto fnTy = funcTy.unbridged()->castTo<language::SILFunctionType>();
  return {fnTy->getLifetimeDependencies()};
}

//===----------------------------------------------------------------------===//
//                                BridgedType
//===----------------------------------------------------------------------===//

BridgedType::BridgedType() : opaqueValue(nullptr) {}

BridgedType::BridgedType(language::SILType t) : opaqueValue(t.getOpaqueValue()) {}

language::SILType BridgedType::unbridged() const {
  return language::SILType::getFromOpaqueValue(opaqueValue);
}

static_assert(sizeof(BridgedType::EnumElementIterator) >= sizeof(language::EnumDecl::ElementRange::iterator));

static inline BridgedType::EnumElementIterator bridge(language::EnumDecl::ElementRange::iterator i) {
  BridgedType::EnumElementIterator bridgedIter;
  *reinterpret_cast<language::EnumDecl::ElementRange::iterator *>(&bridgedIter.storage) = i;
  return bridgedIter;
}

static inline language::EnumDecl::ElementRange::iterator unbridge(BridgedType::EnumElementIterator i) {
  return *reinterpret_cast<const language::EnumDecl::ElementRange::iterator *>(&i.storage);
}

BridgedType::EnumElementIterator BridgedType::EnumElementIterator::getNext() const {
  return bridge(std::next(unbridge(*this)));
}

BridgedType BridgedType::createSILType(BridgedCanType canTy) {
  auto ty = canTy.unbridged();
  if (ty->isLegalSILType())
    return language::SILType::getPrimitiveObjectType(ty);
  return language::SILType();
}

BridgedOwnedString BridgedType::getDebugDescription() const {
  return BridgedOwnedString(unbridged().getDebugDescription());
}

bool BridgedType::isNull() const {
  return unbridged().isNull();
}

bool BridgedType::isAddress() const {
  return unbridged().isAddress();
}

BridgedCanType BridgedType::getCanType() const {
  return unbridged().getASTType();
}

BridgedType BridgedType::getAddressType() const {
  return unbridged().getAddressType();
}

BridgedType BridgedType::getObjectType() const {
  return unbridged().getObjectType();
}

bool BridgedType::isTrivial(BridgedFunction f) const {
  return unbridged().isTrivial(f.getFunction());
}

bool BridgedType::isNonTrivialOrContainsRawPointer(BridgedFunction f) const {
  return unbridged().isNonTrivialOrContainsRawPointer(f.getFunction());
}

bool BridgedType::isLoadable(BridgedFunction f) const {
  return unbridged().isLoadable(f.getFunction());
}

bool BridgedType::isReferenceCounted(BridgedFunction f) const {
  return unbridged().isReferenceCounted(f.getFunction());
}

bool BridgedType::containsNoEscapeFunction() const {
  return unbridged().containsNoEscapeFunction();
}

bool BridgedType::isEmpty(BridgedFunction f) const {
  return unbridged().isEmpty(*f.getFunction());
}

bool BridgedType::isMoveOnly() const {
  return unbridged().isMoveOnly();
}

bool BridgedType::isEscapable(BridgedFunction f) const {
  return unbridged().isEscapable(*f.getFunction());
}

bool BridgedType::isExactSuperclassOf(BridgedType t) const {
  return unbridged().isExactSuperclassOf(t.unbridged());
}

bool BridgedType::isMarkedAsImmortal() const {
  return unbridged().isMarkedAsImmortal();
}

bool BridgedType::isAddressableForDeps(BridgedFunction f) const {
  return unbridged().isAddressableForDeps(*f.getFunction());
}

CodiraInt BridgedType::getCaseIdxOfEnumType(BridgedStringRef name) const {
  return unbridged().getCaseIdxOfEnumType(name.unbridged());
}

CodiraInt BridgedType::getNumBoxFields(BridgedCanType boxTy) {
  return boxTy.unbridged()->castTo<language::SILBoxType>()->getLayout()->getFields().size();
}

BridgedType BridgedType::getBoxFieldType(BridgedCanType boxTy, CodiraInt idx, BridgedFunction f) {
  auto *fn = f.getFunction();
  return language::getSILBoxFieldType(fn->getTypeExpansionContext(), boxTy.unbridged()->castTo<language::SILBoxType>(),
                                   fn->getModule().Types, idx);
}

bool BridgedType::getBoxFieldIsMutable(BridgedCanType boxTy, CodiraInt idx) {
  return boxTy.unbridged()->castTo<language::SILBoxType>()->getLayout()->getFields()[idx].isMutable();
}

CodiraInt BridgedType::getNumNominalFields() const {
  return unbridged().getNumNominalFields();
}


BridgedType BridgedType::getFieldType(CodiraInt idx, BridgedFunction f) const {
  return unbridged().getFieldType(idx, f.getFunction());
}

CodiraInt BridgedType::getFieldIdxOfNominalType(BridgedStringRef name) const {
  return unbridged().getFieldIdxOfNominalType(name.unbridged());
}

BridgedStringRef BridgedType::getFieldName(CodiraInt idx) const {
  return unbridged().getFieldName(idx);
}

BridgedType::EnumElementIterator BridgedType::getFirstEnumCaseIterator() const {
  language::EnumDecl *enumDecl = unbridged().getEnumOrBoundGenericEnum();
  return bridge(enumDecl->getAllElements().begin());
}

bool BridgedType::isEndCaseIterator(EnumElementIterator i) const {
  language::EnumDecl *enumDecl = unbridged().getEnumOrBoundGenericEnum();
  return unbridge(i) == enumDecl->getAllElements().end();
}

BridgedType BridgedType::getEnumCasePayload(EnumElementIterator i, BridgedFunction f) const {
  language::EnumElementDecl *elt = *unbridge(i);
  if (elt->hasAssociatedValues())
    return unbridged().getEnumElementType(elt, f.getFunction());
  return language::SILType();
}

CodiraInt BridgedType::getNumTupleElements() const {
  return unbridged().getNumTupleElements();
}

BridgedType BridgedType::getTupleElementType(CodiraInt idx) const {
  return unbridged().getTupleElementType(idx);
}

BridgedType BridgedType::getFunctionTypeWithNoEscape(bool withNoEscape) const {
  auto fnType = unbridged().getAs<language::SILFunctionType>();
  auto newTy = fnType->getWithExtInfo(fnType->getExtInfo().withNoEscape(true));
  return language::SILType::getPrimitiveObjectType(newTy);
}

BridgedArgumentConvention BridgedType::getCalleeConvention() const {
  auto fnType = unbridged().getAs<language::SILFunctionType>();
  return getArgumentConvention(fnType->getCalleeConvention());
}

//===----------------------------------------------------------------------===//
//                                BridgedValue
//===----------------------------------------------------------------------===//

static inline BridgedValue::Ownership bridge(language::OwnershipKind ownership) {
  switch (ownership) {
    case language::OwnershipKind::Any:
      toolchain_unreachable("Invalid ownership for value");
    case language::OwnershipKind::Unowned:    return BridgedValue::Ownership::Unowned;
    case language::OwnershipKind::Owned:      return BridgedValue::Ownership::Owned;
    case language::OwnershipKind::Guaranteed: return BridgedValue::Ownership::Guaranteed;
    case language::OwnershipKind::None:       return BridgedValue::Ownership::None;
  }
}

language::ValueOwnershipKind BridgedValue::unbridge(Ownership ownership) {
  switch (ownership) {
    case BridgedValue::Ownership::Unowned:    return language::OwnershipKind::Unowned;
    case BridgedValue::Ownership::Owned:      return language::OwnershipKind::Owned;
    case BridgedValue::Ownership::Guaranteed: return language::OwnershipKind::Guaranteed;
    case BridgedValue::Ownership::None:       return language::OwnershipKind::None;
  }
}

language::ValueBase * _Nonnull BridgedValue::getSILValue() const {
  return static_cast<language::ValueBase *>(obj);
}

language::ValueBase * _Nullable OptionalBridgedValue::getSILValue() const {
  if (obj)
    return static_cast<language::ValueBase *>(obj);
  return nullptr;
}

OptionalBridgedOperand BridgedValue::getFirstUse() const {
  return {*getSILValue()->use_begin()};
}

BridgedType BridgedValue::getType() const {
  return getSILValue()->getType();
}

BridgedValue::Ownership BridgedValue::getOwnership() const {
  return bridge(getSILValue()->getOwnershipKind());
}

BridgedFunction BridgedValue::SILUndef_getParentFunction() const {
  return {toolchain::cast<language::SILUndef>(getSILValue())->getParent()};
}

BridgedFunction BridgedValue::PlaceholderValue_getParentFunction() const {
  return {toolchain::cast<language::PlaceholderValue>(getSILValue())->getParent()};
}

//===----------------------------------------------------------------------===//
//                                BridgedOperand
//===----------------------------------------------------------------------===//

bool BridgedOperand::isTypeDependent() const { return op->isTypeDependent(); }

bool BridgedOperand::isLifetimeEnding() const { return op->isLifetimeEnding(); }

bool BridgedOperand::canAcceptOwnership(BridgedValue::Ownership ownership) const {
  return op->canAcceptKind(BridgedValue::unbridge(ownership));
}

bool BridgedOperand::isDeleted() const {
  return op->getUser() == nullptr;
}

OptionalBridgedOperand BridgedOperand::getNextUse() const {
  return {op->getNextUse()};
}

BridgedValue BridgedOperand::getValue() const { return {op->get()}; }

BridgedInstruction BridgedOperand::getUser() const {
  return {op->getUser()->asSILNode()};
}

BridgedOperand::OperandOwnership BridgedOperand::getOperandOwnership() const {
  switch (op->getOperandOwnership()) {
  case language::OperandOwnership::NonUse:
    return OperandOwnership::NonUse;
  case language::OperandOwnership::TrivialUse:
    return OperandOwnership::TrivialUse;
  case language::OperandOwnership::InstantaneousUse:
    return OperandOwnership::InstantaneousUse;
  case language::OperandOwnership::UnownedInstantaneousUse:
    return OperandOwnership::UnownedInstantaneousUse;
  case language::OperandOwnership::ForwardingUnowned:
    return OperandOwnership::ForwardingUnowned;
  case language::OperandOwnership::PointerEscape:
    return OperandOwnership::PointerEscape;
  case language::OperandOwnership::BitwiseEscape:
    return OperandOwnership::BitwiseEscape;
  case language::OperandOwnership::Borrow:
    return OperandOwnership::Borrow;
  case language::OperandOwnership::DestroyingConsume:
    return OperandOwnership::DestroyingConsume;
  case language::OperandOwnership::ForwardingConsume:
    return OperandOwnership::ForwardingConsume;
  case language::OperandOwnership::InteriorPointer:
    return OperandOwnership::InteriorPointer;
  case language::OperandOwnership::AnyInteriorPointer:
    return OperandOwnership::AnyInteriorPointer;
  case language::OperandOwnership::GuaranteedForwarding:
    return OperandOwnership::GuaranteedForwarding;
  case language::OperandOwnership::EndBorrow:
    return OperandOwnership::EndBorrow;
  case language::OperandOwnership::Reborrow:
    return OperandOwnership::Reborrow;
  }
}

BridgedOperand OptionalBridgedOperand::advancedBy(CodiraInt index) const { return {op + index}; }

// Assumes that `op` is not null.
CodiraInt OptionalBridgedOperand::distanceTo(BridgedOperand element) const { return element.op - op; }

//===----------------------------------------------------------------------===//
//                                BridgedArgument
//===----------------------------------------------------------------------===//

language::SILArgument * _Nonnull BridgedArgument::getArgument() const {
  return static_cast<language::SILArgument *>(obj);
}

BridgedBasicBlock BridgedArgument::getParent() const {
  return {getArgument()->getParent()};
}

bool BridgedArgument::isReborrow() const { return getArgument()->isReborrow(); }
void BridgedArgument::setReborrow(bool reborrow) const {
  getArgument()->setReborrow(reborrow);
}

bool BridgedArgument::FunctionArgument_isLexical() const {
  return toolchain::cast<language::SILFunctionArgument>(getArgument())->getLifetime().isLexical();
}

bool BridgedArgument::FunctionArgument_isClosureCapture() const {
  return toolchain::cast<language::SILFunctionArgument>(
    getArgument())->isClosureCapture();
}

OptionalBridgedDeclObj BridgedArgument::getDecl() const {
  return {getArgument()->getDecl()};
}

void BridgedArgument::copyFlags(BridgedArgument fromArgument) const {
  auto *fArg = static_cast<language::SILFunctionArgument *>(getArgument());
  fArg->copyFlags(static_cast<language::SILFunctionArgument *>(fromArgument.getArgument()));
}

language::SILArgument * _Nullable OptionalBridgedArgument::unbridged() const {
  if (!obj)
    return nullptr;
  return static_cast<language::SILArgument *>(obj);
}

//===----------------------------------------------------------------------===//
//                                BridgedLocation
//===----------------------------------------------------------------------===//

BridgedLocation::BridgedLocation(const language::SILDebugLocation &loc) {
  *reinterpret_cast<language::SILDebugLocation *>(&storage) = loc;
}
const language::SILDebugLocation &BridgedLocation::getLoc() const {
  return *reinterpret_cast<const language::SILDebugLocation *>(&storage);
}
BridgedLocation BridgedLocation::getAutogeneratedLocation() const {
  return getLoc().getAutogeneratedLocation();
}
BridgedLocation BridgedLocation::getCleanupLocation() const {
  return getLoc().getCleanupLocation();
}
BridgedLocation BridgedLocation::withScopeOf(BridgedLocation other) const {
  return language::SILDebugLocation(getLoc().getLocation(), other.getLoc().getScope());
}
bool BridgedLocation::hasValidLineNumber() const {
  return getLoc().hasValidLineNumber();
}
bool BridgedLocation::isAutoGenerated() const {
  return getLoc().isAutoGenerated();
}
bool BridgedLocation::isInlined() const {
  return getLoc().getScope()->InlinedCallSite;
}
bool BridgedLocation::isEqualTo(BridgedLocation rhs) const {
  return getLoc().isEqualTo(rhs.getLoc());
}
BridgedSourceLoc BridgedLocation::getSourceLocation() const {
  language::SILDebugLocation debugLoc = getLoc();
  language::SILLocation silLoc = debugLoc.getLocation();
  language::SourceLoc sourceLoc = silLoc.getSourceLoc();
  return BridgedSourceLoc(sourceLoc.getOpaquePointerValue());
}
bool BridgedLocation::isFilenameAndLocation() const {
  return getLoc().getLocation().isFilenameAndLocation();
}
BridgedLocation::FilenameAndLocation BridgedLocation::getFilenameAndLocation() const {
  auto fnal = getLoc().getLocation().getFilenameAndLocation();
  return {BridgedStringRef(fnal->filename), (CodiraInt)fnal->line, (CodiraInt)fnal->column};
}
bool BridgedLocation::hasSameSourceLocation(BridgedLocation rhs) const {
  return getLoc().hasSameSourceLocation(rhs.getLoc());
}
OptionalBridgedDeclObj BridgedLocation::getDecl() const {
  return {getLoc().getLocation().getAsASTNode<language::Decl>()};
}
BridgedLocation BridgedLocation::fromNominalTypeDecl(BridgedDeclObj decl) {
  return language::SILDebugLocation(decl.unbridged(), nullptr);
}
BridgedLocation BridgedLocation::getArtificialUnreachableLocation() {
  return language::SILDebugLocation::getArtificialUnreachableLocation();
}

//===----------------------------------------------------------------------===//
//                                BridgedFunction
//===----------------------------------------------------------------------===//

language::SILFunction * _Nonnull BridgedFunction::getFunction() const {
  return static_cast<language::SILFunction *>(obj);
}

BridgedStringRef BridgedFunction::getName() const {
  return getFunction()->getName();
}

BridgedLocation BridgedFunction::getLocation() const {
  return {language::SILDebugLocation(getFunction()->getLocation(), getFunction()->getDebugScope())}; 
}

bool BridgedFunction::isAccessor() const {
  if (auto *valDecl = getFunction()->getDeclRef().getDecl()) {
    return toolchain::isa<language::AccessorDecl>(valDecl);
  }
  return false;
}

BridgedStringRef BridgedFunction::getAccessorName() const {
  auto *accessorDecl  = toolchain::cast<language::AccessorDecl>(getFunction()->getDeclRef().getDecl());
  return accessorKindName(accessorDecl->getAccessorKind());
}

bool BridgedFunction::hasOwnership() const { return getFunction()->hasOwnership(); }

bool BridgedFunction::hasLoweredAddresses() const { return getFunction()->getModule().useLoweredAddresses(); }

BridgedCanType BridgedFunction::getLoweredFunctionTypeInContext() const {
  auto expansion = getFunction()->getTypeExpansionContext();
  return getFunction()->getLoweredFunctionTypeInContext(expansion);
}

BridgedGenericSignature BridgedFunction::getGenericSignature() const {
  return {getFunction()->getGenericSignature().getPointer()};
}

BridgedSubstitutionMap BridgedFunction::getForwardingSubstitutionMap() const {
  return {getFunction()->getForwardingSubstitutionMap()};
}

BridgedASTType BridgedFunction::mapTypeIntoContext(BridgedASTType ty) const {
  return {getFunction()->mapTypeIntoContext(ty.unbridged()).getPointer()};
}

OptionalBridgedBasicBlock BridgedFunction::getFirstBlock() const {
  return {getFunction()->empty() ? nullptr : getFunction()->getEntryBlock()};
}

OptionalBridgedBasicBlock BridgedFunction::getLastBlock() const {
  return {getFunction()->empty() ? nullptr : &*getFunction()->rbegin()};
}

CodiraInt BridgedFunction::getNumIndirectFormalResults() const {
  return (CodiraInt)getFunction()->getLoweredFunctionType()->getNumIndirectFormalResults();
}

BridgedDeclRef BridgedFunction::getDeclRef() const {
  return getFunction()->getDeclRef();
}

bool BridgedFunction::hasIndirectErrorResult() const {
  return (CodiraInt)getFunction()->getLoweredFunctionType()->hasIndirectErrorResult();
}

CodiraInt BridgedFunction::getNumSILArguments() const {
  return language::SILFunctionConventions(getFunction()->getConventionsInContext()).getNumSILArguments();
}

BridgedType BridgedFunction::getSILArgumentType(CodiraInt idx) const {
  language::SILFunctionConventions conv(getFunction()->getConventionsInContext());
  return conv.getSILArgumentType(idx, getFunction()->getTypeExpansionContext());
}

BridgedType BridgedFunction::getSILResultType() const {
  language::SILFunctionConventions conv(getFunction()->getConventionsInContext());
  return conv.getSILResultType(getFunction()->getTypeExpansionContext());
}

bool BridgedFunction::isCodira51RuntimeAvailable() const {
  if (getFunction()->getResilienceExpansion() != language::ResilienceExpansion::Maximal)
    return false;

  language::ASTContext &ctxt = getFunction()->getModule().getASTContext();
  return language::AvailabilityRange::forDeploymentTarget(ctxt).isContainedIn(
      ctxt.getCodira51Availability());
}

bool BridgedFunction::isPossiblyUsedExternally() const {
  return getFunction()->isPossiblyUsedExternally();
}

bool BridgedFunction::isTransparent() const {
  return getFunction()->isTransparent() == language::IsTransparent;
}

bool BridgedFunction::isAsync() const {
  return getFunction()->isAsync();
}

bool BridgedFunction::isGlobalInitFunction() const {
  return getFunction()->isGlobalInit();
}

bool BridgedFunction::isGlobalInitOnceFunction() const {
  return getFunction()->isGlobalInitOnceFunction();
}

bool BridgedFunction::isDestructor() const {
  if (auto *declCtxt = getFunction()->getDeclContext()) {
    return toolchain::isa<language::DestructorDecl>(declCtxt);
  }
  return false;
}

bool BridgedFunction::isGeneric() const {
  return getFunction()->isGeneric();
}

bool BridgedFunction::hasSemanticsAttr(BridgedStringRef attrName) const {
  return getFunction()->hasSemanticsAttr(attrName.unbridged());
}

bool BridgedFunction::hasUnsafeNonEscapableResult() const {
  return getFunction()->hasUnsafeNonEscapableResult();
}

bool BridgedFunction::hasDynamicSelfMetadata() const {
  return getFunction()->hasDynamicSelfMetadata();
}

BridgedFunction::EffectsKind BridgedFunction::getEffectAttribute() const {
  return (EffectsKind)getFunction()->getEffectsKind();
}

BridgedFunction::PerformanceConstraints BridgedFunction::getPerformanceConstraints() const {
  return (PerformanceConstraints)getFunction()->getPerfConstraints();
}

BridgedFunction::InlineStrategy BridgedFunction::getInlineStrategy() const {
  return (InlineStrategy)getFunction()->getInlineStrategy();
}

BridgedFunction::ThunkKind BridgedFunction::isThunk() const {
  return (ThunkKind)getFunction()->isThunk();
}

void BridgedFunction::setThunk(ThunkKind kind) const {
  getFunction()->setThunk((language::IsThunk_t)kind);
}

BridgedFunction::SerializedKind BridgedFunction::getSerializedKind() const {
  return (SerializedKind)getFunction()->getSerializedKind();
}

bool BridgedFunction::canBeInlinedIntoCaller(SerializedKind kind) const {
  return getFunction()->canBeInlinedIntoCaller(language::SerializedKind_t(kind));
}

bool BridgedFunction::hasValidLinkageForFragileRef(SerializedKind kind) const {
  return getFunction()->hasValidLinkageForFragileRef(language::SerializedKind_t(kind));
}

bool BridgedFunction::needsStackProtection() const {
  return getFunction()->needsStackProtection();
}

bool BridgedFunction::shouldOptimize() const {
  return getFunction()->shouldOptimize();
}

bool BridgedFunction::isReferencedInModule() const {
  return getFunction()->getRefCount() != 0;
}

bool BridgedFunction::wasDeserializedCanonical() const {
  return getFunction()->wasDeserializedCanonical();
}

void BridgedFunction::setNeedStackProtection(bool needSP) const {
  getFunction()->setNeedStackProtection(needSP);
}

void BridgedFunction::setIsPerformanceConstraint(bool isPerfConstraint) const {
  getFunction()->setIsPerformanceConstraint(isPerfConstraint);
}

BridgedLinkage BridgedFunction::getLinkage() const {
  return (BridgedLinkage)getFunction()->getLinkage();
}

void BridgedFunction::setLinkage(BridgedLinkage linkage) const {
  getFunction()->setLinkage((language::SILLinkage)linkage);
}

void BridgedFunction::setIsSerialized(bool isSerialized) const {
  getFunction()->setSerializedKind(isSerialized ? language::IsSerialized : language::IsNotSerialized);
}

bool BridgedFunction::conformanceMatchesActorIsolation(BridgedConformance conformance) const {
  return language::matchesActorIsolation(conformance.unbridged(), getFunction());
}

bool BridgedFunction::isSpecialization() const {
  return getFunction()->isSpecialization();
}

bool BridgedFunction::isResilientNominalDecl(BridgedDeclObj decl) const {
  return decl.getAs<language::NominalTypeDecl>()->isResilient(getFunction()->getModule().getCodiraModule(),
                                                           getFunction()->getResilienceExpansion());
}

BridgedType BridgedFunction::getLoweredType(BridgedASTType type, bool maximallyAbstracted) const {
  if (maximallyAbstracted) {
    return BridgedType(getFunction()->getLoweredType(language::Lowering::AbstractionPattern::getOpaque(), type.type));
  }
  return BridgedType(getFunction()->getLoweredType(type.type));
}

BridgedType BridgedFunction::getLoweredType(BridgedType type) const {
  return BridgedType(getFunction()->getLoweredType(type.unbridged()));
}

language::SILFunction * _Nullable OptionalBridgedFunction::getFunction() const {
  return static_cast<language::SILFunction *>(obj);
}

BridgedFunction::OptionalSourceFileKind BridgedFunction::getSourceFileKind() const {
  if (auto *dc = getFunction()->getDeclContext()) {
    if (auto *sourceFile = dc->getParentSourceFile())
      return (OptionalSourceFileKind)sourceFile->Kind;
  }
  return OptionalSourceFileKind::None;
}

//===----------------------------------------------------------------------===//
//                                BridgedGlobalVar
//===----------------------------------------------------------------------===//

language::SILGlobalVariable * _Nonnull BridgedGlobalVar::getGlobal() const {
  return static_cast<language::SILGlobalVariable *>(obj);
}

OptionalBridgedDeclObj BridgedGlobalVar::getDecl() const {
  return {getGlobal()->getDecl()};
}

BridgedStringRef BridgedGlobalVar::getName() const {
  return getGlobal()->getName();
}

bool BridgedGlobalVar::isLet() const { return getGlobal()->isLet(); }

void BridgedGlobalVar::setLet(bool value) const { getGlobal()->setLet(value); }

BridgedType BridgedGlobalVar::getType() const {
  return getGlobal()->getLoweredType();
}

BridgedLinkage BridgedGlobalVar::getLinkage() const {
  return (BridgedLinkage)getGlobal()->getLinkage();
}

BridgedSourceLoc BridgedGlobalVar::getSourceLocation() const {
  if (getGlobal()->hasLocation())
    return getGlobal()->getLocation().getSourceLoc();
  else
    return BridgedSourceLoc();
}

bool BridgedGlobalVar::isPossiblyUsedExternally() const {
  return getGlobal()->isPossiblyUsedExternally();
}

OptionalBridgedInstruction BridgedGlobalVar::getFirstStaticInitInst() const {
  if (getGlobal()->begin() == getGlobal()->end()) {
    return {nullptr};
  }
  language::SILInstruction *firstInst = &*getGlobal()->begin();
  return {firstInst->asSILNode()};
}

//===----------------------------------------------------------------------===//
//                                BridgedMultiValueResult
//===----------------------------------------------------------------------===//

language::MultipleValueInstructionResult * _Nonnull BridgedMultiValueResult::unbridged() const {
  return static_cast<language::MultipleValueInstructionResult *>(obj);
}

BridgedInstruction BridgedMultiValueResult::getParent() const {
  return {unbridged()->getParent()};
}

CodiraInt BridgedMultiValueResult::getIndex() const {
  return (CodiraInt)unbridged()->getIndex();
}

//===----------------------------------------------------------------------===//
//                              BridgedSILTypeArray
//===----------------------------------------------------------------------===//

BridgedType BridgedSILTypeArray::getAt(CodiraInt index) const {
  return typeArray.unbridged<language::SILType>()[index];
}

//===----------------------------------------------------------------------===//
//                                BridgedInstruction
//===----------------------------------------------------------------------===//

OptionalBridgedInstruction BridgedInstruction::getNext() const {
  auto iter = std::next(unbridged()->getIterator());
  if (iter == unbridged()->getParent()->end())
    return {nullptr};
  return {iter->asSILNode()};
}

OptionalBridgedInstruction BridgedInstruction::getPrevious() const {
  auto iter = std::next(unbridged()->getReverseIterator());
  if (iter == unbridged()->getParent()->rend())
    return {nullptr};
  return {iter->asSILNode()};
}

BridgedBasicBlock BridgedInstruction::getParent() const {
  assert(!unbridged()->isStaticInitializerInst() &&
         "cannot get the parent of a static initializer instruction");
  return {unbridged()->getParent()};
}

BridgedInstruction BridgedInstruction::getLastInstOfParent() const {
  return {unbridged()->getParent()->back().asSILNode()};
}

bool BridgedInstruction::isDeleted() const {
  return unbridged()->isDeleted();
}

bool BridgedInstruction::isInStaticInitializer() const {
  return unbridged()->isStaticInitializerInst();
}

BridgedOperandArray BridgedInstruction::getOperands() const {
  auto operands = unbridged()->getAllOperands();
  return {{operands.data()}, (CodiraInt)operands.size()};
}

BridgedOperandArray BridgedInstruction::getTypeDependentOperands() const {
  auto typeOperands = unbridged()->getTypeDependentOperands();
  return {{typeOperands.data()}, (CodiraInt)typeOperands.size()};
}

void BridgedInstruction::setOperand(CodiraInt index, BridgedValue value) const {
  unbridged()->setOperand((unsigned)index, value.getSILValue());
}

BridgedLocation BridgedInstruction::getLocation() const {
  return unbridged()->getDebugLocation();
}

BridgedMemoryBehavior BridgedInstruction::getMemBehavior() const {
  return (BridgedMemoryBehavior)unbridged()->getMemoryBehavior();
}

bool BridgedInstruction::mayRelease() const {
  return unbridged()->mayRelease();
}

bool BridgedInstruction::mayHaveSideEffects() const {
  return unbridged()->mayHaveSideEffects();
}

bool BridgedInstruction::maySuspend() const {
  return unbridged()->maySuspend();
}

bool BridgedInstruction::shouldBeForwarding() const {
  return toolchain::isa<language::OwnershipForwardingSingleValueInstruction>(unbridged()) ||
         toolchain::isa<language::OwnershipForwardingTermInst>(unbridged()) ||
         toolchain::isa<language::OwnershipForwardingMultipleValueInstruction>(unbridged());
}

CodiraInt BridgedInstruction::MultipleValueInstruction_getNumResults() const {
  return getAs<language::MultipleValueInstruction>()->getNumResults();
}

BridgedMultiValueResult BridgedInstruction::MultipleValueInstruction_getResult(CodiraInt index) const {
  return {getAs<language::MultipleValueInstruction>()->getResult(index)};
}

BridgedSuccessorArray BridgedInstruction::TermInst_getSuccessors() const {
  auto successors = getAs<language::TermInst>()->getSuccessors();
  return {{successors.data()}, (CodiraInt)successors.size()};
}

language::ForwardingInstruction * _Nonnull BridgedInstruction::getAsForwardingInstruction() const {
  auto *forwardingInst = language::ForwardingInstruction::get(unbridged());
  assert(forwardingInst && "instruction is not defined as ForwardingInstruction");
  return forwardingInst;
}

OptionalBridgedOperand BridgedInstruction::ForwardingInst_singleForwardedOperand() const {
  return {language::ForwardingOperation(unbridged()).getSingleForwardingOperand()};
}

BridgedOperandArray BridgedInstruction::ForwardingInst_forwardedOperands() const {
  auto operands =
      language::ForwardingOperation(unbridged()).getForwardedOperands();
  return {{operands.data()}, (CodiraInt)operands.size()};
}

BridgedValue::Ownership BridgedInstruction::ForwardingInst_forwardingOwnership() const {
  return bridge(getAsForwardingInstruction()->getForwardingOwnershipKind());
}

void BridgedInstruction::ForwardingInst_setForwardingOwnership(BridgedValue::Ownership ownership) const {
  return getAsForwardingInstruction()->setForwardingOwnershipKind(BridgedValue::unbridge(ownership));
}

bool BridgedInstruction::ForwardingInst_preservesOwnership() const {
  return getAsForwardingInstruction()->preservesOwnership();
}

BridgedStringRef BridgedInstruction::CondFailInst_getMessage() const {
  return getAs<language::CondFailInst>()->getMessage();
}

CodiraInt BridgedInstruction::LoadInst_getLoadOwnership() const {
  return (CodiraInt)getAs<language::LoadInst>()->getOwnershipQualifier();
}

bool BridgedInstruction::LoadBorrowInst_isUnchecked() const {
  return (CodiraInt)getAs<language::LoadBorrowInst>()->isUnchecked();
}

BridgedInstruction::BuiltinValueKind BridgedInstruction::BuiltinInst_getID() const {
  return (BuiltinValueKind)getAs<language::BuiltinInst>()->getBuiltinInfo().ID;
}

BridgedInstruction::IntrinsicID BridgedInstruction::BuiltinInst_getIntrinsicID() const {
  switch (getAs<language::BuiltinInst>()->getIntrinsicInfo().ID) {
    case toolchain::Intrinsic::memcpy:  return IntrinsicID::memcpy;
    case toolchain::Intrinsic::memmove: return IntrinsicID::memmove;
    default: return IntrinsicID::unknown;
  }
}

BridgedStringRef BridgedInstruction::BuiltinInst_getName() const {
  return getAs<language::BuiltinInst>()->getName().str();
}

BridgedSubstitutionMap BridgedInstruction::BuiltinInst_getSubstitutionMap() const {
  return getAs<language::BuiltinInst>()->getSubstitutions();
}

bool BridgedInstruction::PointerToAddressInst_isStrict() const {
  return getAs<language::PointerToAddressInst>()->isStrict();
}

bool BridgedInstruction::PointerToAddressInst_isInvariant() const {
  return getAs<language::PointerToAddressInst>()->isInvariant();
}

uint64_t BridgedInstruction::PointerToAddressInst_getAlignment() const {
  auto maybeAlign = getAs<language::PointerToAddressInst>()->alignment();
  if (maybeAlign.has_value()) {
    assert(maybeAlign->value() != 0);
    return maybeAlign->value();
  }
  return 0;
}

void BridgedInstruction::PointerToAddressInst_setAlignment(uint64_t alignment) const {
  getAs<language::PointerToAddressInst>()->setAlignment(toolchain::MaybeAlign(alignment));
}

bool BridgedInstruction::AddressToPointerInst_needsStackProtection() const {
  return getAs<language::AddressToPointerInst>()->needsStackProtection();
}

bool BridgedInstruction::IndexAddrInst_needsStackProtection() const {
  return getAs<language::IndexAddrInst>()->needsStackProtection();
}

BridgedConformanceArray BridgedInstruction::InitExistentialRefInst_getConformances() const {
  return {getAs<language::InitExistentialRefInst>()->getConformances()};
}

BridgedCanType BridgedInstruction::InitExistentialRefInst_getFormalConcreteType() const {
  return getAs<language::InitExistentialRefInst>()->getFormalConcreteType();
}

bool BridgedInstruction::OpenExistentialAddr_isImmutable() const {
  switch (getAs<language::OpenExistentialAddrInst>()->getAccessKind()) {
    case language::OpenedExistentialAccess::Immutable: return true;
    case language::OpenedExistentialAccess::Mutable: return false;
  }
}

BridgedGlobalVar BridgedInstruction::GlobalAccessInst_getGlobal() const {
  return {getAs<language::GlobalAccessInst>()->getReferencedGlobal()};
}

BridgedGlobalVar BridgedInstruction::AllocGlobalInst_getGlobal() const {
  return {getAs<language::AllocGlobalInst>()->getReferencedGlobal()};
}

BridgedFunction BridgedInstruction::FunctionRefBaseInst_getReferencedFunction() const {
  return {getAs<language::FunctionRefBaseInst>()->getInitiallyReferencedFunction()};
}

BridgedOptionalInt BridgedInstruction::IntegerLiteralInst_getValue() const {
  toolchain::APInt result = getAs<language::IntegerLiteralInst>()->getValue();
  return getFromAPInt(result);
}

BridgedStringRef BridgedInstruction::StringLiteralInst_getValue() const {
  return getAs<language::StringLiteralInst>()->getValue();
}

int BridgedInstruction::StringLiteralInst_getEncoding() const {
  return (int)getAs<language::StringLiteralInst>()->getEncoding();
}

CodiraInt BridgedInstruction::TupleExtractInst_fieldIndex() const {
  return getAs<language::TupleExtractInst>()->getFieldIndex();
}

CodiraInt BridgedInstruction::TupleElementAddrInst_fieldIndex() const {
  return getAs<language::TupleElementAddrInst>()->getFieldIndex();
}

CodiraInt BridgedInstruction::StructExtractInst_fieldIndex() const {
  return getAs<language::StructExtractInst>()->getFieldIndex();
}

OptionalBridgedValue BridgedInstruction::StructInst_getUniqueNonTrivialFieldValue() const {
  return {getAs<language::StructInst>()->getUniqueNonTrivialFieldValue()};
}

CodiraInt BridgedInstruction::StructElementAddrInst_fieldIndex() const {
  return getAs<language::StructElementAddrInst>()->getFieldIndex();
}

bool BridgedInstruction::BeginBorrow_isLexical() const {
  return getAs<language::BeginBorrowInst>()->isLexical();
}

bool BridgedInstruction::BeginBorrow_hasPointerEscape() const {
  return getAs<language::BeginBorrowInst>()->hasPointerEscape();
}

bool BridgedInstruction::BeginBorrow_isFromVarDecl() const {
  return getAs<language::BeginBorrowInst>()->isFromVarDecl();
}

bool BridgedInstruction::MoveValue_isLexical() const {
  return getAs<language::MoveValueInst>()->isLexical();
}

bool BridgedInstruction::MoveValue_hasPointerEscape() const {
  return getAs<language::MoveValueInst>()->hasPointerEscape();
}

bool BridgedInstruction::MoveValue_isFromVarDecl() const {
  return getAs<language::MoveValueInst>()->isFromVarDecl();
}

CodiraInt BridgedInstruction::ProjectBoxInst_fieldIndex() const {
  return getAs<language::ProjectBoxInst>()->getFieldIndex();
}

bool BridgedInstruction::EndCOWMutationInst_doKeepUnique() const {
  return getAs<language::EndCOWMutationInst>()->doKeepUnique();
}

bool BridgedInstruction::DestroyValueInst_isDeadEnd() const {
  return getAs<language::DestroyValueInst>()->isDeadEnd();
}

CodiraInt BridgedInstruction::EnumInst_caseIndex() const {
  return getAs<language::EnumInst>()->getCaseIndex();
}

CodiraInt BridgedInstruction::UncheckedEnumDataInst_caseIndex() const {
  return getAs<language::UncheckedEnumDataInst>()->getCaseIndex();
}

CodiraInt BridgedInstruction::InitEnumDataAddrInst_caseIndex() const {
  return getAs<language::InitEnumDataAddrInst>()->getCaseIndex();
}

CodiraInt BridgedInstruction::UncheckedTakeEnumDataAddrInst_caseIndex() const {
  return getAs<language::UncheckedTakeEnumDataAddrInst>()->getCaseIndex();
}

CodiraInt BridgedInstruction::InjectEnumAddrInst_caseIndex() const {
  return getAs<language::InjectEnumAddrInst>()->getCaseIndex();
}

CodiraInt BridgedInstruction::RefElementAddrInst_fieldIndex() const {
  return getAs<language::RefElementAddrInst>()->getFieldIndex();
}

bool BridgedInstruction::RefElementAddrInst_fieldIsLet() const {
  return getAs<language::RefElementAddrInst>()->getField()->isLet();
}

bool BridgedInstruction::RefElementAddrInst_isImmutable() const {
  return getAs<language::RefElementAddrInst>()->isImmutable();
}

void BridgedInstruction::RefElementAddrInst_setImmutable(bool isImmutable) const {
  getAs<language::RefElementAddrInst>()->setImmutable(isImmutable);
}

bool BridgedInstruction::RefTailAddrInst_isImmutable() const {
  return getAs<language::RefTailAddrInst>()->isImmutable();
}

CodiraInt BridgedInstruction::PartialApplyInst_numArguments() const {
  return getAs<language::PartialApplyInst>()->getNumArguments();
}

CodiraInt BridgedInstruction::ApplyInst_numArguments() const {
  return getAs<language::ApplyInst>()->getNumArguments();
}

bool BridgedInstruction::ApplyInst_getNonThrowing() const {
  return getAs<language::ApplyInst>()->isNonThrowing();
}

bool BridgedInstruction::ApplyInst_getNonAsync() const {
  return getAs<language::ApplyInst>()->isNonAsync();
}

BridgedGenericSpecializationInformation BridgedInstruction::ApplyInst_getSpecializationInfo() const {
  return {getAs<language::ApplyInst>()->getSpecializationInfo()};
}

bool BridgedInstruction::TryApplyInst_getNonAsync() const {
  return getAs<language::TryApplyInst>()->isNonAsync();  
}

BridgedGenericSpecializationInformation BridgedInstruction::TryApplyInst_getSpecializationInfo() const {
  return {getAs<language::TryApplyInst>()->getSpecializationInfo()};
}

BridgedDeclRef BridgedInstruction::ClassMethodInst_getMember() const {
  return getAs<language::ClassMethodInst>()->getMember();
}

BridgedDeclRef BridgedInstruction::WitnessMethodInst_getMember() const {
  return getAs<language::WitnessMethodInst>()->getMember();
}

BridgedCanType BridgedInstruction::WitnessMethodInst_getLookupType() const {
  return getAs<language::WitnessMethodInst>()->getLookupType();  
}

BridgedDeclObj BridgedInstruction::WitnessMethodInst_getLookupProtocol() const {
  return getAs<language::WitnessMethodInst>()->getLookupProtocol();
}

BridgedConformance BridgedInstruction::WitnessMethodInst_getConformance() const {
  return getAs<language::WitnessMethodInst>()->getConformance();
}

CodiraInt BridgedInstruction::ObjectInst_getNumBaseElements() const {
  return getAs<language::ObjectInst>()->getNumBaseElements();
}

CodiraInt BridgedInstruction::PartialApply_getCalleeArgIndexOfFirstAppliedArg() const {
  return language::ApplySite(unbridged()).getCalleeArgIndexOfFirstAppliedArg();
}

bool BridgedInstruction::PartialApplyInst_isOnStack() const {
  return getAs<language::PartialApplyInst>()->isOnStack();
}

bool BridgedInstruction::PartialApplyInst_hasUnknownResultIsolation() const {
  return getAs<language::PartialApplyInst>()->getResultIsolation() ==
         language::SILFunctionTypeIsolation::forUnknown();
}

bool BridgedInstruction::AllocStackInst_hasDynamicLifetime() const {
  return getAs<language::AllocStackInst>()->hasDynamicLifetime();
}

bool BridgedInstruction::AllocStackInst_isFromVarDecl() const {
  return getAs<language::AllocStackInst>()->isFromVarDecl();
}

bool BridgedInstruction::AllocStackInst_usesMoveableValueDebugInfo() const {
  return getAs<language::AllocStackInst>()->usesMoveableValueDebugInfo();
}

bool BridgedInstruction::AllocStackInst_isLexical() const {
  return getAs<language::AllocStackInst>()->isLexical();
}

bool BridgedInstruction::AllocBoxInst_hasDynamicLifetime() const {
  return getAs<language::AllocBoxInst>()->hasDynamicLifetime();
}

bool BridgedInstruction::AllocRefInstBase_isObjc() const {
  return getAs<language::AllocRefInstBase>()->isObjC();
}

bool BridgedInstruction::AllocRefInstBase_canAllocOnStack() const {
  return getAs<language::AllocRefInstBase>()->canAllocOnStack();
}

CodiraInt BridgedInstruction::AllocRefInstBase_getNumTailTypes() const {
  return getAs<language::AllocRefInstBase>()->getNumTailTypes();
}

BridgedSILTypeArray BridgedInstruction::AllocRefInstBase_getTailAllocatedTypes() const {
  return {getAs<const language::AllocRefInstBase>()->getTailAllocatedTypes()};
}

bool BridgedInstruction::AllocRefDynamicInst_isDynamicTypeDeinitAndSizeKnownEquivalentToBaseType() const {
  return getAs<language::AllocRefDynamicInst>()->isDynamicTypeDeinitAndSizeKnownEquivalentToBaseType();
}

CodiraInt BridgedInstruction::BeginApplyInst_numArguments() const {
  return getAs<language::BeginApplyInst>()->getNumArguments();
}

bool BridgedInstruction::BeginApplyInst_isCalleeAllocated() const {
  return getAs<language::BeginApplyInst>()->isCalleeAllocated();
}

CodiraInt BridgedInstruction::TryApplyInst_numArguments() const {
  return getAs<language::TryApplyInst>()->getNumArguments();
}

BridgedArgumentConvention BridgedInstruction::YieldInst_getConvention(BridgedOperand forOperand) const {
  return castToArgumentConvention(getAs<language::YieldInst>()->getArgumentConventionForOperand(*forOperand.op));
}

BridgedBasicBlock BridgedInstruction::BranchInst_getTargetBlock() const {
  return {getAs<language::BranchInst>()->getDestBB()};
}

CodiraInt BridgedInstruction::SwitchEnumInst_getNumCases() const {
  return getAs<language::SwitchEnumInst>()->getNumCases();
}

CodiraInt BridgedInstruction::SwitchEnumInst_getCaseIndex(CodiraInt idx) const {
  auto *seInst = getAs<language::SwitchEnumInst>();
  return seInst->getModule().getCaseIndex(seInst->getCase(idx).first);
}

CodiraInt BridgedInstruction::StoreInst_getStoreOwnership() const {
  return (CodiraInt)getAs<language::StoreInst>()->getOwnershipQualifier();
}

CodiraInt BridgedInstruction::AssignInst_getAssignOwnership() const {
  return (CodiraInt)getAs<language::AssignInst>()->getOwnershipQualifier();
}

BridgedInstruction::MarkDependenceKind BridgedInstruction::MarkDependenceInst_dependenceKind() const {
  return (MarkDependenceKind)getAs<language::MarkDependenceInst>()->dependenceKind();
}

void BridgedInstruction::MarkDependenceInstruction_resolveToNonEscaping() const {
  if (auto *mdi = toolchain::dyn_cast<language::MarkDependenceInst>(unbridged())) {
    mdi->resolveToNonEscaping();
  } else {
    getAs<language::MarkDependenceAddrInst>()->resolveToNonEscaping();
  }
}

void BridgedInstruction::MarkDependenceInstruction_settleToEscaping() const {
  if (auto *mdi = toolchain::dyn_cast<language::MarkDependenceInst>(unbridged())) {
    mdi->settleToEscaping();
  } else {
    getAs<language::MarkDependenceAddrInst>()->settleToEscaping();
  }
}

BridgedInstruction::MarkDependenceKind BridgedInstruction::MarkDependenceAddrInst_dependenceKind() const {
  return (MarkDependenceKind)getAs<language::MarkDependenceAddrInst>()->dependenceKind();
}

CodiraInt BridgedInstruction::BeginAccessInst_getAccessKind() const {
  return (CodiraInt)getAs<language::BeginAccessInst>()->getAccessKind();
}

bool BridgedInstruction::BeginAccessInst_isStatic() const {
  return getAs<language::BeginAccessInst>()->getEnforcement() == language::SILAccessEnforcement::Static;
}

bool BridgedInstruction::BeginAccessInst_isUnsafe() const {
  return getAs<language::BeginAccessInst>()->getEnforcement() == language::SILAccessEnforcement::Unsafe;
}

void BridgedInstruction::BeginAccess_setAccessKind(CodiraInt accessKind) const {
  getAs<language::BeginAccessInst>()->setAccessKind((language::SILAccessKind)accessKind);
}

bool BridgedInstruction::CopyAddrInst_isTakeOfSrc() const {
  return getAs<language::CopyAddrInst>()->isTakeOfSrc();
}

bool BridgedInstruction::CopyAddrInst_isInitializationOfDest() const {
  return getAs<language::CopyAddrInst>()->isInitializationOfDest();
}

void BridgedInstruction::CopyAddrInst_setIsTakeOfSrc(bool isTakeOfSrc) const {
  return getAs<language::CopyAddrInst>()->setIsTakeOfSrc(isTakeOfSrc ? language::IsTake : language::IsNotTake);
}

void BridgedInstruction::CopyAddrInst_setIsInitializationOfDest(bool isInitializationOfDest) const {
  return getAs<language::CopyAddrInst>()->setIsInitializationOfDest(
      isInitializationOfDest ? language::IsInitialization : language::IsNotInitialization);
}

bool BridgedInstruction::ExplicitCopyAddrInst_isTakeOfSrc() const {
  return getAs<language::ExplicitCopyAddrInst>()->isTakeOfSrc();
}

bool BridgedInstruction::ExplicitCopyAddrInst_isInitializationOfDest() const {
  return getAs<language::ExplicitCopyAddrInst>()->isInitializationOfDest();
}

CodiraInt BridgedInstruction::MarkUninitializedInst_getKind() const {
  return (CodiraInt)getAs<language::MarkUninitializedInst>()->getMarkUninitializedKind();
}

CodiraInt BridgedInstruction::MarkUnresolvedNonCopyableValue_getCheckKind() const {
  return (CodiraInt)getAs<language::MarkUnresolvedNonCopyableValueInst>()->getCheckKind();
}

bool BridgedInstruction::MarkUnresolvedNonCopyableValue_isStrict() const {
  return getAs<language::MarkUnresolvedNonCopyableValueInst>()->isStrict();
}

void BridgedInstruction::RefCountingInst_setIsAtomic(bool isAtomic) const {
  getAs<language::RefCountingInst>()->setAtomicity(
      isAtomic ? language::RefCountingInst::Atomicity::Atomic
               : language::RefCountingInst::Atomicity::NonAtomic);
}

bool BridgedInstruction::RefCountingInst_getIsAtomic() const {
  return getAs<language::RefCountingInst>()->getAtomicity() == language::RefCountingInst::Atomicity::Atomic;
}

CodiraInt BridgedInstruction::CondBranchInst_getNumTrueArgs() const {
  return getAs<language::CondBranchInst>()->getNumTrueArgs();
}

void BridgedInstruction::AllocRefInstBase_setIsStackAllocatable() const {
  getAs<language::AllocRefInstBase>()->setStackAllocatable();
}

bool BridgedInstruction::AllocRefInst_isBare() const {
  return getAs<language::AllocRefInst>()->isBare();
}

void BridgedInstruction::AllocRefInst_setIsBare() const {
  getAs<language::AllocRefInst>()->setBare(true);
}

void BridgedInstruction::TermInst_replaceBranchTarget(BridgedBasicBlock from, BridgedBasicBlock to) const {
  getAs<language::TermInst>()->replaceBranchTarget(from.unbridged(),
                                                to.unbridged());
}

CodiraInt BridgedInstruction::KeyPathInst_getNumComponents() const {
  if (language::KeyPathPattern *pattern = getAs<language::KeyPathInst>()->getPattern()) {
    return (CodiraInt)pattern->getComponents().size();
  }
  return 0;
}

void BridgedInstruction::KeyPathInst_getReferencedFunctions(CodiraInt componentIdx,
                                                            KeyPathFunctionResults * _Nonnull results) const {
  language::KeyPathPattern *pattern = getAs<language::KeyPathInst>()->getPattern();
  const language::KeyPathPatternComponent &comp = pattern->getComponents()[componentIdx];
  results->numFunctions = 0;

  comp.visitReferencedFunctionsAndMethods([results](language::SILFunction *fn) {
      assert(results->numFunctions < KeyPathFunctionResults::maxFunctions);
      results->functions[results->numFunctions++] = {fn};
    }, [](language::SILDeclRef) {});
}

void BridgedInstruction::GlobalAddrInst_clearToken() const {
  getAs<language::GlobalAddrInst>()->clearToken();
}

bool BridgedInstruction::GlobalValueInst_isBare() const {
  return getAs<language::GlobalValueInst>()->isBare();
}

void BridgedInstruction::GlobalValueInst_setIsBare() const {
  getAs<language::GlobalValueInst>()->setBare(true);
}

void BridgedInstruction::LoadInst_setOwnership(CodiraInt ownership) const {
  getAs<language::LoadInst>()->setOwnershipQualifier((language::LoadOwnershipQualifier)ownership);
}

void BridgedInstruction::CheckedCastBranch_updateSourceFormalTypeFromOperandLoweredType() const {
  getAs<language::CheckedCastBranchInst>()->updateSourceFormalTypeFromOperandLoweredType();
}

BridgedCanType BridgedInstruction::UnconditionalCheckedCast_getSourceFormalType() const {
  return {getAs<language::UnconditionalCheckedCastInst>()->getSourceFormalType()};  
}

BridgedCanType BridgedInstruction::UnconditionalCheckedCast_getTargetFormalType() const {
  return {getAs<language::UnconditionalCheckedCastInst>()->getTargetFormalType()};    
}

BridgedInstruction::CheckedCastInstOptions
BridgedInstruction::UnconditionalCheckedCast_getCheckedCastOptions() const {
  return BridgedInstruction::CheckedCastInstOptions{
      getAs<language::UnconditionalCheckedCastInst>()->getCheckedCastOptions()
        .getStorage()};
}

BridgedCanType BridgedInstruction::UnconditionalCheckedCastAddr_getSourceFormalType() const {
  return {getAs<language::UnconditionalCheckedCastAddrInst>()->getSourceFormalType()};  
}

BridgedCanType BridgedInstruction::UnconditionalCheckedCastAddr_getTargetFormalType() const {
  return {getAs<language::UnconditionalCheckedCastAddrInst>()->getTargetFormalType()};    
}

BridgedInstruction::CheckedCastInstOptions
BridgedInstruction::UnconditionalCheckedCastAddr_getCheckedCastOptions() const {
  return BridgedInstruction::CheckedCastInstOptions{
      getAs<language::UnconditionalCheckedCastAddrInst>()->getCheckedCastOptions()
        .getStorage()};
}

BridgedBasicBlock BridgedInstruction::CheckedCastBranch_getSuccessBlock() const {
  return {getAs<language::CheckedCastBranchInst>()->getSuccessBB()};
}

BridgedBasicBlock BridgedInstruction::CheckedCastBranch_getFailureBlock() const {
  return {getAs<language::CheckedCastBranchInst>()->getFailureBB()};
}

BridgedInstruction::CheckedCastInstOptions
BridgedInstruction::CheckedCastBranch_getCheckedCastOptions() const {
  return BridgedInstruction::CheckedCastInstOptions{
      getAs<language::CheckedCastBranchInst>()->getCheckedCastOptions()
        .getStorage()};
}

BridgedCanType BridgedInstruction::CheckedCastAddrBranch_getSourceFormalType() const {
  return {getAs<language::CheckedCastAddrBranchInst>()->getSourceFormalType()};
}

BridgedCanType BridgedInstruction::CheckedCastAddrBranch_getTargetFormalType() const {
  return {getAs<language::CheckedCastAddrBranchInst>()->getTargetFormalType()};  
}

BridgedBasicBlock BridgedInstruction::CheckedCastAddrBranch_getSuccessBlock() const {
  return {getAs<language::CheckedCastAddrBranchInst>()->getSuccessBB()};
}

BridgedBasicBlock BridgedInstruction::CheckedCastAddrBranch_getFailureBlock() const {
  return {getAs<language::CheckedCastAddrBranchInst>()->getFailureBB()};
}

BridgedInstruction::CastConsumptionKind BridgedInstruction::CheckedCastAddrBranch_getConsumptionKind() const {
  static_assert((int)BridgedInstruction::CastConsumptionKind::TakeAlways ==
                (int)language::CastConsumptionKind::TakeAlways);
  static_assert((int)BridgedInstruction::CastConsumptionKind::TakeOnSuccess ==
                (int)language::CastConsumptionKind::TakeOnSuccess);
  static_assert((int)BridgedInstruction::CastConsumptionKind::CopyOnSuccess ==
                (int)language::CastConsumptionKind::CopyOnSuccess);

  return static_cast<BridgedInstruction::CastConsumptionKind>(
           getAs<language::CheckedCastAddrBranchInst>()->getConsumptionKind());
}

BridgedInstruction::CheckedCastInstOptions
BridgedInstruction::CheckedCastAddrBranch_getCheckedCastOptions() const {
  return BridgedInstruction::CheckedCastInstOptions{
      getAs<language::CheckedCastAddrBranchInst>()->getCheckedCastOptions()
        .getStorage()};
}

BridgedSubstitutionMap BridgedInstruction::ApplySite_getSubstitutionMap() const {
  auto as = language::ApplySite(unbridged());
  return as.getSubstitutionMap();
}

BridgedCanType BridgedInstruction::ApplySite_getSubstitutedCalleeType() const {
  auto as = language::ApplySite(unbridged());
  return as.getSubstCalleeType();
}

CodiraInt BridgedInstruction::ApplySite_getNumArguments() const {
  return language::ApplySite(unbridged()).getNumArguments();
}

bool BridgedInstruction::ApplySite_isCalleeNoReturn() const {
  return language::ApplySite(unbridged()).isCalleeNoReturn();
}

CodiraInt BridgedInstruction::FullApplySite_numIndirectResultArguments() const {
  auto fas = language::FullApplySite(unbridged());
  return fas.getNumIndirectSILResults();
}

bool BridgedInstruction::ConvertFunctionInst_withoutActuallyEscaping() const {
  return getAs<language::ConvertFunctionInst>()->withoutActuallyEscaping();
}

BridgedCanType BridgedInstruction::TypeValueInst_getParamType() const {
  return getAs<language::TypeValueInst>()->getParamType();
}

//===----------------------------------------------------------------------===//
//                     VarDeclInst and DebugVariableInst
//===----------------------------------------------------------------------===//

static_assert(sizeof(language::SILDebugVariable) <= sizeof(BridgedSILDebugVariable));

BridgedSILDebugVariable::BridgedSILDebugVariable(const language::SILDebugVariable &var) {
  new (&storage) language::SILDebugVariable(var);
}

BridgedSILDebugVariable::BridgedSILDebugVariable(const BridgedSILDebugVariable &rhs) {
  new (&storage) language::SILDebugVariable(rhs.unbridge());
}

BridgedSILDebugVariable::~BridgedSILDebugVariable() {
  reinterpret_cast<language::SILDebugVariable *>(&storage)->~SILDebugVariable();
}

BridgedSILDebugVariable &BridgedSILDebugVariable::operator=(const BridgedSILDebugVariable &rhs) {
  *reinterpret_cast<language::SILDebugVariable *>(&storage) = rhs.unbridge();
  return *this;
}

language::SILDebugVariable BridgedSILDebugVariable::unbridge() const {
  return *reinterpret_cast<const language::SILDebugVariable *>(&storage);
}

OptionalBridgedDeclObj BridgedInstruction::DebugValue_getDecl() const {
  return {getAs<language::DebugValueInst>()->getDecl()};
}

OptionalBridgedDeclObj BridgedInstruction::AllocStack_getDecl() const {
  return {getAs<language::AllocStackInst>()->getDecl()};
}

OptionalBridgedDeclObj BridgedInstruction::AllocBox_getDecl() const {
  return {getAs<language::AllocBoxInst>()->getDecl()};
}

OptionalBridgedDeclObj BridgedInstruction::GlobalAddr_getDecl() const {
  return {getAs<language::GlobalAddrInst>()->getReferencedGlobal()->getDecl()};
}

OptionalBridgedDeclObj BridgedInstruction::RefElementAddr_getDecl() const {
  return {getAs<language::RefElementAddrInst>()->getField()};
}

bool BridgedInstruction::DebugValue_hasVarInfo() const {
  return getAs<language::DebugValueInst>()->getVarInfo().has_value();
}
BridgedSILDebugVariable BridgedInstruction::DebugValue_getVarInfo() const {
  return BridgedSILDebugVariable(getAs<language::DebugValueInst>()->getVarInfo().value());
}

bool BridgedInstruction::AllocStack_hasVarInfo() const {
  return getAs<language::AllocStackInst>()->getVarInfo().has_value();
}
BridgedSILDebugVariable BridgedInstruction::AllocStack_getVarInfo() const {
  return BridgedSILDebugVariable(getAs<language::AllocStackInst>()->getVarInfo().value());
}

bool BridgedInstruction::AllocBox_hasVarInfo() const {
  return getAs<language::AllocBoxInst>()->getVarInfo().has_value();
}
BridgedSILDebugVariable BridgedInstruction::AllocBox_getVarInfo() const {
  return BridgedSILDebugVariable(getAs<language::AllocBoxInst>()->getVarInfo().value());
}

//===----------------------------------------------------------------------===//
//                       OptionalBridgedInstruction
//===----------------------------------------------------------------------===//

language::SILInstruction * _Nullable OptionalBridgedInstruction::unbridged() const {
  if (!obj)
    return nullptr;
  return toolchain::cast<language::SILInstruction>(static_cast<language::SILNode *>(obj)->castToInstruction());
}

//===----------------------------------------------------------------------===//
//                                BridgedBasicBlock
//===----------------------------------------------------------------------===//

BridgedBasicBlock::BridgedBasicBlock(language::SILBasicBlock * _Nonnull block)
  : obj(block) {}

language::SILBasicBlock * _Nonnull BridgedBasicBlock::unbridged() const {
  return static_cast<language::SILBasicBlock *>(obj);
}

OptionalBridgedBasicBlock BridgedBasicBlock::getNext() const {
  auto iter = std::next(unbridged()->getIterator());
  if (iter == unbridged()->getParent()->end())
    return {nullptr};
  return {&*iter};
}

OptionalBridgedBasicBlock BridgedBasicBlock::getPrevious() const {
  auto iter = std::next(unbridged()->getReverseIterator());
  if (iter == unbridged()->getParent()->rend())
    return {nullptr};
  return {&*iter};
}

BridgedFunction BridgedBasicBlock::getFunction() const {
  return {unbridged()->getParent()};
}

OptionalBridgedInstruction BridgedBasicBlock::getFirstInst() const {
  if (unbridged()->empty())
    return {nullptr};
  return {unbridged()->front().asSILNode()};
}

OptionalBridgedInstruction BridgedBasicBlock::getLastInst() const {
  if (unbridged()->empty())
    return {nullptr};
  return {unbridged()->back().asSILNode()};
}

CodiraInt BridgedBasicBlock::getNumArguments() const {
  return unbridged()->getNumArguments();
}

BridgedArgument BridgedBasicBlock::getArgument(CodiraInt index) const {
  return {unbridged()->getArgument(index)};
}

BridgedArgument BridgedBasicBlock::addBlockArgument(BridgedType type, BridgedValue::Ownership ownership) const {
  return {unbridged()->createPhiArgument(
      type.unbridged(), BridgedValue::unbridge(ownership))};
}

BridgedArgument BridgedBasicBlock::addFunctionArgument(BridgedType type) const {
  return {unbridged()->createFunctionArgument(type.unbridged())};
}

BridgedArgument BridgedBasicBlock::insertFunctionArgument(CodiraInt atPosition, BridgedType type,
                                                          BridgedValue::Ownership ownership,
                                                          OptionalBridgedDeclObj decl) const {
  return {unbridged()->insertFunctionArgument((unsigned)atPosition, type.unbridged(),
                                              BridgedValue::unbridge(ownership),
                                              decl.getAs<language::ValueDecl>())};
}

void BridgedBasicBlock::eraseArgument(CodiraInt index) const {
  unbridged()->eraseArgument(index);
}

void BridgedBasicBlock::moveAllInstructionsToBegin(BridgedBasicBlock dest) const {
  dest.unbridged()->spliceAtBegin(unbridged());
}

void BridgedBasicBlock::moveAllInstructionsToEnd(BridgedBasicBlock dest) const {
  dest.unbridged()->spliceAtEnd(unbridged());
}

void BridgedBasicBlock::moveArgumentsTo(BridgedBasicBlock dest) const {
  dest.unbridged()->moveArgumentList(unbridged());
}

OptionalBridgedSuccessor BridgedBasicBlock::getFirstPred() const {
  return {unbridged()->pred_begin().getSuccessorRef()};
}

language::SILBasicBlock * _Nullable OptionalBridgedBasicBlock::unbridged() const {
  return obj ? static_cast<language::SILBasicBlock *>(obj) : nullptr;
}

//===----------------------------------------------------------------------===//
//                                BridgedSuccessor
//===----------------------------------------------------------------------===//

OptionalBridgedSuccessor BridgedSuccessor::getNext() const {
  return {succ->getNext()};
}

BridgedBasicBlock BridgedSuccessor::getTargetBlock() const {
  return succ->getBB();
}

BridgedInstruction BridgedSuccessor::getContainingInst() const {
  return {succ->getContainingInst()};
}

BridgedSuccessor OptionalBridgedSuccessor::advancedBy(CodiraInt index) const {
  return {succ + index};
}

//===----------------------------------------------------------------------===//
//                                BridgedDeclRef
//===----------------------------------------------------------------------===//

static_assert(sizeof(BridgedDeclRef) >= sizeof(language::SILDeclRef),
              "BridgedDeclRef has wrong size");

BridgedDeclRef::BridgedDeclRef(language::SILDeclRef declRef) {
  *reinterpret_cast<language::SILDeclRef *>(&storage) = declRef;
}

language::SILDeclRef BridgedDeclRef::unbridged() const {
  return *reinterpret_cast<const language::SILDeclRef *>(&storage);
}

bool BridgedDeclRef::isEqualTo(BridgedDeclRef rhs) const {
  return unbridged() == rhs.unbridged();
}

BridgedLocation BridgedDeclRef::getLocation() const {
  return language::SILDebugLocation(unbridged().getDecl(), nullptr);
}

BridgedDeclObj BridgedDeclRef::getDecl() const {
  return {unbridged().getDecl()};
}

BridgedDiagnosticArgument BridgedDeclRef::asDiagnosticArgument() const {
  return language::DiagnosticArgument(unbridged().getDecl()->getName());
}

//===----------------------------------------------------------------------===//
//                                BridgedVTable
//===----------------------------------------------------------------------===//

BridgedVTableEntry::BridgedVTableEntry(const language::SILVTableEntry &entry) {
  *reinterpret_cast<language::SILVTableEntry *>(&storage) = entry;
}

const language::SILVTableEntry &BridgedVTableEntry::unbridged() const {
  return *reinterpret_cast<const language::SILVTableEntry *>(&storage);
}

BridgedVTableEntry::Kind BridgedVTableEntry::getKind() const {
  return (Kind)unbridged().getKind();
}

BRIDGED_INLINE bool BridgedVTableEntry::isNonOverridden() const {
  return unbridged().isNonOverridden();
}

BridgedDeclRef BridgedVTableEntry::getMethodDecl() const {
  return unbridged().getMethod();
}

BridgedFunction BridgedVTableEntry::getImplementation() const {
  return {unbridged().getImplementation()};
}

BridgedVTableEntry BridgedVTableEntry::create(Kind kind, bool nonOverridden,
                                              BridgedDeclRef methodDecl, BridgedFunction implementation) {
  return language::SILVTableEntry(methodDecl.unbridged(), implementation.getFunction(),
                               (language::SILVTableEntry::Kind)kind, nonOverridden);
}

CodiraInt BridgedVTable::getNumEntries() const {
  return CodiraInt(vTable->getEntries().size());
}

BridgedVTableEntry BridgedVTable::getEntry(CodiraInt index) const {
  return BridgedVTableEntry(vTable->getEntries()[index]);
}

BridgedDeclObj BridgedVTable::getClass() const {
  return vTable->getClass();
}

OptionalBridgedVTableEntry BridgedVTable::lookupMethod(BridgedDeclRef member) const {
  if (vTable->getEntries().empty()) {
    return OptionalBridgedVTableEntry();
  }
  language::SILModule &mod = vTable->getEntries()[0].getImplementation()->getModule();
  if (auto entry = vTable->getEntry(mod, member.unbridged()))
    return BridgedVTableEntry(entry.value());

  return OptionalBridgedVTableEntry();
}


BridgedType BridgedVTable::getSpecializedClassType() const {
  return {vTable->getClassType()};
}

void BridgedVTable::replaceEntries(BridgedArrayRef bridgedEntries) const {
  toolchain::SmallVector<language::SILVTableEntry, 8> entries;
  for (const BridgedVTableEntry &e : bridgedEntries.unbridged<BridgedVTableEntry>()) {
    entries.push_back(e.unbridged());
  }
  vTable->replaceEntries(entries);
}

//===----------------------------------------------------------------------===//
//               BridgedWitnessTable, BridgedDefaultWitnessTable
//===----------------------------------------------------------------------===//

BridgedWitnessTableEntry::Kind BridgedWitnessTableEntry::getKind() const {
  return (Kind)unbridged().getKind();
}

BridgedDeclRef BridgedWitnessTableEntry::getMethodRequirement() const {
  return unbridged().getMethodWitness().Requirement;
}

OptionalBridgedFunction BridgedWitnessTableEntry::getMethodWitness() const {
  return {unbridged().getMethodWitness().Witness};
}

BridgedDeclObj BridgedWitnessTableEntry::getAssociatedTypeRequirement() const {
  return {unbridged().getAssociatedTypeWitness().Requirement};
}

BridgedCanType BridgedWitnessTableEntry::getAssociatedTypeWitness() const {
  return unbridged().getAssociatedTypeWitness().Witness;
}

BridgedCanType BridgedWitnessTableEntry::getAssociatedConformanceRequirement() const {
  return unbridged().getAssociatedConformanceWitness().Requirement;
}

BridgedConformance BridgedWitnessTableEntry::getAssociatedConformanceWitness() const {
  return {unbridged().getAssociatedConformanceWitness().Witness};
}

BridgedDeclObj BridgedWitnessTableEntry::getBaseProtocolRequirement() const {
  return {unbridged().getBaseProtocolWitness().Requirement};
}

BridgedConformance BridgedWitnessTableEntry::getBaseProtocolWitness() const {
  return language::ProtocolConformanceRef(unbridged().getBaseProtocolWitness().Witness);
}


BridgedWitnessTableEntry BridgedWitnessTableEntry::createInvalid() {
  return bridge(language::SILWitnessTable::Entry());
}

BridgedWitnessTableEntry BridgedWitnessTableEntry::createMethod(BridgedDeclRef requirement,
                                                                OptionalBridgedFunction witness) {
  return bridge(language::SILWitnessTable::Entry(
    language::SILWitnessTable::MethodWitness{requirement.unbridged(), witness.getFunction()}));
}

BridgedWitnessTableEntry BridgedWitnessTableEntry::createAssociatedType(BridgedDeclObj requirement,
                                                                        BridgedCanType witness) {
  return bridge(language::SILWitnessTable::Entry(
    language::SILWitnessTable::AssociatedTypeWitness{requirement.getAs<language::AssociatedTypeDecl>(),
                                                  witness.unbridged()}));
}

BridgedWitnessTableEntry BridgedWitnessTableEntry::createAssociatedConformance(BridgedCanType requirement,
                                                                               BridgedConformance witness) {
  return bridge(language::SILWitnessTable::Entry(
    language::SILWitnessTable::AssociatedConformanceWitness{requirement.unbridged(),
                                                         witness.unbridged()}));
}

BridgedWitnessTableEntry BridgedWitnessTableEntry::createBaseProtocol(BridgedDeclObj requirement,
                                                                      BridgedConformance witness) {
  return bridge(language::SILWitnessTable::Entry(
    language::SILWitnessTable::BaseProtocolWitness{requirement.getAs<language::ProtocolDecl>(),
                                                witness.unbridged().getConcrete()}));
}

CodiraInt BridgedWitnessTable::getNumEntries() const {
  return CodiraInt(table->getEntries().size());
}

BridgedWitnessTableEntry BridgedWitnessTable::getEntry(CodiraInt index) const {
  return BridgedWitnessTableEntry::bridge(table->getEntries()[index]);
}

bool BridgedWitnessTable::isDeclaration() const {
  return table->isDeclaration();
}

bool BridgedWitnessTable::isSpecialized() const {
  return table->isSpecialized();
}

CodiraInt BridgedDefaultWitnessTable::getNumEntries() const {
  return CodiraInt(table->getEntries().size());
}

BridgedWitnessTableEntry BridgedDefaultWitnessTable::getEntry(CodiraInt index) const {
  return BridgedWitnessTableEntry::bridge(table->getEntries()[index]);
}

//===----------------------------------------------------------------------===//
//                         ConstExprFunctionState
//===----------------------------------------------------------------------===//
BridgedConstExprFunctionState BridgedConstExprFunctionState::create() {
  auto allocator = new language::SymbolicValueBumpAllocator();
  auto evaluator = new language::ConstExprEvaluator(*allocator, 0);
  auto numEvaluatedSILInstructions = new unsigned int(0);
  auto state = new language::ConstExprFunctionState(*evaluator, nullptr, {},
                                                 *numEvaluatedSILInstructions, true);
  return {state, allocator, evaluator, numEvaluatedSILInstructions};
}

bool BridgedConstExprFunctionState::isConstantValue(BridgedValue bridgedValue) {
  auto value = bridgedValue.getSILValue();
  auto symbolicValue = state->getConstantValue(value);
  return symbolicValue.isConstant();
}

void BridgedConstExprFunctionState::deinitialize() {
  delete state;
  delete numEvaluatedSILInstructions;
  delete constantEvaluator;
  delete allocator;
}


//===----------------------------------------------------------------------===//
//                                BridgedBuilder
//===----------------------------------------------------------------------===//

language::SILBuilder BridgedBuilder::unbridged() const {
  switch (insertAt) {
  case BridgedBuilder::InsertAt::beforeInst:
    return language::SILBuilder(BridgedInstruction(insertionObj).unbridged(),
                             loc.getLoc().getScope());
  case BridgedBuilder::InsertAt::endOfBlock:
    return language::SILBuilder(BridgedBasicBlock(insertionObj).unbridged(),
                             loc.getLoc().getScope());
  case BridgedBuilder::InsertAt::startOfFunction:
    return language::SILBuilder(BridgedFunction(insertionObj).getFunction()->getEntryBlock(),
                             loc.getLoc().getScope());
  case BridgedBuilder::InsertAt::intoGlobal:
    return language::SILBuilder(BridgedGlobalVar(insertionObj).getGlobal());
  }
}

language::SILLocation BridgedBuilder::regularLoc() const {
  auto l = loc.getLoc().getLocation();
  switch (l.getKind()) {
    case language::SILLocation::ReturnKind:
    case language::SILLocation::ImplicitReturnKind:
    case language::SILLocation::ArtificialUnreachableKind:
      return language::RegularLocation(l);
    default:
      return l;
  }
}

language::SILLocation BridgedBuilder::returnLoc() const {
  auto l = loc.getLoc().getLocation();
  switch (l.getKind()) {
    case language::SILLocation::ArtificialUnreachableKind:
      return language::RegularLocation(l);
    default:
      return l;
  }
}

BridgedInstruction BridgedBuilder::createBuiltin(BridgedStringRef name, BridgedType type,
                                                 BridgedSubstitutionMap subs,
                                                 BridgedValueArray arguments) const {
  toolchain::SmallVector<language::SILValue, 16> argValues;
  auto builder = unbridged();
  auto ident = builder.getASTContext().getIdentifier(name.unbridged());
  return {builder.createBuiltin(
      regularLoc(), ident, type.unbridged(),
      subs.unbridged(), arguments.getValues(argValues))};
}

BridgedInstruction BridgedBuilder::createBuiltinBinaryFunction(BridgedStringRef name,
                                               BridgedType operandType, BridgedType resultType,
                                               BridgedValueArray arguments) const {
  toolchain::SmallVector<language::SILValue, 16> argValues;
  return {unbridged().createBuiltinBinaryFunction(
      regularLoc(), name.unbridged(), operandType.unbridged(),
      resultType.unbridged(), arguments.getValues(argValues))};
}

BridgedInstruction BridgedBuilder::createCondFail(BridgedValue condition, BridgedStringRef message) const {
  return {unbridged().createCondFail(regularLoc(), condition.getSILValue(),
                                     message.unbridged())};
}

BridgedInstruction BridgedBuilder::createIntegerLiteral(BridgedType type, CodiraInt value) const {
  return {
      unbridged().createIntegerLiteral(regularLoc(), type.unbridged(), value)};
}

BridgedInstruction BridgedBuilder::createAllocRef(BridgedType type,
    bool objc, bool canAllocOnStack, bool isBare,
    BridgedSILTypeArray elementTypes, BridgedValueArray elementCountOperands) const {
  toolchain::SmallVector<language::SILValue, 16> elementCountOperandsValues;
  return {unbridged().createAllocRef(
      regularLoc(), type.unbridged(), objc, canAllocOnStack, isBare,
      elementTypes.typeArray.unbridged<language::SILType>(),
      elementCountOperands.getValues(elementCountOperandsValues)
      )};
}

BridgedInstruction BridgedBuilder::createAllocStack(BridgedType type,
                                                    BridgedSILDebugVariable debugVar,
                                                    bool hasDynamicLifetime,
                                                    bool isLexical,
                                                    bool isFromVarDecl,
                                                    bool wasMoved) const {
  return {unbridged().createAllocStack(
      regularLoc(), type.unbridged(), debugVar.unbridge(),
      language::HasDynamicLifetime_t(hasDynamicLifetime),
      language::IsLexical_t(isLexical), language::IsFromVarDecl_t(isFromVarDecl),
      language::UsesMoveableValueDebugInfo_t(wasMoved), /*skipVarDeclAssert=*/ true)};
}

BridgedInstruction BridgedBuilder::createAllocStack(BridgedType type,
                                                    bool hasDynamicLifetime,
                                                    bool isLexical,
                                                    bool isFromVarDecl,
                                                    bool wasMoved) const {
  return {unbridged().createAllocStack(
      regularLoc(), type.unbridged(), std::nullopt,
      language::HasDynamicLifetime_t(hasDynamicLifetime),
      language::IsLexical_t(isLexical), language::IsFromVarDecl_t(isFromVarDecl),
      language::UsesMoveableValueDebugInfo_t(wasMoved), /*skipVarDeclAssert=*/ true)};
}

BridgedInstruction BridgedBuilder::createDeallocStack(BridgedValue operand) const {
  return {unbridged().createDeallocStack(regularLoc(), operand.getSILValue())};
}

BridgedInstruction BridgedBuilder::createDeallocStackRef(BridgedValue operand) const {
  return {
      unbridged().createDeallocStackRef(regularLoc(), operand.getSILValue())};
}

BridgedInstruction BridgedBuilder::createAddressToPointer(BridgedValue address, BridgedType pointerTy,
                                                          bool needsStackProtection) const {
  return {unbridged().createAddressToPointer(regularLoc(), address.getSILValue(), pointerTy.unbridged(),
                                             needsStackProtection)};
}

BridgedInstruction BridgedBuilder::createPointerToAddress(BridgedValue pointer, BridgedType addressTy,
                                                          bool isStrict, bool isInvariant,
                                                          uint64_t alignment) const {
  return {unbridged().createPointerToAddress(regularLoc(), pointer.getSILValue(), addressTy.unbridged(),
                                             isStrict, isInvariant,
                                             alignment == 0 ? toolchain::MaybeAlign() : toolchain::Align(alignment))};
}

BridgedInstruction BridgedBuilder::createIndexAddr(BridgedValue base, BridgedValue index,
                                                   bool needsStackProtection) const {
  return {unbridged().createIndexAddr(regularLoc(), base.getSILValue(), index.getSILValue(),
                                      needsStackProtection)};
}

BridgedInstruction BridgedBuilder::createUncheckedRefCast(BridgedValue op, BridgedType type) const {
  return {unbridged().createUncheckedRefCast(regularLoc(), op.getSILValue(),
                                             type.unbridged())};
}

BridgedInstruction BridgedBuilder::createUncheckedAddrCast(BridgedValue op, BridgedType type) const {
  return {unbridged().createUncheckedAddrCast(regularLoc(), op.getSILValue(),
                                              type.unbridged())};
}

BridgedInstruction BridgedBuilder::createUpcast(BridgedValue op, BridgedType type) const {
  return {unbridged().createUpcast(regularLoc(), op.getSILValue(),
                                   type.unbridged())};
}

BridgedInstruction BridgedBuilder::createCheckedCastAddrBranch(
    BridgedValue source, BridgedCanType sourceFormalType,
    BridgedValue destination, BridgedCanType targetFormalType,
    BridgedInstruction::CheckedCastInstOptions options,
    BridgedInstruction::CastConsumptionKind consumptionKind,
    BridgedBasicBlock successBlock, BridgedBasicBlock failureBlock) const
{
  return {unbridged().createCheckedCastAddrBranch(
            regularLoc(),
            language::CheckedCastInstOptions(options.storage),
            (language::CastConsumptionKind)consumptionKind,
            source.getSILValue(), sourceFormalType.unbridged(),
            destination.getSILValue(), targetFormalType.unbridged(),
            successBlock.unbridged(), failureBlock.unbridged())};
}

BridgedInstruction BridgedBuilder::createUnconditionalCheckedCastAddr(
    BridgedInstruction::CheckedCastInstOptions options,
    BridgedValue source, BridgedCanType sourceFormalType,
    BridgedValue destination, BridgedCanType targetFormalType) const
{
  return {unbridged().createUnconditionalCheckedCastAddr(
            regularLoc(),
            language::CheckedCastInstOptions(options.storage),
            source.getSILValue(), sourceFormalType.unbridged(),
            destination.getSILValue(), targetFormalType.unbridged())};
}

BridgedInstruction BridgedBuilder::createLoad(BridgedValue op, CodiraInt ownership) const {
  return {unbridged().createLoad(regularLoc(), op.getSILValue(),
                                 (language::LoadOwnershipQualifier)ownership)};
}


BridgedInstruction BridgedBuilder::createUncheckedOwnershipConversion(BridgedValue op,
                                                                      BridgedValue::Ownership ownership) const {
  return {unbridged().createUncheckedOwnershipConversion(regularLoc(), op.getSILValue(),
                                                         BridgedValue::unbridge(ownership))};
}

BridgedInstruction BridgedBuilder::createLoadBorrow(BridgedValue op) const {
  return {unbridged().createLoadBorrow(regularLoc(), op.getSILValue())};
}

BridgedInstruction BridgedBuilder::createBeginDeallocRef(BridgedValue reference, BridgedValue allocation) const {
  return {unbridged().createBeginDeallocRef(
      regularLoc(), reference.getSILValue(), allocation.getSILValue())};
}

BridgedInstruction BridgedBuilder::createEndInitLetRef(BridgedValue op) const {
  return {unbridged().createEndInitLetRef(regularLoc(), op.getSILValue())};
}

BridgedInstruction BridgedBuilder::createRetainValue(BridgedValue op) const {
  auto b = unbridged();
  return {b.createRetainValue(regularLoc(), op.getSILValue(),
                              b.getDefaultAtomicity())};
}

BridgedInstruction BridgedBuilder::createReleaseValue(BridgedValue op) const {
  auto b = unbridged();
  return {b.createReleaseValue(regularLoc(), op.getSILValue(),
                               b.getDefaultAtomicity())};
}

BridgedInstruction BridgedBuilder::createStrongRetain(BridgedValue op) const {
  auto b = unbridged();
  return {b.createStrongRetain(regularLoc(), op.getSILValue(), b.getDefaultAtomicity())};
}

BridgedInstruction BridgedBuilder::createStrongRelease(BridgedValue op) const {
  auto b = unbridged();
  return {b.createStrongRelease(regularLoc(), op.getSILValue(), b.getDefaultAtomicity())};
}

BridgedInstruction BridgedBuilder::createUnownedRetain(BridgedValue op) const {
  auto b = unbridged();
  return {b.createUnownedRetain(regularLoc(), op.getSILValue(), b.getDefaultAtomicity())};
}

BridgedInstruction BridgedBuilder::createUnownedRelease(BridgedValue op) const {
  auto b = unbridged();
  return {b.createUnownedRelease(regularLoc(), op.getSILValue(), b.getDefaultAtomicity())};
}

BridgedInstruction BridgedBuilder::createFunctionRef(BridgedFunction function) const {
  return {unbridged().createFunctionRef(regularLoc(), function.getFunction())};
}

BridgedInstruction BridgedBuilder::createCopyValue(BridgedValue op) const {
  return {unbridged().createCopyValue(regularLoc(), op.getSILValue())};
}

BridgedInstruction BridgedBuilder::createBeginBorrow(BridgedValue op,
                                                     bool isLexical,
                                                     bool hasPointerEscape,
                                                     bool isFromVarDecl) const {
  return {unbridged().createBeginBorrow(regularLoc(), op.getSILValue(),
                                        language::IsLexical_t(isLexical),
                                        language::HasPointerEscape_t(hasPointerEscape),
                                        language::IsFromVarDecl_t(isFromVarDecl))};
}

BridgedInstruction BridgedBuilder::createBorrowedFrom(BridgedValue borrowedValue,
                                                      BridgedValueArray enclosingValues) const {
  toolchain::SmallVector<language::SILValue, 16> evs;
  return {unbridged().createBorrowedFrom(regularLoc(), borrowedValue.getSILValue(),
                                         enclosingValues.getValues(evs))};
}

BridgedInstruction BridgedBuilder::createEndBorrow(BridgedValue op) const {
  return {unbridged().createEndBorrow(regularLoc(), op.getSILValue())};
}

BridgedInstruction BridgedBuilder::createCopyAddr(BridgedValue from, BridgedValue to,
                                  bool takeSource, bool initializeDest) const {
  return {unbridged().createCopyAddr(
      regularLoc(), from.getSILValue(), to.getSILValue(),
      language::IsTake_t(takeSource), language::IsInitialization_t(initializeDest))};
}

BridgedInstruction BridgedBuilder::createDestroyValue(BridgedValue op) const {
  return {unbridged().createDestroyValue(regularLoc(), op.getSILValue())};
}

BridgedInstruction BridgedBuilder::createDestroyAddr(BridgedValue op) const {
  return {unbridged().createDestroyAddr(regularLoc(), op.getSILValue())};
}

BridgedInstruction BridgedBuilder::createEndLifetime(BridgedValue op) const {
  return {unbridged().createEndLifetime(regularLoc(), op.getSILValue())};
}

BridgedInstruction BridgedBuilder::createDebugValue(BridgedValue op,
                                                    BridgedSILDebugVariable var) const {
  return {unbridged().createDebugValue(regularLoc(), op.getSILValue(), var.unbridge())};
}

BridgedInstruction BridgedBuilder::createDebugStep() const {
  return {unbridged().createDebugStep(regularLoc())};
}

BridgedInstruction BridgedBuilder::createApply(BridgedValue function, BridgedSubstitutionMap subMap,
                               BridgedValueArray arguments, bool isNonThrowing, bool isNonAsync,
                               BridgedGenericSpecializationInformation specInfo) const {
  toolchain::SmallVector<language::SILValue, 16> argValues;
  language::ApplyOptions applyOpts;
  if (isNonThrowing) { applyOpts |= language::ApplyFlags::DoesNotThrow; }
  if (isNonAsync) { applyOpts |= language::ApplyFlags::DoesNotAwait; }

  return {unbridged().createApply(
      regularLoc(), function.getSILValue(), subMap.unbridged(),
      arguments.getValues(argValues), applyOpts, specInfo.data)};
}

BridgedInstruction BridgedBuilder::createTryApply(BridgedValue function, BridgedSubstitutionMap subMap,
                               BridgedValueArray arguments,
                               BridgedBasicBlock normalBB, BridgedBasicBlock errorBB,
                               bool isNonAsync,
                               BridgedGenericSpecializationInformation specInfo) const {
  toolchain::SmallVector<language::SILValue, 16> argValues;
  language::ApplyOptions applyOpts;
  if (isNonAsync) { applyOpts |= language::ApplyFlags::DoesNotAwait; }

  return {unbridged().createTryApply(
      regularLoc(), function.getSILValue(), subMap.unbridged(),
      arguments.getValues(argValues), normalBB.unbridged(), errorBB.unbridged(), applyOpts, specInfo.data)};
}

BridgedInstruction BridgedBuilder::createBeginApply(BridgedValue function, BridgedSubstitutionMap subMap,
                               BridgedValueArray arguments, bool isNonThrowing, bool isNonAsync,
                               BridgedGenericSpecializationInformation specInfo) const {
  toolchain::SmallVector<language::SILValue, 16> argValues;
  language::ApplyOptions applyOpts;
  if (isNonThrowing) { applyOpts |= language::ApplyFlags::DoesNotThrow; }
  if (isNonAsync) { applyOpts |= language::ApplyFlags::DoesNotAwait; }

  return {unbridged().createBeginApply(
      regularLoc(), function.getSILValue(), subMap.unbridged(),
      arguments.getValues(argValues), applyOpts, specInfo.data)};
}

BridgedInstruction BridgedBuilder::createWitnessMethod(BridgedCanType lookupType,
                                        BridgedConformance conformance,
                                        BridgedDeclRef member, BridgedType methodType) const {
  return {unbridged().createWitnessMethod(regularLoc(), lookupType.unbridged(), conformance.unbridged(),
                                          member.unbridged(), methodType.unbridged())};
}


BridgedInstruction BridgedBuilder::createReturn(BridgedValue op) const {
  return {unbridged().createReturn(returnLoc(), op.getSILValue())};
}

BridgedInstruction BridgedBuilder::createThrow(BridgedValue op) const {
  return {unbridged().createThrow(regularLoc(), op.getSILValue())};
}

BridgedInstruction BridgedBuilder::createUncheckedEnumData(BridgedValue enumVal, CodiraInt caseIdx,
                                           BridgedType resultType) const {
  language::SILValue en = enumVal.getSILValue();
  return {unbridged().createUncheckedEnumData(
      regularLoc(), enumVal.getSILValue(),
      en->getType().getEnumElement(caseIdx), resultType.unbridged())};
}

BridgedInstruction BridgedBuilder::createUncheckedTakeEnumDataAddr(BridgedValue enumAddr, CodiraInt caseIdx) const {
  language::SILValue en = enumAddr.getSILValue();
  return {unbridged().createUncheckedTakeEnumDataAddr(regularLoc(), en, en->getType().getEnumElement(caseIdx))};
}

BridgedInstruction BridgedBuilder::createInitEnumDataAddr(BridgedValue enumAddr,
                                                          CodiraInt caseIdx,
                                                          BridgedType type) const {
  language::SILValue en = enumAddr.getSILValue();
  return {unbridged().createInitEnumDataAddr(regularLoc(), en, en->getType().getEnumElement(caseIdx),
                                             type.unbridged())};
}

BridgedInstruction BridgedBuilder::createEnum(CodiraInt caseIdx, OptionalBridgedValue payload,
                              BridgedType resultType) const {
  language::EnumElementDecl *caseDecl =
      resultType.unbridged().getEnumElement(caseIdx);
  language::SILValue pl = payload.getSILValue();
  return {unbridged().createEnum(regularLoc(), pl, caseDecl,
                                 resultType.unbridged())};
}

BridgedInstruction BridgedBuilder::createThinToThickFunction(BridgedValue fn, BridgedType resultType) const {
  return {unbridged().createThinToThickFunction(regularLoc(), fn.getSILValue(),
                                                resultType.unbridged())};
}

BridgedInstruction BridgedBuilder::createPartialApply(BridgedValue funcRef, 
                                                      BridgedValueArray bridgedCapturedArgs,
                                                      BridgedArgumentConvention calleeConvention,
                                                      BridgedSubstitutionMap bridgedSubstitutionMap,
                                                      bool hasUnknownIsolation,
                                                      bool isOnStack) const {
  toolchain::SmallVector<language::SILValue, 8> capturedArgs;
  return {unbridged().createPartialApply(
      regularLoc(), funcRef.getSILValue(), bridgedSubstitutionMap.unbridged(),
      bridgedCapturedArgs.getValues(capturedArgs),
      getParameterConvention(calleeConvention),
      hasUnknownIsolation ? language::SILFunctionTypeIsolation::forUnknown()
                          : language::SILFunctionTypeIsolation::forErased(),
      isOnStack ? language::PartialApplyInst::OnStack
                : language::PartialApplyInst::NotOnStack)};
}                                                                                  

BridgedInstruction BridgedBuilder::createBranch(BridgedBasicBlock destBlock, BridgedValueArray arguments) const {
  toolchain::SmallVector<language::SILValue, 16> argValues;
  return {unbridged().createBranch(regularLoc(), destBlock.unbridged(),
                                   arguments.getValues(argValues))};
}

BridgedInstruction BridgedBuilder::createUnreachable() const {
  return {unbridged().createUnreachable(loc.getLoc().getLocation())};
}

BridgedInstruction BridgedBuilder::createObject(BridgedType type,
                                                BridgedValueArray arguments,
                                                CodiraInt numBaseElements) const {
  toolchain::SmallVector<language::SILValue, 16> argValues;
  return {unbridged().createObject(
      language::ArtificialUnreachableLocation(), type.unbridged(),
      arguments.getValues(argValues), numBaseElements)};
}

BridgedInstruction BridgedBuilder::createVector(BridgedValueArray arguments) const {
  toolchain::SmallVector<language::SILValue, 16> argValues;
  return {unbridged().createVector(language::ArtificialUnreachableLocation(), arguments.getValues(argValues))};
}

BridgedInstruction BridgedBuilder::createVectorBaseAddr(BridgedValue vector) const {
  return {unbridged().createVectorBaseAddr(regularLoc(), vector.getSILValue())};
}

BridgedInstruction BridgedBuilder::createGlobalAddr(BridgedGlobalVar global,
                                                    OptionalBridgedValue dependencyToken) const {
  return {unbridged().createGlobalAddr(regularLoc(), global.getGlobal(), dependencyToken.getSILValue())};
}

BridgedInstruction BridgedBuilder::createGlobalValue(BridgedGlobalVar global, bool isBare) const {
  return {
      unbridged().createGlobalValue(regularLoc(), global.getGlobal(), isBare)};
}

BridgedInstruction BridgedBuilder::createStruct(BridgedType type, BridgedValueArray elements) const {
  toolchain::SmallVector<language::SILValue, 16> elementValues;
  return {unbridged().createStruct(regularLoc(), type.unbridged(),
                                   elements.getValues(elementValues))};
}

BridgedInstruction BridgedBuilder::createStructExtract(BridgedValue str, CodiraInt fieldIndex) const {
  language::SILValue v = str.getSILValue();
  return {unbridged().createStructExtract(
      regularLoc(), v, v->getType().getFieldDecl(fieldIndex))};
}

BridgedInstruction BridgedBuilder::createStructElementAddr(BridgedValue addr, CodiraInt fieldIndex) const {
  language::SILValue v = addr.getSILValue();
  return {unbridged().createStructElementAddr(
      regularLoc(), v, v->getType().getFieldDecl(fieldIndex))};
}

BridgedInstruction BridgedBuilder::createDestructureStruct(BridgedValue str) const {
  return {unbridged().createDestructureStruct(regularLoc(), str.getSILValue())};
}

BridgedInstruction BridgedBuilder::createTuple(BridgedType type, BridgedValueArray elements) const {
  toolchain::SmallVector<language::SILValue, 16> elementValues;
  return {unbridged().createTuple(regularLoc(), type.unbridged(),
                                  elements.getValues(elementValues))};
}

BridgedInstruction BridgedBuilder::createTupleExtract(BridgedValue str, CodiraInt elementIndex) const {
  language::SILValue v = str.getSILValue();
  return {unbridged().createTupleExtract(regularLoc(), v, elementIndex)};
}

BridgedInstruction BridgedBuilder::createTupleElementAddr(BridgedValue addr, CodiraInt elementIndex) const {
  language::SILValue v = addr.getSILValue();
  return {unbridged().createTupleElementAddr(regularLoc(), v, elementIndex)};
}

BridgedInstruction BridgedBuilder::createDestructureTuple(BridgedValue str) const {
  return {unbridged().createDestructureTuple(regularLoc(), str.getSILValue())};
}

BridgedInstruction BridgedBuilder::createProjectBox(BridgedValue box, CodiraInt fieldIdx) const {
  return {unbridged().createProjectBox(regularLoc(), box.getSILValue(), (unsigned)fieldIdx)};
}

BridgedInstruction BridgedBuilder::createStore(BridgedValue src, BridgedValue dst,
                               CodiraInt ownership) const {
  return {unbridged().createStore(regularLoc(), src.getSILValue(),
                                  dst.getSILValue(),
                                  (language::StoreOwnershipQualifier)ownership)};
}

BridgedInstruction BridgedBuilder::createInitExistentialRef(BridgedValue instance,
                                            BridgedType type,
                                            BridgedCanType formalConcreteType,
                                            BridgedConformanceArray conformances) const {
  return {unbridged().createInitExistentialRef(
      regularLoc(), type.unbridged(), formalConcreteType.unbridged(),
      instance.getSILValue(), conformances.pcArray.unbridged<language::ProtocolConformanceRef>())};
}

BridgedInstruction BridgedBuilder::createInitExistentialMetatype(BridgedValue metatype,
                                            BridgedType existentialType,
                                            BridgedConformanceArray conformances) const {
  return {unbridged().createInitExistentialMetatype(
      regularLoc(), metatype.getSILValue(), existentialType.unbridged(),
      conformances.pcArray.unbridged<language::ProtocolConformanceRef>())};
}

BridgedInstruction BridgedBuilder::createMetatype(BridgedCanType instanceType,
                                                  BridgedASTType::MetatypeRepresentation representation) const {
  auto *mt =
      language::MetatypeType::get(instanceType.unbridged(),
                               (language::MetatypeRepresentation)representation);
  auto t = language::SILType::getPrimitiveObjectType(language::CanType(mt));
  return {unbridged().createMetatype(regularLoc(), t)};
}

BridgedInstruction BridgedBuilder::createEndCOWMutation(BridgedValue instance, bool keepUnique) const {
  return {unbridged().createEndCOWMutation(regularLoc(), instance.getSILValue(),
                                           keepUnique)};
}

BridgedInstruction
BridgedBuilder::createEndCOWMutationAddr(BridgedValue instance) const {
  return {unbridged().createEndCOWMutationAddr(regularLoc(),
                                               instance.getSILValue())};
}

BridgedInstruction BridgedBuilder::createMarkDependence(BridgedValue value, BridgedValue base, BridgedInstruction::MarkDependenceKind kind) const {
  return {unbridged().createMarkDependence(regularLoc(), value.getSILValue(), base.getSILValue(), language::MarkDependenceKind(kind))};
}

BridgedInstruction BridgedBuilder::createMarkDependenceAddr(BridgedValue value, BridgedValue base, BridgedInstruction::MarkDependenceKind kind) const {
  return {unbridged().createMarkDependenceAddr(
      regularLoc(), value.getSILValue(), base.getSILValue(),
      language::MarkDependenceKind(kind))};
}

BridgedInstruction BridgedBuilder::createMarkUninitialized(BridgedValue value, CodiraInt kind) const {
  return {unbridged().createMarkUninitialized(
      regularLoc(), value.getSILValue(), (language::MarkUninitializedInst::Kind)kind)};
}

BridgedInstruction BridgedBuilder::createMarkUnresolvedNonCopyableValue(BridgedValue value,
                                                                        CodiraInt checkKind, bool isStrict) const {
  return {unbridged().createMarkUnresolvedNonCopyableValueInst(
      regularLoc(), value.getSILValue(), (language::MarkUnresolvedNonCopyableValueInst::CheckKind)checkKind,
      (language::MarkUnresolvedNonCopyableValueInst::IsStrict_t)isStrict)};
}


BridgedInstruction BridgedBuilder::createEndAccess(BridgedValue value) const {
  return {unbridged().createEndAccess(regularLoc(), value.getSILValue(), false)};
}

BridgedInstruction BridgedBuilder::createEndApply(BridgedValue value) const {
  language::ASTContext &ctxt = unbridged().getASTContext();
  return {unbridged().createEndApply(regularLoc(), value.getSILValue(),
                                     language::SILType::getEmptyTupleType(ctxt))};
}

BridgedInstruction BridgedBuilder::createAbortApply(BridgedValue value) const {
  return {unbridged().createAbortApply(regularLoc(), value.getSILValue())};
}

BridgedInstruction BridgedBuilder::createConvertFunction(BridgedValue originalFunction, BridgedType resultType, bool withoutActuallyEscaping) const {
return {unbridged().createConvertFunction(regularLoc(), originalFunction.getSILValue(), resultType.unbridged(), withoutActuallyEscaping)};
}

BridgedInstruction BridgedBuilder::createConvertEscapeToNoEscape(BridgedValue originalFunction, BridgedType resultType, bool isLifetimeGuaranteed) const {
  return {unbridged().createConvertEscapeToNoEscape(regularLoc(), originalFunction.getSILValue(), resultType.unbridged(), isLifetimeGuaranteed)};
}

LANGUAGE_END_NULLABILITY_ANNOTATIONS

#endif
