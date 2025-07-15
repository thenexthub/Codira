//===--- ASTBridgingImpl.h - header for the language ASTBridging module ------===//
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

#ifndef LANGUAGE_AST_ASTBRIDGINGIMPL_H
#define LANGUAGE_AST_ASTBRIDGINGIMPL_H

#include "language/AST/ASTContext.h"
#include "language/AST/ArgumentList.h"
#include "language/AST/AvailabilityDomain.h"
#include "language/AST/ConformanceLookup.h"
#include "language/AST/Decl.h"
#include "language/AST/Expr.h"
#include "language/AST/GenericSignature.h"
#include "language/AST/GenericEnvironment.h"
#include "language/AST/IfConfigClauseRangeInfo.h"
#include "language/AST/MacroDeclaration.h"
#include "language/AST/ProtocolConformance.h"
#include "language/AST/ProtocolConformanceRef.h"
#include "language/AST/SourceFile.h"
#include "language/AST/Stmt.h"
#include "language/Basic/Assertions.h"
#include "language/Basic/Fingerprint.h"

LANGUAGE_BEGIN_NULLABILITY_ANNOTATIONS

//===----------------------------------------------------------------------===//
// MARK: BridgedIdentifier
//===----------------------------------------------------------------------===//

BridgedIdentifier::BridgedIdentifier(language::Identifier ident)
    : Raw(ident.getAsOpaquePointer()) {}

language::Identifier BridgedIdentifier::unbridged() const {
  return language::Identifier::getFromOpaquePointer(Raw);
}

bool BridgedIdentifier::getIsOperator() const {
  return unbridged().isOperator();
}

//===----------------------------------------------------------------------===//
// MARK: BridgedDeclBaseName
//===----------------------------------------------------------------------===//

BridgedDeclBaseName::BridgedDeclBaseName(language::DeclBaseName baseName)
  : Ident(baseName.Ident) {}

language::DeclBaseName BridgedDeclBaseName::unbridged() const {
  return language::DeclBaseName(Ident.unbridged());
}

//===----------------------------------------------------------------------===//
// MARK: BridgedDeclBaseName
//===----------------------------------------------------------------------===//

BridgedConsumedLookupResult::BridgedConsumedLookupResult(
    language::Identifier name, language::SourceLoc sourceLoc, CodiraInt flag)
    : Name(BridgedIdentifier(name)), NameLoc(BridgedSourceLoc(sourceLoc)),
      Flag(flag) {}

//===----------------------------------------------------------------------===//
// MARK: BridgedDeclNameRef
//===----------------------------------------------------------------------===//

BridgedDeclNameRef::BridgedDeclNameRef()
    : BridgedDeclNameRef(language::DeclNameRef()) {}

BridgedDeclNameRef::BridgedDeclNameRef(language::DeclNameRef name)
  : opaque(name.getOpaqueValue()) {}

language::DeclNameRef BridgedDeclNameRef::unbridged() const {
  return language::DeclNameRef::getFromOpaqueValue(opaque);
}

//===----------------------------------------------------------------------===//
// MARK: BridgedDeclNameLoc
//===----------------------------------------------------------------------===//

BridgedDeclNameLoc::BridgedDeclNameLoc(language::DeclNameLoc loc)
    : LocationInfo(loc.LocationInfo),
      NumArgumentLabels(loc.NumArgumentLabels) {}

language::DeclNameLoc BridgedDeclNameLoc::unbridged() const {
  return language::DeclNameLoc(LocationInfo, NumArgumentLabels);
}

//===----------------------------------------------------------------------===//
// MARK: BridgedASTContext
//===----------------------------------------------------------------------===//

BridgedASTContext::BridgedASTContext(language::ASTContext &ctx) : Ctx(&ctx) {}

language::ASTContext &BridgedASTContext::unbridged() const { return *Ctx; }

BridgedASTContext BridgedASTContext_fromRaw(void * _Nonnull ptr) {
  return *static_cast<language::ASTContext *>(ptr);
}

void *_Nullable BridgedASTContext_allocate(BridgedASTContext bridged,
                                           size_t size, size_t alignment) {
  return bridged.unbridged().Allocate(size, alignment);
}

BridgedStringRef BridgedASTContext_allocateCopyString(BridgedASTContext bridged,
                                                      BridgedStringRef cStr) {
  return bridged.unbridged().AllocateCopy(cStr.unbridged());
}

