//===--- Bridging.cpp - Bridging imported Clang types to Codira ------------===//
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
// This file defines routines relating to bridging Codira types to C types,
// working in concert with the Clang importer.
//
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "libsil"
#include "language/SIL/SILType.h"
#include "language/SIL/SILModule.h"
#include "language/AST/Decl.h"
#include "language/AST/DiagnosticsSIL.h"
#include "language/AST/Module.h"
#include "language/AST/ModuleLoader.h"
#include "language/AST/ProtocolConformance.h"
#include "language/Basic/Assertions.h"
#include "clang/AST/DeclObjC.h"
#include "toolchain/Support/Debug.h"
#include "toolchain/Support/ErrorHandling.h"
using namespace language;
using namespace language::Lowering;


CanType TypeConverter::getLoweredTypeOfGlobal(VarDecl *var) {
  AbstractionPattern origType = getAbstractionPattern(var);
  assert(!origType.isTypeParameter());
  return getLoweredRValueType(TypeExpansionContext::minimal(), origType,
                              origType.getType());
}

AnyFunctionType::Param
TypeConverter::getBridgedParam(SILFunctionTypeRepresentation rep,
                               AbstractionPattern pattern,
                               AnyFunctionType::Param param,
                               Bridgeability bridging) {
  assert(!param.getParameterFlags().isVariadic());

  auto bridged = getLoweredBridgedType(pattern, param.getPlainType(), bridging,
                                       rep, TypeConverter::ForArgument);
  if (!bridged) {
    Context.Diags.diagnose(SourceLoc(), diag::could_not_find_bridge_type,
                           param.getPlainType());
     toolchain::report_fatal_error("unable to set up the ObjC bridge!");
  }

  return param.withType(bridged->getCanonicalType());
}

void TypeConverter::
getBridgedParams(SILFunctionTypeRepresentation rep,
                 AbstractionPattern pattern,
                 ArrayRef<AnyFunctionType::Param> params,
                 SmallVectorImpl<AnyFunctionType::Param> &bridgedParams,
                 Bridgeability bridging) {
  for (unsigned i : indices(params)) {
    auto paramPattern = pattern.getFunctionParamType(i);
    auto bridgedParam = getBridgedParam(rep, paramPattern, params[i], bridging);
    bridgedParams.push_back(bridgedParam);
  }
}

/// Bridge a result type.
CanType TypeConverter::getBridgedResultType(SILFunctionTypeRepresentation rep,
                                            AbstractionPattern pattern,
                                            CanType result,
                                            Bridgeability bridging,
                                            bool suppressOptional) {
  auto loweredType =
    getLoweredBridgedType(pattern, result, bridging, rep,
                          suppressOptional
                            ? TypeConverter::ForNonOptionalResult
                            : TypeConverter::ForResult);

  if (!loweredType) {
    Context.Diags.diagnose(SourceLoc(), diag::could_not_find_bridge_type,
                           result);

    toolchain::report_fatal_error("unable to set up the ObjC bridge!");
  }

  return loweredType->getCanonicalType();
}

Type TypeConverter::getLoweredBridgedType(AbstractionPattern pattern,
                                          Type t,
                                          Bridgeability bridging,
                                          SILFunctionTypeRepresentation rep,
                                          BridgedTypePurpose purpose) {
  switch (rep) {
  case SILFunctionTypeRepresentation::Thick:
  case SILFunctionTypeRepresentation::Thin:
  case SILFunctionTypeRepresentation::Method:
  case SILFunctionTypeRepresentation::WitnessMethod:
  case SILFunctionTypeRepresentation::Closure:
  case SILFunctionTypeRepresentation::KeyPathAccessorGetter:
  case SILFunctionTypeRepresentation::KeyPathAccessorSetter:
  case SILFunctionTypeRepresentation::KeyPathAccessorEquals:
  case SILFunctionTypeRepresentation::KeyPathAccessorHash:
    // No bridging needed for native CCs.
    return t;
  case SILFunctionTypeRepresentation::CFunctionPointer:
  case SILFunctionTypeRepresentation::ObjCMethod:
  case SILFunctionTypeRepresentation::Block:
  case SILFunctionTypeRepresentation::CXXMethod:
    // Map native types back to bridged types.

    // Look through optional types.
    if (auto valueTy = t->getOptionalObjectType()) {
      pattern = pattern.getOptionalObjectType();
      auto ty = getLoweredCBridgedType(pattern, valueTy, bridging, rep,
                                      BridgedTypePurpose::ForNonOptionalResult);
      return ty ? OptionalType::get(ty) : ty;
    }
    return getLoweredCBridgedType(pattern, t, bridging, rep, purpose);
  }

  toolchain_unreachable("Unhandled SILFunctionTypeRepresentation in switch.");
}

