//===--- Bridging/MiscBridging.cpp ----------------------------------------===//
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

#include "language/AST/ASTBridging.h"

#include "language/AST/ASTContext.h"
#include "language/AST/Decl.h"
#include "language/AST/Expr.h"
#include "language/AST/ProtocolConformanceRef.h"
#include "language/AST/Stmt.h"
#include "language/AST/TypeRepr.h"
#include "language/Basic/Assertions.h"

#ifdef PURE_BRIDGING_MODE
// In PURE_BRIDGING_MODE, bridging functions are not inlined and therefore
// included in the cpp file.
#include "language/AST/ASTBridgingImpl.h"
#endif

using namespace language;

//===----------------------------------------------------------------------===//
// MARK: Misc
//===----------------------------------------------------------------------===//

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"

void BridgedTopLevelCodeDecl_dump(void *decl) {
  static_cast<TopLevelCodeDecl *>(decl)->dump(toolchain::errs());
}
void BridgedExpr_dump(void *expr) {
  static_cast<Expr *>(expr)->dump(toolchain::errs());
}
void BridgedDecl_dump(void *decl) {
  static_cast<Decl *>(decl)->dump(toolchain::errs());
}
void BridgedStmt_dump(void *statement) {
  static_cast<Stmt *>(statement)->dump(toolchain::errs());
}
void BridgedTypeRepr_dump(void *type) { static_cast<TypeRepr *>(type)->dump(); }

#pragma clang diagnostic pop

//===----------------------------------------------------------------------===//
// MARK: Metatype registration
//===----------------------------------------------------------------------===//

static bool declMetatypesInitialized = false;

// Filled in by class registration in initializeCodiraModules().
static CodiraMetatype declMetatypes[(unsigned)DeclKind::Last_Decl + 1];

// Does return null if initializeCodiraModules() is never called.
CodiraMetatype Decl::getDeclMetatype(DeclKind kind) {
  CodiraMetatype metatype = declMetatypes[(unsigned)kind];
  if (declMetatypesInitialized && !metatype) {
    ABORT([&](auto &out) {
      out << "Decl " << getKindName(kind) << " not registered";
    });
  }
  return metatype;
}

/// Registers the metatype of a Decl class.
/// Called by initializeCodiraModules().
void registerBridgedDecl(BridgedStringRef bridgedClassName,
                         CodiraMetatype metatype) {
  declMetatypesInitialized = true;

  auto declKind = toolchain::StringSwitch<std::optional<language::DeclKind>>(
                      bridgedClassName.unbridged())
#define DECL(Id, Parent) .Case(#Id "Decl", language::DeclKind::Id)
#include "language/AST/DeclNodes.def"
                      .Default(std::nullopt);

  if (!declKind) {
    ABORT([&](auto &out) {
      out << "Unknown Decl class " << bridgedClassName.unbridged();
    });
  }
  declMetatypes[(unsigned)declKind.value()] = metatype;
}

BridgedOwnedString BridgedDeclObj::getDebugDescription() const {
  std::string str;
  toolchain::raw_string_ostream os(str);
  unbridged()->print(os);
  return BridgedOwnedString(str);
}

//===----------------------------------------------------------------------===//
// MARK: BridgedASTType
//===----------------------------------------------------------------------===//

BridgedOwnedString BridgedASTType::getDebugDescription() const {
  std::string str;
  toolchain::raw_string_ostream os(str);
  unbridged().dump(os);
  return BridgedOwnedString(str);
}

//===----------------------------------------------------------------------===//
// MARK: Conformance
//===----------------------------------------------------------------------===//

BridgedOwnedString BridgedConformance::getDebugDescription() const {
  std::string str;
  toolchain::raw_string_ostream os(str);
  unbridged().print(os);
  return BridgedOwnedString(str);
}

//===----------------------------------------------------------------------===//
// MARK: SubstitutionMap
//===----------------------------------------------------------------------===//

static_assert(sizeof(BridgedSubstitutionMap) >= sizeof(language::SubstitutionMap),
              "BridgedSubstitutionMap has wrong size");

BridgedSubstitutionMap BridgedSubstitutionMap::get(BridgedGenericSignature genSig, BridgedArrayRef replacementTypes) {
  return SubstitutionMap::get(genSig.unbridged(),
                              replacementTypes.unbridged<Type>(),
                              language::LookUpConformanceInModule());
}

BridgedOwnedString BridgedSubstitutionMap::getDebugDescription() const {
  std::string str;
  toolchain::raw_string_ostream os(str);
  unbridged().dump(os);
  return BridgedOwnedString(str);
}

//===----------------------------------------------------------------------===//
// MARK: GenericSignature
//===----------------------------------------------------------------------===//

BridgedOwnedString BridgedGenericSignature::getDebugDescription() const {
  std::string str;
  toolchain::raw_string_ostream os(str);
  unbridged().print(os);
  return BridgedOwnedString(str);
}

//===----------------------------------------------------------------------===//
// MARK: BridgedPoundKeyword
//===----------------------------------------------------------------------===//

BridgedPoundKeyword BridgedPoundKeyword_fromString(BridgedStringRef cStr) {
  return toolchain::StringSwitch<BridgedPoundKeyword>(cStr.unbridged())
#define POUND_KEYWORD(NAME) .Case(#NAME, BridgedPoundKeyword_##NAME)
#include "language/AST/TokenKinds.def"
      .Default(BridgedPoundKeyword_None);
}
