// UndefCapturedBlockVarChecker.cpp - Uninitialized captured vars -*- C++ -*-=//
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
// This checker detects blocks that capture uninitialized values.
//
//===----------------------------------------------------------------------===//

#include "language/Core/StaticAnalyzer/Checkers/BuiltinCheckerRegistration.h"
#include "language/Core/AST/Attr.h"
#include "language/Core/StaticAnalyzer/Core/BugReporter/BugType.h"
#include "language/Core/StaticAnalyzer/Core/Checker.h"
#include "language/Core/StaticAnalyzer/Core/CheckerManager.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/CheckerContext.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/ExprEngine.h"
#include "toolchain/ADT/SmallString.h"
#include "toolchain/Support/raw_ostream.h"
#include <optional>

using namespace language::Core;
using namespace ento;

namespace {
class UndefCapturedBlockVarChecker
  : public Checker< check::PostStmt<BlockExpr> > {
  const BugType BT{this, "uninitialized variable captured by block"};

public:
  void checkPostStmt(const BlockExpr *BE, CheckerContext &C) const;
};
} // end anonymous namespace

static const DeclRefExpr *FindBlockDeclRefExpr(const Stmt *S,
                                               const VarDecl *VD) {
  if (const DeclRefExpr *BR = dyn_cast<DeclRefExpr>(S))
    if (BR->getDecl() == VD)
      return BR;

  for (const Stmt *Child : S->children())
    if (Child)
      if (const DeclRefExpr *BR = FindBlockDeclRefExpr(Child, VD))
        return BR;

  return nullptr;
}

void
UndefCapturedBlockVarChecker::checkPostStmt(const BlockExpr *BE,
                                            CheckerContext &C) const {
  if (!BE->getBlockDecl()->hasCaptures())
    return;

  ProgramStateRef state = C.getState();
  auto *R = cast<BlockDataRegion>(C.getSVal(BE).getAsRegion());

  for (auto Var : R->referenced_vars()) {
    // This VarRegion is the region associated with the block; we need
    // the one associated with the encompassing context.
    const VarRegion *VR = Var.getCapturedRegion();
    const VarDecl *VD = VR->getDecl();

    if (VD->hasAttr<BlocksAttr>() || !VD->hasLocalStorage())
      continue;

    // Get the VarRegion associated with VD in the local stack frame.
    if (std::optional<UndefinedVal> V =
            state->getSVal(Var.getOriginalRegion()).getAs<UndefinedVal>()) {
      if (ExplodedNode *N = C.generateErrorNode()) {
        // Generate a bug report.
        SmallString<128> buf;
        toolchain::raw_svector_ostream os(buf);

        os << "Variable '" << VD->getName()
           << "' is uninitialized when captured by block";

        auto R = std::make_unique<PathSensitiveBugReport>(BT, os.str(), N);
        if (const Expr *Ex = FindBlockDeclRefExpr(BE->getBody(), VD))
          R->addRange(Ex->getSourceRange());
        bugreporter::trackStoredValue(*V, VR, *R,
                                      {bugreporter::TrackingKind::Thorough,
                                       /*EnableNullFPSuppression*/ false});
        R->disablePathPruning();
        // need location of block
        C.emitReport(std::move(R));
      }
    }
  }
}

void ento::registerUndefCapturedBlockVarChecker(CheckerManager &mgr) {
  mgr.registerChecker<UndefCapturedBlockVarChecker>();
}

bool ento::shouldRegisterUndefCapturedBlockVarChecker(const CheckerManager &mgr) {
  return true;
}
