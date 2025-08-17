//===-- CheckObjCInstMethSignature.cpp - Check ObjC method signatures -----===//
//
// Copyright (c) 2025, NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
// 
// Author: Tunjay Akbarli
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
// 
//     http://www.apache.org/licenses/LICENSE-2.0
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
//
//  This file defines a CheckObjCInstMethSignature, a flow-insensitive check
//  that determines if an Objective-C class interface incorrectly redefines
//  the method signature in a subclass.
//
//===----------------------------------------------------------------------===//

#include "language/Core/StaticAnalyzer/Checkers/BuiltinCheckerRegistration.h"
#include "language/Core/Analysis/PathDiagnostic.h"
#include "language/Core/AST/ASTContext.h"
#include "language/Core/AST/DeclObjC.h"
#include "language/Core/AST/Type.h"
#include "language/Core/StaticAnalyzer/Core/BugReporter/BugReporter.h"
#include "language/Core/StaticAnalyzer/Core/Checker.h"
#include "toolchain/ADT/DenseMap.h"
#include "toolchain/Support/raw_ostream.h"

using namespace language::Core;
using namespace ento;

static bool AreTypesCompatible(QualType Derived, QualType Ancestor,
                               ASTContext &C) {

  // Right now don't compare the compatibility of pointers.  That involves
  // looking at subtyping relationships.  FIXME: Future patch.
  if (Derived->isAnyPointerType() &&  Ancestor->isAnyPointerType())
    return true;

  return C.typesAreCompatible(Derived, Ancestor);
}

static void CompareReturnTypes(const ObjCMethodDecl *MethDerived,
                               const ObjCMethodDecl *MethAncestor,
                               BugReporter &BR, ASTContext &Ctx,
                               const ObjCImplementationDecl *ID,
                               const CheckerBase *Checker) {

  QualType ResDerived = MethDerived->getReturnType();
  QualType ResAncestor = MethAncestor->getReturnType();

  if (!AreTypesCompatible(ResDerived, ResAncestor, Ctx)) {
    std::string sbuf;
    toolchain::raw_string_ostream os(sbuf);

    os << "The Objective-C class '"
       << *MethDerived->getClassInterface()
       << "', which is derived from class '"
       << *MethAncestor->getClassInterface()
       << "', defines the instance method '";
    MethDerived->getSelector().print(os);
    os << "' whose return type is '" << ResDerived
       << "'.  A method with the same name (same selector) is also defined in "
          "class '"
       << *MethAncestor->getClassInterface() << "' and has a return type of '"
       << ResAncestor
       << "'.  These two types are incompatible, and may result in undefined "
          "behavior for clients of these classes.";

    PathDiagnosticLocation MethDLoc =
      PathDiagnosticLocation::createBegin(MethDerived,
                                          BR.getSourceManager());

    BR.EmitBasicReport(
        MethDerived, Checker, "Incompatible instance method return type",
        categories::CoreFoundationObjectiveC, os.str(), MethDLoc);
  }
}

static void CheckObjCInstMethSignature(const ObjCImplementationDecl *ID,
                                       BugReporter &BR,
                                       const CheckerBase *Checker) {

  const ObjCInterfaceDecl *D = ID->getClassInterface();
  const ObjCInterfaceDecl *C = D->getSuperClass();

  if (!C)
    return;

  ASTContext &Ctx = BR.getContext();

  // Build a DenseMap of the methods for quick querying.
  typedef toolchain::DenseMap<Selector,ObjCMethodDecl*> MapTy;
  MapTy IMeths;
  unsigned NumMethods = 0;

  for (auto *M : ID->instance_methods()) {
    IMeths[M->getSelector()] = M;
    ++NumMethods;
  }

  // Now recurse the class hierarchy chain looking for methods with the
  // same signatures.
  while (C && NumMethods) {
    for (const auto *M : C->instance_methods()) {
      Selector S = M->getSelector();

      MapTy::iterator MI = IMeths.find(S);

      if (MI == IMeths.end() || MI->second == nullptr)
        continue;

      --NumMethods;
      ObjCMethodDecl *MethDerived = MI->second;
      MI->second = nullptr;

      CompareReturnTypes(MethDerived, M, BR, Ctx, ID, Checker);
    }

    C = C->getSuperClass();
  }
}

//===----------------------------------------------------------------------===//
// ObjCMethSigsChecker
//===----------------------------------------------------------------------===//

namespace {
class ObjCMethSigsChecker : public Checker<
                                      check::ASTDecl<ObjCImplementationDecl> > {
public:
  void checkASTDecl(const ObjCImplementationDecl *D, AnalysisManager& mgr,
                    BugReporter &BR) const {
    CheckObjCInstMethSignature(D, BR, this);
  }
};
}

void ento::registerObjCMethSigsChecker(CheckerManager &mgr) {
  mgr.registerChecker<ObjCMethSigsChecker>();
}

bool ento::shouldRegisterObjCMethSigsChecker(const CheckerManager &mgr) {
  return true;
}