#define IDENTIFIER_WITH_NAME(Name, _) \
BridgedIdentifier BridgedASTContext_id_##Name(BridgedASTContext bridged) { \
return bridged.unbridged().Id_##Name; \
}
#include "language/AST/KnownIdentifiers.def"

//===----------------------------------------------------------------------===//
// MARK: BridgedDeclContext
//===----------------------------------------------------------------------===//

bool BridgedDeclContext_isLocalContext(BridgedDeclContext dc) {
  return dc.unbridged()->isLocalContext();
}

bool BridgedDeclContext_isTypeContext(BridgedDeclContext dc) {
  return dc.unbridged()->isTypeContext();
}

bool BridgedDeclContext_isModuleScopeContext(BridgedDeclContext dc) {
  return dc.unbridged()->isModuleScopeContext();
}

bool BridgedDeclContext_isClosureExpr(BridgedDeclContext dc) {
  return toolchain::isa_and_present<language::ClosureExpr>(
      toolchain::dyn_cast<language::AbstractClosureExpr>(dc.unbridged()));
}

BridgedClosureExpr BridgedDeclContext_castToClosureExpr(BridgedDeclContext dc) {
  return toolchain::cast<language::ClosureExpr>(
      toolchain::cast<language::AbstractClosureExpr>(dc.unbridged()));
}

BridgedASTContext BridgedDeclContext_getASTContext(BridgedDeclContext dc) {
  return dc.unbridged()->getASTContext();
}

BridgedSourceFile
BridgedDeclContext_getParentSourceFile(BridgedDeclContext dc) {
  return dc.unbridged()->getParentSourceFile();
}

//===----------------------------------------------------------------------===//
// MARK: BridgedSoureFile
//===----------------------------------------------------------------------===//

bool BridgedSourceFile_isScriptMode(BridgedSourceFile sf) {
  return sf.unbridged()->isScriptMode();
}

//===----------------------------------------------------------------------===//
// MARK: BridgedDeclObj
//===----------------------------------------------------------------------===//

BridgedSourceLoc BridgedDeclObj::getLoc() const {
  language::SourceLoc sourceLoc = unbridged()->getLoc();
  return BridgedSourceLoc(sourceLoc.getOpaquePointerValue());
}

BridgedDeclObj BridgedDeclObj::getModuleContext() const {
  return {unbridged()->getModuleContext()};
}

OptionalBridgedDeclObj BridgedDeclObj::getParent() const {
  return {unbridged()->getDeclContext()->getAsDecl()};
}

BridgedStringRef BridgedDeclObj::Type_getName() const {
  return getAs<language::TypeDecl>()->getName().str();
}

BridgedStringRef BridgedDeclObj::Value_getUserFacingName() const {
  return getAs<language::ValueDecl>()->getBaseName().userFacingName();
}

BridgedSourceLoc BridgedDeclObj::Value_getNameLoc() const {
  return BridgedSourceLoc(getAs<language::ValueDecl>()->getNameLoc().getOpaquePointerValue());
}

bool BridgedDeclObj::hasClangNode() const {
  return unbridged()->hasClangNode();
}

bool BridgedDeclObj::Value_isObjC() const {
  return getAs<language::ValueDecl>()->isObjC();
}

bool BridgedDeclObj::AbstractStorage_isConst() const {
  return getAs<language::AbstractStorageDecl>()->isConstValue();
}

bool BridgedDeclObj::GenericType_isGenericAtAnyLevel() const {
  return getAs<language::GenericTypeDecl>()->isGenericContext();
}

bool BridgedDeclObj::NominalType_isGlobalActor() const {
  return getAs<language::NominalTypeDecl>()->isGlobalActor();
}

OptionalBridgedDeclObj BridgedDeclObj::NominalType_getValueTypeDestructor() const {
  return {getAs<language::NominalTypeDecl>()->getValueTypeDestructor()};
}

bool BridgedDeclObj::Enum_hasRawType() const {
  return getAs<language::EnumDecl>()->hasRawType();
}

bool BridgedDeclObj::Struct_hasUnreferenceableStorage() const {
  return getAs<language::StructDecl>()->hasUnreferenceableStorage();
}

BridgedASTType BridgedDeclObj::Class_getSuperclass() const {
  return {getAs<language::ClassDecl>()->getSuperclass().getPointer()};
}

BridgedDeclObj BridgedDeclObj::Class_getDestructor() const {
  return {getAs<language::ClassDecl>()->getDestructor()};
}

bool BridgedDeclObj::ProtocolDecl_requiresClass() const {
  return getAs<language::ProtocolDecl>()->requiresClass();
}

