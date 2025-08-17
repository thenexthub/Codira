//==- ObjCPropertyChecker.cpp - Check ObjC properties ------------*- C++ -*-==//
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
//  This checker finds issues with Objective-C properties.
//  Currently finds only one kind of issue:
//  - Find synthesized properties with copy attribute of mutable NS collection
//    types. Calling -copy on such collections produces an immutable copy,
//    which contradicts the type of the property.
//
//===----------------------------------------------------------------------===//

#include "language/Core/StaticAnalyzer/Checkers/BuiltinCheckerRegistration.h"
#include "language/Core/StaticAnalyzer/Core/BugReporter/BugReporter.h"
#include "language/Core/StaticAnalyzer/Core/Checker.h"

using namespace language::Core;
using namespace ento;

namespace {
class ObjCPropertyChecker
    : public Checker<check::ASTDecl<ObjCPropertyDecl>> {
  void checkCopyMutable(const ObjCPropertyDecl *D, BugReporter &BR) const;

public:
  void checkASTDecl(const ObjCPropertyDecl *D, AnalysisManager &Mgr,
                    BugReporter &BR) const;
};
} // end anonymous namespace.

void ObjCPropertyChecker::checkASTDecl(const ObjCPropertyDecl *D,
                                       AnalysisManager &Mgr,
                                       BugReporter &BR) const {
  checkCopyMutable(D, BR);
}

void ObjCPropertyChecker::checkCopyMutable(const ObjCPropertyDecl *D,
                                           BugReporter &BR) const {
  if (D->isReadOnly() || D->getSetterKind() != ObjCPropertyDecl::Copy)
    return;

  QualType T = D->getType();
  if (!T->isObjCObjectPointerType())
    return;

  const std::string &PropTypeName(T->getPointeeType().getCanonicalType()
                                                     .getUnqualifiedType()
                                                     .getAsString());
  if (!StringRef(PropTypeName).starts_with("NSMutable"))
    return;

  const ObjCImplDecl *ImplD = nullptr;
  if (const ObjCInterfaceDecl *IntD =
          dyn_cast<ObjCInterfaceDecl>(D->getDeclContext())) {
    ImplD = IntD->getImplementation();
  } else if (auto *CatD = dyn_cast<ObjCCategoryDecl>(D->getDeclContext())) {
    ImplD = CatD->getClassInterface()->getImplementation();
  }

  if (!ImplD || ImplD->HasUserDeclaredSetterMethod(D))
    return;

  SmallString<128> Str;
  toolchain::raw_svector_ostream OS(Str);
  OS << "Property of mutable type '" << PropTypeName
     << "' has 'copy' attribute; an immutable object will be stored instead";

  BR.EmitBasicReport(
      D, this, "Objective-C property misuse", "Logic error", OS.str(),
      PathDiagnosticLocation::createBegin(D, BR.getSourceManager()),
      D->getSourceRange());
}

void ento::registerObjCPropertyChecker(CheckerManager &Mgr) {
  Mgr.registerChecker<ObjCPropertyChecker>();
}

bool ento::shouldRegisterObjCPropertyChecker(const CheckerManager &mgr) {
  return true;
}