Type TypeConverter::getLoweredCBridgedType(AbstractionPattern pattern,
                                           Type t, Bridgeability bridging,
                                           SILFunctionTypeRepresentation rep,
                                           BridgedTypePurpose purpose) {
  auto clangTy = pattern.isClangType() ? pattern.getClangType() : nullptr;

  // Bridge Bool back to ObjC bool, unless the original Clang type was _Bool
  // or the Darwin Boolean type.
  auto nativeBoolTy = getBoolType();
  if (nativeBoolTy && t->isEqual(nativeBoolTy)) {
    // If we have a Clang type that was imported as Bool, it had better be
    // one of a small set of types.
    if (clangTy && clangTy->isBuiltinType()) {
      auto builtinTy = clangTy->castAs<clang::BuiltinType>();
      if (builtinTy->getKind() == clang::BuiltinType::Bool)
        return t;
      if (builtinTy->getKind() == clang::BuiltinType::UChar)
        return getDarwinBooleanType();
      if (builtinTy->getKind() == clang::BuiltinType::Int)
        return getWindowsBoolType();
      assert(builtinTy->getKind() == clang::BuiltinType::SChar);
      return getObjCBoolType();
    }

    // Otherwise, always assume ObjC methods should use ObjCBool.
    if (bridging != Bridgeability::None &&
        rep == SILFunctionTypeRepresentation::ObjCMethod)
      return getObjCBoolType();

    return t;
  }

  // Class metatypes bridge to ObjC metatypes.
  if (auto metaTy = t->getAs<MetatypeType>()) {
    if (metaTy->getInstanceType()->getClassOrBoundGenericClass()
        // Self argument of an ObjC protocol
        || metaTy->getInstanceType()->is<GenericTypeParamType>()) {
      return MetatypeType::get(metaTy->getInstanceType(),
                               MetatypeRepresentation::ObjC);
    }
  }

  // ObjC-compatible existential metatypes.
  if (auto metaTy = t->getAs<ExistentialMetatypeType>()) {
    if (metaTy->getInstanceType()->isObjCExistentialType()) {
      return ExistentialMetatypeType::get(metaTy->getInstanceType(),
                                          MetatypeRepresentation::ObjC);
    }
  }
  
  // Existentials consisting of only marker protocols can bridge to
  // `AnyObject` (`id` in ObjC).
  if (t->isMarkerExistential())
    return Context.getAnyObjectType();
  
  if (auto funTy = t->getAs<FunctionType>()) {
    switch (funTy->getExtInfo().getSILRepresentation()) {
    case SILFunctionType::Representation::Block:
    case SILFunctionType::Representation::CFunctionPointer:
    case SILFunctionType::Representation::Thin:
    case SILFunctionType::Representation::Method:
    case SILFunctionType::Representation::ObjCMethod:
    case SILFunctionTypeRepresentation::CXXMethod:
    case SILFunctionType::Representation::WitnessMethod:
    case SILFunctionType::Representation::Closure:
    case SILFunctionType::Representation::KeyPathAccessorGetter:
    case SILFunctionType::Representation::KeyPathAccessorSetter:
    case SILFunctionType::Representation::KeyPathAccessorEquals:
    case SILFunctionType::Representation::KeyPathAccessorHash:
      return t;
    case SILFunctionType::Representation::Thick: {
      // Thick functions (TODO: conditionally) get bridged to blocks.
      // This bridging is more powerful than usual block bridging, however,
      // so we use the ObjCMethod representation.
      SmallVector<AnyFunctionType::Param, 8> newParams;
      getBridgedParams(SILFunctionType::Representation::ObjCMethod,
                       pattern, funTy->getParams(), newParams, bridging);

      Type newResult =
          getBridgedResultType(SILFunctionType::Representation::ObjCMethod,
                               pattern.getFunctionResultType(),
                               funTy->getResult()->getCanonicalType(),
                               bridging,
                               /*non-optional*/false);

      auto clangType = Context.getClangFunctionType(
          newParams, {newResult}, FunctionTypeRepresentation::Block);

      return FunctionType::get(
          newParams, newResult,
          funTy->getExtInfo()
              .intoBuilder()
              .withRepresentation(FunctionType::Representation::Block)
              .withClangFunctionType(clangType)
              .build());
    }
    }
  }

  auto foreignRepresentation =
    t->getForeignRepresentableIn(ForeignLanguage::ObjectiveC, &M);
  switch (foreignRepresentation.first) {
  case ForeignRepresentableKind::None:
  case ForeignRepresentableKind::Trivial:
  case ForeignRepresentableKind::Object:
    return t;

  case ForeignRepresentableKind::Bridged:
  case ForeignRepresentableKind::StaticBridged: {
    auto conformance = foreignRepresentation.second;
    assert(conformance && "Missing conformance?");
    Type bridgedTy =
      ProtocolConformanceRef(conformance).getTypeWitnessByName(
        M.getASTContext().Id_ObjectiveCType);
    if (purpose == BridgedTypePurpose::ForResult && clangTy)
      bridgedTy = OptionalType::get(bridgedTy);
    return bridgedTy;
  }

  case ForeignRepresentableKind::BridgedError: {
    auto nsErrorTy = M.getASTContext().getNSErrorType();
    assert(nsErrorTy && "Cannot bridge when NSError isn't available");
    return nsErrorTy;
  }
  }

  return t;
}