bool BridgedDeclObj::AbstractFunction_isOverridden() const {
  return getAs<language::AbstractFunctionDecl>()->isOverridden();
}

bool BridgedDeclObj::Destructor_isIsolated() const {
  auto dd = getAs<language::DestructorDecl>();
  auto ai = language::getActorIsolation(dd);
  return ai.isActorIsolated();
}

//===----------------------------------------------------------------------===//
// MARK: BridgedASTNode
//===----------------------------------------------------------------------===//

BridgedASTNode::BridgedASTNode(void *_Nonnull pointer, BridgedASTNodeKind kind)
    : opaque(intptr_t(pointer) | kind) {
  assert(getPointer() == pointer && getKind() == kind);
}

BridgedExpr BridgedASTNode::castToExpr() const {
  assert(getKind() == BridgedASTNodeKindExpr);
  return static_cast<language::Expr *>(getPointer());
}
BridgedStmt BridgedASTNode::castToStmt() const {
  assert(getKind() == BridgedASTNodeKindStmt);
  return static_cast<language::Stmt *>(getPointer());
}
BridgedDecl BridgedASTNode::castToDecl() const {
  assert(getKind() == BridgedASTNodeKindDecl);
  return static_cast<language::Decl *>(getPointer());
}

language::ASTNode BridgedASTNode::unbridged() const {
  switch (getKind()) {
  case BridgedASTNodeKindExpr:
    return castToExpr().unbridged();
  case BridgedASTNodeKindStmt:
    return castToStmt().unbridged();
  case BridgedASTNodeKindDecl:
    return castToDecl().unbridged();
  }
}

//===----------------------------------------------------------------------===//
// MARK: Diagnostic Engine
//===----------------------------------------------------------------------===//

BridgedDiagnosticArgument::BridgedDiagnosticArgument(const language::DiagnosticArgument &arg) {
  *reinterpret_cast<language::DiagnosticArgument *>(&storage) = arg;
}

const language::DiagnosticArgument &BridgedDiagnosticArgument::unbridged() const {
  return *reinterpret_cast<const language::DiagnosticArgument *>(&storage);
}

//===----------------------------------------------------------------------===//
// MARK: BridgedDeclAttributes
//===----------------------------------------------------------------------===//

BridgedDeclAttributes::BridgedDeclAttributes(language::DeclAttributes attrs)
    : chain(attrs.getRawAttributeChain()) {}

language::DeclAttributes BridgedDeclAttributes::unbridged() const {
  language::DeclAttributes attrs;
  attrs.setRawAttributeChain(chain.unbridged());
  return attrs;
}

//===----------------------------------------------------------------------===//
// MARK: AvailabilityDomainOrIdentifier
//===----------------------------------------------------------------------===//

BridgedAvailabilityDomainOrIdentifier::BridgedAvailabilityDomainOrIdentifier(
    language::AvailabilityDomainOrIdentifier domainOrIdentifier)
    : opaque(domainOrIdentifier.getOpaqueValue()) {}

language::AvailabilityDomainOrIdentifier
BridgedAvailabilityDomainOrIdentifier::unbridged() const {
  return language::AvailabilityDomainOrIdentifier::fromOpaque(opaque);
}

bool BridgedAvailabilityDomainOrIdentifier_isDomain(
    BridgedAvailabilityDomainOrIdentifier cVal) {
  return cVal.unbridged().isDomain();
}

BridgedIdentifier BridgedAvailabilityDomainOrIdentifier_getAsIdentifier(
    BridgedAvailabilityDomainOrIdentifier cVal) {
  if (auto ident = cVal.unbridged().getAsIdentifier())
    return *ident;
  return language::Identifier();
}

//===----------------------------------------------------------------------===//
// MARK: BridgedParamDecl
//===----------------------------------------------------------------------===//

language::ParamSpecifier unbridge(BridgedParamSpecifier specifier) {
  switch (specifier) {
#define CASE(ID)                                                               \
  case BridgedParamSpecifier##ID:                                              \
    return language::ParamSpecifier::ID;
    CASE(Default)
    CASE(InOut)
    CASE(Borrowing)
    CASE(Consuming)
    CASE(LegacyShared)
    CASE(LegacyOwned)
    CASE(ImplicitlyCopyableConsuming)
#undef CASE
  }
}

void BridgedParamDecl_setTypeRepr(BridgedParamDecl cDecl,
                                  BridgedTypeRepr cType) {
  cDecl.unbridged()->setTypeRepr(cType.unbridged());
}

