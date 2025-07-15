//===--- Bridging/TypeAttributeBridging.cpp -------------------------------===//
//
// This source file is part of the Codira.org open source project
//
// Copyright (c) 2022-2025 Apple Inc. and the Codira project authors
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See https://language.org/LICENSE.txt for license information
// See https://language.org/CONTRIBUTORS.txt for the list of Codira project authors
//
//===----------------------------------------------------------------------===//

#include "language/AST/ASTBridging.h"

#include "language/AST/ASTContext.h"
#include "language/AST/Attr.h"
#include "language/AST/Identifier.h"
#include "language/Basic/Assertions.h"

using namespace language;

//===----------------------------------------------------------------------===//
// MARK: TypeAttributes
//===----------------------------------------------------------------------===//

// Define `.asTypeAttr` on each BridgedXXXTypeAttr type.
#define SIMPLE_TYPE_ATTR(...)
#define TYPE_ATTR(SPELLING, CLASS)                                             \
  LANGUAGE_NAME("getter:Bridged" #CLASS "TypeAttr.asTypeAttribute(self:)")        \
  BridgedTypeAttribute Bridged##CLASS##TypeAttr_asTypeAttribute(               \
      Bridged##CLASS##TypeAttr attr) {                                         \
    return attr.unbridged();                                                   \
  }
#include "language/AST/TypeAttr.def"

BridgedOptionalTypeAttrKind
BridgedOptionalTypeAttrKind_fromString(BridgedStringRef cStr) {
  auto optKind = TypeAttribute::getAttrKindFromString(cStr.unbridged());
  if (!optKind) {
    return BridgedOptionalTypeAttrKind();
  }
  return *optKind;
}

BridgedTypeAttribute BridgedTypeAttribute_createSimple(
    BridgedASTContext cContext, language::TypeAttrKind kind,
    BridgedSourceLoc cAtLoc, BridgedSourceLoc cNameLoc) {
  return TypeAttribute::createSimple(cContext.unbridged(), kind,
                                     cAtLoc.unbridged(), cNameLoc.unbridged());
}

BridgedConventionTypeAttr BridgedConventionTypeAttr_createParsed(
    BridgedASTContext cContext, BridgedSourceLoc cAtLoc,
    BridgedSourceLoc cKwLoc, BridgedSourceRange cParens, BridgedStringRef cName,
    BridgedSourceLoc cNameLoc, BridgedDeclNameRef cWitnessMethodProtocol,
    BridgedStringRef cClangType, BridgedSourceLoc cClangTypeLoc) {
  return new (cContext.unbridged()) ConventionTypeAttr(
      cAtLoc.unbridged(), cKwLoc.unbridged(), cParens.unbridged(),
      {cName.unbridged(), cNameLoc.unbridged()},
      cWitnessMethodProtocol.unbridged(),
      {cClangType.unbridged(), cClangTypeLoc.unbridged()});
}

BridgedDifferentiableTypeAttr BridgedDifferentiableTypeAttr_createParsed(
    BridgedASTContext cContext, BridgedSourceLoc cAtLoc,
    BridgedSourceLoc cNameLoc, BridgedSourceRange cParensRange,
    BridgedDifferentiabilityKind cKind, BridgedSourceLoc cKindLoc) {
  return new (cContext.unbridged()) DifferentiableTypeAttr(
      cAtLoc.unbridged(), cNameLoc.unbridged(), cParensRange.unbridged(),
      {unbridged(cKind), cKindLoc.unbridged()});
}

BridgedIsolatedTypeAttr BridgedIsolatedTypeAttr_createParsed(
    BridgedASTContext cContext, BridgedSourceLoc cAtLoc,
    BridgedSourceLoc cNameLoc, BridgedSourceRange cParensRange,

    BridgedIsolatedTypeAttrIsolationKind cIsolation,
    BridgedSourceLoc cIsolationLoc) {
  auto isolationKind = [=] {
    switch (cIsolation) {
    case BridgedIsolatedTypeAttrIsolationKind_DynamicIsolation:
      return IsolatedTypeAttr::IsolationKind::Dynamic;
    }
    toolchain_unreachable("bad kind");
  }();
  return new (cContext.unbridged()) IsolatedTypeAttr(
      cAtLoc.unbridged(), cNameLoc.unbridged(), cParensRange.unbridged(),
      {isolationKind, cIsolationLoc.unbridged()});
}

BridgedOpaqueReturnTypeOfTypeAttr
BridgedOpaqueReturnTypeOfTypeAttr_createParsed(
    BridgedASTContext cContext, BridgedSourceLoc cAtLoc,
    BridgedSourceLoc cKwLoc, BridgedSourceRange cParens,
    BridgedStringRef cMangled, BridgedSourceLoc cMangledLoc, size_t index,
    BridgedSourceLoc cIndexLoc) {
  return new (cContext.unbridged()) OpaqueReturnTypeOfTypeAttr(
      cAtLoc.unbridged(), cKwLoc.unbridged(), cParens.unbridged(),
      {cMangled.unbridged(), cMangledLoc.unbridged()},
      {static_cast<unsigned int>(index), cIndexLoc.unbridged()});
}
