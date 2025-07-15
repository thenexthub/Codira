//===--- DerivedConformanceError.cpp ----------------------------*- C++ -*-===//
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
//  This file implements implicit derivation of the Error
//  protocol.
//
//===----------------------------------------------------------------------===//

#include "CodeSynthesis.h"
#include "DerivedConformance.h"
#include "TypeChecker.h"
#include "language/AST/Decl.h"
#include "language/AST/Expr.h"
#include "language/AST/Module.h"
#include "language/AST/Stmt.h"
#include "language/AST/CodiraNameTranslation.h"
#include "language/AST/Types.h"

using namespace language;
using namespace language::objc_translation;

static std::pair<BraceStmt *, bool>
deriveBodyBridgedNSError_enum_nsErrorDomain(AbstractFunctionDecl *domainDecl,
                                            void *) {
  // enum SomeEnum {
  //   @derived
  //   static var _nsErrorDomain: String {
  //     return String(reflecting: self)
  //   }
  // }

  auto M = domainDecl->getParentModule();
  auto &C = M->getASTContext();
  auto self = domainDecl->getImplicitSelfDecl();

  auto selfRef = new (C) DeclRefExpr(self, DeclNameLoc(), /*implicit*/ true);
  auto stringType = TypeExpr::createImplicitForDecl(
      DeclNameLoc(), C.getStringDecl(), domainDecl,
      C.getStringDecl()->getInterfaceType());
  auto *argList = ArgumentList::forImplicitSingle(
      C, C.getIdentifier("reflecting"), selfRef);
  auto *initReflectingCall = CallExpr::createImplicit(C, stringType, argList);
  auto *ret = ReturnStmt::createImplicit(C, initReflectingCall);

  auto body = BraceStmt::create(C, SourceLoc(), ASTNode(ret), SourceLoc());
  return { body, /*isTypeChecked=*/false };
}

static std::pair<BraceStmt *, bool>
deriveBodyBridgedNSError_printAsObjCEnum_nsErrorDomain(
                    AbstractFunctionDecl *domainDecl, void *) {
  // enum SomeEnum {
  //   @derived
  //   static var _nsErrorDomain: String {
  //     return "ModuleName.SomeEnum"
  //   }
  // }

  auto M = domainDecl->getParentModule();
  auto &C = M->getASTContext();
  auto TC = domainDecl->getInnermostTypeContext();
  auto ED = TC->getSelfEnumDecl();

  StringRef value(C.AllocateCopy(getErrorDomainStringForObjC(ED)));

  auto string = new (C) StringLiteralExpr(value, SourceRange(), /*implicit*/ true);
  auto *ret = ReturnStmt::createImplicit(C, SourceLoc(), string);
  auto body = BraceStmt::create(C, SourceLoc(),
                                ASTNode(ret),
                                SourceLoc());
  return { body, /*isTypeChecked=*/false };
}

static ValueDecl *
deriveBridgedNSError_enum_nsErrorDomain(
    DerivedConformance &derived,
    std::pair<BraceStmt *, bool> (*synthesizer)(AbstractFunctionDecl *, void*)) {
  // enum SomeEnum {
  //   @derived
  //   static var _nsErrorDomain: String {
  //     ...
  //   }
  // }

  auto stringTy = derived.Context.getStringType();

  // Define the property.
  VarDecl *propDecl;
  PatternBindingDecl *pbDecl;
  std::tie(propDecl, pbDecl) = derived.declareDerivedProperty(
      DerivedConformance::SynthesizedIntroducer::Var,
      derived.Context.Id_nsErrorDomain, stringTy, /*isStatic=*/true,
      /*isFinal=*/true);
  addNonIsolatedToSynthesized(derived.Nominal, propDecl);

  // Define the getter.
  auto getterDecl = derived.addGetterToReadOnlyDerivedProperty(propDecl);
  getterDecl->setBodySynthesizer(synthesizer);

  derived.addMembersToConformanceContext({propDecl, pbDecl});

  return propDecl;
}

ValueDecl *DerivedConformance::deriveBridgedNSError(ValueDecl *requirement) {
  if (!isa<EnumDecl>(Nominal))
    return nullptr;

  if (requirement->getBaseName() == Context.Id_nsErrorDomain) {
    auto synthesizer = deriveBodyBridgedNSError_enum_nsErrorDomain;

    auto scope = Nominal->getFormalAccessScope(Nominal->getModuleScopeContext());
    if (scope.isPublic() || scope.isInternal())
      // PrintAsClang may print this domain, so we should make sure we use the
      // same string it will.
      synthesizer = deriveBodyBridgedNSError_printAsObjCEnum_nsErrorDomain;

    return deriveBridgedNSError_enum_nsErrorDomain(*this, synthesizer);
  }

  Context.Diags.diagnose(requirement->getLoc(),
                         diag::broken_errortype_requirement);
  return nullptr;
}