void BridgedParamDecl_setSpecifier(BridgedParamDecl cDecl,
                                   BridgedParamSpecifier cSpecifier) {
  cDecl.unbridged()->setSpecifier(unbridge(cSpecifier));
}

void BridgedParamDecl_setImplicit(BridgedParamDecl cDecl) {
  cDecl.unbridged()->setImplicit();
}

//===----------------------------------------------------------------------===//
// MARK: BridgedSubscriptDecl
//===----------------------------------------------------------------------===//

BridgedAbstractStorageDecl
BridgedSubscriptDecl_asAbstractStorageDecl(BridgedSubscriptDecl decl) {
  return decl.unbridged();
}

//===----------------------------------------------------------------------===//
// MARK: BridgedVarDecl
//===----------------------------------------------------------------------===//

BridgedAbstractStorageDecl
BridgedVarDecl_asAbstractStorageDecl(BridgedVarDecl decl) {
  return decl.unbridged();
}

//===----------------------------------------------------------------------===//
// MARK: BridgedCallArgument
//===----------------------------------------------------------------------===//

language::Argument BridgedCallArgument::unbridged() const {
  return language::Argument(labelLoc.unbridged(), label.unbridged(),
                         argExpr.unbridged());
}

//===----------------------------------------------------------------------===//
// MARK: BridgedLabeledStmtInfo
//===----------------------------------------------------------------------===//

language::LabeledStmtInfo BridgedLabeledStmtInfo::unbridged() const {
  return {Name.unbridged(), Loc.unbridged()};
}

//===----------------------------------------------------------------------===//
// MARK: BridgedASTType
//===----------------------------------------------------------------------===//

language::Type BridgedASTType::unbridged() const {
  return type;
}

BridgedCanType BridgedASTType::getCanonicalType() const {
  return unbridged()->getCanonicalType();
}

BridgedDiagnosticArgument BridgedASTType::asDiagnosticArgument() const {
  return language::DiagnosticArgument(unbridged());
}

bool BridgedASTType::hasArchetype() const {
  return unbridged()->hasArchetype();
}

bool BridgedASTType::isLegalFormalType() const {
  return unbridged()->isLegalFormalType();
}

bool BridgedASTType::isGenericAtAnyLevel() const {
  return unbridged()->isSpecialized();
}


bool BridgedASTType::hasTypeParameter() const {
  return unbridged()->hasTypeParameter();
}

bool BridgedASTType::hasLocalArchetype() const {
  return unbridged()->hasLocalArchetype();
}

bool BridgedASTType::hasDynamicSelf() const {
  return unbridged()->hasDynamicSelfType();
}

bool BridgedASTType::isArchetype() const {
  return unbridged()->is<language::ArchetypeType>();
}

bool BridgedASTType::archetypeRequiresClass() const {
  return unbridged()->castTo<language::ArchetypeType>()->requiresClass();
}

bool BridgedASTType::isExistentialArchetype() const {
  return unbridged()->is<language::ExistentialArchetypeType>();
}

bool BridgedASTType::isExistentialArchetypeWithError() const {
  return unbridged()->isOpenedExistentialWithError();
}

bool BridgedASTType::isExistential() const {
  return unbridged()->is<language::ExistentialType>();
}

bool BridgedASTType::isDynamicSelf() const {
  return unbridged()->is<language::DynamicSelfType>();
}

bool BridgedASTType::isClassExistential() const {
  return unbridged()->isClassExistentialType();
}

bool BridgedASTType::isGenericTypeParam() const {
  return unbridged()->is<language::GenericTypeParamType>();
}

bool BridgedASTType::isEscapable() const {
  return unbridged()->isEscapable();
}

bool BridgedASTType::isNoEscape() const {
  return unbridged()->isNoEscape();
}

bool BridgedASTType::isInteger() const {
  return unbridged()->is<language::IntegerType>();
}

bool BridgedASTType::isMetatypeType() const {
  return unbridged()->is<language::MetatypeType>();
}

bool BridgedASTType::isExistentialMetatypeType() const {
  return unbridged()->is<language::ExistentialMetatypeType>();
}

bool BridgedASTType::isTuple() const {
  return unbridged()->is<language::TupleType>();
}

bool BridgedASTType::isFunction() const {
  return unbridged()->is<language::FunctionType>();
}

bool BridgedASTType::isLoweredFunction() const {
  return unbridged()->is<language::SILFunctionType>();
}

