//===--- UndefinedAssignmentChecker.h ---------------------------*- C++ -*--==//
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
// This defines UndefinedAssignmentChecker, a builtin check in ExprEngine that
// checks for assigning undefined values.
//
//===----------------------------------------------------------------------===//

#include "language/Core/StaticAnalyzer/Checkers/BuiltinCheckerRegistration.h"
#include "language/Core/StaticAnalyzer/Core/BugReporter/BugType.h"
#include "language/Core/StaticAnalyzer/Core/Checker.h"
#include "language/Core/StaticAnalyzer/Core/CheckerManager.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/CheckerContext.h"

using namespace language::Core;
using namespace ento;

namespace {
class UndefinedAssignmentChecker
  : public Checker<check::Bind> {
  const BugType BT{this, "Assigned value is uninitialized"};

public:
  void checkBind(SVal location, SVal val, const Stmt *S, bool AtDeclInit,
                 CheckerContext &C) const;
};
}

void UndefinedAssignmentChecker::checkBind(SVal location, SVal val,
                                           const Stmt *StoreE, bool AtDeclInit,
                                           CheckerContext &C) const {
  if (!val.isUndef())
    return;

  // Do not report assignments of uninitialized values inside swap functions.
  // This should allow to swap partially uninitialized structs
  if (const FunctionDecl *EnclosingFunctionDecl =
      dyn_cast<FunctionDecl>(C.getStackFrame()->getDecl()))
    if (C.getCalleeName(EnclosingFunctionDecl) == "swap")
      return;

  ExplodedNode *N = C.generateErrorNode();

  if (!N)
    return;

  // Generate a report for this bug.
  toolchain::SmallString<128> Str;
  toolchain::raw_svector_ostream OS(Str);

  const Expr *ex = nullptr;

  while (StoreE) {
    if (const UnaryOperator *U = dyn_cast<UnaryOperator>(StoreE)) {
      OS << "The expression uses uninitialized memory";

      ex = U->getSubExpr();
      break;
    }

    if (const BinaryOperator *B = dyn_cast<BinaryOperator>(StoreE)) {
      if (B->isCompoundAssignmentOp()) {
        if (C.getSVal(B->getLHS()).isUndef()) {
          OS << "The left expression of the compound assignment uses "
             << "uninitialized memory";
          ex = B->getLHS();
          break;
        }
      }

      ex = B->getRHS();
      break;
    }

    if (const DeclStmt *DS = dyn_cast<DeclStmt>(StoreE)) {
      const VarDecl *VD = cast<VarDecl>(DS->getSingleDecl());
      ex = VD->getInit();
    }

    if (const auto *CD =
            dyn_cast<CXXConstructorDecl>(C.getStackFrame()->getDecl())) {
      if (CD->isImplicit()) {
        for (auto *I : CD->inits()) {
          if (I->getInit()->IgnoreImpCasts() == StoreE) {
            OS << "Value assigned to field '" << I->getMember()->getName()
               << "' in implicit constructor is uninitialized";
            break;
          }
        }
      }
    }

    break;
  }

  if (OS.str().empty())
    OS << BT.getDescription();

  auto R = std::make_unique<PathSensitiveBugReport>(BT, OS.str(), N);
  if (ex) {
    R->addRange(ex->getSourceRange());
    bugreporter::trackExpressionValue(N, ex, *R);
  }
  C.emitReport(std::move(R));
}

void ento::registerUndefinedAssignmentChecker(CheckerManager &mgr) {
  mgr.registerChecker<UndefinedAssignmentChecker>();
}

bool ento::shouldRegisterUndefinedAssignmentChecker(const CheckerManager &mgr) {
  return true;
}