bool BridgedASTType::isNoEscapeFunction() const {
  if (auto *fTy = unbridged()->getAs<language::SILFunctionType>()) {
    return fTy->isNoEscape();
  }
  return false;
}

bool BridgedASTType::isThickFunction() const {
  if (auto *fTy = unbridged()->getAs<language::SILFunctionType>()) {
    return fTy->getRepresentation() == language::SILFunctionType::Representation::Thick;
  }
  return false;
}

bool BridgedASTType::isAsyncFunction() const {
  if (auto *fTy = unbridged()->getAs<language::SILFunctionType>()) {
    return fTy->isAsync();
  }
  return false;
}

bool BridgedASTType::isCalleeConsumedFunction() const {
  auto *funcTy = unbridged()->castTo<language::SILFunctionType>();
  return funcTy->isCalleeConsumed() && !funcTy->isNoEscape();
}

bool BridgedASTType::isBuiltinInteger() const {
  return unbridged()->is<language::BuiltinIntegerType>();
}

bool BridgedASTType::isBuiltinFloat() const {
  return unbridged()->is<language::BuiltinFloatType>();
}

bool BridgedASTType::isBuiltinVector() const {
  return unbridged()->is<language::BuiltinVectorType>();
}

bool BridgedASTType::isBuiltinFixedArray() const {
  return unbridged()->is<language::BuiltinFixedArrayType>();
}

bool BridgedASTType::isBox() const {
  return unbridged()->is<language::SILBoxType>();
}

BridgedASTType BridgedASTType::getBuiltinVectorElementType() const {
  return {unbridged()->castTo<language::BuiltinVectorType>()->getElementType().getPointer()};
}

BridgedCanType BridgedASTType::getBuiltinFixedArrayElementType() const {
  return unbridged()->castTo<language::BuiltinFixedArrayType>()->getElementType();
}

BridgedCanType BridgedASTType::getBuiltinFixedArraySizeType() const {
  return unbridged()->castTo<language::BuiltinFixedArrayType>()->getSize();
}

bool BridgedASTType::isBuiltinFixedWidthInteger(CodiraInt width) const {
  if (auto *intTy = unbridged()->getAs<language::BuiltinIntegerType>())
    return intTy->isFixedWidth((unsigned)width);
  return false;
}

bool BridgedASTType::isOptional() const {
  return unbridged()->getCanonicalType()->isOptional();
}

bool BridgedASTType::isUnownedStorageType() const {
  return unbridged()->is<language::UnownedStorageType>();
}

bool BridgedASTType::isBuiltinType() const {
  return unbridged()->isBuiltinType();
}

OptionalBridgedDeclObj BridgedASTType::getNominalOrBoundGenericNominal() const {
  return {unbridged()->getNominalOrBoundGenericNominal()};
}

BridgedASTType::TraitResult BridgedASTType::canBeClass() const {
  return (TraitResult)unbridged()->canBeClass();
}

OptionalBridgedDeclObj BridgedASTType::getAnyNominal() const {
  return {unbridged()->getAnyNominal()};
}

BridgedASTType BridgedASTType::getInstanceTypeOfMetatype() const {
  return {unbridged()->getAs<language::AnyMetatypeType>()->getInstanceType().getPointer()};
}

BridgedASTType BridgedASTType::getStaticTypeOfDynamicSelf() const {
  return {unbridged()->getAs<language::DynamicSelfType>()->getSelfType().getPointer()};
}

BridgedASTType BridgedASTType::getSuperClassType() const {
  return {unbridged()->getSuperclass().getPointer()};
}

BridgedASTType::MetatypeRepresentation BridgedASTType::getRepresentationOfMetatype() const {
  return MetatypeRepresentation(unbridged()->getAs<language::AnyMetatypeType>()->getRepresentation());
}

BridgedOptionalInt BridgedASTType::getValueOfIntegerType() const {
  return getFromAPInt(unbridged()->getAs<language::IntegerType>()->getValue());
}

BridgedSubstitutionMap BridgedASTType::getContextSubstitutionMap() const {
  return unbridged()->getContextSubstitutionMap();
}

BridgedGenericSignature BridgedASTType::getInvocationGenericSignatureOfFunctionType() const {
  return {unbridged()->castTo<language::SILFunctionType>()->getInvocationGenericSignature().getPointer()};
}

BridgedASTType BridgedASTType::subst(BridgedSubstitutionMap substMap) const {
  return {unbridged().subst(substMap.unbridged()).getPointer()};
}

BridgedConformance BridgedASTType::checkConformance(BridgedDeclObj proto) const {
  return language::checkConformance(unbridged(), proto.getAs<language::ProtocolDecl>(), /*allowMissing=*/ false);
}  

static_assert((int)BridgedASTType::TraitResult::IsNot == (int)language::TypeTraitResult::IsNot);
static_assert((int)BridgedASTType::TraitResult::CanBe == (int)language::TypeTraitResult::CanBe);
static_assert((int)BridgedASTType::TraitResult::Is == (int)language::TypeTraitResult::Is);

static_assert((int)BridgedASTType::MetatypeRepresentation::Thin == (int)language::MetatypeRepresentation::Thin);
static_assert((int)BridgedASTType::MetatypeRepresentation::Thick == (int)language::MetatypeRepresentation::Thick);
static_assert((int)BridgedASTType::MetatypeRepresentation::ObjC == (int)language::MetatypeRepresentation::ObjC);

//===----------------------------------------------------------------------===//
// MARK: BridgedCanType
//===----------------------------------------------------------------------===//

BridgedCanType::BridgedCanType() : type(nullptr) {
}

BridgedCanType::BridgedCanType(language::CanType ty) : type(ty.getPointer()) {
}

language::CanType BridgedCanType::unbridged() const {
  return language::CanType(type);
}

BridgedASTType BridgedCanType::getRawType() const {
  return {type};
}

//===----------------------------------------------------------------------===//
// MARK: BridgedASTTypeArray
//===----------------------------------------------------------------------===//

BridgedASTType BridgedASTTypeArray::getAt(CodiraInt index) const {
  return {typeArray.unbridged<language::Type>()[index].getPointer()};
}


//===----------------------------------------------------------------------===//
// MARK: BridgedConformance
//===----------------------------------------------------------------------===//

static_assert(sizeof(BridgedConformance) == sizeof(language::ProtocolConformanceRef));

BridgedConformance::BridgedConformance(language::ProtocolConformanceRef conformance)
    : opaqueValue(conformance.getOpaqueValue()) {}

language::ProtocolConformanceRef BridgedConformance::unbridged() const {
  return language::ProtocolConformanceRef::getFromOpaqueValue(opaqueValue);
}

bool BridgedConformance::isConcrete() const {
  return unbridged().isConcrete();
}

bool BridgedConformance::isValid() const {
  return !unbridged().isInvalid();
}

bool BridgedConformance::isSpecializedConformance() const {
  return language::isa<language::SpecializedProtocolConformance>(unbridged().getConcrete());
}

bool BridgedConformance::isInheritedConformance() const {
  return language::isa<language::InheritedProtocolConformance>(unbridged().getConcrete());
}

BridgedASTType BridgedConformance::getType() const {
  return {unbridged().getConcrete()->getType().getPointer()};
}

BridgedDeclObj BridgedConformance::getRequirement() const {
  return {unbridged().getProtocol()};
}

BridgedConformance BridgedConformance::getGenericConformance() const {
  auto *specPC = language::cast<language::SpecializedProtocolConformance>(unbridged().getConcrete());
  return {language::ProtocolConformanceRef(specPC->getGenericConformance())};
}

BridgedConformance BridgedConformance::getInheritedConformance() const {
  auto *inheritedConf = language::cast<language::InheritedProtocolConformance>(unbridged().getConcrete());
  return {language::ProtocolConformanceRef(inheritedConf->getInheritedConformance())};
}

BridgedSubstitutionMap BridgedConformance::getSpecializedSubstitutions() const {
  auto *specPC = language::cast<language::SpecializedProtocolConformance>(unbridged().getConcrete());
  return {specPC->getSubstitutionMap()};
}

BridgedConformance BridgedConformance::getAssociatedConformance(BridgedASTType assocType, BridgedDeclObj proto) const {
  return {unbridged().getConcrete()->getAssociatedConformance(assocType.unbridged(),
                                                              proto.getAs<language::ProtocolDecl>())};
}

BridgedConformance BridgedConformanceArray::getAt(CodiraInt index) const {
  return pcArray.unbridged<language::ProtocolConformanceRef>()[index];
}

//===----------------------------------------------------------------------===//
// MARK: BridgedLayoutConstraint
//===----------------------------------------------------------------------===//

BridgedLayoutConstraint::BridgedLayoutConstraint()
    : raw(language::LayoutConstraint().getPointer()) {}

BridgedLayoutConstraint::BridgedLayoutConstraint(
    language::LayoutConstraint constraint)
    : raw(constraint.getPointer()) {}

language::LayoutConstraint BridgedLayoutConstraint::unbridged() const {
  return raw;
}

bool BridgedLayoutConstraint::getIsNull() const { return unbridged().isNull(); }

bool BridgedLayoutConstraint::getIsKnownLayout() const {
  return unbridged()->isKnownLayout();
}

bool BridgedLayoutConstraint::getIsTrivial() const {
  return unbridged()->isTrivial();
}

//===----------------------------------------------------------------------===//
// MARK: Macros
//===----------------------------------------------------------------------===//

language::MacroRole unbridge(BridgedMacroRole cRole) {
  switch (cRole) {
#define MACRO_ROLE(Name, Description)                                          \
  case BridgedMacroRole##Name:                                                 \
    return language::MacroRole::Name;
#include "language/Basic/MacroRoles.def"
  case BridgedMacroRoleNone:
    break;
  }
  toolchain_unreachable("invalid macro role");
}

language::MacroIntroducedDeclNameKind
unbridge(BridgedMacroIntroducedDeclNameKind kind) {
  switch (kind) {
#define CASE(ID)                                                               \
  case BridgedMacroIntroducedDeclNameKind##ID:                                 \
    return language::MacroIntroducedDeclNameKind::ID;
    CASE(Named)
    CASE(Overloaded)
    CASE(Prefixed)
    CASE(Suffixed)
    CASE(Arbitrary)
#undef CASE
  }
}

bool BridgedMacroRole_isAttached(BridgedMacroRole role) {
  return isAttachedMacro(unbridge(role));
}

language::MacroIntroducedDeclName
BridgedMacroIntroducedDeclName::unbridged() const {
  return language::MacroIntroducedDeclName(unbridge(kind),
                                        name.unbridged().getFullName());
}

//===----------------------------------------------------------------------===//
// MARK: BridgedSubstitutionMap
//===----------------------------------------------------------------------===//

BridgedSubstitutionMap::BridgedSubstitutionMap(language::SubstitutionMap map) {
  *reinterpret_cast<language::SubstitutionMap *>(&storage) = map;
}

language::SubstitutionMap BridgedSubstitutionMap::unbridged() const {
  return *reinterpret_cast<const language::SubstitutionMap *>(&storage);
}

BridgedSubstitutionMap::BridgedSubstitutionMap()
  : BridgedSubstitutionMap(language::SubstitutionMap()) {}

bool BridgedSubstitutionMap::isEmpty() const {
  return unbridged().empty();
}

bool BridgedSubstitutionMap::isEqualTo(BridgedSubstitutionMap rhs) const {
  return unbridged() == rhs.unbridged();
}

bool BridgedSubstitutionMap::hasAnySubstitutableParams() const {
  return unbridged().hasAnySubstitutableParams();
}

CodiraInt BridgedSubstitutionMap::getNumConformances() const {
  return (CodiraInt)unbridged().getConformances().size();
}

BridgedConformance BridgedSubstitutionMap::getConformance(CodiraInt index) const {
  return unbridged().getConformances()[index];
}

BridgedASTTypeArray BridgedSubstitutionMap::getReplacementTypes() const {
  return {unbridged().getReplacementTypes()};
}

//===----------------------------------------------------------------------===//
// MARK: BridgedGenericSignature
//===----------------------------------------------------------------------===//

language::GenericSignature BridgedGenericSignature::unbridged() const {
  return language::GenericSignature(impl);
}

BridgedASTTypeArray BridgedGenericSignature::getGenericParams() const {
  return {unbridged().getGenericParams()};
}

BridgedASTType BridgedGenericSignature::mapTypeIntoContext(BridgedASTType type) const {
  return {unbridged().getGenericEnvironment()->mapTypeIntoContext(type.unbridged()).getPointer()};
}

//===----------------------------------------------------------------------===//
// MARK: BridgedFingerprint
//===----------------------------------------------------------------------===//

language::Fingerprint BridgedFingerprint::unbridged() const {
  return language::Fingerprint(language::Fingerprint::Core{this->v1, this->v2});
}

//===----------------------------------------------------------------------===//
// MARK: BridgedIfConfigClauseRangeInfo
//===----------------------------------------------------------------------===//

language::IfConfigClauseRangeInfo BridgedIfConfigClauseRangeInfo::unbridged() const {
  language::IfConfigClauseRangeInfo::ClauseKind clauseKind;
  switch (kind) {
  case IfConfigActive:
    clauseKind = language::IfConfigClauseRangeInfo::ActiveClause;
    break;

  case IfConfigInactive:
    clauseKind = language::IfConfigClauseRangeInfo::InactiveClause;
    break;

  case IfConfigEnd:
    clauseKind = language::IfConfigClauseRangeInfo::EndDirective;
    break;
  }

  return language::IfConfigClauseRangeInfo(directiveLoc.unbridged(),
                                        bodyLoc.unbridged(),
                                        endLoc.unbridged(),
                                        clauseKind);
}

//===----------------------------------------------------------------------===//
// MARK: BridgedRegexLiteralPatternFeature
//===----------------------------------------------------------------------===//

BridgedRegexLiteralPatternFeatureKind::BridgedRegexLiteralPatternFeatureKind(
    CodiraInt rawValue)
    : RawValue(rawValue) {
  ASSERT(rawValue >= 0);
  ASSERT(rawValue == RawValue);
}

BridgedRegexLiteralPatternFeatureKind::BridgedRegexLiteralPatternFeatureKind(
    UnbridgedTy kind)
    : RawValue(kind.getRawValue()) {}

BridgedRegexLiteralPatternFeatureKind::UnbridgedTy
BridgedRegexLiteralPatternFeatureKind::unbridged() const {
  return UnbridgedTy(RawValue);
}

BridgedRegexLiteralPatternFeature::BridgedRegexLiteralPatternFeature(
    UnbridgedTy feature)
    : Range(feature.getRange()), Kind(feature.getKind()) {}

BridgedRegexLiteralPatternFeature::UnbridgedTy
BridgedRegexLiteralPatternFeature::unbridged() const {
  return UnbridgedTy(Kind.unbridged(), Range.unbridged());
}

BridgedRegexLiteralPatternFeatures::UnbridgedTy
BridgedRegexLiteralPatternFeatures::unbridged() const {
  return UnbridgedTy(Data, Count);
}

//===----------------------------------------------------------------------===//
// MARK: BridgedStmtConditionElement
//===----------------------------------------------------------------------===//

BridgedStmtConditionElement::BridgedStmtConditionElement(language::StmtConditionElement elem)
    : Raw(elem.getOpaqueValue()) {}

language::StmtConditionElement BridgedStmtConditionElement::unbridged() const {
  return language::StmtConditionElement::fromOpaqueValue(Raw);
}

//===----------------------------------------------------------------------===//
// MARK: BridgedCaptureListEntry
//===----------------------------------------------------------------------===//

BridgedCaptureListEntry::BridgedCaptureListEntry(language::CaptureListEntry CLE)
    : PBD(CLE.PBD) {}

language::CaptureListEntry BridgedCaptureListEntry::unbridged() const {
  return language::CaptureListEntry(PBD);
}

BridgedVarDecl BridgedCaptureListEntry::getVarDecl() const {
  return unbridged().getVar();
}

//===----------------------------------------------------------------------===//
// MARK: NumberLiteralExpr
//===----------------------------------------------------------------------===//

void BridgedFloatLiteralExpr_setNegative(BridgedFloatLiteralExpr cExpr,
                                         BridgedSourceLoc cLoc) {
  cExpr.unbridged()->setNegative(cLoc.unbridged());
}

void BridgedIntegerLiteralExpr_setNegative(BridgedIntegerLiteralExpr cExpr,
                                           BridgedSourceLoc cLoc) {
  cExpr.unbridged()->setNegative(cLoc.unbridged());
}

//===----------------------------------------------------------------------===//
// MARK: BridgedTypeOrCustomAttr
//===----------------------------------------------------------------------===//

BridgedTypeOrCustomAttr::BridgedTypeOrCustomAttr(void *_Nonnull pointer,
                                                 Kind kind)
    : opaque(intptr_t(pointer) | kind) {
  assert(getPointer() == pointer && getKind() == kind);
}

BridgedTypeAttribute BridgedTypeOrCustomAttr::castToTypeAttr() const {
  assert(getKind() == Kind::TypeAttr);
  return static_cast<language::TypeAttribute *>(getPointer());
}

BridgedCustomAttr BridgedTypeOrCustomAttr::castToCustomAttr() const {
  assert(getKind() == Kind::CustomAttr);
  return static_cast<language::CustomAttr *>(getPointer());
}

LANGUAGE_END_NULLABILITY_ANNOTATIONS

#endif // LANGUAGE_AST_ASTBRIDGINGIMPL_H
